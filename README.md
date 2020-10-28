# tos

Tairy's Operating System.

## Develop

## Bochs

```bash
cd .
bochs -f .bochsrc
```

### Boot

```bash
cd boot
nasm boot.asm -o boot.bin
dd if=boot.bin of=boot.img bs=512 count=1 conv=notrunc
```

