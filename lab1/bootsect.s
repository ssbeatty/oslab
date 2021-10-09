.globl begtext, begdata, begbss, endtext, enddata, endbss
.text
begtext:
.data
begdata:
.bss
begbss:
.text

SETUPLEN=2
SETUPSEG=0x07e0

entry _start
_start:
    mov ah,#0x03      ! 读取光标位置
    xor bh,bh         ! bh置零
    int 0x10
    mov cx,#36        ! 打印的字符长度 string是30byte 加上3对/r/n
    mov bx,#0x0007    ! page0 attribute 7
    mov bp,#msg1
    mov ax,#0x07c0    ! BOOTSEG  = 0x07c0
    mov es,ax
    mov ax,#0x1301    ! 写光标
    int 0x10
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
    ! 即我们要设置 SETUPSEG=0x07e0 原来是0x9020  + 20
    jmpi    0,SETUPSEG
msg1:
    .byte   13,10
    .ascii  "Hello OS world, my name is LZJ"
    .byte   13,10,13,10
! boot_flag 必须在最后两个字节
.org 510             ! 512是一个扇区大小，设置成510保证boot_flag在最后两个字节
boot_flag:
    .word   0xAA55

.text
endtext:
.data
enddata:
.bss
endbss:
