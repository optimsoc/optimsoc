/* Copyright (c) 2013 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#ifndef REPORT_H_
#define REPORT_H_

#include <optimsoc.h>

#define OPTIMSOC_NOP_RTS_STATUS              0x200
#define OPTIMSOC_NOP_RTS_CREATETHREAD_FNC    0x201
#define OPTIMSOC_NOP_RTS_CREATETHREAD_TID    0x202
#define OPTIMSOC_NOP_RTS_SCHEDADD_QUEUE      0x203
#define OPTIMSOC_NOP_RTS_SCHEDADD_THREAD     0x204
#define OPTIMSOC_NOP_RTS_THREAD_STARTED      0x205
#define OPTIMSOC_NOP_RTS_THREAD_FINISHED     0x206
#define OPTIMSOC_NOP_RTS_SCHEDULER_SELECT    0x207
#define OPTIMSOC_NOP_RTS_SCHEDULER_YIELD     0x208
#define OPTIMSOC_NOP_RTS_SCHEDULER_SUSPEND   0x209
#define OPTIMSOC_NOP_RTS_SCHEDULER_RESUME    0x20a
#define OPTIMSOC_NOP_RTS_THREAD_JOINWAIT     0x20b
#define OPTIMSOC_NOP_RTS_THREAD_EXIT         0x20c
#define OPTIMSOC_NOP_RTS_THREAD_JOINSIGNAL   0x20d


static inline void runtime_report_schedulertick() {
	OPTIMSOC_REPORT(OPTIMSOC_NOP_RTS_STATUS,1);
}

static inline void runtime_report_createthread_function(unsigned int fnc) {
	OPTIMSOC_REPORT(OPTIMSOC_NOP_RTS_CREATETHREAD_FNC,fnc);
}

static inline void runtime_report_createthread_tid(unsigned int tid) {
	OPTIMSOC_REPORT(OPTIMSOC_NOP_RTS_CREATETHREAD_TID,tid);
}

static inline void runtime_report_scheduleradd_queue(unsigned int queue) {
	OPTIMSOC_REPORT(OPTIMSOC_NOP_RTS_SCHEDADD_QUEUE,queue);
}

static inline void runtime_report_scheduleradd_thread(unsigned int thread) {
	OPTIMSOC_REPORT(OPTIMSOC_NOP_RTS_SCHEDADD_THREAD,thread);
}

static inline void runtime_report_thread_started(unsigned int thread) {
	OPTIMSOC_REPORT(OPTIMSOC_NOP_RTS_THREAD_STARTED,thread);
}

static inline void runtime_report_thread_finished(unsigned int thread) {
	OPTIMSOC_REPORT(OPTIMSOC_NOP_RTS_THREAD_FINISHED,thread);
}

static inline void runtime_report_scheduler_select(unsigned int thread) {
	OPTIMSOC_REPORT(OPTIMSOC_NOP_RTS_SCHEDULER_SELECT,thread);
}

static inline void runtime_report_scheduler_yield() {
	OPTIMSOC_REPORT(OPTIMSOC_NOP_RTS_SCHEDULER_YIELD,0);
}

static inline void runtime_report_scheduler_suspend() {
	OPTIMSOC_REPORT(OPTIMSOC_NOP_RTS_SCHEDULER_SUSPEND,0);
}

static inline void runtime_report_scheduler_resume(unsigned int thread) {
	OPTIMSOC_REPORT(OPTIMSOC_NOP_RTS_SCHEDULER_RESUME,thread);
}

static inline void runtime_report_thread_joinwait(unsigned int thread) {
	OPTIMSOC_REPORT(OPTIMSOC_NOP_RTS_THREAD_JOINWAIT,thread);
}

static inline void runtime_report_thread_exit(unsigned int thread) {
	OPTIMSOC_REPORT(OPTIMSOC_NOP_RTS_THREAD_EXIT,thread);
}

static inline void runtime_report_thread_joinsignal(unsigned int thread) {
	OPTIMSOC_REPORT(OPTIMSOC_NOP_RTS_THREAD_JOINSIGNAL,thread);
}

#endif /* REPORT_H_ */
