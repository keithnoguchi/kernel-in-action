swapper_pg_dir in x86-64
========================
This document explores how swapper_pg_dir usage and the purpose in x86-64
architecture, referenced by the Linux kernel version 4.17.1 source code.

Definition
----------
inside arch/x86/include/asm/pgtable_64.h, swapper_pg_dir is defined
to point to the init_top_pdt[] array.

extern pgd_t init_top_pgt[];

#define swapper_pg_dir init_top_pgt

pgd_t type
----------
pgd_t is typedefed as below in arch/x86/include/asm/pgtable_types.h

typedef struct { pgdval_t pgd; } pgd_t;

and pgdval_t is unsigned long, just like other page related types,
as in arch/x86/include/asm/pgtable_64_types.h

typedef unsigned long	pteval_t;
typedef unsigned long	pmdval_t;
typedef unsigned long	pudval_t;
typedef unsigned long	p4dval_t;
typedef unsigned long	pgdval_t;

By the way, PGD stands for Page Global Directory, as explained in following
article:

https://www.kernel.org/doc/gorman/html/understand/understand006.html

This LWN article explains the four level page table:

https://lwn.net/Articles/117749/

Initialization
--------------
init_top_pgt array is initialized in x86_64_start_kernel() function,
the very early stage of the initialization, as in arch/x86/kernel/head64.c:

	/* Kill off the identify-map trampoline */
	reset_early_page_tables();

	clear_bss();

	clear_page(init_top_pgt);

Also, note that there are similar initialization call,
reset_early_page_tables(), which initializes the early_top_pgt
except the kernel symbol map, as below:

/* Wipe all early page tables except for the kernel symbol map */
static void __init reset_early_page_tables(void)
{
        memset(early_top_pgt, 0, sizeof(pgd_t)*(PTRS_PER_PGD-1));
        next_early_pgt = 0;
        write_cr3(__sme_pa_nodebug(early_top_pgt));
}
