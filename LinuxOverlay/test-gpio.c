#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define INPUT 0
#define OUTPUT 1

#define HIGH 0
#define LOW 1

#define SPEED 500000

#define PD14 110
#define PD15 111

int init_gpio(int pinNumber, int mode);
int close_gpio(int pinNumber);
int dig_write_gpio(int pinNumber, int mode);
int check_gpio(int pinNumber);
int transfer_spi(int input);

char locationBuffer[100];
char miscBuffer[20];


int main()
{
    int fd = open("/dev/nixieChar", O_WRONLY);
    if (fd == -1) {
        perror("Unable to open /dev/nixieChar");
        exit(1);
    }

    int hr0 = 0;
    int hr1 = 0;

    while(hr1 <= 2){
        hr0 = 0;
        while(hr0 <= 9){
            if(hr1 == 2 && hr0 == 4){
                break;
            }
            sprintf(miscBuffer, "%d%d:00", hr1, hr0);
            if (write(fd, miscBuffer, 5) != 5) {
                perror("Error writing to /dev/nixieChar");
                exit(1);
            }
            printf("%s\n", miscBuffer);
            usleep(SPEED);
            sprintf(miscBuffer, "%d%d 00", hr1, hr0);
            if (write(fd, miscBuffer, 5) != 5) {
                perror("Error writing to /dev/nixieChar");
                exit(1);
            }
            printf("%s\n", miscBuffer);
            usleep(SPEED);
            hr0++;
        }
        hr1++;
    }
   
    close(fd);
        
    return 0;
}
