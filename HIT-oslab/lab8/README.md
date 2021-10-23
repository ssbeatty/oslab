### 实验目的

- 掌握虚拟文件系统的实现原理；
- 实践文件、目录、文件系统等概念。

### 实验内容
- 在 Linux 0.11 上实现 procfs（proc 文件系统）内的 psinfo 结点。当读取此结点的内容时，可得到系统当前所有进程的状态信息。

### 实验过程
1. `include/sys/stat.h`中增加文件类型
```c
#define S_IFPROC 0030000
#define S_ISPROC(m) (((m) & S_IFMT) == S_IFPROC)
```

2. 让`fs/namei.c`中`mknod()`系统调用支持新的文件类型
```c
if (S_ISBLK(mode) || S_ISCHR(mode) || S_ISPROC(mode))
     inode->i_zone[0] = dev;
// 文件系统初始化
```
3. `init/main.c`初始化挂载文件
（1）建立 /proc 目录；建立 /proc 目录下的各个结点。本实验只建立 /proc/psinfo。

（2）建立目录和结点分别需要调用 mkdir() 和 mknod() 系统调用。因为初始化时已经在用户态，所以不能直接调用 sys_mkdir() 和 sys_mknod()。必须在初始化代码所在文件中实现这两个系统调用的用户态接口，即 API：
```c
#ifndef __LIBRARY__
#define __LIBRARY__
#endif

_syscall2(int,mkdir,const char*,name,mode_t,mode)
_syscall3(int,mknod,const char*,filename,mode_t,mode,dev_t,dev)

/*...*/
    setup((void *) &drive_info);
    (void) open("/dev/tty0",O_RDWR,0);
    (void) dup(0);
    (void) dup(0);
    mkdir("/proc",0755);
    mknod("/proc/psinfo",S_IFPROC|0444,0);
    mknod("/proc/hdinfo",S_IFPROC|0444,1);
    mknod("/proc/inodeinfo",S_IFPROC|0444,2);
/*...*/
```

`mkdir()` 时 mode 参数的值可以是 “0755”（对应 rwxr-xr-x），表示只允许 root 用户改写此目录，其它人只能进入和读取此目录。

procfs 是一个只读文件系统，所以用 mknod() 建立 psinfo 结点时，必须通过 mode 参数将其设为只读。建议使用 S_IFPROC|0444 做为 mode 值，表示这是一个 proc 文件，权限为 0444（r--r--r--），对所有用户只读。

mknod() 的第三个参数 dev 用来说明结点所代表的设备编号。对于 procfs 来说，此编号可以完全自定义。proc 文件的处理函数将通过这个编号决定对应文件包含的信息是什么。例如，可以把 0 对应 psinfo，1 对应 meminfo，2 对应 cpuinfo。

4. 让 proc 文件可读 `sys_read`（在文件 `fs/read_write.c` 中）
```c
int sys_read(unsigned int fd,char * buf,int count)
{
    struct file * file;
    struct m_inode * inode;

    if (fd>=NR_OPEN || count<0 || !(file=current->filp[fd]))
        return -EINVAL;
    if (!count)
        return 0;
    verify_area(buf,count);
    inode = file->f_inode;
    if (inode->i_pipe)
        return (file->f_mode&1)?read_pipe(inode,buf,count):-EIO;
    if (S_ISPROC(inode->i_mode))
        return proc_read(inode->i_zone[0],&file->f_pos,buf,count);
    if (S_ISCHR(inode->i_mode))
        return rw_char(READ,inode->i_zone[0],buf,count,&file->f_pos);
    if (S_ISBLK(inode->i_mode))
        return block_read(inode->i_zone[0],&file->f_pos,buf,count);
    if (S_ISDIR(inode->i_mode) || S_ISREG(inode->i_mode)) {
        if (count+file->f_pos > inode->i_size)
            count = inode->i_size - file->f_pos;
        if (count<=0)
            return 0;
        return file_read(inode,file,buf,count);
    }
    printk("(Read)inode->i_mode=%06o\n\r",inode->i_mode); /*上个实验一直打印的的东西*/
    return -EINVAL;
}
```

要在这里一群 if 的排比中，加上 S_IFPROC() 的分支，进入对 proc 文件的处理函数。需要传给处理函数的参数包括：
- `inode->i_zone[0]`，这就是 `mknod()` 时指定的 `dev` ——设备编号
- `buf`，指向用户空间，就是 `read()` 的第二个参数，用来接收数据
- `count`，就是 `read()` 的第三个参数，说明 `buf` 指向的缓冲区大小
- `&file->f_pos`，`f_pos` 是上一次读文件结束时“文件位置指针”的指向。这里必须传指针，因为处理函数需要根据传给 `buf` 的数据量修改 `f_pos` 的值。

5. 实现proc.c
proc 文件的处理函数的功能是根据设备编号，把不同的内容写入到用户空间的 buf。写入的数据要从 f_pos 指向的位置开始，每次最多写 count 个字节，并根据实际写入的字节数调整 f_pos 的值，最后返回实际写入的字节数。当设备编号表明要读的是 psinfo 的内容时，就要按照 psinfo 的形式组织数据。

实现此函数可能要用到如下几个函数：

- malloc() 函数
- free() 函数

包含 linux/kernel.h 头文件后，就可以使用 malloc() 和 free() 函数。它们是可以被核心态代码调用的，唯一的限制是一次申请的内存大小不能超过一个页面。

实现`sprintf()`
```c
int sprintf(char *buf, const char *fmt, ...)
{
    va_list args; int i;
    va_start(args, fmt);
    i=vsprintf(buf, fmt, args);
    va_end(args);
    return i;
}
```

具体代码见`proc.c`

6. 修改`fs/Makefile`

### 问题
1. 如果要求你在 psinfo 之外再实现另一个结点，具体内容自选，那么你会实现一个给出什么信息的结点？为什么？

meminfo，可以获得内存相关信息，看那些程序占用内存较多，方便管理。

2. 一次 read() 未必能读出所有的数据，需要继续 read()，直到把数据读空为止。而数次 read() 之间，进程的状态可能会发生变化。你认为后几次 read() 传给用户的数据，应该是变化后的，还是变化前的？ + 如果是变化后的，那么用户得到的数据衔接部分是否会有混乱？如何防止混乱？ + 如果是变化前的，那么该在什么样的情况下更新 psinfo 的内容？

是变化前的，在读取位置f_pos为0时才更新psinfo内容。该inode对应的i_zone[0]依然存在。也就是说，只是从inode映射中取消映射该inode，但是实际上硬盘上的数据还在。