# tos

Tairy's Operating System.

## Develop

### Bochs

```bash
cd .
bochs -f .bochsrc
```

### Boot

```bash
cd boot

# make
make clean
make all

# write boot.bin
dd if=boot.bin of=boot.img bs=512 count=1 conv=notrunc

# sync loader.bin
sudo mount ./boot.img /tos -t vfat -o loop
sudo cp loader.bin /tos
sync
sudo umount /tos
```

