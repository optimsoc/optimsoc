#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

extern void _exceptions_add_handler(unsigned long vector, void (*handler) (void));

#endif /* EXCEPTIONS_H_ */
