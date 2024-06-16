#ifndef AHT_TEST_LIB_H
#define AHT_TEST_LIB_H

float convert_humidity(unsigned long raw_humidity);
float convert_temperature(unsigned long raw_temperature);
int read_humidity(int fd, float *humidity);
int read_temperature(int fd, float *temperature);
int read_both(int fd, float *humidity, float *temperature);

#endif // AHT_TEST_LIB_H
