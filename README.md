# tos

Tairy's Operating System.

## Develop

### Bochs

#### Install bochs
```bash
./configure --with-x11 --with-wx --enable-disasm --enable-all-optimizations --enable-readline --enable-long-phy-address --enable-ltdl-install --enable-idle-hack --enable-plugins --enable-a20-pin --enable-repeat-speedups --enable-fast-function-calls --enable-handlers-chaining --enable-trace-linking --enable-configurable-msrs --enable-show-ips --enable-cpp --enable-debugger-gui --enable-iodebug --enable-logging --enable-assert-checks --enable-monitor-mwait --enable-x86-debugger --enable-pci --enable-usb --enable-voodoo --enable-x86-64 --enable-debugger                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
make -j8
cp misc/bximage.cpp misc/bximage.cc && \
cp iodev/hdimage/hdimage.cpp iodev/hdimage/hdimage.cc && \
cp iodev/hdimage/vmware3.cpp iodev/hdimage/vmware3.cc && \
cp iodev/hdimage/vmware4.cpp iodev/hdimage/vmware4.cc && \
cp iodev/hdimage/vpc-img.cpp iodev/hdimage/vpc-img.cc && \
cp iodev/hdimage/vbox.cpp iodev/hdimage/vbox.cc
sudo make install
```

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

### Reference

- [调试虚拟机OS时断点设置问题和总结](https://groups.google.com/g/xv6-jos/c/AY478r9P3n0)
