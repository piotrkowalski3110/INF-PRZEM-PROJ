#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char SHM_NAME[] = "garden_shm";
char SEM_NAME[] = "garden_sem";

typedef struct
{
    float temperature;
    float air_humidity;
    float soil_moisture;
    float sunlight;
} garden_data;

int main(int argc, char *argv[])
{
    int shm_descriptor;
    garden_data *shared_data;
    sem_t *semaphore_descriptor;
    int SHM_SIZE = sizeof(garden_data);

    if ((shm_descriptor = shm_open(SHM_NAME, O_RDWR, 0666)) == -1)
    {
        perror("shm_open failure");
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

    if ((semaphore_descriptor = sem_open(SEM_NAME, 0)) == SEM_FAILED)
    {
        perror("sem_open failure");
        _exit(-1);
    }

    while (1)
    {
        float air_humidity;
        char buffer[10];

        system("clear");
        printf("Podaj wilgotność powietrza w szklarni: ");

        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        {
            printf("Błąd odczytu. Spróbuj ponownie.\n");
            continue;
        }

        buffer[strcspn(buffer, "\n")] = 0;

        if (sscanf(buffer, "%f", &air_humidity) != 1)
        {
            printf("Błąd: wprowadź poprawną liczbę.\n");
            continue;
        }

        sem_wait(semaphore_descriptor);
        shared_data->air_humidity = air_humidity;
        sem_post(semaphore_descriptor);
    }

    munmap(shared_data, SHM_SIZE);
    close(shm_descriptor);
    sem_close(semaphore_descriptor);
    return 0;
}