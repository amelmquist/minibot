#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <libevdev/libevdev.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "i2c_motor.h"

#define ABS_XBOX_MAX 32767 
#define ABS_XBOX_MIN -32768
#define CONTROLLER_VENDOR_ID 0x046D
#define CONTROLLER_PRODUCT_ID 0xC21F

#define MOTOR_1 0x01
#define MOTOR_2 0x02 

typedef struct _xbox_data
{
    int leftX;
    int leftY;
    int rightX;
    int rightY;
} XboxData;


int NormalizeAxisValue(int val, int min, int max)
{
    return (int) ((200.0 / (max - min)) * (val) - 100 * ((max + min) / (max - min)));
}

struct libevdev * DetectXboxController()
{
    DIR *js_dir = opendir("/dev/input");
    if (!js_dir)
        return NULL;
    
    // This loop is the equivalent of doing:
    // ls  /dev/input/event* and then checking each one
    struct dirent *devinfo;
    struct libevdev *dev = NULL;

    while ((devinfo = readdir(js_dir)) != NULL)
    {      
        int fd;
        char buffer[32];

        if (!strstr(devinfo->d_name, "event"))
            continue;

        memset(buffer, 0, 32);
        snprintf(buffer, 32, "/dev/input/%s", devinfo->d_name);

        if ((fd = open(buffer, O_RDONLY)) < 0)
            continue;

        if (libevdev_new_from_fd(fd, &dev) < 0)
        {
            close(fd);
            dev = NULL;
            continue;
        }
        
        if (libevdev_get_id_vendor(dev) != CONTROLLER_VENDOR_ID ||
            libevdev_get_id_product(dev) != CONTROLLER_PRODUCT_ID)
        {
            libevdev_free(dev);
            close(fd);
            dev = NULL;
            continue;
        }

        break;
    }
    
    closedir(js_dir);
    return dev;
}

int main(void)
{
    //controller characteristics
    double a = .02041;
    double b = 1.224;
    double c = 18.37;
    int deadzone = 30;
    int leftWheel = 0;
    int rightWheel = 0;

    int rc;

    //initialize i2c device
    motor_init();

    struct libevdev *dev = DetectXboxController();
    if (!dev)
    {
        printf("Failed to detect controller!\n");
        return 1;
    }

    XboxData data;
    memset(&data, 0, sizeof(XboxData));

    do
    {
        /* read the joystick data */
        struct input_event ev;

        rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL | LIBEVDEV_READ_FLAG_BLOCKING, &ev);

        switch (rc)
        {
            case LIBEVDEV_READ_STATUS_SYNC:
                while (rc == LIBEVDEV_READ_STATUS_SYNC)
                    rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_SYNC, &ev);
                break;
            case LIBEVDEV_READ_STATUS_SUCCESS:
                switch (ev.type)
                {
                    case EV_ABS:
                        switch (ev.code)
                        {
                            case ABS_X:
                                data.leftX = NormalizeAxisValue(ev.value, ABS_XBOX_MIN, ABS_XBOX_MAX);
                                break;
                            case ABS_Y:
                                data.leftY = -NormalizeAxisValue(ev.value, ABS_XBOX_MIN, ABS_XBOX_MAX);
                                break;
                            case ABS_RX:
                                data.rightX = NormalizeAxisValue(ev.value, ABS_XBOX_MIN, ABS_XBOX_MAX);
                                break;
                            case ABS_RY:
                                data.rightY = -NormalizeAxisValue(ev.value, ABS_XBOX_MIN, ABS_XBOX_MAX);
                                break;
                            default:
                                break;
                        }
                        break;
                    default:
                       continue;
                }
                break;
            default:
                break;
        }


        /* do stuff */
        printf("LX: %d | LY: %d | RX: %d | RY: %d\n", data.leftX, data.leftY, data.rightX, data.rightY);
	
	//Left Wheel
	set_slave(MOTOR_1);
	//calculate output base on parabola and dead zone
	if((data.leftY < deadzone)  & (data.leftY > -deadzone)) {leftWheel = 0;}
	else if(data.leftY < 0){
		leftWheel = (int)(-a*data.leftY*data.leftY - b*data.leftY - c);
	}else{
		leftWheel = (int)(a*data.leftY*data.leftY - b*data.leftY + c);
	}
	bcm2835_delay(1);
	send_values(leftWheel);
	bcm2835_delay(1);
	
	
	//Right Wheel
	set_slave(MOTOR_2);
	//calculate output base on parabola and dead zone
	if((data.rightY < deadzone)  & (data.rightY > -deadzone)) {rightWheel = 0;}
	else if(data.rightY < 0){
		rightWheel = (int)(-a*data.rightY*data.rightY - b*data.rightY - c);
	}else{
		rightWheel = (int)(a*data.rightY*data.rightY - b*data.rightY + c);
	}

	bcm2835_delay(1);
	send_values(rightWheel);

	
	
    }while(1);
}











