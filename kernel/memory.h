//
// Created by tairy on 2020/11/6.
//

#ifndef TOS_MEMORY_H
#define TOS_MEMORY_H

#include "printk.h"
#include "lib.h"

// 20B
struct Memory_E820_Format {
    unsigned int address1;
    unsigned int address2;
    unsigned int length1;
    unsigned int length2;
    unsigned int type;
};

void init_memory();

#endif //TOS_MEMORY_H
