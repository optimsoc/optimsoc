#!/usr/bin/env perl

use strict;
sub run_cmd {
  my $cmd = shift;
  print "executing [$cmd]\n";
  my $output;
  $output = `$cmd 2>&1`;
  my $error_code = $?;
  if ($error_code) {
    print "$cmd failed with output:\n$output\n";
  }
  return $error_code;
}

run_cmd ("rm -rf *.txt");
run_cmd ("rm -rf tmp");

my @threads;
my $infile = "tmp";
run_cmd("grep TID out | cut -d' ' -f5 | sort -u > $infile");

open IN,"$infile" or die "Could not open $infile for reading.";
while(<IN>) {
  if (/(\d+)/) {
   print "1=$1\n";
   push (@threads,$1);
  }
}
close IN;

foreach my $t (@threads) {
  my $filename="$t".".txt";
  run_cmd("grep $t out > $filename");
  run_cmd("tail $filename >> \"threads.txt\"");
  my $cmd = "echo ---------------------------------------------------------- >> threads.txt";
 `$cmd`;
  print "wrote $filename\n";
}

print  "wrote threads.txt\n";

run_cmd("../scripts/clean.pl tc_");
