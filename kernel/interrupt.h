//
// Created by tairy on 2020/11/7.
//

#ifndef TOS_INTERRUPT_H
#define TOS_INTERRUPT_H

#include "linkage.h"

void init_interrupt();

void do_IRQ(unsigned long regs, unsigned long nr);

#endif //TOS_INTERRUPT_H
