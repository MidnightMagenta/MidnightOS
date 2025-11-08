#ifndef _NYX_IDTENTRY_H
#define _NYX_IDTENTRY_H

#define DECLARE_IDTENTRY(func)                                                                                         \
    extern void func(struct int_info *info);                                                                           \
    extern void exc_##func(struct int_info *info);

#define DEFINE_IDTENTRY(func)                                                                                          \
    void exc_##func(struct int_info *info);                                                                            \
    __visible void do_##func(struct int_info *info) { exc_##func(info); }                                                        \
    void exc_##func(struct int_info *info)

#endif