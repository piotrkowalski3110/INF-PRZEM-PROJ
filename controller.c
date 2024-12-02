#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

char SHM_NAME[] = "garden_shm";
char SEM_NAME[] = "garden_sem";

typedef struct
{
    float temperature;
    float air_humidity;
    float soil_moisture;
    float sunlight;
} garden_data;

void handle_sigint(int sig)
{
    printf("\nOperacja niedozwolona\n");
}

int main(int argc, char *argv[])
{
    int shm_descriptor;
    garden_data *shared_data;
    garden_data garden_local;
    sem_t *semaphore_descriptor;
    int SHM_SIZE = sizeof(garden_data);
    signal(SIGINT, handle_sigint);

    if ((shm_descriptor = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666)) == -1)
    {
        perror("shm_open failure");
        _exit(-1);
    }

    if (ftruncate(shm_descriptor, SHM_SIZE) == -1)
    {
        perror("ftruncate failure");
        _exit(-1);
    }

    shared_data = mmap(NULL, SHM_SIZE,
                       PROT_READ | PROT_WRITE,
                       MAP_SHARED, shm_descriptor, 0);

    if (shared_data == MAP_FAILED)
    {
        perror("mmap failure");
        _exit(-1);
    }

    if ((semaphore_descriptor = sem_open(SEM_NAME, O_CREAT, 0666, 1)) == SEM_FAILED)
    {
        perror("sem_open failure");
        _exit(-1);
    }

    shared_data->temperature = 0;
    shared_data->air_humidity = 0;
    shared_data->soil_moisture = 0;
    shared_data->sunlight = 0;

    while (1)
    {
        sem_wait(semaphore_descriptor);
        garden_local.temperature = shared_data->temperature;
        garden_local.air_humidity = shared_data->air_humidity;
        garden_local.soil_moisture = shared_data->soil_moisture;
        garden_local.sunlight = shared_data->sunlight;
        sem_post(semaphore_descriptor);

        system("clear");
        printf("\n--- Stan czujników ---\n");
        printf("Temperatura: %.2f°C\n", garden_local.temperature);
        printf("Wilgotność powietrza: %.2f%%\n", garden_local.air_humidity);
        printf("Wilgotność ziemi: %.2f%%\n", garden_local.soil_moisture);
        printf("Nasłonecznienie: %.2f lux\n", garden_local.sunlight);

        printf("\n--- Urządzenia ---\n");
        printf("Mata grzewcza: %s\n", garden_local.temperature < 10 ? "Włączona" : "Wyłączona");
        printf("Wentylator: %s\n", garden_local.air_humidity > 80 ? "Włączony" : "Wyłączony");
        printf("Nawilżacz powietrza: %s\n", garden_local.air_humidity < 40 ? "Włączony" : "Wyłączony");
        printf("System nawadniania: %s\n", garden_local.soil_moisture < 30 ? "Włączony" : "Wyłączony");
        printf("Sztuczne oświetlenie: %s\n", garden_local.sunlight < 1500 ? "Włączone" : "Wyłączone");

        printf("\n--- Diody ostrzegawcze ---\n");
        printf("Temp LED: %s\n", garden_local.temperature < 10 ? "czerwona" : garden_local.temperature <= 15 ? "pomaranczowa"
                                                                          : garden_local.temperature <= 25   ? "zielona"
                                                                          : garden_local.temperature < 30    ? "pomaranczowa"
                                                                                                             : "czerwona");

        printf("Air Humidity LED: %s\n", garden_local.air_humidity < 40 ? "czerwona" : garden_local.air_humidity <= 50 ? "pomaranczowa"
                                                                                   : garden_local.air_humidity <= 70   ? "zielona"
                                                                                   : garden_local.air_humidity < 80    ? "pomaranczowa"
                                                                                                                       : "czerwona");

        printf("Soil Moisture LED: %s\n", garden_local.soil_moisture < 30 ? "czerwona" : garden_local.soil_moisture <= 40 ? "pomaranczowa"
                                                                                     : garden_local.soil_moisture <= 80   ? "zielona"
                                                                                     : garden_local.soil_moisture < 90    ? "pomaranczowa"
                                                                                                                          : "czerwona");

        printf("Lux LED: %s\n", garden_local.sunlight < 1500 ? "czerwona" : garden_local.sunlight <= 3000 ? "pomaranczowa"
                                                                        : garden_local.sunlight <= 7000   ? "zielona"
                                                                        : garden_local.sunlight < 8500    ? "pomaranczowa"
                                                                                                          : "czerwona");
        sleep(1);
    }

    munmap(shared_data, SHM_SIZE);
    close(shm_descriptor);
    sem_close(semaphore_descriptor);
    sem_unlink(SEM_NAME);
    shm_unlink(SHM_NAME);
    return 0;
}