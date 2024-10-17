/* link with pthread and realtime libraries            */
/* e.g. gcc -std=c99 reader.c -lpthread -lrt -o reader */

/**********  writer.c  ***********/

#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <stdio.h>

/* check /dev/shm for OS representation                 */
/* of shm and sem POSIX objects                         */

char shm_fn[] = "my_shm";
char sem_fn[] = "my_sem";


int main(){
  void* shmptr;
  unsigned int mode;
  int shmdes, index;
  sem_t *sem_des;
  int SHM_SIZE;

  mode = S_IRWXU|S_IRWXG;

  /* Open the shared memory object */

  if ( (shmdes = shm_open(shm_fn,O_CREAT|O_RDWR|O_TRUNC, mode)) == -1 ) {
     perror("shm_open failure");
     _exit(-1);
   }

/*  Preallocate a shared memory area   */

  SHM_SIZE = sysconf(_SC_PAGESIZE);
/* The size equal to one OS page e.g. 4kb */

  if(ftruncate(shmdes, SHM_SIZE) == -1){
    perror("ftruncate failure");
    _exit(-1);
  }

  if((shmptr = mmap(0, SHM_SIZE, PROT_WRITE|PROT_READ, MAP_SHARED,
                shmdes,0)) == (void*)-1){
    perror("mmap failure");
    _exit(-1);
  }

  /* Create a semaphore in locked state */

 sem_des = sem_open(sem_fn, O_CREAT, 0644, 0);

 if(sem_des == (void*)-1){
   perror("sem_open failure");
   _exit(-1);
 }

    /* Access to the shared memory area */

/* cast  the *void pointer in order to use e.g. as char... */

char *shm_ptr = shmptr;
    for(index = 0; index < 100; index++){
       printf("write %d into the shared memory shmptr[%d]\n", index*2, index);
       shm_ptr[index]=index*2;
       }

  /* Release the semaphore lock */

sleep(5);

  sem_post(sem_des);
  munmap(shmptr, SHM_SIZE);

  /* Close the shared memory object */

  close(shmdes);

  /* Close the Semaphore */

  sem_close(sem_des);

  /* Delete the shared memory object */

  shm_unlink(shm_fn);
}
