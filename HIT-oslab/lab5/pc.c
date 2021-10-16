#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <semaphore.h>
#include <wait.h>

const char *FILENAME = "./buffer_file";    /* 消费生产的产品存放的缓冲文件的路径 */
const int NR_CONSUMERS = 5;                        /* 消费者的数量 */
const int NR_ITEMS = 500;                          /* 产品的最大量 */
const int BUFFER_SIZE = 10;                        /* 缓冲区大小，表示可同时存在的产品数量 */
sem_t *metux, *full, *empty;                    /* 3个信号量 */
unsigned int item_pro, item_used;                /* 刚生产的产品号；刚消费的产品号 */
int fi, fo;                                        /* 供生产者写入或消费者读取的缓冲文件的句柄 */


int main()
{
    int pid;
    int i;

    fi = open(FILENAME, O_CREAT| O_TRUNC| O_WRONLY, 0600);    /* 以只写方式打开文件给生产者写入产品编号 */
    fo = open(FILENAME, O_TRUNC| O_RDONLY);                   /* 以只读方式打开文件给消费者读出产品编号 */

    metux = sem_open("METUX", O_CREAT, 0666, 1);    /* 互斥信号量，防止生产消费同时进行 */
    full = sem_open("FULL", O_CREAT, 0666, 0);        /* 产品剩余信号量，大于0则可消费 */
    empty = sem_open("EMPTY", O_CREAT, 0666, BUFFER_SIZE);    /* 空信号量，它与产品剩余信号量此消彼长，大于0时生产者才能继续生产 */

    item_pro = 0;

    if ((pid = fork()))    /* 父进程用来执行消费者动作 */
    {
        printf("pid %d:\tproducer created....\n", pid);

        while (item_pro <= NR_ITEMS)    /* 生产完所需产品 */
        {
            sem_wait(empty);
            sem_wait(metux);

            if(!(item_pro % BUFFER_SIZE))
                lseek(fi, 0, 0);

            write(fi, (char *) &item_pro, sizeof(item_pro));        /* 写入产品编号 */
//            printf("pid %d:\tproduces item %d\n", pid, item_pro);
            item_pro++;

            sem_post(metux);
            sem_post(full);        /* 唤醒消费者进程 */
        }
    }
    else    /* 子进程来创建消费者 */
    {
        i = NR_CONSUMERS;
        while(i--)
        {
            if(!(pid=fork()))    /* 创建i个消费者进程 */
            {
                pid = getpid();
                printf("pid %d:\tconsumer %d created....\n", pid, NR_CONSUMERS-i);

                while(1)
                {
                    sem_wait(full);
                    sem_wait(metux);

                    /* read()读到文件末尾时返回0，将文件的位置指针重新定位到文件首部 */
                    if(!read(fo, (char *)&item_used, sizeof(item_used)))
                    {
                        lseek(fo, 0, 0);
                        read(fo, (char *)&item_used, sizeof(item_used));
                    }

                    printf("pid %d:\tconsumer %d consumes item %d\n", pid, NR_CONSUMERS-i, item_used);
                    sem_post(metux);
                    sem_post(empty);    /* 唤醒生产者进程 */
                }
            }
        }
    }

    i = NR_CONSUMERS + 1;
    while (i--) {
        int status;
        wait(&status);
    }

    close(fi);
    close(fo);
    return 0;
}