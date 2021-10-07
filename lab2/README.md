### 实验目的


实现系统调用并注册到内核

### 概念
> 系统调用

1. 把系统调用的编号存入 EAX；
2. 把函数参数存入其它通用寄存器；
3. 触发 0x80 号中断（int 0x80）。

> 从 Linux 0.11 现在的机制看，它的系统调用最多能传递几个参数？你能想出办法来扩大这个限制吗？

从linux-0.11/include/unistd.h中可以知道_syscall宏展开的系统调用最多3个参数，使用ebx，ecx，edx三个寄存器传递参数。
    解决限制的方法：将需要传递的多个参数保存在有特定结构的区间中，并将该用户态地址空间的这个区间的首地址作为一个参数传递给系统调用。
    最后通过寄存器间接寻址方式便可以访问所有参数。当然，这么做的话，参数合法性验证尤其必要。
实际上，linux2.6内核废除了_syscall宏，而使用syscall函数，其接受一个可变参数，原理类似，参考《深入理解Linux内核（第三版）》 P409。


### 过程
1. 修改`kernel/system_call.s`系统调用总数
    ```text
    nr_system_calls = 74
    ```
2. 修改sys_call_table`include/linux/sys.h` 
    ```text
    extern int sys_iam();
    extern int sys_whoami();
   
   fn_ptr sys_call_table[] = { ..., sys_iam, sys_whoami}
    ```
3. 在内核实现sys_iam()和sys_whoami()`kernel/who.c`
4. 修改`kernel/Makefile`并编译 详见实验楼说明
5. 在unistd.h增加__NR_whoami和__NR_iam 后面的`iam.c`引入
   这里的unistd.h是指linux中/usr/include下的而不是源码中的
    ```text
    #define __NR_iam        72
    #define __NR_whoami  	73
    ```
6. 编写应用程序具体可见`iam.c`

   



