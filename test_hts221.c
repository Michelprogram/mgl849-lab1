#include "hts221.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    int fd = hts221_init();
    if (fd < 0) {
        printf("Erreur : Impossible d'accéder au capteur d'humidité.\n");
        return 1;
    }

    printf("--- Test Humidité  (Tâche 5) ---\n");

    for(int i = 0; i < 5; i++) {
        float h = hts221_read_humidity(fd);
        printf("Mesure %d : %.2f %%\n", i + 1, h);
        
        sleep(2); // Ta contrainte de 2 secondes
    }

    hts221_close(fd);
    printf("Test terminé.\n");
    return 0;
}