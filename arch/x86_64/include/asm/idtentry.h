#ifndef _NYX_IDTENTRY_H
#define _NYX_IDTENTRY_H

#define DEFINE_IDTENTRY(func) __visible void do_##func(struct int_info *info)

#endif