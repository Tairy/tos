//
// Created by tairy on 2020/11/6.
//

#ifndef TOS_MEMORY_H
#define TOS_MEMORY_H

#include "printk.h"
#include "lib.h"

// 页表项个数
// 64 位模式下，每个页表项占用字节数由原来的 4 字节变为 8 字节
// 每个页表的大小为 4KB
// 因此页表项个数为 4KB / 8B = 512
#define PTRS_PER_PAGE   512

// 内核的起始线性地址，该线性地址代表物理地址 0 处
#define PAGE_OFFSET ((unsigned long)0xffff800000000000)

#define PAGE_GDT_SHIFT  39

// 2 ^ 39 =  1G
#define PAGE_1G_SHIFT   30
// 2 ^ 21 = 2M
#define PAGE_2M_SHIFT   21
// 2 ^ 12 = 4K
#define PAGE_4K_SHIFT   12

// 2 MB 页容量
#define PAGE_2M_SIZE    (1UL << PAGE_2M_SHIFT)
#define PAGE_4k_SIZE    (1UL << PAGE_4K_SHIFT)

// 屏蔽码，屏蔽低于 2 MB 的数值
#define PAGE_2M_MASK    (~ (PAGE_2M_SIZE - 1 ))
#define PAGE_4K_MASK    (~ (PAGE_4K_SIZE - 1 ))

// 将参数 addr 按 2 MB 页的上边界对齐
#define PAGE_2M_ALIGN(addr) (((unsigned long) (addr) + PAGE_2M_SIZE - 1) & PAGE_2M_MASK )
#define PAGE_4K_ALIGN(addr) (((unsigned long) (addr) + PAGE_4K_SIZE - 1) & PAGE_4K_MASK )

// 将内核层虚拟地址转换成物理地址
// 该函数使用有限制，只有前 10 MB 的内存空间被映射到线性地址 0xffff80000000000 处
#define Virt_To_Phy(addr)   ((unsigned long) (addr) - PAGE_OFFSET)
#define Phy_To_Virt(addr)   ((unsigned long *)(unsigned long)(addr) + PAGE_OFFSET)

// __attribute__((packed)) 修饰不会生成对齐空间，改用紧凑格式
struct E820 {
    unsigned long address;
    unsigned long length;
    unsigned int type;
}__attribute__((packed));

struct Global_Memory_Descriptor {
    struct E820 e820[32];
    unsigned long e820_length;
};

extern struct Global_Memory_Descriptor memory_management_struct;

void init_memory();

#endif //TOS_MEMORY_H
