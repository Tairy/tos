//
// Created by tairy on 2020/11/5.
//

#ifndef TOS_LINKPAGE_H
#define TOS_LINKPAGE_H

#define L1_CACHE_BYTES 32

#define asmlinkpage __attribute__((regparm(0)))

#define ____cacheline_aligned __attribute__((__aligned__(L1_CACHE_BYTES)))

#define SYMBOL_NAME(X) X

#define SYMBOL_NAME_STR(X)  #X

#define SYMBOL_NAME_LABEL(X)    X##:

#define ENTRY(name) \
.global SYMBOL_NAME(name); \
SYMBOL_NAME_LABEL(name)

#endif //TOS_LINKPAGE_H
