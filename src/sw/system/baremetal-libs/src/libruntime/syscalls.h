#ifndef SYSCALLS_H_
#define SYSCALLS_H_

/**
 * Initialize the syscall handling
 *
 * This function is called during initialization and properly configures the
 * syscall handling.
 */
void _optimsoc_syscalls_init(void);

/**
 * This is the syscall entry
 *
 * The function is called when a syscall exception occurs and it calls the
 * registered handler.
 */
void _optimsoc_sycall_entry(void);

#endif /* SYSCALLS_H_ */
