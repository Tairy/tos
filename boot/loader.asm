org 10000h

  mov ax, cs
  mov ds, ax
  mov es, ax
  mov ax, 0x00
  mov ss, ax
  mov sp, 0x7c00

;======== display on screen: Start Loader......

  mov ax, 1301h   ; AH = 6, AL = 0h
  mov bx, 000fh   ; 黑底白字(BL = 07h)
  mov cx, 12      ; 左上角(0,0)
  mov dx, 0200h   ; 右下角(80, 50)
  push ax
  mov ax, ds
  mov es, ax
  pop ax
  mov bp, StartLoaderMessage
  int 10h

  jmp $

;====== display message

StartLoaderMessage: db "Start Loader"


