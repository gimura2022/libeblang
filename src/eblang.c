#include "eblang.h"

struct gstd__memmanager* eblang__memmanager = NULL;

void eblang__init(struct gstd__memmanager* memmanager)
{
	eblang__memmanager = memmanager;
}
