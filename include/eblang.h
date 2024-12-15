#ifndef _eblang_h
#define _eblang_h

#include <gstd/dunarr.h>
#include <gstd/allocators.h>

extern struct gstd__memmanager* eblang__memmanager;

void eblang__init(struct gstd__memmanager* memmanager);

#endif
