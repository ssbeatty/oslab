#define   __LIBRARY__
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

_syscall2(sem_t*,sem_open,const char *,name,unsigned int,value);
_syscall1(int,sem_wait,sem_t*,sem);
_syscall1(int,sem_post,sem_t*,sem);
_syscall1(int,sem_unlink,const char *,name);
_syscall1(void*,shmat,int,shmid);
_syscall1(int,shmget,char*,name);

#define SIZE 10
#define M 510

int main()
{
    int shm_id;
    int count = 0;
    int *p;
    int curr;
    sem_t *sem_empty, *sem_full, *sem_shm;
    sem_empty = sem_open("empty", SIZE);
    sem_full = sem_open("full", 0);
    sem_shm = sem_open("shm", 1);
    shm_id = shmget("buffer");
    p = (int *)shmat(shm_id);
    while (count <= M) {
        sem_wait(sem_empty);
        sem_wait(sem_shm);
        curr = count % SIZE;
        *(p + curr) = count;
        printf("Producer: %d\n", *(p + curr));
        fflush(stdout);
        sem_post(sem_shm);
        sem_post(sem_full);
        count++;
    }
    printf("producer end.\n");
    fflush(stdout);
    /*这里不能释放信号量，消费者还没消费完呢*/
    return 0;
}