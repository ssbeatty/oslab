### 实验目的

改写bootsect.s主要完成如下功能：

1. bootsect.s 能在屏幕上打印一段提示信息"XXX is booting..."，其中 XXX 是你给自己的操作系统起的名字，例如 LZJos、Sunix 等（可以上论坛上秀秀谁的 OS 名字最帅，也可以显示一个特色 logo，以表示自己操作系统的与众不同。）

改写 setup.s 主要完成如下功能：

1. bootsect.s 能完成 setup.s 的载入，并跳转到 setup.s 开始地址执行。而 setup.s 向屏幕输出一行"Now we are in SETUP"。
2. setup.s 能获取至少一个基本的硬件参数（如内存参数、显卡参数、硬盘参数等），将其存放在内存的特定地址，并输出到屏幕上。
3. setup.s 不再加载 Linux 内核，保持上述信息显示在屏幕上即可。

### 实验过程

1. 编写bootsect.s打印字符串
```text
entry _start
_start:
    mov ah,#0x03     ! 读取光标位置
    xor bh,bh        ! bh置零
    int 0x10
    mov cx,#36       ! 打印的字符长度 string是30byte 加上3对/r/n
    mov bx,#0x0007   ! page0 attribute 7
    mov bp,#msg1
    mov ax,#0x07c0   ! BOOTSEG  = 0x07c0
    mov es,ax
    mov ax,#0x1301   ! 写光标
    int 0x10
inf_loop:
    jmp inf_loop     ! 设置一个无限循环
msg1:
    .byte   13,10
    .ascii  "Hello OS world, my name is LZJ"
    .byte   13,10,13,10
! boot_flag 必须在最后两个字节
.org 510             ! 512是一个扇区大小，设置成510保证boot_flag在最后两个字节
boot_flag:
    .word   0xAA55
```

> 编译和链接 bootsect.s

```shell script
as86 -0 -a -o bootsect.o bootsect.s
ld86 -0 -s -o bootsect bootsect.o
```

其中 bootsect.o 是中间文件。bootsect 是编译、链接后的目标文件。

需要留意的文件是 bootsect 的文件大小是 544 字节，而引导程序必须要正好占用一个磁盘扇区，即 512 个字节。造成多了 32 个字节的原因是 ld86 产生的是 Minix 可执行文件格式，这样的可执行文件除了文本段、数据段等部分以外，还包括一个 Minix 可执行文件头部，它的结构如下：

```
struct exec {
    unsigned char a_magic[2];  //执行文件魔数
    unsigned char a_flags;
    unsigned char a_cpu;       //CPU标识号
    unsigned char a_hdrlen;    //头部长度，32字节或48字节
    unsigned char a_unused;
    unsigned short a_version;
    long a_text; long a_data; long a_bss; //代码段长度、数据段长度、堆长度
    long a_entry;    //执行入口地址
    long a_total;    //分配的内存总量
    long a_syms;     //符号表大小
};
```
> 去掉32个字节头

```shell script
dd bs=1 if=bootsect of=Image skip=32

# 当前的工作路径为 /home/shiyanlou/oslab/linux-0.11/boot/

# 将刚刚生成的 Image 复制到 linux-0.11 目录下
cp ./Image ../Image

# 执行 oslab 目录中的 run 脚本
../../run
```

2. bootsect.s编写引导setup.s的内容 并去掉之前的循环

```text
load_setup:
    mov dx,#0x0000    ! 设置驱动器和磁头(drive 0, head 0): 软盘 0 磁头
    mov cx,#0x0002    ! 设置扇区号和磁道(sector 2, track 0): 0 磁头、0 磁道、2 扇区
    mov bx,#0x0200    ! 设置读入的内存地址：BOOTSEG+address = 512，偏移512字节
    ! 设置读入的扇区个数(service 2, nr of sectors)，
    ! SETUPLEN是读入的扇区个数，Linux 0.11 设置的是 4，
    ! 我们不需要那么多，我们设置为 2（因此还需要添加变量 SETUPLEN=2）
    mov ax,#0x0200+SETUPLEN
    int 0x13          ! 应用 0x13 号 BIOS 中断读入 2 个 setup.s扇区
    jnc ok_load_setup ! 读入成功，跳转到 ok_load_setup: ok - continue
    mov dx,#0x0000    ! 软驱、软盘有问题才会执行到这里
    mov ax,#0x0000    ! 否则复位软驱 reset the diskette
    int 0x13
    jmp load_setup    ! 重新循环，再次尝试读取
ok_load_setup:
    ! 跳到 setup 执行。
    ! 要注意：我们没有将 bootsect 移到 0x9000，因此跳转后的段地址应该是 0x7ce0
    ! 即我们要设置 SETUPSEG=0x07e0 = 0x07c0 << 4 + 512 0x07c0原来的地址移动一个扇区，setup.s连着bootsect
    jmpi    0,SETUPSEG
```

3. 编写简单的setup.s
```text
entry _start
_start:
    mov ah,#0x03      ! 读取光标位置
    xor bh,bh         ! bh置零
    int 0x10
    mov cx,#25        ! 从新修改长度
    mov bx,#0x000c    ! page0 attribute c (red)
    mov bp,#msg2
    mov ax,cs         ! 从cs获取ex的值
    mov es,ax
    mov ax,#0x1301    ! 写光标
    int 0x10

msg2:
    .byte   13,10
    .ascii  "NOW we are in SETUP"
    .byte   13,10,13,10
.org 510
boot_flag:
    .word   0xAA55
```

4. 修改`tools/build.c`

```
// 注释掉这里是为了make BootImage的时候忽略第三个参数（system相关）
//	if ((id=open(argv[3],O_RDONLY,0))<0)
//		die("Unable to open 'system'");
//	if (read(id,buf,GCC_HEADER) != GCC_HEADER)
//		die("Unable to read header of 'system'");
//	if (((long *) buf)[5] != 0)
//		die("Non-GCC header of 'system'");
//	for (i=0 ; (c=read(id,buf,sizeof buf))>0 ; i+=c )
//		if (write(1,buf,c)!=c)
//			die("Write call failed");
//	close(id);
//	fprintf(stderr,"System is %d bytes.\n",i);
//	if (i > SYS_SIZE*16)
//		die("System is too big");
```

5. 获取硬件参数到0x9000
用 ah=#0x03 调用 0x10 中断可以读出光标的位置，用 ah=#0x88 调用 0x15 中断可以读出内存的大小。有些硬件参数的获取要稍微复杂一些，如磁盘参数表。在 PC 机中 BIOS 设定的中断向量表中 int 0x41 的中断向量位置(4*0x41 = 0x0000:0x0104)存放的并不是中断程序的地址，而是第一个硬盘的基本参数表。第二个硬盘的基本参数表入口地址存于 int 0x46 中断向量位置处。每个硬盘参数表有 16 个字节大小。下表给出了硬盘基本参数表的内容：

 位移 | 大小 | 说明  
 -|-|-
 0x00 | 字 | 柱面数 
0x02 | 字节 | 磁头数
... | ... | ...
0x0E | 字节 | 每磁道扇区数
0x0F | 字节 | 保留
```text
INITSEG=0x9000
mov    ax,#INITSEG
! 设置 ds = 0x9000
mov    ds,ax
mov    ah,#0x03
! 读入光标位置
xor    bh,bh
! 调用 0x10 中断
int    0x10
! 将光标位置写入 0x90000.
mov    [0],dx

! 读入内存大小位置
mov    ah,#0x88
int    0x15
mov    [2],ax

! 从 0x41 处拷贝 16 个字节（磁盘参数表）
mov    ax,#0x0000
mov    ds,ax
lds    si,[4*0x41]
mov    ax,#INITSEG
mov    es,ax
mov    di,#0x0004
mov    cx,#0x10
! 重复16次
rep
movsb
```

6. 打印参数并与`bochs/bochsrc.bxrc`对照，详见代码
