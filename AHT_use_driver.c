#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

#define DEVICE_PATH "/dev/aht20" // Adjust this to match your actual device path

#define IOCTL_BASE 'W'
#define IOCTL_READ_HUMIDITY _IO(IOCTL_BASE, 0)
#define IOCTL_READ_TEMPERATURE _IO(IOCTL_BASE, 1)
#define IOCTL_READ_BOTH _IO(IOCTL_BASE, 2)


int main() {
    int fd;
    float humidity, temperature;

    // Open the device file
    fd = open(DEVICE_PATH, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open the device");
        return errno;
    }

    while (1) {
        // Read both humidity and temperature
        char buffer[64];
        int ret;

        ret = ioctl(fd, IOCTL_READ_BOTH, buffer);
        if (ret < 0) {
            perror("Failed to read both humidity and temperature");
            return ret;
        }

        unsigned long raw_humidity, raw_temperature;
        sscanf(buffer, "Humidity raw: %lu, Temperature raw: %lu", &raw_humidity, &raw_temperature);
        
        humidity= ((float)raw_humidity / 1048576.0) * 100.0;
        temperature= ((float)raw_temperature / 1048576.0) * 200.0 - 50.0;

        printf("Humidity: %.2f%%\n", humidity);
        printf("Temperature: %.2f°C\n", temperature);

    
        // Sleep for a while before the next read (e.g., 1 second)
        sleep(1);
    }

    // Close the device file (not reached in this infinite loop example)
    close(fd);
    return 0;

}
