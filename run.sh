#!/bin/bash

RED_COLOR='\E[1;31m'
RESET='\E[0m'
SRC_DIR='./src'
BOOT_DIR=${SRC_DIR}'/boot'

echo -e "${RED_COLOR}=== env check ===${RESET}"

if [ ! -e ${SRC_DIR}/.bochsrc ]; then
  echo "no .bochsrc,please checkout!"
  exit 1
fi

if [ -e /usr/local/bin/bochs ]; then
  echo "find /usr/local/bin/bochs!"
else
  echo "no bochs find!,Please check your bochs environment"
fi

if [ -e /usr/local/bin/bximage ]; then
  echo "find /usr/local/bin/bximage!"
else
  echo "no bximge find!,Please check your bochs environment"
fi

if [ -e ${BOOT_DIR}/boot.img ]; then
  echo "find boot.img !"
else
  echo "no boot.img! generating..."
  echo -e "1\nfd\n\nboot.img\n" | bximage
fi

if [ ! -e /usr/local/share/bochs/BIOS-bochs-latest ]; then
  echo " /usr/local/share/bochs/BIOS-bochs-latest does not exist..."
  exit 1
else
  file /usr/local/share/bochs/BIOS-bochs-latest
fi

if [ ! -e /usr/local/share/bochs/VGABIOS-lgpl-latest ]; then
  echo "/usr/local/share/bochs/VGABIOS-lgpl-latest does not exist..."
  exit 1
else
  file /usr/local/share/bochs/VGABIOS-lgpl-latest
fi

echo -e "${RED_COLOR}=== gen boot.bin ===${RESET}"
nasm ${BOOT_DIR}/boot.asm -o ${BOOT_DIR}/boot.bin
nasm ${BOOT_DIR}/loader.asm -o ${BOOT_DIR}/loader.bin
echo -e "${RED_COLOR}=== write boot.bin  to boot.img ===${RESET}"
dd if=${BOOT_DIR}/boot.bin of=${BOOT_DIR}/boot.img bs=512 count=1 conv=notrunc
echo -e "${RED_COLOR}=== running..PS:emulator will stop at beginning,press 'c' to running ===${RESET}"

if [ ! -e tmp ]; then
  mkdir tmp
fi

mount -t vfat -o loop ${BOOT_DIR}/boot.img tmp/

cp ${BOOT_DIR}/loader.bin tmp/
sync
umount tmp/

rm -rf tmp

if [ -e /usr/local/bin/bochs ]; then
  /usr/local/bin/bochs -qf ${SRC_DIR}/.bochsrc
else
  echo "Please check your bochs environment!!!"
  exit 1
fi
