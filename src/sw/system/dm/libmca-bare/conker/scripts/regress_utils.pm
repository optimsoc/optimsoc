##
## Place regression utility functions specific to ADL in this file.
##
#
# Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
#
# You may distribute under the terms of the Artistic License, as specified in
# the COPYING file.
#

package regress_utils;

require Exporter;

our @ISA = ("Exporter"); 
our @EXPORT = qw( 
                 file_diff ext_file_diff
                 parseint printhex
                );

use FindBin;
use lib "$FindBin::RealBin/../share";
use rdriver;
use Data::Dumper;
use File::Basename;
use File::Temp "tempfile";
#use Math::BigInt lib => 'GMP';
use Cwd;

# This imports variables that Automake defines in the makefile.  For
# consistancy, it defines them in the regression even if they're not set
# externally. If the variable is not found in the environment, then we put it
# there so that any child-processes will also see a consistent view between
# running the regression manually and being invoked by make. 
$src = $ENV{srcdir};

$ENV{srcdir} = $src = "." if (!$src);

# This attempts to figure out the root of the build tree by looking for
# "config.status", which should always exist at the build-root.
#$root = getRoot(); 
$root = getRoot("regress_utils.pm");

#print "Root:  $root\n";

use strict;


##
## Place generic helper functions here.
## <HELPERS>
##

# To find the root, we look at %INC to find from where this directory was 
# loaded, then go up a level and consider that our root.  If there's no bin
# directory there, then we start at our current directory and crawl up it,
# looking for a bin directory.
sub getRoot {
  my ($base,$path,$type) = fileparse($INC{$_[0]},".pm");
  if ( -d "$path../bin" ) {
	return "$path..";
  }
  
  # If not found, then we crawl up from our current directory, looking for the
  # bin directory.
  my @dirs = split /\/+/,getcwd();
  while (@dirs) {
	my $d = join '/',@dirs;
	if ( -d "$d/bin") {
	  return $d;
	}
	pop @dirs; 
      }
  
  # Couldn't find it, so just default to the usual case. 
 return "../.."; 
} 

# Given a string representing an integer (hex must have 0x prefix), returns
# either an integer if the string is small enough to fit within a 32-bit
# integer, or else a BigInt.
sub parseint {
  my $x = shift;

  # Handling decimal is tough, and this won't handle leading 0s, so it's overly
  # conservative.
  if (($x =~ "^0x" && length($x) <= 10) || (length($x) <= 9)) {
    return eval $x;
  } else {
    return new Math::BigInt($x);
  }
}

# Prints a hex integer.  The value may be either an integer or a BigInt.
sub printhex {
  my $x = shift;

  if (!ref($x)) {
    return sprintf "0x%x",$x;
  } elsif ($x->isa("Math::BigInt")) {
    return $x->as_hex();
  } else {
    print "printhex:  Don't know how to handle $x.\n";
    die;
  }
}


# Runs diff on two files.
# arg0:  Generated file- will be erased if diff finds no differences.
# arg1:  Reference file.
# arg2:  If set to 1, do not delete the generated file.
sub file_diff {
  my ($gen,$exp,$nodel) = @_;
  dprint ("Comparing $gen (found) vs. $exp (expected)\n");
  if (system "diff -w $gen $exp") {
	print "Differences were found.\n",
	  " <---- Generated $gen | Reference $exp ---->\n";
	die;
  }
  unlink $gen unless $keepoutput;
}

#to check tracediff in incremetal mode
sub tracediff_inc {
  my ($gen,$exp) = @_;
  if (system "$datdiff --inc-scan --max-cmd=1 $gen $exp") {
	print "Differences were found for tracediff in incremental mode.\n",
	die;
  }
}

# extended file_diff 
# Takes named parameters:
# in:      Input object file.
# exp:     Reference file.
# nodel:   Do not delete file when done.
# start:   If defined, regular expression to match to start checking.
# stop:    If defined, regular expression to match to stop checking.
# stripcr: Remove all newlines from the input file.
# trans:   Apply a substitution regular expression to each line of the file.
#          The format is [ <from> , <to> ].
sub ext_file_diff {
  my %args = @_;
  
  my ($in,$exp) = ($args{in},$args{exp});

  my $out;

  my $b = 0;
  my $stripcr = $args{stripcr};
  if (exists $args{start} || exists $args{stop} || exists $args{trans} || $stripcr) {
	$out = "$in.out";
	open IN,$in or die "Could not open $in\n";
	open OUT,">$out" or die "Could not open output file $out\n";
	my $b = (exists $args{start}) ? 0 : 1;
	my ($trans_cmp,$trans_sub);
	if ( exists $args{trans} ) {
	  $trans_cmp = qr/$args{trans}->[0]/;
	  $trans_sub = qr/$args{trans}->[1]/;
	}
	while (<IN>) {
	  $b = 1 if ( !$b && /^$args{start}/ );
	  s/\n//g if $stripcr;
	  if ( $trans_cmp ) {
		s/$trans_cmp/$trans_sub/;
	  }
	  print OUT if ($b);
	  last if (exists $args{stop} && /^$args{stop}/ );
	}
	close IN;
	close OUT;
	
	if ($stripcr) {
	  my ($ebase,$epath) = fileparse($exp);
	  my $exp_orig = $exp;
	  $exp = "$ebase.out";
	  open IN,$exp_orig or die "Could not open $exp_orig\n";
	  open OUT,">$exp" or die "Could not open output file $exp\n";
	  while (<IN>) {
		s/\n//g if $stripcr;
		print OUT if ($b);
	  }
	  close IN;
	  close OUT;
	}
	
	file_diff($out,$exp);

	unlink $out unless $keepoutput;
	if ($stripcr) {
	  unlink $exp unless $keepoutput;
	}
  } else {
	file_diff($in,$exp);
  }
}



