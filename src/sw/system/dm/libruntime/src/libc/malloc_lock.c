
volatile unsigned int malloc_lock_flag = 0;

void __malloc_lock() {
	disable_timer();
/*	asm ("l.nop 0x665");
	while (malloc_lock_flag!=0) {
		asm ("l.nop 0x666");
		thread_yield();
	}
	malloc_lock_flag = 1;
	asm ("l.nop 0x667");*/
}

void __malloc_unlock() {
	enable_timer();
/*	asm ("l.nop 0x668");
	malloc_lock_flag = 0;*/
}
