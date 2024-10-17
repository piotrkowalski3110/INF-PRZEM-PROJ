/* link with pthread and realtime libraries            */
/* e.g. gcc -std=c99 reader.c -lpthread -lrt -o reader */

/**********  reader.c  ***********/
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <stdio.h>

char shm_fn[] = "my_shm";
char sem_fn[] = "my_sem";

/**** READER ****/

int main(){
  void* shmptr;
  int shmdes, index;
  sem_t *semdes;
  int SHM_SIZE;

  /* Open the shared memory object */

  SHM_SIZE = sysconf(_SC_PAGESIZE);

  if ( (shmdes = shm_open(shm_fn, O_RDWR, 0)) == -1 ) {
     perror("shm_open failure");
     _exit(-1);
   }

  if((shmptr = mmap(0, SHM_SIZE, PROT_WRITE|PROT_READ, MAP_SHARED,
               shmdes,0)) == (void*) -1){
     perror("mmap failure");
    _exit(-1);
  }

 /* Open the Semaphore */

 semdes = sem_open(sem_fn, 0, 0644, 0);

 if(semdes == (void*) -1){
   perror("sem_open failure");
   _exit(-1);
 }

 /* Lock the semaphore */

 if(!sem_wait(semdes)){

  /* Access to the shared memory area */
char *shm_ptr = shmptr;
   for(index = 0; index < 100; index++)
        printf("The shared memory shmptr[%d] = %d\n", index,shm_ptr[index]);

  /* Release the semaphore lock */

   sem_post(semdes);
  }

  munmap(shmptr, SHM_SIZE);

  /* Close the shared memory object */

  close(shmdes);

  /* Close the Semaphore */

  sem_close(semdes);
  sem_unlink(sem_fn);
}
