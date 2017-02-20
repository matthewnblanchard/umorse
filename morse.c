/* Matthew Blanchard
 * ECE 331
 * 2/18/2017
 * Title: morse.c
 * Description: Takes a single text string as an argument, translates it to morse code, and then prints it to
 * standard output.
 */

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "encoding.h"

#define HIGH 1
#define LOW 0

int GPIO_open(int pin);                      // Open pin 'pin' for output
int GPIO_close(int pin);                     // Close pin 'pin'
int GPIO_write(int pin, int val);            // Write "value" to pin
void p_encode(char c, unsigned char *sum);   // Translates a character to the appropriate morse string and prints it. Tracks checksum

int main(int argc, char *argv[]) {
        
        int i = 0;                        // String index 
        unsigned char checksum = 0;       // Checksum of 'high' Morse time units

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

        // Preamble
        printf("__*_");
        checksum++;

        // Iterate through each character. The last character does not include the three time unit separator
        for (i = 1; text[i] != '\0'; i++) {
                p_encode(text[i - 1], &checksum);
                printf("___");
        }
        p_encode(text[i - 1], &checksum);
        
        // Print checksum
        printf("_");            // Separating 0
        checksum = ~checksum;   // One's complement
        while (checksum != 0b0) {
                if (checksum & 0b1)
                        printf("*");
                else 
                        printf("_");
                checksum >>= 1;
        }
        printf("\n");
        
        GPIO_write(4, 1);
        GPIO_write(17,1);

        usleep(1000 * 1000 * 10);
        
        GPIO_write(4, 0);
        GPIO_write(17, 0);

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
        usleep(500);    // Brief delay for adjustment
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
        usleep(500);    // Brief delay for adjustment
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

void p_encode(char c, unsigned char *sum) 
{
        unsigned int binary = 0b0;        // Binary Morse translation of char

        // Space is a special case: translation is one Morse time unit (in addition to the standard separator of three on both sides)
        if (c == ' ') {
                printf("_");
        } else {
                binary = morse[(int)c];
                while (binary != 0b0) {   // Read each bit, adding '*' for 1 and '_' for 0
                        if (binary & 0b1) {
                                printf("*");
                                (*sum)++;
                        } else {
                                printf("_");
                        }
                        binary >>= 1;
                }
        }
        return;
}
