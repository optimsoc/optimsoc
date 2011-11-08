/*
 * services.c
 *
 *  Created on: Aug 11, 2011
 *      Author: wallento
 */

#define SYSCALL_SVC_REQ 8

unsigned int svc_identify(char* id) {
	asm volatile("l.addi r5,r3,0"); // move id here
	asm volatile("l.addi r3,r0,0"); // kernel service
	asm volatile("l.addi r4,r0,0"); // find request
	asm volatile("l.addi r11,r0,%0" : : "K" (SYSCALL_SVC_REQ) );
	asm volatile("l.sys 1");
}

unsigned int svc_req0(unsigned int id,unsigned int req) {
	asm volatile("l.addi r11,r0,%0" : : "K" (SYSCALL_SVC_REQ) );
	asm volatile("l.sys 1");
}

unsigned int svc_req1(unsigned int id,unsigned int req,void* arg1) {
	asm volatile("l.addi r11,r0,%0" : : "K" (SYSCALL_SVC_REQ) );
	asm volatile("l.sys 1");
}

unsigned int svc_req2(unsigned int id,unsigned int req,void* arg1,void* arg2) {
	asm volatile("l.addi r11,r0,%0" : : "K" (SYSCALL_SVC_REQ) );
	asm volatile("l.sys 1");
}

unsigned int svc_req3(unsigned int id,unsigned int req,void* arg1,void* arg2,void* arg3) {
	asm volatile("l.addi r11,r0,%0" : : "K" (SYSCALL_SVC_REQ) );
	asm volatile("l.sys 1");
}

unsigned int svc_req4(unsigned int id,unsigned int req,void* arg1,void* arg2,void* arg3,void* arg4) {
	asm volatile("l.addi r11,r0,%0" : : "K" (SYSCALL_SVC_REQ) );
	asm volatile("l.sys 1");
}
