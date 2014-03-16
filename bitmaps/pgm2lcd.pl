#!/usr/bin/perl
#
# Script to convert ASCII PGM file
# to a C data structure suitable for a
# PCD8544 LCD controller
#
# Copyright 2014 Nicholas Humfrey
#

use strict;
use warnings;

if (scalar(@ARGV) < 1) {
    print "pgm2lcd.pl <filename.pgm>\n";
    exit(-1);
}


open(PGM, $ARGV[0]) or die "Failed to open file: $!";

# First Line
my $magic = <PGM>;
chomp($magic);
if ($magic ne 'P2') {
    warn "This script only supports the P2 format.\n";
    exit(-1);
}

# Second Line
my $comment = <PGM>;
chomp($comment);
if ($comment !~ /^#/) {
    warn "Expected the second line to be a comment.\n";
    exit(-1);
}

# Third Line
my ($width, $height);
my $wh = <PGM>;
chomp($wh);
if ($wh =~ /(\d+)\s(\d+)/) {
    $width = $1;
    $height = $2;
} else {
    warn "Expected the third line to be a width and height.\n";
    exit(-1);
}

if (($width % 6) != 0) {
    warn "Error: the image width is not a multiple of 6.\n";
    exit(-1);
}

if (($height % 8) != 0) {
    warn "Error: the image height is not a multiple of 8.\n";
    exit(-1);
}

# Fourth Line
my $depth = <PGM>;
chomp($depth);
if ($depth !~ /^(\d+)$/ or $depth <= 0 or $depth >= 65536) {
    warn "Expected the depth to be a positive integer less than 65536.\n";
    exit(-1);
}

# Read in the pixels, row by row
my @bitmap = ();
for (my $y=0; $y < $height; $y++) {
    my @row = split(/\s+/, <PGM>);
    for(my $x=0; $x < $width; $x++) {
        $bitmap[$x][$y] = $row[$x] ? 0 : 1;
    }
}

close(PGM);




# Now convert the bitmap into an array of bytes
my @bytes = ();
for (my $r=0; $r < ($height/8); $r++) {
    my $y = $r * 8;
    for (my $x=0; $x < $width; $x++) {
        push(@bytes, (
            ($bitmap[$x][$y+0] && 0x01) +
            ($bitmap[$x][$y+1] && 0x02) +
            ($bitmap[$x][$y+2] && 0x04) +
            ($bitmap[$x][$y+3] && 0x08) +
            ($bitmap[$x][$y+4] && 0x10) +
            ($bitmap[$x][$y+5] && 0x20) +
            ($bitmap[$x][$y+6] && 0x40) +
            ($bitmap[$x][$y+7] && 0x80)  
        ));
    }
}

# Finally print out the bytes as a C array
print "uint8_t bitmap[".($height/8)."*$width] PROGMEM = {\n";
for (my $b=0; $b < scalar(@bytes); $b++) {
    print "  " if ($b % 15 == 0);
    printf("0x%2.2x,", $bytes[$b]);
    print "\n" if ($b % 15 == 14);
}
print "};\n";
