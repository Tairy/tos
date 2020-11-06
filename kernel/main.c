//
// Created by tairy on 2020/10/31.
//

#include "lib.h"
#include "printk.h"
#include "gate.h"
#include "trap.h"
#include "memory.h"

// 这些标识符会被放在 kernel.lds 链接脚本制定的地址处
extern char _text;
extern char _etext;
extern char _edata;
extern char _end;

struct Global_Memory_Descriptor memory_management_struct = {{0}, 0};

void Start_Kernel(void) {
    int *addr = (int *) 0xffff800000a00000; // 帧缓冲区被映射的线性地址
    int i;

    Pos.XResolution = 1440;
    Pos.YResolution = 900;

    Pos.XPosition = 0;
    Pos.YPosition = 0;

    Pos.XCharSize = 8;
    Pos.YCharSize = 16;

    Pos.FB_addr = (unsigned int *) 0xffff800000a00000;
    Pos.FB_length = (Pos.XResolution * Pos.YResolution * 4);

    load_TR(8);

    set_tss64(0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00,
              0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00);

    sys_vector_init();

    memory_management_struct.start_code = (unsigned long) &_text;
    memory_management_struct.end_code = (unsigned long) &_etext;
    memory_management_struct.end_data = (unsigned long) &_edata;
    memory_management_struct.end_brk = (unsigned long) &_end;

    color_printk(RED, BLACK, "memory init \n");
    init_memory();

    while (1);
}

