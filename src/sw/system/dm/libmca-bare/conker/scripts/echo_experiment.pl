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
my $NUM_SENDS=10000;
my $MIN_DSIZE=1;
my $MAX_DSIZE=60;
my $DSIZE_INCR=10;
my $MIN_NODES=4;
my $MAX_NODES=4;
my $NODE_INCR=1;
my $FILENAME="echo_experiment.log";
my $a;
my $BASE_DSIZE;
my $BYTES;
my $help;


my %tests;
if (!&GetOptions
		(
		 "h|help" => \$help,	
		 "t:i"    =>\$NUM_TRIALS,	
		 "s:i"    =>\$NUM_SENDS,	
		 "d:i"    =>\$MIN_DSIZE,
		 "m:i"    =>\$MAX_DSIZE,
		 "i:i"    =>\$DSIZE_INCR,
		 "min:i"    =>\$MIN_NODES,
		 "x:i"    =>\$MAX_NODES,
		 "N:i"    => \$NODE_INCR,
		 "f:s"    => \$FILENAME,
		 "a"      => \$a,	
		)) {
  usage();
}
if ($help) { usage(); }

if ($a) {
	$a="-a";
}

my $num_tests;

sub run_cmd {
  $num_tests++;
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
	print "usage: echo_experiment.sh -t <num_trials> -s <num_sends> -d <min_data_size> -m <max_data_size> -i <data_size_incr> -min <min_nodes> -x <max_nodes> -N <node_incr> -f <filename> -a \n";
	print "defaults:\n";
	print "num_trials =  $NUM_TRIALS\n";
	print "num_sends =  $NUM_SENDS\n";
	print "min_data_size =  $MIN_DSIZE\n";
	print "max_data_size =  $MAX_DSIZE\n";
	print "data_size_incr =  $DSIZE_INCR\n";
	print "min_nodes =  $MIN_NODES\n";
	print "max_nodes =  $MAX_NODES\n";
	print "node_incr =  $NODE_INCR\n";
	print "filename =  $FILENAME\n";
	exit (1);
}

$BASE_DSIZE=$MIN_DSIZE;

# funky stuff to deal with special case of zero or one min dsize
if ( $MIN_DSIZE == 1 ) {
	$MIN_DSIZE=0;
}

if ( $MIN_DSIZE == 0) {
	$BASE_DSIZE=1;
}


# first loop over number of nodes
# then loop over number of bytes
# then loop over number of trials
my ($i,$j,$k);

sub mcapi {
	my ($test,$type,$blocking,$i_start,$j_start,$k_start,$run)=@_;
	if (($type eq "s") && ($blocking eq "-a")) {
		#skip scalar non-blocking, it doesn't make sense
		return;
	}
	for ($i=$i_start; $i<=$MAX_NODES; $i+=$NODE_INCR){
		for ($j=$j_start; $j<=$MAX_DSIZE; $j+=$DSIZE_INCR){
			for ($k=$k_start; $k<$NUM_TRIALS; $k+=1){
				$BYTES=$j;
				if ( $BYTES == 0 ){
					$BYTES=$BASE_DSIZE;
				}
				if (($type eq "s") && (!(( $BYTES == 1 ) || ( $BYTES == 2 ) || ( $BYTES == 4 ) || ( $BYTES == 8 )))){
				} else {
					if ($run) {
						# actually run it
						if (!($tests{$test}{$type}{$blocking}{$i}{$BYTES}{$NUM_SENDS} eq "DONE")) {
							run_cmd("$test -$type $blocking -i $i -b $BYTES -n $NUM_SENDS $a -f $FILENAME &> out");
						}
					} else {
						# just record the test type
						$tests{$test}{$type}{$blocking}{$i}{$BYTES}{$NUM_SENDS} = "NOT_RUN";
					} 
				}
			}
		}
	}
}	

sub pipes {
	my ($test,$i_start,$j_start,$k_start)=@_;
	for ($i=$i_start; $i<=$MAX_NODES; $i+=$NODE_INCR){
		for ($j=$j_start; $j<=$MAX_DSIZE; $j+=$DSIZE_INCR){
			for ($k=$k_start; $k<$NUM_TRIALS; $k+=1){
				$BYTES=$j;
				if ( $BYTES == 0 ){
					$BYTES=$BASE_DSIZE;
				}
				run_cmd("$test -i $i -b $BYTES -n $NUM_SENDS $a -f $FILENAME -P");
			}
		}
	}
}

sub sockets {
	my ($test,$i_start,$j_start,$k_start)=@_;
	for ($i=$i_start; $i<=$MAX_NODES; $i+=$NODE_INCR){
		for ($j=$j_start; $j<=$MAX_DSIZE; $j+=$DSIZE_INCR){
			for ($k=$k_start; $k<$NUM_TRIALS; $k+=1){
				$BYTES=$j;
				if ( $BYTES == 0 ){
					$BYTES=$BASE_DSIZE;
				}
				run_cmd("$test -i $i -b $BYTES -n $NUM_SENDS $a -f $FILENAME -S");
			}
		}
	}
}

use Data::Dumper;

if (-e $FILENAME) {
	# logile exists, assume run is being re-started
	my ($TH,$OpType,$NumHops,$OpSize,$OpCount,$blocking);
	my @optypes = ("s","p","m");
	my @testharnesses = ("../src/tc_echo_pthread","../src/tc_echo_fork");
	my @blocking = ("","-a");
	
	# first populate the test structure will all tests to be run
	foreach $TH (@testharnesses) {
		foreach $OpType (@optypes) {
			foreach my $b (@blocking) {
				next if (($OpType eq "s") && ($b eq "-a")); #skip scalar non-blocking, it doesn't make sense
				mcapi($TH,$OpType,$b,$MIN_NODES,$MIN_DSIZE,0,0);
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
			
			$TH = "../src/tc_echo_".$TH;
			$b = "";
			if ($OpType =~ /async/) {$b = "-a";}
			if (($OpType =~ /msg/) or ($OpType =~ /message/)) { $OpType = "m"; }
			if (($OpType =~ /pkt/) or ($OpType =~ /packet/)) { $OpType = "p"; }
			if (($OpType =~ /scalar/)) { $OpType = "s"; }
			$tests{$TH}{$OpType}{$b}{$NumHops}{$OpSize}{$OpCount} = "DONE";
		}
	}
	close FILE;
	#print "dumper: ",Dumper(\%tests);

	#clean up any unfreed resources
	my @lines = `ipcs`; 
	my $prog = "tc_echo";
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
	print FILE " num_trials: $NUM_TRIALS \n";
	print FILE " num_sends: $NUM_SENDS \n";  
	print FILE " min_dsize: $MIN_DSIZE \n";
	print FILE " max_dsize: $MAX_DSIZE \n";
	print FILE " dsize_incr: $DSIZE_INCR \n";
	print FILE " min_nodes: $MIN_NODES \n";
	print FILE " max_nodes: $MAX_NODES \n";
	print FILE " node_incr: $NODE_INCR \n";
	print FILE " filename: $FILENAME \n";
	
	print FILE"\nTH      OpType  NumHops OpSize  OpCount AvgTime StdDev  %error\n";
	#print FILE `date`;
	print FILE "\n";
	close FILE;

	# now run the tests (assume we'll never have to restart pipes and sockets)
	# pipes
	pipes("../src/tc_echo_pthread",$MIN_NODES,$MIN_DSIZE,0);
	pipes("../src/tc_echo_fork",$MIN_NODES,$MIN_DSIZE,0);
	
	# sockets
	sockets("../src/tc_echo_pthread",$MIN_NODES,$MIN_DSIZE,0);
	sockets("../src/tc_echo_fork",$MIN_NODES,$MIN_DSIZE,0);
}


#mcapi
my @optypes = ("s","p","m");
@optypes = ("m");
my @testharnesses = ("../src/tc_echo_pthread","../src/tc_echo_fork");
my @blocking = ("","-a");
foreach my $test (@testharnesses) {
	foreach my $type (@optypes) {
		foreach my $b (@blocking) {
			mcapi($test,$type,$b,$MIN_NODES,$MIN_DSIZE,0,1);
		}
	}
}


print "num_tests run=$num_tests\n";

