.globl begtext, begdata, begbss, endtext, enddata, endbss
.text
begtext:
.data
begdata:
.bss
begbss:
.text

INITSEG=0x9000

entry _start
_start:
    mov ah,#0x03      ! 读取光标位置
    xor bh,bh         ! bh置零
    int 0x10
    mov cx,#25
    mov bx,#0x000c    ! page0 attribute c
    mov bp,#msg2
    mov ax,cs
    mov es,ax
    mov ax,#0x1301    ! 写光标
    int 0x10

    mov ax,cs
    mov es,ax
! init ss:sp
    mov ax,#INITSEG
    mov ss,ax
    mov sp,#0xFF00

! 获取硬件参数
    mov ax,#INITSEG
    mov ds,ax
    mov ah,#0x03
    xor bh,bh
    int 0x10
    mov [0],dx
    mov ah,#0x88
    int 0x15
    mov [2],ax
    mov ax,#0x0000
    mov ds,ax
    lds si,[4*0x41]
    mov ax,#INITSEG
    mov es,ax
    mov di,#0x0004
    mov cx,#0x10
    rep
    movsb

! Be Ready to Print
    mov ax,cs
    mov es,ax
    mov ax,#INITSEG
    mov ds,ax

! Cursor Position
    mov ah,#0x03
    xor bh,bh
    int 0x10
    mov cx,#18
    mov bx,#0x0007
    mov bp,#msg_cursor
    mov ax,#0x1301
    int 0x10
    mov dx,[0]           ! 0x9000
    call    print_hex
! Memory Size  打印内存
    mov ah,#0x03
    xor bh,bh
    int 0x10
    mov cx,#14
    mov bx,#0x0007
    mov bp,#msg_memory
    mov ax,#0x1301
    int 0x10
    mov dx,[2]           ! offset + 2
    call    print_hex
! Add KB 后缀增加kb
    mov ah,#0x03
    xor bh,bh
    int 0x10
    mov cx,#2
    mov bx,#0x0007
    mov bp,#msg_kb
    mov ax,#0x1301
    int 0x10
! Cyles
    mov ah,#0x03
    xor bh,bh
    int 0x10
    mov cx,#7
    mov bx,#0x0007
    mov bp,#msg_cyles
    mov ax,#0x1301
    int 0x10
    mov dx,[4]          ! offset + 2
    call    print_hex
! Heads
    mov ah,#0x03
    xor bh,bh
    int 0x10
    mov cx,#8
    mov bx,#0x0007
    mov bp,#msg_heads
    mov ax,#0x1301
    int 0x10
    mov dx,[6]         ! offset + 6
    call    print_hex
! Secotrs
    mov ah,#0x03
    xor bh,bh
    int 0x10
    mov cx,#10
    mov bx,#0x0007
    mov bp,#msg_sectors
    mov ax,#0x1301
    int 0x10
    mov dx,[18]       ! offset + e + 4  实验楼是12 不对的
    call    print_hex

inf_loop:
    jmp inf_loop

print_hex:
    mov    cx,#4
print_digit:
! 循环以使低 4 比特用上 !! 取 dx 的高 4 比特移到低 4 比特处。
    rol dx,#4
! ah = 请求的功能值，al = 半字节(4 个比特)掩码。
    mov ax,#0xe0f
! 取 dl 的低 4 比特值。
    and al,dl
! 给 al 数字加上十六进制 0x30
    add al,#0x30
    cmp al,#0x3a
! 是一个不大于十的数字
    jl  outp
! 是a～f，要多加 7
    add al,#0x07
outp:
    int 0x10
    loop    print_digit
    ret
! 这里用到了一个 loop 指令;
! 每次执行 loop 指令，cx 减 1，然后判断 cx 是否等于 0。
! 如果不为 0 则转移到 loop 指令后的标号处，实现循环；
! 如果为0顺序执行。
!
! 另外还有一个非常相似的指令：rep 指令，
! 每次执行 rep 指令，cx 减 1，然后判断 cx 是否等于 0。
！ 如果不为 0 则继续执行 rep 指令后的串操作指令，直到 cx 为 0，实现重复。

! 打印回车换行
print_nl:
! CR
    mov ax,#0xe0d
    int 0x10
! LF
    mov al,#0xa
    int 0x10
    ret

msg2:
    .byte   13,10
    .ascii  "NOW we are in SETUP"
    .byte   13,10,13,10
msg_cursor:
    .byte 13,10
    .ascii "Cursor position:"
msg_memory:
    .byte 13,10
    .ascii "Memory Size:"
msg_cyles:
    .byte 13,10
    .ascii "Cyls:"
msg_heads:
    .byte 13,10
    .ascii "Heads:"
msg_sectors:
    .byte 13,10
    .ascii "Sectors:"
msg_kb:
    .ascii "KB"
.org 510
boot_flag:
    .word   0xAA55

.text
endtext:
.data
enddata:
.bss
endbss:
