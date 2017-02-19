/* Matthew Blanchard
 * ECE 331
 * 2/18/2017
 * Title: morse.c
 * Description: Takes a single text string as an argument, translates it to morse code, and then prints it to
 * standard output.
 */

#include <stdio.h>
#include "encoding.h"

void p_encode(char c);   // Translates a character to the appropriate morse string and prints it

int main(int argc, char *argv[]) {
        
        int i = 0;      // String index 

        // Expecting one argument (which will be interpreted as a string)
        if (argc != 2) {
                printf("Error: a single string as an argument\n");
                return 1;
        }

        char *text = argv[1];
        
        // Iterate through each character. The last character does not include the three time unit separator
        for (i = 1; text[i] != '\0'; i++) {
                p_encode(text[i - 1]);
                printf("___");
        }

        p_encode(text[i - 1]);
        printf("\n");
        return 0;
}

void p_encode(char c) 
{
        unsigned int binary = 0b0;        // Binary Morse translation of char

        // Space is a special case: translation is four Morse time units (in addition to the standard separator of three)
        if (c == ' ') {
                printf("____");
        } else {
                binary = morse[(int)c];
                while (binary != 0b0) {   // Read each bit, adding '*' for 1 and '_' for 0
                        if (binary & 0b1)
                                printf("*");
                        else
                                printf("_");
                        binary >>= 1;
                }
        }
        return;
}
