#ifndef _NYX_COMPILER_H
#define _NYX_COMPILER_H

#define __packed __attribute__((packed))
#define __align(a) __attribute__((aligned(a)))
#define __section(sect) __attribute__((section(sect)))

#define __noreturn __attribute__((noreturn))

#endif