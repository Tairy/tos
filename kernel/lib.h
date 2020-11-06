//
// Created by tairy on 2020/11/1.
//

#ifndef TOS_LIB_H
#define TOS_LIB_H

#define NULL 0

// 函数 strlen 先将 AL 寄存器赋值为 0, 随后借助 SCASB 汇编指令逐字节扫描 String 字符串，
// 每次扫描都会与 AL 寄存器进行对比，并根据对比结果置位相应标志位，如果扫描的数值与 AL 寄存器
// 的数值相等（同为 0 值），ZF 标志位被置位。

// 代码中重复指令 REPNE 会一直重复执行 SCASB 指令，直至 ECX 寄存器递减为 0 或 ZF 标志位被
// 置位。

// 又因为 ECX 寄存器的初始值是负数（0xffffffff), REPNE 指令执行结束后，ECX 寄存器依然是
// 负值（ECX寄存器在函数执行过程中递减，使用负值可统计出扫描次数），对 ECX 寄存器取反减 1 后可
// 得到字符串长度。
inline static int strlen(char *String) {
    register int __res;
    __asm__ __volatile__ (      "cld \n\t"
                                "repne \n\t"
                                "scasb \n\t"
                                "notl   %0    \n\t"
                                "decl   %0    \n\t"
    :"=c"(__res)
    :"D"(String), "a"(0), "0"(0xffffffff)
    );

    return __res;
}

inline static void *memset(void *Address, unsigned char C, long Count) {
    int d0, d1;
    unsigned long tmp = C * 0x0101010101010101UL;
    __asm__ __volatile__ (
    "cld                \n\t"
    "rep                \n\t"
    "stosq              \n\t"
    "testb  $4, %b3     \n\t"
    "je     1f          \n\t"
    "stosl              \n\t"
    "1:\ttestb  $2, %b3 \n\t"
    "je     2f          \n\t"
    "stosw              \n\t"
    "2:\ttestb  $1, %b3 \n\t"
    "je 3f              \n\t"
    "stosb              \n\t"
    "3:                 \n\t"
    :"=&c"(d0), "=&D"(d1)
    :"a"(tmp), "q"(Count), "0"(Count / 8), "1"(Address)
    :"memory"
    );
    return Address;
}

#endif //TOS_LIB_H
