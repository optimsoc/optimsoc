#!/usr/bin/env perl
# -- -*-perl-*-a
#
#
# Copyright (C) 2006 by Freescale Semiconductor Inc.  All rights reserved.
#
# You may distribute under the terms of the Artistic License, as specified in
# the COPYING file.
#

	
use Getopt::Long;
use Pod::Usage;
use strict;
use File::Basename;

my $NUM_TRIALS=3;
my $MIN_SENDS = 100;
my $MAX_SENDS=1000;
my $SEND_INCR=100;
my $DSIZE=100;
my $FILENAME="fury_experiment.log";


my $a;
my $BASE_DSIZE;
my $help;


my %tests;
if (!&GetOptions
		(
		 "h|help" => \$help,	
		 "num_trials:i"    =>\$NUM_TRIALS,	
		 "min_sends:i"    =>\$MIN_SENDS,	
		 "max_sends:i"    =>\$MAX_SENDS,
		 "send_incr:i"    =>\$SEND_INCR,
		 "dsize:i"    =>\$DSIZE,
		 "filename|f:s"    => \$FILENAME,
		 "a"      => \$a,	
		)) {
  usage();
}
if ($help) { usage(); }


if ($a) {
	$a="-a";
}

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

sub usage {
	print "usage: fury_experiment.sh -t <num_trials> -s <num_sends> -d <min_data_size> -m <max_data_size> -i <data_size_incr> -n <min_nodes> -x <max_nodes> -N <node_incr> -f <filename> -a \n";
	print "defaults:\n";
	print "num_trials =  $NUM_TRIALS\n";
	print "min_sends =  $MIN_SENDS\n";
	print "max_sends =  $MAX_SENDS\n";
	print "send_incr =  $SEND_INCR\n";
	print "dsize: $DSIZE\n";
	print "filename =  $FILENAME\n";
	exit (1);
}

# first loop over number of sends
# then loop over number of trials
my ($i,$j);

sub mcapi {
	my ($test,$type,$blocking,$i_start,$j_start,$run)=@_;
	for ($i=$i_start; $i<=$MAX_SENDS; $i+=$SEND_INCR){
		for ($j=$j_start; $j<$NUM_TRIALS; $j+=1){
			if ($run) {
				# actually run it
				if (!($tests{$test}{$type}{$blocking}{$i}{$DSIZE}{$j} eq "DONE")) {
             	                     run_cmd("$test -$type $blocking -b $DSIZE -n $i -f $FILENAME");
				} else { print "skipping $test $type $blocking $i $DSIZE $j\n";}
			} else {
				# just record the test type
				$tests{$test}{$type}{$blocking}{$i}{$DSIZE}{$j} = "NOT_RUN";
			} 
		}
	}
}


use Data::Dumper;

# error checking
my $MAX_PKT_SIZE = `grep MAX_PKT ../../mca_config.h`;
if ($MAX_PKT_SIZE =~ /\S+\s+\S+\s+(\d+)/) {
	if ($DSIZE > eval($1)) {
		die "FAIL: mcapi configured w/ MAX_PKT_SIZE ($1) < $DSIZE!\n";
	}
}
my $MAX_MSG_SIZE = `grep MAX_MSG ../../mca_config.h`;
print "$MAX_MSG_SIZE\n";
if ($MAX_MSG_SIZE =~ /\S+\s+\S+\s+(\d+)/) {
	if ($DSIZE > eval($1)) {
		die "FAIL: mcapi configured w/ MAX_MSG_SIZE ($1) < $DSIZE!\n";
	}
}


if (-e $FILENAME) {
	# logile exists, assume run is being re-started
	my ($TH,$OpType,$NumHops,$OpSize,$OpCount,$blocking);
	my @optypes = ("p","m");
	my @testharnesses = ("../src/tc_fury_pthread","../src/tc_fury_fork");
	my @blocking = ("","-a");
	
	# first populate the test structure will all tests to be run
	foreach $TH (@testharnesses) {
		foreach $OpType (@optypes) {
			foreach my $b (@blocking) {
				mcapi($TH,$OpType,$b,$MIN_SENDS,0,0);
			}
		}
	}
	
	# now scan the file to see what has already run
	open FILE,"<$FILENAME" or die "Could not open $FILENAME for reading.";
	while (<FILE>) {

		#pthread message 3       1       10000   0.002694        26.938124
		if (/(\S+)\s+(\S+)\s+(\d+)\s+(\S+)\s+(\S+)/) {
			$TH = $1;
			$OpType = $2;
			$NumHops=$3;
			$OpSize=$4;
			$OpCount=$5;
			
			$TH = "../src/tc_fury_".$TH;
			$b = "";
			if ($OpType =~ /async/) {$b = "-a";}
			if (($OpType =~ /msg/) or ($OpType =~ /message/)) { $OpType = "m"; }
			if (($OpType =~ /pkt/) or ($OpType =~ /packet/)) { $OpType = "p"; }
			for ($j = 0; $ j < $NUM_TRIALS; $j++) {
				if ($tests{$TH}{$OpType}{$b}{$OpCount}{$OpSize}{$j} eq "NOT_RUN") {
					$tests{$TH}{$OpType}{$b}{$OpCount}{$OpSize}{$j} = "DONE";
					last;
				}
			}
		}
	}
	close FILE;
	#print "dumper: ",Dumper(\%tests); die;

	#clean up any unfreed resources
	my @lines = `ipcs`; 
	my $prog = "tc_fury";
	my $proc = `ps -ef | grep $prog | grep -v grep`;
	if ($proc =~ /\s+\S+\s+(\S+)\s+/) { run_cmd("kill -9 $1");}
	foreach (@lines) {
		#0x752efd72 896172045  r3aajx    666
		if (/(\S+)\s+(\S+)\s+r3aajx\s+666/) { 
			run_cmd("ipcrm -m $1"); 
			run_cmd("ipcrm -s $1"); 
			run_cmd("ipcclean");
		}
	}
} else {
	# brand new run (no existing log file)
	# print the header
	open FILE,">$FILENAME" or die "Could not open $FILENAME for writing.";
	print FILE `date`;
	print FILE "num_trials =  $NUM_TRIALS\n";
	print FILE "min_sends =  $MIN_SENDS\n";
	print FILE "max_sends =  $MAX_SENDS\n";
	print FILE "send_incr =  $SEND_INCR\n";
	print FILE "dsize: $DSIZE\n";
	print FILE "filename =  $FILENAME\n";
	print FILE "\nTH      OpType  NumHops OpSize  OpCount TotTim  Retries\n";	

	print FILE "\n";
	close FILE;
	
}


#mcapi
my @optypes = ("p","m");
my @testharnesses = ("../src/tc_fury_pthread","../src/tc_fury_fork");
my @blocking = ("","-a");
foreach my $test (@testharnesses) {
	foreach my $type (@optypes) {
		foreach my $b (@blocking) {
			mcapi($test,$type,$b,$MIN_SENDS,0,1);
		}
	}
}




