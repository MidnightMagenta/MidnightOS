#ifndef _IDTENTRY_H
#define _IDTENTRY_H

#define DEFINE_IDTENTRY(func) __visible void do_##func(struct int_info *info)

#endif