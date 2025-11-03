#ifndef _MDOS_STDDEF_H
#define _MDOS_STDDEF_H

#undef NULL
#define NULL ((void *) 0)

typedef __SIZE_TYPE__ size_t;

#ifndef SIZE_MAX
#define SIZE_MAX __SIZE_MAX__
#endif

#endif