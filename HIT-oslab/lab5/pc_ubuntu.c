#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/wait.h>
#define ALLNUM 550
#define CUSTOMERNUM 5
#define BUFFERSIZE 10
void Producters(FILE *fp);
void Customer(FILE *fp);
sem_t *empty,*full,*mutex;
FILE *fp;
int Inpos=0;
int Outpos=0;

int main(int argc, char ** argv)
{
    pid_t producter;
    pid_t customer;
    empty=sem_open("empty",O_CREAT,0777,10);
    full=sem_open("full",O_CREAT,0777,0);
    mutex=sem_open("mutex",O_CREAT,0777,1);
    fp=fopen("./products.txt","wb+");
    fseek(fp,10*sizeof(int),SEEK_SET);
    fwrite(&Outpos,sizeof(int),1,fp);
    fflush(fp);
    producter=fork();
    if(producter != 0)
    {
        Producters(fp);
    }
    else
    {
        for (int i=0;i<CUSTOMERNUM;i++)
        {

            customer=fork();
            if(customer==0)
            {
                Customer(fp);
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
    fclose(fp);
    return 0;
}

void Producters(FILE *fp)
{
    int i=0;
    for (i=0;i<ALLNUM;i++)
    {
        sem_wait(empty);
        sem_wait(mutex);
        fseek( fp, Inpos * sizeof(int), SEEK_SET );
        fwrite(&i,sizeof(int),1,fp);
        fflush(fp);
        Inpos=(Inpos +1) % BUFFERSIZE;
        sem_post(mutex);
        sem_post(full);
    }
}

void Customer(FILE *fp)
{
    int j,productid;
    for (j=0;j<ALLNUM/CUSTOMERNUM;j++)
    {
        sem_wait(full);
        sem_wait(mutex);
        fseek(fp,10*sizeof(int),SEEK_SET);
        fread(&Outpos,sizeof(int),1,fp);
        fseek(fp,Outpos*sizeof(int),SEEK_SET);
        fread(&productid,sizeof(int),1,fp);
        printf("%d:   %d\n",getpid(),productid);
        fflush(stdout);
        Outpos=(Outpos+1)% BUFFERSIZE;
        fseek(fp,10*sizeof(int),SEEK_SET);
        fwrite(&Outpos,sizeof(int),1,fp);
        fflush(fp);
        sem_post(mutex);
        sem_post(empty);
    }
}