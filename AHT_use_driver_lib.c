#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "aht_test_lib.h"

#define DEVICE_PATH "/dev/aht20" // Adjust this to match your actual device path

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
        if (read_both(fd, &humidity, &temperature) == 0) {
            printf("Humidity: %.2f%%\n", humidity);
            printf("Temperature: %.2f°C\n", temperature);
        }

        // Sleep for a while before the next read (e.g., 1 second)
        sleep(1);
    }

    // Close the device file (not reached in this infinite loop example)
    close(fd);
    return 0;
}
