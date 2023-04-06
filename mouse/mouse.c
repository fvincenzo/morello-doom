#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

#define MOUSE "/dev/input/mouse0"

int main()
{
    int fd;
    struct input_event ievent;
    unsigned char *ptr = (unsigned char*)&ievent;
    
    unsigned char button,left_button,middle_button,right_button;
    char x, y;
    int abs_x =0, abs_y = 0;

    if((fd = open(MOUSE, O_RDONLY | O_NONBLOCK)) == -1)
    {
        printf("Open %s: ERROR\n", MOUSE);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Open %s: OK\n", MOUSE);
    }

    while(1)
    {       
        if(read(fd, &ievent, sizeof(struct input_event))!=-1)
        {
            button=ptr[0];
            left_button = button & 0x1;
            middle_button = (button & 0x4) > 0;
            right_button = (button & 0x2) > 0;
            
            if(left_button | middle_button | right_button) {
                printf("left_button: %u middle_button: %u right_button: %u\n",
                                        left_button, middle_button, right_button);
            }
            
            x = (char)ptr[1];
            y = (char)ptr[2];
            
            abs_x += x;
            abs_y -= y;
            
            printf("X: %i Y: %i\n",abs_x,abs_y);
            fflush(stdout);
        }
    }
    
    close(fd);
    return 0;
}