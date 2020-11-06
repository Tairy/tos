//
// Created by tairy on 2020/11/6.
//
#include "memory.h"
#include "lib.h"

void init_memory() {
    int i, j;
    unsigned long TotalMem = 0;
    struct Memory_E820_Formate *p = NULL;

    color_printk(BLUE, BLACK,
                 "Display Address MAP,Type(1:RAM,2:ROM or Reserved,3:ACPI Reclaim Memory,4:ACPI NVS Memory,Others:undefined)\n");
    p = (struct Memory_E820_Formate *) 0xffff800000007e00; // BIOS 通过 INT15 获取到的物理内存信息存储位置

    for (i = 0; i < 32; i++) {
        color_printk(ORIGIN, BLACK, "Address:%#010x,%08x\tLength:%#010x,%08x\tType:%#010x\n", p->address2, p->address1,
                     p->length2, p->address1, p->type);
        unsigned long tmp = 0;
        if (p->type == 1) {
            tmp = p->length2;
            TotalMem += p->length1;
            TotalMem += tmp << 32;
        }

        p++;

        if (p->type > 4) {
            break;
        }
    }

    color_printk(ORIGIN, BLACK, "OS Can Used Total RAM:%#018lx\n", TotalMem);


}