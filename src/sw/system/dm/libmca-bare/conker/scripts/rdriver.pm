#
# Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
#
# You may distribute under the terms of the Artistic License, as specified in
# the COPYING file.
#

=head1 NAME

 driver:  Regression driver.

=head1 SYNOPSIS

The regression driver is a framework for a regression suite.  Refer
to the full man page for more information.

=head1 OPTIONS

Command-line options:

=over 8

=item B<--list, --l>

List the tests in this regression.

=item B<--tests=range, --t=range>

Specify what tests to run.  Should be a number, range, or list of numbers or
ranges, where the numbers refer to indices of the test array (1 is the first
test).  See below fora description of the range format.  Multiple instances of
this parameter may be used.

=item B<--exclude=range, --x=range>

Exclude tests.  Should be a number, range, or list of numbers of ranges.  See
below for a description of the range format.  Multiple instances of this option
may be specified.

=item B<--string=str, --s=str>

Run any tests which contain string I<str>.  This is a simple substring search,
not a regular expression search. Multiple instances of this option may be
specified and are used as a conjunction.

=item B<--regex=re, --r=re>

Run any tests which match regular expression I<re>. Multiple instances of this
option may be specified and are used as a conjunction.

=item B<--keepoutput, --ko>

Do not delete temporary files. 

=item B<--showcmd, --sc>

Print the command to be executed.  This is implied if I<--debug> is set.

=item B<--debug[=level], --d[=level]>

Enable debug messages.  The default (no argument) is to display a minimal amount
of information.  Extra debug levels are available, however.

=item B<--seed>

Specify a seed value.

=back

The valid range forms are:

      x-y:  Run tests x through y.  Example:  --tests=3-5

       -y:  Run tests from start, up to and including y.

       x-:  Run tests x through the last one.

Note:  Multiple parms are allowed and are concatenated together.

=head1 DESCRIPTION

Library usage:

use lib "..";
use rdriver;

doTest(@tests_list);

B<doTest>, the main entry function, takes an array reference as a parameter.

=head2 Hash Keys

Each entry in the supplied array should be a hash.  These are the allowed keys:

=over 8

=item B<cmd>

Command-line to execute.  If the token &seed is found, it will be replaced with
a numerical seed.  This value will either be the value specified by the seed
command-line parameter, or the current result of time().

=item B<pre>

Function to execute before running the command.

=item B<nocmts>

Remove comments from diff.  Default is 0 (preserve).  Set this to the comment
character or string that you want to remove, e.g. "#".

=item B<stdio>

Capture standard output.

=item B<stderr>

Direct standard error to standard output for capture.

=item B<diff>

Diff the output with the file specified.

=item B<dpfx>

Only diff the output that starts with the given prefix.  This may be a regular
expression.

=item B<checker>

If present, this should be a function reference.  The function is called with
the test output as its argument and should die if an error occurs.

=item B<fail>

If this is non-zero then the cmd run is expected to have a non-zero return
value. The test will fail if 0 is returned.  The checking commands will still be
executed if they are present.  The value of fail should match the error code
returned by the program, or should be 999 to represent a don't-care condition.

=item B<temps>

Specify temporary files.  These will be unlinked after the test is complete,
unless the --keepoutput option is set on the command-line.

=over

=head2 Utility Functions

The library also contains a number of utility functions.  These are listed
below.

=back 8

=item file_rdiff(<filename>,<array ref of regular expressions | string>)

This is a simple function for checking a file.  The second argument is a
reference to an array of strings.  Each string is considered to be a regular
expression.  The function must be able to successfully match lines in the file
against the array elements, proceeding to the end of the array, or else it fails
and calls die.

The regular expressions can be either a reference to an array of strings/regular
expressions or a single string.  If the latter, we'll split the string on
newlines.  This can be useful when you have a block of data that you want to
check: Rather than quoting each item, simply use here-document syntax.

=item str_rdiff(<input string>,<array ref of regular expressions | string>)

Same as file_rdiff, except that we take the input, split on newlines, then try
and match against the regular expressions.

=item dprint(...)

Same functionality as print if the --debug option is specified on the
command-line.  Otherwise, nothing is printed.

=item dprintf(...)

Same functionality as printf if the --debug option is specified on the
command-line.  Otherwise, nothing is printed.

=back

=cut
    
  package rdriver;

use FindBin;

require Exporter;

our @ISA = ("Exporter");
our @EXPORT = qw( doTest doDifffile_rdiff str_rdiff dprint dlprint dprintf $debug $keepoutput $showcmd );

$diff = "diff -w";
$tmpfile = "cmp.out";

$debug = -1;
$showcmd = 0;

# These are global because Perl's closures are *broken*!!!!.
$seed = 0;
$cmd = "";
$testindex;
$fails = 0;
@failedtests;
$keepoutput = 0;
$DontCare = 999;

sub error {
  print shift;
  print "  Executed command:  '$cmd'\n";
  ++$fails;
  push @failedtests,$testindex;
  die;
}
;

use strict;
use Data::Dumper;

use vars qw(@Tests $diff $tmpfile $cmd $seed $showcmd $fails $testindex @failedtests $keepoutput $debug $DontCare);

# Main test code:  For each test, execute it, then scan the output for
# the tags we look for.  After that, check everything.
sub doTest($) {
  # get_run_list actually returns a hash, where the keys are
  # indices and the values are the items specified in the 
  # original test array.
  my $tests = get_run_list(shift);

  print "\n";
  # We sort the keys of the hash here, numerically, so that
  # the user sees the tests execute in the expected order.
 TEST: for $testindex (sort { $a <=> $b} (keys %{$tests})) {
    print " Test $testindex...\n";
    my $t = $$tests{$testindex};

    if ($t->{pre}) {
      &{$t->{pre}}();
    }

    my $outfile =  $t->{outfile};
    $cmd = $t->{cmd};
    $cmd =~ s/&seed/$seed/g;
    if ($t->{stderr}) {
      $cmd .= " 2>&1";
    } else {
      $cmd .= " 2>/dev/null";
    }
    print "  Cmd: $cmd\n" if ($showcmd);
    if ($t->{stdio} || $t->{stderr}) {
      $outfile = `$cmd`;
    } else {
      `$cmd`;
    }

    my $failexecution = ($t->{fail});
    my $failchecker =  ($t->{failchecker});
    #print "Output:\n\n$output\n\n";
    eval {
      my $fc = ($? >> 8);
      if ($?) {
		# Check return code from running the command
		if (!($failexecution)) {
		  error (" Test failed and was not expected to.  Return code was $?. Output is in:\n\n$outfile\n");
		} else {
		  if ($failexecution != $DontCare) {
			if ($fc != $failexecution) {
			  error (" Test failed with a value of $? (return code of $fc), but $failexecution was expected.");
			}
		  }
		  print "  ...expected fail found.\n";
		}
      } elsif ($failexecution) {
		error (" Test did not fail but was expected to.  Return code was $?. Output is in:\n\n$outfile\n");
      }
      
      if ( $t->{diff} ) {
		error() if (!doDiff($outfile,$t->{diff},$t->{dpfx},$t->{nocmts}));
      }
      if ($t->{checker}) {
		# Call the check function and check its return code.
		my $checker = $t->{checker};
		eval { &$checker($outfile) };
		if ($@) {
		  if ($failchecker) {
			print "  ...expected fail found.\n";
		  } else {
			error ("  Failed checker:  $@\n");
		  }
		} else {
		  if ($failchecker) {
			error ("  Checker test did not fail but was expected to.\n");
		  } else {
			print "  ...checker test passed.\n";
		  }
		}
      }
    };
    # Remove listed temporary files unless overridden by the user.
    if (!$keepoutput) {
      for (@{ $t->{temps} }) {
		dprint("Unlinking $_\n");
		unlink glob($_);
      }
    }
  }

  if (!$fails) {
    print "\nRegression SUCCEEDED.\n";
  } else {
    print "\nRegression FAILED:  $fails fails found:  @failedtests.\n";
    exit 1;
  }
}

# Perform the diff between the output from the test and the
# regression file.
#
# arg0:  Test output file.
# arg1:  Name of regression file.
# arg2:  (optional).  Diff prefix.  If specified, we extract
#        only the output starting with the given prefix, then 
#        do the diff.
sub doDiff {
  my $outfile = shift;
  my $rfile = shift;
  my $dpfx = shift;
  my $nocmts = shift;

  unlink $tmpfile;
  open OUT,">$tmpfile" or die "Could not open $tmpfile for writing.";
  if ($dpfx) {
    # Prefix specified
    my @lines = split '\n',$outfile;
    for my $l (@lines) {
      if ($l =~ $dpfx) {
		print OUT $l,"\n";
      }
    }
  } elsif ($nocmts) {
	my $cmt = $nocmts;
    # We do not want to preserve comments, so strip all blank lines and
    # comment lines.
    my @lines = split '\n',$outfile;
    for my $l (@lines) {
      next if ($l =~ /^$cmt/ || $l =~ /^\s*$/);
	  if ( $l =~ /^([^#]+)$cmt/ ) {
		my $noncmt = $1;
		next if ( length($noncmt) == 0 || $noncmt =~ /^\s*$/);
		# Comment in line- print only non-comment portion.
		print OUT "$noncmt\n";
	  } else {
		# No comment.
		print OUT $l,"\n";
	  }
    }
  } else {
    # No prefix specified, so just print output to the file.
    print OUT $outfile;
  }
  close OUT;

  my $dout = `$diff $tmpfile $rfile`;
  if ($? > 8) {
    print " Error!  Output differs from regression file '$rfile'.\n";
    print "Diff output was:\n";
    print " <----- Test output file $tmpfile | Regression file $rfile ----->\n";
    print $dout;
    return 0;
  } else {
    unlink $tmpfile unless $keepoutput;
    print "  ...diff check passed.\n";
    return 1;
  }
}

use Getopt::Long;
use Pod::Usage;

# This takes the original test array and returns a hash reference
# where the keys are test indices and the values are the elements of
# the original test.  The default is to return all tests; we modify that
# by the include and exclude specifiers.
sub get_run_list {
  my $tests = shift;
  my $size = scalar( @$tests );
  my (%includes, %excludes);
  my (@ilist,@xlist,@slist,@rlist);
  my ($help,$man,$list,$do_nothing);

  # Process the command-line options, generating up a 
  # list of ranges to include and a list of ranges to exclude.
  if (!&GetOptions
      (
       "l|list"        => \$list,
       "h|help"        => \$help,
       "m|man"         => \$man,
       "d|debug:i"     => \$debug,
       "t|tests=s"     => \@ilist,
       "x|excludes=s"  => \@xlist,
       "s|string=s"    => \@slist,
       "r|regex=s",    => \@rlist,
	   "n|do-nothing", => \$do_nothing,
       "ko|keepoutput" => \$keepoutput,
       "seed=i"        => \$seed,
       "sc|showcmd"    => \$showcmd,
      )) {
    printhelp(1,1);
  }

  # A value on debug is optional.  If not specified, but the option is,
  # we want it to default to a value of 1.
  if ($debug == 0) {
    $debug = 1;
  } elsif ($debug < 0) {
    $debug = 0;
  }

  # Debugging implies showcmd.
  $showcmd = 1 if ($debug);

  # Print help if requested to do so.
  printhelp(0,1) if $help;

  printhelp(0,2) if $man;

  # List tests, if requested to do so.
  if ($list) {
    listtests($tests);
  }

  if (!$seed) {
    $seed = time();
  }

  # This converts a list of range expressions (arg1)
  # into a hash (stored into hash reference arg0), where
  # the keys are indices of tests.
  sub get_indices {
    my $h = shift;
    my $pl = shift;

    for my $i (@$pl) {
      if ($i =~ /^([0-9]+)$/) {
		$$h{$1-1} = 1;
      } elsif ($i =~ /^([0-9]+)-$/) {
		for my $c ($1..$size) {
		  $$h{$c-1} = 1;
		}
      } elsif ($i =~ /^-([0-9]+)$/) {
		for my $c (1..$1) {
		  $$h{$c-1} = 1;
		}
      } elsif ($i =~ /^([0-9]+)-([0-9]+)$/) {
		for my $c ($1..$2) {
		  $$h{$c-1} = 1;
		}
      }
    }
  }

  # First, break white-space delimited lists into individual elements.
  my (@p_ilist,@p_xlist);
  for (@ilist) {
	push @p_ilist,(split / /,$_);
  }
  for (@xlist) {
	push @p_xlist,(split / /,$_);
  }

  # Now convert the range specifications into hashes, where the
  # keys of each hash is an index of a test to include (or exclude).
  get_indices(\%includes,\@p_ilist);
  get_indices(\%excludes,\@p_xlist);

  # If the user didn't specify any includes, use all of them.
  if (!scalar(keys %includes)) {
    for my $i (0..$size-1) {
      $includes{$i} = 1;
    }
  }

  # Now remove all excluded indices from the include list.
  for my $i (keys %excludes) {
    delete $includes{$i};
  }

  # Now create a new test list with only the included indices.
  # If substrings were specified, include each item only if
  # the command contains all substrings.
  my %newtests;
 L1: for my $i (0..$size-1) {
    if ($includes{$i}) {
      my $test = $$tests[$i];
      if (@slist) {
		# Have substrings, so do search.
		for my $s (@slist) {
		  if ( index($test->{cmd},$s) < 0) {
			next L1;
		  }
		}
      }
	  if (@rlist) {
		# Have regular expressions, so do search.
		for my $s (@rlist) {
		  if ( $test->{cmd} !~ /$s/ ) {
			next L1;
		  }
		}
      }
	  # Passed all tests, so include.
	  $newtests{$i+1} = $test;
    }
  }

  print "Tests to be run:  ",join(' ',sort({ $a <=> $b} keys %newtests)),"\n\n" if ($debug);

  exit 0 if ($do_nothing);

  return \%newtests;
}

sub listtests() {
  my $tests = shift;

  print "\nTests in this regression (commands to be executed):\n\n";
  my $i = 1;
  for my $t (@$tests) {
    print "  $i.  $t->{cmd}\n";
    ++$i;
  }
  print "\n";
  exit 0;
}

# To be safe, we use the path to the module from %INC, since we know that that's
# from where the module was loaded.
sub printhelp() {
  my ($eval,$verbose) = (shift,shift);
  my $path = $INC{__PACKAGE__.".pm"};
  pod2usage(-exitval => $eval,-input=>$path,-verbose=>$verbose);
  exit 0;
}

sub file_rdiff {
  my $f = shift;
  my $r = shift;

  open IN,$f or die "Could not open $f for comparing.";

  if (!ref($r)) {
    # Not a reference, so assume that it's a string that we'll divide into
    # an array of strings.
    my @rl = split /\n/,$r;
    $r = \@rl;
  }
  dprint ("Match value:  ",Dumper($r),"\n");

  my $i = 0;
  my $max = scalar(@$r);
  while (<IN>) {
    ++$i if ( $i < $max && $_ =~ $r->[$i] )
  }
  if ($i != $max) {
    print "  Failed to match '$r->[$i]' in file '$f'.\n";
    die;
  }
}

sub str_rdiff {
  my @lines = split /\n/,shift;
  my $r = shift;

  if (!ref($r)) {
    # Not a reference, so assume that it's a string that we'll divide into
    # an array of strings.
    my @rl = split /\n/,$r;
    $r = \@rl;
  }
  dprint ("Match value:  ",Dumper($r),"\n");

  my $i = 0;
  my $max = scalar(@$r);
  for (@lines) {
    ++$i if ( $i < $max && $_ =~ $r->[$i] )
  }
  if ($i != $max) {
    print "  Failed to match '$r->[$i]'.\n";
    die;
  }
}

# This is a debug print function:  If debug is enabled, the output will show up.
sub dprint {
  if ($debug) {
	print @_;
  }
}

# Same as above, except that the first parameter is a debug level value.
sub dlprint {
  my ($level,@data) = @_;
  if ($level <= $debug) {
	print @data;
  }
}

# Equivalent of above for printf.
sub dprintf {
  if ($debug) {
	printf @_;
  }
}

1;
