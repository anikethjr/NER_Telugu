=comment
  YamCha -- Yet Another Multipurpose CHunk Annotator
 
  $Id: PKI.pm,v 1.2 2004/09/20 09:59:16 taku-ku Exp $;

  Copyright (C) 2000-2004 Taku Kudo <taku-ku@is.aist-nara.ac.jp>
  This is free software with ABSOLUTELY NO WARRANTY.
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
=cut

package PKI;
use File::Spec;
use strict;
use vars qw (@sv %fi2si %example 
             $svidx $svsize $dsize $ndsize $dasize $fsize $tsize
             $kernel_type $degree $param_g $param_s $param_r	     
             $outfile $tooldir $mkdarts);

sub initialize
{ 
    ($outfile, $tooldir) = @_;

    die "FATAL: empty file name\n" if (! defined $outfile || $outfile eq "");

    $mkdarts = File::Spec->catfile ($tooldir, "mkdarts");

    @sv      = ();
    %fi2si   = ();
    %example = ();
    $dsize   = 0;
    $ndsize  = 0;
    $svidx   = 0;
}

sub get_type   { return 1; } # PKB:0 PKI:1 PKE:2

sub get_header 
{  
    # INT x 8
    return ($dsize,      # dimension of model, 
	    $ndsize,     # max non dimension, which is used cache of dot products.
	    $dasize,     # size of double array
	    $svsize,     # size of support vectors;
	    $tsize,      # table size
	    $fsize,      # feature size
	    0, 0);    # dummy (reserved area)
}

sub finalize
{
    return if (! defined $outfile || $outfile eq "");
    for (get_concat_files()) {
	unlink $_ if (-f $_);
    }
}

sub set_kernel_param
{
    ($kernel_type,$degree, $param_g, $param_s, $param_r) = @_;
}

sub process_line
{
    my ($line, $m) = @_;
    
    my ($alpha, $ex) = split /\s+/, $line, 2;
    my @tmp = split /\s+/, $ex;

    if (! defined $example{$ex}) {
	for (@tmp) {
	    my ($i ,$v) = split /:/, $_;
	    $dsize = $dsize > $i ? $dsize : $i;
	    push @{$fi2si{$i}}, $svidx;
	}
	$ndsize = $ndsize > ($#tmp+1) ? $ndsize: ($#tmp+1);
	$example{$ex} = $svidx;
	$sv[$svidx]->{$m} += $alpha;
	++$svidx;
    } else {
	my $idx = $example{$ex};
	$sv[$idx]->{$m} += $alpha;
    }
}

sub mkmodel 
{
    my @dic   = @{$_[0]}; # dic
    my @model = @{$_[1]}; # model param

    my $msize = scalar (@model);

    ++$svidx;
    $svsize = $svidx;

    ++$dsize;
    
    %example = ();
    undef %example;

    $tsize = 0;
    my %old2new = ();
    open (S, "> $outfile.t")  || die "$!: $outfile.t\n"; binmode S;
    for (my $i = 0; $i < $dsize; ++$i) {
	print "."  if ($i % 1000 == 0);
	if (defined $fi2si{$i}) {
	    $old2new{$i} = $tsize;
	    $tsize += (scalar (@{$fi2si{$i}}) + 1);
	    print S pack ("i*", @{$fi2si{$i}});
	    print S pack ("i", -1);
	} else {
	    $old2new{$i} = -1;
	}
    }
    close (S);

    $fsize = 0;
    open (S1, "> $outfile.idx")   || die "$!: $outfile.idx\n"; binmode S1;
    open (S2, "> $outfile.alpha")   || die "$!: $outfile.alpha\n"; binmode S2;
    for (my $m = 0; $m < $msize; ++$m) {
	for my $idx (0 .. $#sv) {
	    print "."  if ($idx % 10000 == 0);
	    if (defined $sv[$idx]->{$m}) {
		print S1 pack("i", $idx);
		print S2 pack("d", $sv[$idx]->{$m});
		++$fsize; 
	    }
	}
	print S1 pack("i", -1);
	print S2 pack("d", 0.0);
	++$fsize; 
    }
    close (S1);
    close (S2);

    print "\n";

    open (S, "| $mkdarts - $outfile.da") || die "$!: $mkdarts\n";
    for (@dic) {
	my ($i, $str) = @{$_};
	my $n = $old2new{$i};
	print S "$n $str\n" if ($n != -1);
    }
    close (S);
    $dasize = (stat("$outfile.da"))[7];
    
    # aligned 8
    my $n = $dasize + $tsize + $fsize;
    if ($n % 2 == 1) {
        open (S1, ">> $outfile.idx")    || die "$!: $outfile.idx\n"; binmode S1;
	open (S2, ">> $outfile.alpha")  || die "$!: $outfile.alpha\n"; binmode S2;
	print S1 pack("i", -1);
	print S2 pack("d", 0.0);
	close (S1);
	close (S2);
	++$fsize;
    }
}

sub get_concat_files
{
    return ("$outfile.da", "$outfile.t", "$outfile.idx", "$outfile.alpha");
}

1;
