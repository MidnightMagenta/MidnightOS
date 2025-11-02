#ifndef _MSOD_TYPES_H
#define _MSOD_TYPES_H

typedef unsigned char __u8;
typedef unsigned short __u16;
typedef unsigned int __u32;
typedef unsigned long long __u64;

typedef char __s8;
typedef short __s16;
typedef int __s32;
typedef long long __s64;

typedef unsigned char __attribute__((aligned(1))) __aligned_u8;
typedef unsigned short __attribute__((aligned(2))) __aligned_u16;
typedef unsigned int __attribute__((aligned(4))) __aligned_u32;
typedef unsigned long long __attribute__((aligned(8))) __aligned_u64;

typedef char __attribute__((aligned(1))) __aligned_s8;
typedef short __attribute__((aligned(2))) __aligned_s16;
typedef int __attribute__((aligned(4))) __aligned_s32;
typedef long long __attribute__((aligned(8))) __aligned_s64;

#endif