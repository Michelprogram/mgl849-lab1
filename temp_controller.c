// Compiler :  gcc -Wall -Wextra -Werror exe_p04.c -o exe_p04
// Execiter : ./exe_04

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>   // wait()
#include <stdlib.h>  


float read_room_temp() {
    // Example: replace with actual sensor read
    return 21.5;
}

float read_user_temp() {
    // Example: replace with ADC read from potentiometer
    return 25.0;
}


int main() {
    int pipefd[2];
    pipe(pipefd);

    pid_t pid1 = fork();

    if (pid1 == 0) {
        // Child 1 → Room temperature
        close(pipefd[0]); // Close read end
        float room_temp = read_room_temp();
        write(pipefd[1], &room_temp, sizeof(room_temp));
        close(pipefd[1]);
        exit(0);
    }

    pid_t pid2 = fork();

    if (pid2 == 0) {
        // Child 2 → User-defined temperature
        close(pipefd[0]);
        float user_temp = read_user_temp();

        write(pipefd[1], &user_temp, sizeof(user_temp));
        close(pipefd[1]);
        exit(0);
    }

    // Parent process
    close(pipefd[1]); // Close write end

    float room_temp, user_temp;

    read(pipefd[0], &room_temp, sizeof(room_temp));
    read(pipefd[0], &user_temp, sizeof(user_temp));

    close(pipefd[0]);

    wait(NULL);
    wait(NULL);

    float power = (user_temp - room_temp) / 6.0 * 100.0;

    // Clamp power between 0 and 100
    if (power < 0) power = 0;
    if (power > 100) power = 100;

    printf("Room temp: %.2f °C\n", room_temp);
    printf("User temp: %.2f °C\n", user_temp);
    printf("Power: %.2f %%\n", power);

    return 0;
}

                                                                        //changer la temp pourrait envoyer un signal??????????

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <sys/mman.h>

typedef struct {
    float room_temp;
    float user_temp;
    float power;
} shared_data_t;

shared_data_t *data;


void power_signal_handler(int sig) {
    data->power = (data->user_temp - data->room_temp) / 6.0 * 100.0;

    if (data->power < 0) data->power = 0;
    if (data->power > 100) data->power = 100;

    printf("[HIGH PRIORITY] Power updated: %.2f %%\n", data->power);

    int main() {
    // Create shared memory
    data = mmap(NULL, sizeof(shared_data_t),
                PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // Fork power process FIRST
    pid_t power_pid = fork();

    if (power_pid == 0) {
        // POWER PROCESS (high priority)
        struct sched_param param = { .sched_priority = 80 };
        sched_setscheduler(0, SCHED_FIFO, &param);

        signal(SIGUSR1, power_signal_handler);

        while (1)
            pause(); // Wait for signal
    }

    // Fork room temp process
    pid_t room_pid = fork();
    if (room_pid == 0) {
        while (1) {
            data->room_temp = read_room_temp();
            sleep(1);
        }
    }

    // Fork user temp process
    pid_t user_pid = fork();
    if (user_pid == 0) {
        float last = 0;
        while (1) {
            float new_temp = read_user_temp();
            if (new_temp != last) {
                data->user_temp = new_temp;
                kill(power_pid, SIGUSR1); // Wake power process
                last = new_temp;
            }
            usleep(200000);
        }
    }

    // Parent → other low priority tasks
    nice(10);
    while (1) {
        // Background work                  SCRIPT D'AFFICHAGE DES VALEURS???
        sleep(5);
    }
}


//                                         !!!!!!!!!!!!! UTILISATION DE SCHEDFIFO!!!!!!!!!!!!!!  VOIR OU RAJOUTER FORK POUR TACHES CONCURRENTES
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <unistd.h>

float room_temp;
float user_temp;
float power;

pthread_mutex_t temp_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t temp_changed = PTHREAD_COND_INITIALIZER;


// power calculation thread

void* power_task(void* arg) {
    struct sched_param param;
    param.sched_priority = 80; // High (1–99)      !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);

    while (1) {
        pthread_mutex_lock(&temp_mutex);
        pthread_cond_wait(&temp_changed, &temp_mutex);

        power = (user_temp - room_temp) / 6.0 * 100.0;

        if (power < 0) power = 0;
        if (power > 100) power = 100;

        printf("[HIGH PRIORITY] Power updated: %.2f %%\n", power);
        pthread_mutex_unlock(&temp_mutex);
    }
}

//user_temp thread

void* user_temp_task(void* arg) {
    while (1) {
        float new_user_temp = read_user_temp();

        pthread_mutex_lock(&temp_mutex);
        if (new_user_temp != user_temp) {
            user_temp = new_user_temp;
            pthread_cond_signal(&temp_changed); // Wake power task
        }
        pthread_mutex_unlock(&temp_mutex);

        usleep(200000);
    }
}

//room_temp thread

void* room_temp_task(void* arg) {
    while (1) {
        pthread_mutex_lock(&temp_mutex);
        room_temp = read_room_temp();
        pthread_mutex_unlock(&temp_mutex);
        usleep(500000);
    }
}

//background tasks

void* background_task(void* arg) {
    nice(10); // Lower priority
    while (1) {
        // Non-critical work
        usleep(1000000);
    }
}
