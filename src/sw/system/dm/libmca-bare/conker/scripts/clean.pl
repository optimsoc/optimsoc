#!/usr/bin/env perl
# -- -*-perl-*-a
#
#
# Copyright (C) 2006 by Freescale Semiconductor Inc.  All rights reserved.
#
# You may distribute under the terms of the Artistic License, as specified in
# the COPYING file.
#

use strict;


sub run_cmd {
  my $cmd = shift;
  print "executing [$cmd]\n";
  my $output = `$cmd 2>&1`;
  my $error_code = $?;
  if ($error_code) {
    print "$cmd failed with output:\n$output\n";
  }
  return $error_code;
}

        #clean up any unfreed resources
        my $prog = "tc_fury";
        my @lines = `ps -ef | grep $prog | grep -v grep`;
        foreach (@lines) {
          if (/b29511\s+(\S+)\s+/) { 
            run_cmd("kill -9 $1");
         }
        }
        @lines = `ipcs`;
        foreach (@lines) {
                #0x752efd72 896172045  r3aajx    666
                if (/(\S+)\s+(\S+)\s+(\S+)\s+666/) {
                        run_cmd("ipcrm -m $2");
                        run_cmd("ipcrm -s $2");
                        run_cmd("ipcclean");
            }
        }

