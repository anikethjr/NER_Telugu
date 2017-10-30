=comment
  YamCha -- Yet Another Multipurpose CHunk Annotator
 
  $Id: PKE.pm,v 1.4 2004/09/20 09:59:16 taku-ku Exp $;

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

package PKE;
use IO::File;
use File::Spec;
use strict;
use vars qw ($fh $dasize $dsize $ndsize $tsize $fsize $minsup $sigma 
             $kernel_type $degree $param_g $param_s $param_r
	     $outfile $tooldir $mkdarts $mktrie $pkemine);

sub initialize
{ 
    ($outfile, $tooldir, $sigma, $minsup) = @_;

    die "FATAL: empty file name\n" if (! defined $outfile || $outfile eq "");

    $mkdarts = File::Spec->catfile ($tooldir, "mkdarts");
    $mktrie  = File::Spec->catfile ($tooldir, "mktrie");
    $pkemine = File::Spec->catfile ($tooldir, "pkemine");
    
    $fh      = 0;
    $dasize  = 0;
    $tsize   = 0;
    $fsize   = 0;
    $dsize   = 0;
    $ndsize  = 0;
}

sub get_type   { return 2; } # PKB:0 PKI:1 PKE:2

sub get_header 
{  
    # INT x 8
    return ($dsize,      # dimension of model,  (not used)
	    $ndsize,     # max non dimension, which is used cache of dot products. (not used)
	    $dasize,     # size of double array
	    0,           # size of support vectors
	    $tsize,      # size of trie (table size)
	    $fsize,      # feature size
	    0, 0);    # dummy (reserved area)
}

sub finalize
{
    return if (! defined $outfile || $outfile eq "");
    for (get_concat_files()) {
	unlink $_ if (-f $_); 
    }
    unlink "$outfile.mine";
}

sub set_kernel_param
{
    ($kernel_type,$degree, $param_g, $param_s, $param_r) = @_;
}

sub process_line
{
    my ($line, $m) = @_;
    if ($fh == 0) {
	$fh = new IO::File ("| $pkemine $sigma $minsup $degree - $outfile.mine") || die "$!: $pkemine\n";
    }

    my ($alpha, @l) = split /\s+/, $line;
    my @t;
    $ndsize = ($#l + 1) > $ndsize ? ($#l + 1) : $ndsize;
    for (@l) {
	my ($i, $v) = split /:/, $_;
	push @t, $i;
	$dsize = $i > $dsize ? $i : $dsize; 
    }
    printf $fh "%d %12.12f ", $m, $alpha;
    print  $fh "@t\n";
}

sub mkmodel 
{
    $fh->close (); 

    my @dic = @{shift @_}; # dic
    
    open(S, "| $mkdarts - $outfile.da") || die "$!: $mkdarts\n";
    for (@dic) {
	my ($i, $str) = @{$_};
	print S "$i $str\n";
    }
    close (S);
    $dasize = (stat("$outfile.da"))[7];

    ++$dsize;

    my %alpha = ();
    my $id = 0;
    open (F, "$outfile.mine") || die "$!: $outfile.mine\n";
    while (<F>) {
	my ($vec, $feat) = split "  ", $_; # must be double spaces
	if (! defined $alpha{$vec}) {
	    $alpha{$vec} = $id;
	    my @t = split /\s+/, $vec;
	    $id += scalar @t; # num
	}
    }
    close (F);

    open (F, "$outfile.mine") || die "$!: $outfile.mine\n";
    open (S, "| $mktrie - $outfile.trie") || die "$!: $mktrie\n";
    while (<F>) {
	chomp;
	my ($vec, $feat) = split "  ", $_; # must be double spaces
	my $id = $alpha{$vec};
	print S "$id $feat\n";
    }
    close (S);
    close (F);
    $tsize = (stat("$outfile.trie"))[7];

    open (S1, "> $outfile.idx"  ) || die "$!: $outfile.idx\n";
    open (S2, "> $outfile.alpha") || die "$!: $outfile.alpha\n";
    binmode S1;
    binmode S2;
    $fsize = 0;
    for my $vec (sort { $alpha{$a} <=> $alpha{$b} } keys %alpha) {
	my @tmp = split /\s+/, $vec;
	for (@tmp) {
	    my ($i, $v) = split /:/, $_;
	    print S1 pack ("i", $i); #
	    print S2 pack ("d", $v); # doubleint
	    ++$fsize;
	}
    }
    %alpha = ();
    undef %alpha;

    ++$fsize;
    print S1 pack ("i", 0);
    print S2 pack ("d", 0.0);

    # aliighed 8
    my $n = $dasize + $tsize + 4 * $fsize;
    if ($n % 8 != 0) {
	print S1 pack ("i", 0);
    	print S2 pack ("d", 0.0);
        ++$fsize;
    }

    close (S1);
    close (S2);
}

sub get_concat_files 
{
    return ("$outfile.da", "$outfile.trie", "$outfile.idx", "$outfile.alpha");
}

1;
