#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

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
        printf("\n--- Stan czujników ---\n");
        printf("Temperatura: %.2f°C\n", shared_data->temperature);
        printf("Wilgotność powietrza: %.2f%%\n", shared_data->air_humidity);
        printf("Wilgotność ziemi: %.2f%%\n", shared_data->soil_moisture);
        printf("Nasłonecznienie: %.2f lux\n", shared_data->sunlight);
        sem_post(semaphore_descriptor);
        sleep(2);
    }

    munmap(shared_data, SHM_SIZE);
    close(shm_descriptor);
    sem_close(semaphore_descriptor);
    sem_unlink(SEM_NAME);
    shm_unlink(SHM_NAME);
    return 0;
}