CFLAGS = -g

all: system
	objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary system kernel.bin

system:	head.o entry.o main.o printk.o trap.o memory.o interrupt.o task.o
	ld -b elf64-x86-64 -z muldefs -o system head.o entry.o main.o printk.o trap.o memory.o interrupt.o task.o -T kernel.lds

head.o: head.S
	gcc $(CFLAGS) -E head.S > head.s
	as --64 -o head.o head.s

entry.o: entry.S
	gcc $(CFLAGS) -E entry.S > entry.s
	as --64 -o entry.o entry.s

main.o: main.c
	gcc $(CFLAGS) -mcmodel=large -fno-builtin -m64 -c main.c -fno-stack-protector

printk.o: printk.c
	gcc $(CFLAGS) -mcmodel=large -fno-builtin -m64 -c printk.c -fno-stack-protector

trap.o: trap.c
	gcc $(CFLAGS) -mcmodel=large -fno-builtin -m64 -c trap.c -fno-stack-protector

memory.o: memory.c
	gcc $(CFLAGS) -mcmodel=large -fno-builtin -m64 -c memory.c -fno-stack-protector

interrupt.o: interrupt.c
	gcc $(CFLAGS) -mcmodel=large -fno-builtin -m64 -c interrupt.c -fno-stack-protector

task.o: task.c
	gcc $(CFLAGS) -mcmodel=large -fno-builtin -m64 -c task.c -fno-stack-protector

clean:
	rm -rf *.o *.s~ *.s *.S~ *.c~ *.h~ *.h.gch system Makefile~ kernel.lds~ kernel.bin