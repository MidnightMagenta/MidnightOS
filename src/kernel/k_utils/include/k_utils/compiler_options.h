#ifndef MDOS_COMPILER_OPTIONS_H
#define MDOS_COMPILER_OPTIONS_H
#ifdef __cplusplus
extern "C" {
#endif

#define MDOS_NORETURN __attribute__((noreturn))
#define MSOD_NOOPTIMIZE __attribute__((optimize("O0")))
#define MDOS_PACKED __attribute__((packed))
#define MDOS_UNUSED __attribute__((unused))
#define MDOS_ALIGNED(alignment) __attribute__((aligned(alignment)))

#ifdef __cplusplus
}
#endif
#endif