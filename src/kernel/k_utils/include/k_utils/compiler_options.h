#ifndef MDOS_COMPILER_OPTIONS_H
#define MDOS_COMPILER_OPTIONS_H
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __clang__
#define MDOS_NORETURN __attribute__((noreturn))
#define MDOS_NOOPTIMIZE __attribute__((optimize("O0")))
#define MDOS_PACKED __attribute__((packed))
#define MDOS_UNUSED __attribute__((unused))
#define MDOS_ALIGNED(alignment) __attribute__((aligned(alignment)))
#else
#define MDOS_NORETURN
#define MDOS_NOOPTIMIZE
#define MDOS_PACKED
#define MDOS_UNUSED
#define MDOS_ALIGNED(alignment)
#endif

#ifdef __cplusplus
}
#endif
#endif