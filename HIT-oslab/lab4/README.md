### 实验目的

- 深入理解进程和进程切换的概念；
- 综合应用进程、CPU 管理、PCB、LDT、内核栈、内核态等知识解决实际问题；
- 开始建立系统认识。

### 实验过程
1. 修改`kernal/sched.c`的switch_to调用

```c
if ((*p)->state == TASK_RUNNING && (*p)->counter > c)
    c = (*p)->counter, next = i, pnext = *p;

//.......

switch_to(pnext, _LDT(next));
```

2. 注释掉`include/linux/sched.h`中的switch_to宏

```c
//#define switch_to(n) {\
//struct {long a,b;} __tmp; \
//__asm__("cmpl %%ecx,current\n\t" \
//	"je 1f\n\t" \
//	"movw %%dx,%1\n\t" \
//	"xchgl %%ecx,current\n\t" \
//	"ljmp *%0\n\t" \
//	"cmpl %%ecx,last_task_used_math\n\t" \
//	"jne 1f\n\t" \
//	"clts\n" \
//	"1:" \
//	::"m" (*&__tmp.a),"m" (*&__tmp.b), \
//	"d" (_TSS(n)),"c" ((long) task[n])); \
//}
```

3. 实现新的switch_to方法
[详细说明](https://www.cxymm.net/article/qq_42518941/119182097)

4. 修改`fork.c`中copy_process方法

去掉tss切换的部分, 通过krnstack切换（本实验的目的）

5. 实现first_return_from_kernel, 五段论弹回用户态的一段
[详细说明](https://blog.csdn.net/qq_37857224/article/details/119172255)
