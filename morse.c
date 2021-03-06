/* Matthew Blanchard
 * ECE 331
 * 2/18/2017
 * Title: morse.c
 * Description: Takes a single text string as an argument, translates it to morse code, and then prints it to
 * standard output.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "encoding.h"

#define HIGH 1
#define LOW 0
#define PHASE_ZERO  0           // 0 radian phase
#define PHASE_PI  1             // pi radian phase
#define MORSE_UNIT 500000       // Morse time unit in usec (1/2 a second)

int GPIO_open(int pin);                          // Open pin 'pin' for output
int GPIO_close(int pin);                         // Close pin 'pin'
int GPIO_write(int pin, int val);                // Write "value" to pin
void p_encode(char c, unsigned char *sum, int *phase);    // Translates a character to the appropriate morse string and prints it. Tracks checksum
void print_bpsk(char *morse, int *phase);        // Prints the input morse text (a combination of '*' and '_') to the screen while controlling the GPIO pins 4 & 17
int cycle(int phase);                            // Performs one cycle on GPIO pin 17, with period MORSE_UNIT*2 and the given phase

int main(int argc, char *argv[]) {
        
        int i = 0;                        // String index 
        unsigned char checksum = 0;       // Checksum of 'high' Morse time units
        int phase = PHASE_ZERO;           // Start at zero phase

        // Expecting one argument (which will be interpreted as a string)
        if (argc != 2) {
                printf("Error: a single string as an argument\n");
                return 1;
        }

        // Set up GPIO pins
        if (GPIO_open(4)) {
                printf("Fatal Error: Could not open GPIO pin 4 ...\n");
                return 2;
        }
        if (GPIO_open(17)) {
                printf("Fatal Error: Could not open GPIO pin 17 ...\n");
                return 2;
        }
        
        // Copy string argument
        char *text = argv[1];

        // Bring enable low one full cycle before transmission
        GPIO_write(17, 0);
        GPIO_write(4, 0);
        usleep(MORSE_UNIT * 2);  // Sleep for a full cycle

        // Preamble
        print_bpsk("__*_", &phase);
        checksum++;

        // Iterate through each character. The last character does not include the three time unit separator
        for (i = 1; text[i] != '\0'; i++) {
                p_encode(text[i - 1], &checksum, &phase);
                print_bpsk("___", &phase);
        }
        p_encode(text[i - 1], &checksum, &phase);
        
        // Print checksum
        print_bpsk("_", &phase);        // Separating 0
        checksum = ~checksum;           // One's complement
        while (checksum != 0b0) {
                if (checksum & 0b1)
                        print_bpsk("*", &phase);
                else 
                        print_bpsk("_", &phase);
                checksum >>= 1;
        }
        printf("\n");
        
        // Bring enable high again one full cycle after transmission
        GPIO_write(17, 0);
        usleep(MORSE_UNIT * 2);
        GPIO_write(4, 1);

        // Close GPIO pins
        if (GPIO_close(4)) {
                printf("Error: Could not close GPIO pin 4 ...\n");
                return 3;
        }
        if (GPIO_close(17)) {
                printf("Error: Could not close GPIO pin 17 ...\n");
                return 3;
        }

        return 0;
}

int GPIO_open(int pin)
{
        char buf[3];            // Max two digits in pin string, plus null termintator
        ssize_t size;           // Size of 'pin' string
        int file;               // File descriptor
        char dir_path[35];      // File path for direction

        // Open GPIO pin
        file = open("/sys/class/gpio/export", O_WRONLY);  // Open GPIO export
        if (file == -1) {
                printf("Failed to open GPIO export\n");
                return 1;
        }
        size = snprintf(buf, 3, "%d", pin);   // Write GPIO pin to open to file
        write(file, buf, size);               
        close(file);                          // Close export file

        usleep(50000);    // Brief delay for adjustment

        // Set GPIO pin direction
        snprintf(dir_path, 35, "/sys/class/gpio/gpio%d/direction", pin);
        file = open(dir_path, O_WRONLY);
        if (file == -1) {
                printf("Failed to open GPIO direction\n");
                return 2;
        }
        
        if (!(write(file, "out", 3))) {
                printf("Failed to write direction to GPIO\n");
                return 3;
        }       
        close(file);
        return 0;
}

int GPIO_close(int pin)
{
        char buf[3];   // Max two digits in pin string, plus null terminator
        ssize_t size;  // Size of 'pin' string
        int file;      // File descriptor

        file = open("/sys/class/gpio/unexport", O_WRONLY);  // Open GPIO unexport
        if (file == -1) {
                printf("Failed to open GPIO unexport\n");
                return 1;
        }

        size = snprintf(buf, 3, "%d", pin); // Write GPIO pin to close to file
        write(file, buf, size);
        close(file);
        return 0;      
}

int GPIO_write(int pin, int value)
{
        int file;                // File descriptor
        char write_path[30];     // File path for value    
        char input[2] = "01";    // Possible inputs

        snprintf(write_path, 30, "/sys/class/gpio/gpio%d/value", pin);
        file = open(write_path, O_WRONLY);
        if (file == -1) {
                printf("Failed to open GPIO value\n");
                return 1;
        }
        
        if (1 != write(file, value == 0 ? &input[0] : &input[1], 1)) {
                printf("Failed to write to GPIO\n");
                return 2;
        }
        close(file);
        return 0;
}

void p_encode(char c, unsigned char *sum, int *phase) 
{
        unsigned int binary = 0b0;        // Binary Morse translation of char

        // Space is a special case: translation is one Morse time unit (in addition to the standard separator of three on both sides)
        if (c == ' ') {
                print_bpsk("_", phase);
        } else {
                binary = morse[(int)c];
                while (binary != 0b0) {   // Read each bit, adding '*' for 1 and '_' for 0
                        if (binary & 0b1) {
                                print_bpsk("*", phase);
                                (*sum)++;
                        } else {
                                print_bpsk("_", phase);
                        }
                        binary >>= 1;
                }
        }
        return;
}

void print_bpsk(char *morse, int *phase)
{
        char *c;         // Current character being analyzed

        for (c = &morse[0]; *c != '\0'; c++) {
                printf("%c", *c);       // Print character
                if (*c == '_') {        // On a '_', cycle GPIO 17 with the same phase
                        cycle(*phase); 
                } else if (*c == '*') { // On a '*', shift the phase, then cycle GPIO 17
                        *phase = !(*phase);
                        cycle(*phase);
                }
        }

}

int cycle(int phase)
{
        if (phase == PHASE_ZERO) {
                GPIO_write(17, 0);
                usleep(MORSE_UNIT);
                GPIO_write(17, 1);
                usleep(MORSE_UNIT);
                return 0;
        } else if (phase == PHASE_PI) {
                GPIO_write(17, 1);
                usleep(MORSE_UNIT);
                GPIO_write(17, 0);
                usleep(MORSE_UNIT);
                return 0;
        } else {
                printf("Warning: failed to cycle GPIO pin 17 ... \n");
                return 1;
        }
}
