# Matthew Blanchard 
# 2/17/2017
# Title: encoding.pl
# Descrption: Generates C source & header files which encode all 256 ASCII C
#       characters to Morse code. Reads encodings from file 'encoding.txt', and
#       fills in remaining ASCII characters with unused Morse encodings.

#!/usr/bin/perl
use strict;
use warnings;

# Variables
my $text;       # File handle for reading encoding.txt
my $src;        # File handle for writing encoding.c
my $head;       # File handle for writingi encoding.h
my %encode;     # Morse encodings keyed with decimal #'s for ASCII chars
my %decode;     # Decimal #'s for ASCII chars keyed with their Morse encodings

# Open files for reading/writing
open ($text, '<', 'encoding.txt')
        or die "Failed to open file 'encoding.txt'";

open ($src, '>', 'encoding.c')
        or die "Failed to open file 'encoding.c'";

open ($head, '>', 'encoding.h')
        or die "Failed to open file 'encoding.h'";

# Populate encode & decode using predefined encodings from .txt file
while (my $line = <$text>) {
        my $morse = "";      # String containing binary morse encoding
        chomp $line;
        my @x = split(/ /, $line);
        for(my $i = 1; $i < scalar(@x); $i++) { # Construct binary Morse encoding
                if ($x[$i] eq '---') {
                        $morse = '0111' . $morse;
                } elsif ($x[$i] eq '*') {
                        $morse = '01' . $morse;
                } else {
                        print "Did not recognize '$x[$i]', ignoring\n";
                }
        }
        $morse = reverse($morse); # Remove excess 'space' (The last 0)
        chop $morse;
        $morse = reverse($morse);
        $morse = "0b" . $morse;   # Add in binary identifier       
        $encode{ord($x[0])} = $morse;
        $decode{$morse} = ord($x[0]);
}

# Populate rest of %encode from 0-255 with unused morse encoding
my $cur_morse = "0b1";   # Start from minimum encoding: one 'dit';
for (my $i = 0; $i < 256; $i++) {
        unless (exists $encode{$i}) {    # Encode char if it doesn't yet exist
                while (exists $decode{$cur_morse}) {    # Find unused Morse code
                        $cur_morse = inc_morse($cur_morse);
                }
                $encode{$i} = $cur_morse;
                $decode{$cur_morse} = $i;
        }
}

# DEBUG: Print all encoded values to check effectiveness
for (my $i = 0; $i < 256; $i++) {
        print("$i = $encode{$i}\n");
}

# Subroutine to increment binary encoded Morse code to next possible encoding.
# Counts in binary, interpretting 'dits' as 0 and 'dahs' as 1, adding more bits
# as necessary.

sub inc_morse {
        my $morse = $_[0]; # Sole argument is a string of binary encoded morse
        my @bits = split(/[0b]/, $morse); # Separate 'bits' of the morse
        @bits = grep /\S/, @bits;         # Remove empty elements      

        my $i = scalar(@bits) - 1;        # Start at LSB
        $i-- until ($bits[$i] == 1 || $i == 0); # Search for most significant '0'
        if ($i == 0 && $bits[$i] != 1) {        # Add a bit if we couldn't find a '0'
                unshift @bits, '1';       # Add in a new '0'
        } else {
                $bits[$i] = 111;          # Change most siginicant '0' to '1'
        }

        # Change all less significant bits to '0'
        for (++$i; $i < scalar(@bits); $i++) {
               $bits[$i] = '1';
        }

        # Recompose binary morse
        $morse = join('0', @bits);
        $morse = '0b' . $morse;
        return $morse;       
}
