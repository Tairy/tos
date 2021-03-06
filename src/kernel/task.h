//
// Created by tairy on 2020/11/8.
//

#ifndef TOS_TASK_H
#define TOS_TASK_H

#include "memory.h"
#include "cpu.h"
#include "lib.h"
#include "ptrace.h"

#define KERNEL_CS   (0x00)
#define KERNEL_DS   (0x10)
#define USER_CS     (0x28)
#define USER_DS     (0x30)

#define CLONE_FS        (1<<0)
#define CLONE_FILES     (1<<1)
#define CLONE_SIGNAL    (1<<2)

#define STACK_SIZE 32768

extern char _text;
extern char _etext;
extern char _data;
extern char _edata;
extern char _rodata;
extern char _erodata;
extern char _bss;
extern char _ebss;
extern char _end;

extern unsigned long _stack_start;

extern void ret_from_intr();

#define TASK_RUNNING            (1<<0)
#define TASK_INTERRUPTIBLE      (1<<1)
#define TASK_UNINTERRUPTIBLE    (1<<2)
#define TASK_ZOMBIE             (1<<3)
#define TASK_STOPPED            (1<<4)

struct mm_struct {
    // 页目录基地址
    pml4t_t *pgd;    // page table point

    unsigned long start_code, end_code;         // 代码段
    unsigned long start_data, end_data;         // 数据段
    unsigned long start_ro_data, end_ro_data;   // 只读数据段
    unsigned long start_brk, end_brk;           // 动态内存分配区域
    unsigned long start_stack;                  // 应用栈顶地址
};


// 该结构体在进程发生切换时，保存执行现场的寄存器值
struct thread_struct {
    unsigned long rsp0;         // 内核层栈基地址

    unsigned long rip;          // 内核层代码指针
    unsigned long rsp;          // 内核层当前栈指针

    unsigned long fs;           // FS 段寄存器
    unsigned long gs;           // GS 段寄存器

    unsigned long cr2;          // CR2 控制寄存器
    unsigned long trap_nr;      // 产生异常的异常号
    unsigned long error_code;   // 异常的错误码
};


#define PF_KTHREAD  (1<<0)

struct task_struct {
    struct List list;       // 双向链表，用于连接各个进程控制结构体
    volatile long state;    // 进程状态：运行态、停止态、可终端态等， volatile 表示变量可以被某些编译器未知因素更改
    unsigned long flags;    // 进程标志: 进程、线程、内核线程

    // 这两个成员变量负载在进程调度过程中保存或还原 CR3 控制寄存器和通用寄存器的值
    struct mm_struct *mm;   // 内存空间分布结构体，记录内存页表和程序段信息
    struct thread_struct *thread;   // 进程切换时保留的状态信息

    unsigned long addr_limit;       /*进程地址空间范围： 0x0000,0000,0000,0000 - 0x0000,7fff,ffff,ffff user 0xffff,8000,0000,0000 - 0xffff,ffff,ffff,ffff kernel*/

    long pid;   // 进程 ID

    long counter;   // 进程可用时间片

    long signal;    // 进程持有的信号

    long priority;  // 进程优先级
};

union task_union {
    struct task_struct task;    // 进程控制结构体
    unsigned long stack[STACK_SIZE / sizeof(unsigned long)]; // 进程的内核层栈空间
}__attribute__((aligned(8)));


struct mm_struct init_mm;
struct thread_struct init_thread;

#define INIT_TASK(tsk)                                  \
{                                                       \
    .state = TASK_UNINTERRUPTIBLE,                      \
    .flags = PF_KTHREAD,                                \
    .mm = &init_mm,                                     \
    .thread = &init_thread,                             \
    .addr_limit = 0xffff80000000000,                    \
    .pid = 0,                                           \
    .counter = 1,                                       \
    .signal = 0,                                        \
    .priority = 0                                       \
}

// __attribute__((__section__(".data.init_task"))) 修饰，将该全局变量链接到一个特别的程序段内
// 在链接脚本 kernel.lds 中已经为这个特别的程序段规划了地址空间
union task_union init_task_union __attribute__((__section__(".data.init_task"))) = {INIT_TASK(init_task_union.task)};
struct task_struct *init_task[NR_CPUS] = {&init_task_union.task, 0};
struct mm_struct init_mm = {0};
struct thread_struct init_thread = {
        .rsp0 = (unsigned long) (init_task_union.stack + STACK_SIZE / sizeof(unsigned long)),
        .rsp = (unsigned long) (init_task_union.stack + STACK_SIZE / sizeof(unsigned long)),
        .fs = KERNEL_DS,
        .gs = KERNEL_DS,
        .cr2 = 0,
        .trap_nr = 0,
        .error_code = 0
};

struct tss_struct {
    unsigned int reserved0;
    unsigned long rsp0;
    unsigned long rsp1;
    unsigned long rsp2;
    unsigned long reserved1;
    unsigned long ist1;
    unsigned long ist2;
    unsigned long ist3;
    unsigned long ist4;
    unsigned long ist5;
    unsigned long ist6;
    unsigned long ist7;
    unsigned long reserved2;
    unsigned short reserved3;
    unsigned short iomapbaseaddr;
}__attribute__((packed));

#define INIT_TSS                                                                                    \
{       .reserved0 = 0,                                                                             \
        .rsp0 = (unsigned long)(init_task_union.stack + STACK_SIZE / sizeof(unsigned long)),        \
        .rsp1 = (unsigned long)(init_task_union.stack + STACK_SIZE / sizeof(unsigned long)),        \
        .rsp2 = (unsigned long)(init_task_union.stack + STACK_SIZE / sizeof(unsigned long)),        \
        .reserved1 = 0,                                                                             \
        .ist1 = 0xffff800000007c00,                                                                 \
        .ist2 = 0xffff800000007c00,                                                                 \
        .ist3 = 0xffff800000007c00,                                                                 \
        .ist4 = 0xffff800000007c00,                                                                 \
        .ist5 = 0xffff800000007c00,                                                                 \
        .ist6 = 0xffff800000007c00,                                                                 \
        .ist7 = 0xffff800000007c00,                                                                 \
        .reserved2 = 0,                                                                             \
        .reserved3 = 0,                                                                             \
        .iomapbaseaddr = 0                                                                          \
}

struct tss_struct init_tss[NR_CPUS] = {[0 ... NR_CPUS - 1] = INIT_TSS};

inline static struct task_struct *get_current() {
    struct task_struct *current = NULL;
    __asm__ __volatile__ ("andq %%rsp,%0    \n\t":"=r"(current):"0"(~32767UL));
    return current;
}

#define current get_current()

#define GET_CURRENT                 \
    "movq %rsp,     %rbx    \n\t"   \
    "andq $-32768,  %rbx    \n\t"

#define switch_to(prev, next)                                                                   \
do {                                                                                            \
    __asm__ __volatile__ (  "pushq  %%rbp               \n\t"                                   \
                            "pushq  %%rax               \n\t"                                   \
                            "movq   %%rsp,      %0      \n\t"                                   \
                            "movq   %2,         %%rsp   \n\t"                                   \
                            "leaq   1f(%%rip),  %%rax   \n\t"                                   \
                            "movq   %%rax,      %1      \n\t"                                   \
                            "pushq  %3                  \n\t"                                   \
                            "jmp    __switch_to         \n\t"                                   \
                            "1:                         \n\t"                                   \
                            "popq   %%rax               \n\t"                                   \
                            "popq   %%rbp               \n\t"                                   \
                            :"=m"(prev->thread->rsp),"=m"(prev->thread->rip)                    \
                            :"m"(next->thread->rsp),"m"(next->thread->rip),"D"(prev),"S"(next)  \
                            :"memory"                                                           \
    );                                                                                          \
} while(0)

unsigned long
do_fork(struct pt_regs *regs, unsigned long clone_flags, unsigned long stack_start, unsigned long stack_size);

void task_init();

#define MAX_SYSTEM_CALL_NR 128

typedef unsigned long (*system_call_t)(struct pt_regs *regs);

unsigned long no_system_call(struct pt_regs *regs) {
    color_printk(RED, BLACK, "no_system_call is calling, NR:%#04x\n", regs->rax);
    return -1;
}

unsigned long sys_printf(struct pt_regs *regs) {
    color_printk(BLACK, WHITE, (char *) regs->rdi);
    return 1;
}

system_call_t system_call_table[MAX_SYSTEM_CALL_NR] = {
        [0] = no_system_call,
        [1] = sys_printf,
        [2 ... MAX_SYSTEM_CALL_NR - 1] = no_system_call
};

#endif //TOS_TASK_H
