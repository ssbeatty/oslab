#define __LIBRARY__
#include <unistd.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/segment.h>
#include <asm/system.h>

// head定义在unistd.h

sem_t semaphores[SEM_COUNT];

/*信号量是否已打开，是返回位置*/
int sem_location(const char* name)
{
    int i;
    for(i = 0;i < SEM_COUNT; i++)
    {
        if(strcmp(name,semaphores[i].name) == 0 && semaphores[i].occupied == 1)
        {
            return i;
        }
    }
    return -1;
}
/*打开信号量*/
sem_t* sys_sem_open(const char* name,unsigned int value)
{
    char tmp[SEM_NAME_LEN];
    char c;
    int i;
    for( i = 0; i<SEM_NAME_LEN; i++)
    {
        c = get_fs_byte(name+i);
        tmp[i] = c;
        if(c =='\0') break;
    }
    if(c >= SEM_NAME_LEN)
    {
        printk("Semaphore name is too long!");
        return NULL;
    }
    if((i = sem_location(tmp)) != -1)
    {
        return &semaphores[i];
    }
    for(i = 0;i< SEM_COUNT; i++)
    {
        if(!semaphores[i].occupied)
        {
            strcpy(semaphores[i].name,tmp);
            semaphores[i].occupied = 1;
            semaphores[i].value = value;
            return &semaphores[i];
        }
    }
    printk("Numbers of semaphores are limited!\n");
    return NULL;
}
/*P原子操作*/
int sys_sem_wait(sem_t* sem)
{
    cli();
    while(sem->value<=0)
        sleep_on(&(sem->s_wait));
    sem->value--;
    sti();
    return 0;
}
/*V原子操作*/
int sys_sem_post(sem_t* sem)
{
    cli();
    sem->value++;
    if((sem->value) > 0)
    {
        wake_up(&(sem->s_wait));
        return 0;
    }
    sti();
    return 0;
}
/*释放信号量*/
int sys_sem_unlink(const char *name)
{
    char tmp[SEM_NAME_LEN];
    char c;
    int i;
    for( i = 0; i<SEM_NAME_LEN; i++)
    {
        c = get_fs_byte(name+i);
        tmp[i] = c;
        if(c =='\0') break;
    }
    if(c >= SEM_NAME_LEN)
    {
        printk("Semphore name is too long!");
        return -1;
    }
    int ret = sem_location(tmp);
    if(ret != -1)
    {
        semaphores[ret].value = 0;
        strcpy(semaphores[ret].name,"\0");
        semaphores[ret].occupied = 0;
        return 0;
    }
    return -1;
}