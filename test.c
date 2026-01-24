#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "socket.h"
#include "lps25h.h"
#include "hts221.h"
#include "i2c.h"

// --- Buffer Partagé (Structure R) ---
typedef struct {
    float temp_amb;
    float pressure;
    float humidity;
    float temp_consigne; // T° désirée
    float puissance;     // Calculée en T2
    pthread_mutex_t lock;
} system_data_t;

system_data_t sys = {
    .temp_amb = 20.0,
    .pressure = 1013.25,
    .humidity = 40.0,
    .temp_consigne = 15.0, // On commence bas
    .puissance = 0.0,
    .lock = PTHREAD_MUTEX_INITIALIZER
};

// --- Tâche 1 : Simulateur de Consigne (Période : 5s) ---
// Remplace le joystick : augmente la T° de 2°C à chaque cycle
void* task_setpoint_logic(void* arg) {
    printf("[T1] Simulateur de consigne actif (+2°C toutes les 5s)\n");
    while (1) {
        sleep(5); 
        pthread_mutex_lock(&sys.lock);
        sys.temp_consigne += 2.0;
        
        // Sécurité aérospatiale : on cap à 40°C pour éviter de griller le labo !
        if (sys.temp_consigne > 40.0) sys.temp_consigne = 15.0; 
        
        printf("[T1] Nouvelle consigne : %.2f°C\n", sys.temp_consigne);
        pthread_mutex_unlock(&sys.lock);
    }
    return NULL;
}

// --- Tâche 3 & 5 : Acquisition Capteurs (Période : 2s) ---
void* task_environment(void* arg) {
    int file_lps = i2c_init_lps25h();
    int file_hts = i2c_init_hts221();
    
    printf("[T3/T5] Threads capteurs initialisés\n");
    while (1) {
        float t = lps25h_read_temperature(file_lps);
        float p = lps25h_read_pressure(file_lps);
        float h = hts221_read_humidity(file_hts);

        pthread_mutex_lock(&sys.lock);
        sys.temp_amb = t;
        sys.pressure = p;
        sys.humidity = h;
        pthread_mutex_unlock(&sys.lock);

        sleep(2); 
    }
    return NULL;
}

// --- Tâche 2 : Contrôleur de Puissance (Période : 1s) ---
void* task_power(void* arg) {
    while (1) {
        pthread_mutex_lock(&sys.lock);
        // Loi de commande proportionnelle
        // P = K * (T_consigne - T_amb)
        float erreur = sys.temp_consigne - sys.temp_amb;
        sys.puissance = erreur * 15.0; // Gain K=15
        
        if (sys.puissance > 100) sys.puissance = 100;
        if (sys.puissance < 0) sys.puissance = 0;
        pthread_mutex_unlock(&sys.lock);

        sleep(1);
    }
    return NULL;
}

// --- Tâche 4 : Transmission Socket (Période : 1s) ---
void* task_display(void* arg) {
    Socket* s = (Socket*)arg;
    char buffer[256];
    while (1) {
        pthread_mutex_lock(&sys.lock);
        // On envoie tout au Java : Temp, Pression, Humidité, Consigne, Puissance
        sprintf(buffer, "TP%.2f\nPR%.2f\nHU%.2f\nST%.2f\nPW%.2f\n\n", 
                sys.temp_amb, sys.pressure, sys.humidity, sys.temp_consigne, sys.puissance);
        pthread_mutex_unlock(&sys.lock);

        if (socket_send_str(s, buffer) < 0) {
            printf("[T4] Erreur Socket\n");
        }
        sleep(1);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t th_logic, th_power, th_env, th_disp;
    Socket sock;
    const char *ip = (argc > 1) ? argv[1] : "127.0.0.1";

    // 1. Connexion au Java
    socket_init(&sock, ip, 8888);
    if (socket_connect(&sock) < 0) {
        fprintf(stderr, "Impossible de se connecter à l'afficheur sur %s\n", ip);
        return 1;
    }

    // 2. Création des Threads (Le coeur du système temps réel)
    pthread_create(&th_logic, NULL, task_setpoint_logic, NULL);
    pthread_create(&th_power, NULL, task_power, NULL);
    pthread_create(&th_env,   NULL, task_environment, NULL);
    pthread_create(&th_disp,  NULL, task_display, &sock);

    printf("=== SYSTÈME THERMOSTAT ÉTS EN LIGNE ===\n");
    printf("Cible : %s | Mode : Simulation de consigne active\n", ip);

    // 3. On attend que les threads finissent (ce qui n'arrive jamais en boucle while(1))
    pthread_join(th_logic, NULL);
    
    socket_close(&sock);
    return 0;
}