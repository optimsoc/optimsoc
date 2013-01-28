/*
Copyright (c) 2010, The Multicore Association
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

(1) Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

(2) Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

(3) Neither the name of the Multicore Association nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

#define MAX_SIGNALS 20

// Stores old signal handlers for propagation purposes.
struct sigaction signals [MAX_SIGNALS];

void block_signals() {
  sigset_t block_alarm;

  /* Initialize the signal mask. */
  sigemptyset (&block_alarm);
  sigaddset (&block_alarm, SIGALRM);
  sigaddset (&block_alarm, SIGINT);
  sigaddset (&block_alarm, SIGHUP);
  sigaddset (&block_alarm, SIGILL);
  sigaddset (&block_alarm, SIGSEGV);
  sigaddset (&block_alarm, SIGTERM);
  sigaddset (&block_alarm, SIGFPE);
  sigaddset (&block_alarm, SIGABRT);

  sigprocmask (SIG_BLOCK, &block_alarm, NULL);

}

void unblock_signals() {
  sigset_t block_alarm;

  /* Initialize the signal mask. */
  sigemptyset (&block_alarm);
  sigaddset (&block_alarm, SIGALRM);
  sigaddset (&block_alarm, SIGINT);
  sigaddset (&block_alarm, SIGHUP);
  sigaddset (&block_alarm, SIGILL);
  sigaddset (&block_alarm, SIGSEGV);
  sigaddset (&block_alarm, SIGTERM);
  sigaddset (&block_alarm, SIGFPE);
  sigaddset (&block_alarm, SIGABRT);

  sigprocmask (SIG_UNBLOCK, &block_alarm, NULL);
}

void signal_handler ( int sig )
{
  struct sigaction new_action, old_action;

  block_signals();

  /* restore the old action */
  new_action = signals[sig];
  sigemptyset (&new_action.sa_mask);
  new_action.sa_flags = 0;

  sigaction (sig, NULL, &old_action);
  if (old_action.sa_handler != SIG_IGN)
    sigaction (sig, &new_action, NULL);

  /* print info on which signal was caught */
  printf("\nFAIL: app received signal, freeing semaphore and shared memory\n");
  switch (sig) {
  case (SIGALRM):{printf( "signal_handler: app caught SIGALRM\n"); }
    break;
  case (SIGINT):{ printf( "signal_handler: app caught SIGINT\n"); }
    break;
  case (SIGHUP):{ printf( "signal_handler: app caught SIGHUP\n"); }
    break;
  case (SIGILL):{ printf( "signal_handler: app caught SIGILL\n"); }
    break;
  case (SIGSEGV):{printf( "signal_handler: app caught SIGSEGV\n"); }
    break;
  case (SIGTERM):{printf( "signal_handler: app caught SIGTERM\n"); }
    break;
  case (SIGFPE):{ printf( "signal_handler: app caught SIGFPE\n"); }
    break;
  case (SIGABRT):{printf( "signal_handler: app aught SIGABRT\n"); }
    break;
  default: { printf( "signal_handler: app caught unknown signal\n"); }
    break;
  };

  unblock_signals();

  printf("PASSED");

  // forward it on to mcapi so that it can clean up
  raise(sig);
}


void register_handlers() {

  /* register signal handlers so that we can still clean up resources
     if an interrupt occurs
     http://www.gnu.org/software/libtool/manual/libc/Sigaction-Function-Example.html
  */
  struct sigaction new_action, old_action;
  printf("app registering signal handlers\n");

  /* Set up the structure to specify the new action. */
  new_action.sa_handler = signal_handler;
  sigemptyset (&new_action.sa_mask);
  new_action.sa_flags = 0;

  sigaction (SIGINT, NULL, &old_action);
  signals[SIGINT] = old_action;
  if (old_action.sa_handler != SIG_IGN)
    sigaction (SIGINT, &new_action, NULL);

  sigaction (SIGHUP, NULL, &old_action);
  signals[SIGHUP] = old_action;
  if (old_action.sa_handler != SIG_IGN)
    sigaction (SIGHUP, &new_action, NULL);

  sigaction (SIGILL, NULL, &old_action);
  signals[SIGILL] = old_action;
  if (old_action.sa_handler != SIG_IGN)
    sigaction (SIGILL, &new_action, NULL);

  sigaction (SIGSEGV, NULL, &old_action);
  signals[SIGSEGV] = old_action;
  if (old_action.sa_handler != SIG_IGN)
    sigaction (SIGSEGV, &new_action, NULL);

  sigaction (SIGTERM, NULL, &old_action);
  signals[SIGTERM] = old_action;
  if (old_action.sa_handler != SIG_IGN)
    sigaction (SIGTERM, &new_action, NULL);

  sigaction (SIGFPE, NULL, &old_action);
  signals[SIGFPE] = old_action;
  if (old_action.sa_handler != SIG_IGN)
    sigaction (SIGFPE, &new_action, NULL);

  sigaction (SIGABRT, NULL, &old_action);
  signals[SIGABRT] = old_action;
  if (old_action.sa_handler != SIG_IGN)
    sigaction (SIGABRT, &new_action, NULL);
}
