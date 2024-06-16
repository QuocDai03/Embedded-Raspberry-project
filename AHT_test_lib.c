// aht20_driver_lib.c

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

#define IOCTL_BASE 'W'
#define IOCTL_READ_HUMIDITY _IO(IOCTL_BASE, 0)
#define IOCTL_READ_TEMPERATURE _IO(IOCTL_BASE, 1)
#define IOCTL_READ_BOTH _IO(IOCTL_BASE, 2)

// Function to convert raw data to humidity
float convert_humidity(unsigned long raw_humidity) {
    return ((float)raw_humidity / 1048576.0) * 100.0;
}

// Function to convert raw data to temperature
float convert_temperature(unsigned long raw_temperature) {
    return ((float)raw_temperature / 1048576.0) * 200.0 - 50.0;
}

// Function to read humidity from the driver
int read_humidity(int fd, float *humidity) {
    char buffer[64];
    int ret;

    ret = ioctl(fd, IOCTL_READ_HUMIDITY, buffer);
    if (ret < 0) {
        perror("Failed to read humidity");
        return ret;
    }

    unsigned long raw_humidity;
    sscanf(buffer, "Humidity raw: %lu", &raw_humidity);
    *humidity = convert_humidity(raw_humidity);
    return 0;
}

// Function to read temperature from the driver
int read_temperature(int fd, float *temperature) {
    char buffer[64];
    int ret;

    ret = ioctl(fd, IOCTL_READ_TEMPERATURE, buffer);
    if (ret < 0) {
        perror("Failed to read temperature");
        return ret;
    }

    unsigned long raw_temperature;
    sscanf(buffer, "Temperature raw: %lu", &raw_temperature);
    *temperature = convert_temperature(raw_temperature);
    return 0;
}

// Function to read both humidity and temperature from the driver
int read_both(int fd, float *humidity, float *temperature) {
    char buffer[64];
    int ret;

    ret = ioctl(fd, IOCTL_READ_BOTH, buffer);
    if (ret < 0) {
        perror("Failed to read both humidity and temperature");
        return ret;
    }

    unsigned long raw_humidity, raw_temperature;
    sscanf(buffer, "Humidity raw: %lu, Temperature raw: %lu", &raw_humidity, &raw_temperature);
    *humidity = convert_humidity(raw_humidity);
    *temperature = convert_temperature(raw_temperature);
    return 0;
}
