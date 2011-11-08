#include "vmm.h"

#include "arch.h"
#include ARCH_INCL(printf.h)
#include ARCH_INCL(utils.h) // TODO: remove later
#include "list.h"
#include "scheduler.h"

#include ARCH_INCL(mm.h)
#include "malloc.h"

list_t page_pool = { NULL, NULL };

#define DEBUG_PRINT(...) if ( DEBUG_VMM ) printf(__VA_ARGS__)

void vmm_initialize() {
	void *start_page = (void*) (PHYSMEM_START & ~PAGEMASK);
	void *end_page   = (void*) (PHYSMEM_END   & ~PAGEMASK);
	DEBUG_PRINT("page pool from page %p to %p\n",start_page,end_page);

	for ( void *p = start_page; p <= end_page; p += PAGESIZE ) {
		list_elem_t *page = malloc(sizeof(list_elem_t));
		page->data = (void*) (p);
		list_append_node(&page_pool,page);
	}
}

void vmm_reserve_page(void *page) {
	list_elem_t *p = list_find(&page_pool,page);
	if (!p) {
		printf("The page %p cannot be reserved, because it is not in pool\n",page);
	} else {
		list_remove_node(&page_pool,p);
	}
}

void *vmm_alloc_page() {
	void *page = NULL;
	list_elem_t *p = page_pool.head;
	if (p) {
		page = p->data;
		list_remove_node(&page_pool,p);
		free(p);
	}
	return page;
}

void vmm_free_page(void* page) {
	list_elem_t *p = malloc(sizeof(list_elem_t));
	p->data = page;
	list_append_node(&page_pool,p);
}

size_t vmm_available_pages() {
	return list_length(&page_pool);
}

page_table_t pagedirectory_findtable(page_directory_t* dir, void *vaddr) {
	page_table_t t = NULL;
	page_directory_entry_t *d = (page_directory_entry_t*) list_find((list_t*)dir,(void*)vaddr);
	if ( d ) {
		t = d->table;
	}
	return t;
}

page_table_t pagetable_create(page_directory_t *dir,void *tablebase) {
	page_table_t table = calloc(PAGETABLE_ENTRIES,sizeof(page_table_entry_t));
	page_directory_entry_t *entry = malloc(sizeof(page_directory_entry_t));
	entry->table = table;
	entry->vaddr_base = tablebase;
	list_append_node((list_t*)dir,(list_elem_t*)entry);
	return table;
}

void pagetable_addmapping(page_directory_t *dir,void *vaddr,void *paddr) {
	void *tablebase = (void*)((unsigned int) vaddr & ~PAGETABLE_MASK);
	page_table_t table = pagedirectory_findtable(dir,tablebase);
	if ( !table ) {
		table = pagetable_create(dir,tablebase);
	}
	unsigned int index = ADDR_TO_TABLEINDEX(vaddr);
	page_table_entry_t entry = ((unsigned int) paddr & PAGETABLEENTRY_VPN) | PAGETABLEENTRY_VALID;
	table[index] = entry;
}

page_directory_t *pagedirectory_create() {
	page_directory_t *dir = malloc(sizeof(page_directory_t));
	dir->head = NULL;
	dir->tail = NULL;
	return dir;
}

int pagetable_virt2phys(page_directory_t *dir,void *vaddr,void **paddr) {
	void *tablebase = (void*)((unsigned int) vaddr & ~PAGETABLE_MASK);
	page_table_t table = pagedirectory_findtable(dir,tablebase);
	if (!table) {
		return -1;
	} else {
		page_table_entry_t entry = table[ADDR_TO_TABLEINDEX(vaddr)];
		if ( entry & PAGETABLEENTRY_VALID ) {
			*paddr = (void*) ((entry & PAGETABLEENTRY_VPN) | ((unsigned int) vaddr & PAGEMASK));
			return 0;
		} else {
			return -1;
		}
	}
}

void itlb_miss_handler(void *vaddr,unsigned int pc) {
	DEBUG_PRINT("[%u] ITLB miss: %010p pc: 0x%08x\n",coreid(),vaddr,pc);
	thread_t  *thread   = active_thread[coreid()];
	process_t *process  = thread->process;
	void *paddr;
	if (pagetable_virt2phys(process->page_dir, vaddr, &paddr) < 0) {
		printf("Error: page fault in process %u: access to %08p at pc %08p\n",active_thread[coreid()]->id,vaddr,pc);
		or32exit(1);
	} else {
		DEBUG_PRINT("  Add itlb %p->%p\n",PAGE_BASE(vaddr),PAGE_BASE(paddr));
		arch_set_itlb(PAGE_BASE(vaddr),PAGE_BASE(paddr));
	}
}

void dtlb_miss_handler(void *vaddr,unsigned int pc) {
	DEBUG_PRINT("[%u] DTLB miss: %010p pc: 0x%08x\n",coreid(),vaddr,pc);
	thread_t  *thread   = active_thread[coreid()];
	process_t *process  = thread->process;
	void *paddr;
	if ( pagetable_virt2phys(process->page_dir, vaddr, &paddr) < 0 ) {
		printf("Error: page fault in process %u: access to %08p at pc %08p\n",active_thread[coreid()]->id,vaddr,pc);
		or32exit(1);
	} else {
		DEBUG_PRINT("  Add dtlb %p->%p\n",PAGE_BASE(vaddr),PAGE_BASE(paddr));
		arch_set_dtlb(PAGE_BASE(vaddr),PAGE_BASE(paddr));
	}
}

void vmm_page_alloc_handler(void *vbase,unsigned int n) {
	for ( unsigned int i = 0; i < n; ++i) {
		void *vaddr = vbase + 0x2000*i;
		void *page = vmm_alloc_page();
		pagetable_addmapping(active_thread[coreid()]->process->page_dir,vaddr,page);
	}
}
