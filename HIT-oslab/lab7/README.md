### 实验目的

- 加深对操作系统设备管理基本原理的认识，实践键盘中断、扫描码等概念；
- 通过实践掌握 Linux 0.11 对键盘终端和显示器终端的处理过程。

### 实验内容
- 本实验的基本内容是修改 Linux 0.11 的终端设备处理代码，对键盘输入和字符显示进行非常规的控制。

- 在初始状态，一切如常。用户按一次 F12 后，把应用程序向终端输出所有字母都替换为“*”。用户再按一次 F12，又恢复正常。第三次按 F12，再进行输出替换。依此类推。

### 实验过程
1. 键盘I/O是中断驱动，在`kernel/chr_drv/console.c`文件中：
```c
void con_init(void)  //控制台的初始化
{
    // 键盘中断响应函数设为 keyboard_interrupt
    set_trap_gate(0x21, &keyboard_interrupt);
}
```

所以每次按键有动作，keyboard_interrupt 函数就会被调用，它在文件 `kernel/chr_drv/keyboard.S`

修改`keyboard.S`当按下f12时调用函数

原来调用链如下
```c
keyboard_interrupt -> call key_table

keytable
/.../
.long func,none,none,none		/* 58-5B f12 ? ? ? */
/.../
-> func -> call show_stat
```

可以直接修改func中show_stat方法，在`console.c`中写一个全局变量，并写一个新方法，当按下f12改变其值
```c
int F12_flag = 0;

void change_F12_flag(void)
{
    if(F12_flag)
    {
        F12_flag = 0;
    }
    else
    {
        F12_flag = 1;
    }
}
```
2. 修改输出字符的方法
修改con_write中字符ascii的分支`if (c>31 && c<127)`在这一行代码前添加
```c
// 添加的代码
if(F12_flag == 1 && ( (c >= 48 && c<= 57) || (c>=65 && c<=90) || (c>=97 && c<=122)))
                        c = '*';
// 添加的代码
__asm__("movb attr,%%ah\n\t"
                            "movw %%ax,%1\n\t"
                    ::"a" (c),"m" (*(short *)pos)
                    );
```

### 问题
1. 在原始代码中，按下 F12，中断响应后，中断服务程序会调用 func，它实现的是什么功能？

正常情况下打开模拟器中，按下功能键F12即可显示内核栈中各个进程的状态信息，而当把func中的call show_stat屏蔽掉后，再按下F12就什么也没有了，可见func实现的功能就是调用show_stat函数来显示内核栈中各个进程的状态信息。

2. 在你的实现中，是否把向文件输出的字符也过滤了？

只过滤了向终端输出的字符，向文件输出的字符没有被过滤。因为我们最后修改的是控制向显存输出的con_write()函数，使其当flag=1的时候向显存输出星号。如果要过滤掉向文件输出的字符的话，需要找到对应的那个控制向文件输出的函数并修改。