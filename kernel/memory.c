//
// Created by tairy on 2020/11/6.
//
#include "memory.h"
#include "lib.h"

void init_memory() {
    int i, j;

    // 显示物理内存分布信息
    unsigned long TotalMem = 0;
    struct E820 *p = NULL;

    color_printk(BLUE, BLACK,
                 "Display Address MAP,Type(1:RAM,2:ROM or Reserved,3:ACPI Reclaim Memory,4:ACPI NVS Memory,Others:undefined)\n");
    p = (struct E820 *) 0xffff800000007e00; // BIOS 通过 INT15 获取到的物理内存信息存储位置

    for (i = 0; i < 32; i++) {
        color_printk(ORANGE, BLACK, "Address:%#018lx\tLength:%#018lx\tType:%#010x\n", p->address, p->length, p->type);
        unsigned long tmp = 0;
        if (p->type == 1) {
            TotalMem += p->length;
        }

        memory_management_struct.e820[i].address += p->address; // 这里的 += 与 = 没区别
        memory_management_struct.e820[i].length += p->length;
        memory_management_struct.e820[i].type = p->type;
        memory_management_struct.e820_length = i;

        p++;

        if (p->type > 4) {
            break;
        }
    }

    color_printk(ORANGE, BLACK, "OS Can Used Total RAM:%#018lx\n", TotalMem);

    // 对 e820 结构体数组中的可用物理内存段进行 2 MB 物理内存对齐，并统计出物理页的总量
    TotalMem = 0;
    for (int i = 0; i < memory_management_struct.e820_length; i++) {
        unsigned long start, end;

        // 可用物理内存 type = 1
        if (memory_management_struct.e820[i].type != 1) {
            continue;
        }

        start = PAGE_2M_ALIGN(memory_management_struct.e820[i].address);
        end = ((memory_management_struct.e820[i].address + memory_management_struct.e820[i].length) >> PAGE_2M_SHIFT)
                << PAGE_2M_SHIFT;
        if (end <= start) {
            continue;
        }

        TotalMem += (end - start) >> PAGE_2M_SHIFT;
    }

    color_printk(ORANGE, BLACK, "OS Can Used Total 2M PAGEs:%#010x=%010d\n", TotalMem, TotalMem);
}