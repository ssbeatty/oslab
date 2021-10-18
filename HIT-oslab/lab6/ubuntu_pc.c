#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/shm.h>

#define ALLNUM 550
#define CUSTOMERNUM 5
#define BUFFERSIZE 10
void Producters(int shmid);
void Customer(int shmid);
sem_t *empty,*full,*mutex;
int Inpos=0;
int Outpos=0;

int main(int argc, char ** argv)
{
    int shmid = shmget(IPC_PRIVATE, BUFFERSIZE, IPC_CREAT|0600);
    pid_t producter;
    pid_t customer;
    empty=sem_open("empty",O_CREAT,0777,10);
    full=sem_open("full",O_CREAT,0777,0);
    mutex=sem_open("mutex",O_CREAT,0777,1);
    producter=fork();
    if(producter != 0)
    {
        Producters(shmid);
    }
    else
    {
        for (int i=0;i<CUSTOMERNUM;i++)
        {

            customer=fork();
            if(customer==0)
            {
                Customer(shmid);
                break;
            }
        }
    }
    wait(NULL);
    wait(NULL);
    wait(NULL);
    wait(NULL);
    wait(NULL);
    sem_unlink("empty");
    sem_unlink("full");
    sem_unlink("mutex");
    return 0;
}

void Producters(int shmid)
{
    int *shmaddr = (int *)shmat(shmid, NULL, 0 );
    int i=0;
    for (i=0;i<ALLNUM;i++)
    {
        sem_wait(empty);
        sem_wait(mutex);
        shmaddr[Inpos] = i;
        Inpos=(Inpos +1) % BUFFERSIZE;
        sem_post(mutex);
        sem_post(full);
    }
}

void Customer(int shmid)
{
    int *shmaddr = (int *)shmat(shmid, NULL, 0 );
    int j,productid;
    for (j=0;j<ALLNUM/CUSTOMERNUM;j++)
    {
        sem_wait(full);
        sem_wait(mutex);
        productid = shmaddr[Outpos];
        printf("%d:   %d\n",getpid(),productid);
        fflush(stdout);
        Outpos=(Outpos+1)% BUFFERSIZE;
        sem_post(mutex);
        sem_post(empty);
    }
}