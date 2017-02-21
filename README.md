# Introduction #
Homework #5 for ECE 331: Introduction to Unix Systems Administration

## encoding.[ch(pl)(txt)] ##
A Perl script (encoding.pl) reads encoding.txt, which contains accepted Morse encodings of ASCII characters.
The Morse encodings are translated to binary, and additional ASCII characters which do not have encodings 
(falling within decimal valus 0-255) are assigned the smallest encodings available. The script then generates
a C source and header file (encoding.[ch]) which tabulates these encodings into an array, indexed with the 
decimal value of the encoding's appropriate ASCII character.

## morse.c ##
This program accepts a single string as a command line argument and translated it to Morse code, assisted by
encoding.[ch]. The result is printed to standard output, with '*' representing a 'high' value and '_'
representing a 'low' value. Words are space with three 'low' values, and a preamble of '__*_' is addeded. A
checksum is added to the end of the transmission, which sums the number of 'high' values. Additionally, the 
translated data is sent in BPSK format over GPIO pin 17, with a '0' or '_' causing a phase shift of 0, and 
a '1' or '*' causing a phase shift of 180 degrees. GPIO pin 4 acts as an active low enable signal, which
is run low through the duration of the transmission, in addition to once cycle before and after it. 

## Makefile ##
The makefile compiles the necessary object files (morse.o & encoding.o), and compiles the executable (morse)
from there. It also includes a clean command which removes the object files and the executable.
