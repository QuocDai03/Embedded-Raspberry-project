There are 5 files: 
- AHT_driver.c: the device driver for module AHT20.
- AHT_use_driver.c: an example of how to use ioctl function of my AHT20 driver to get and print in terminal the temparerature and humidity value.
- aht_test_lib.c: a library provided for users to easily get temperature and humidity value.
- aht_test_lib.h: a header file of functions in library mentioned.
- AHT_use_driver_lib.c: an example of how to use the above library to get and print in terminal the temperature and humidity value.  

Here is how I use these files of my project in Raspberry Pi 4 model B:

Step 1: Using Makefile to build AHT_driver.c by using command 'make' in terminal

Step 2: Using command "sudo insmod AHT_driver.ko" to install my driver.

*Note: I don't use device tree, so here is how i use my driver:
Step 3: Using command "sudo su"

Step 4: Using command "cd /sys/bus/i2c/devices/i2c-1" (I use i2c-1 in my Raspberry Pi)

Step 5: Using command "echo aht20 0x38 > new_device" (0x38 is the address of module I2C). 

*Note: After finishing above steps, now i can use my driver.

**If you want to use file AHT_use_driver.c ( using driver without library). 

Step 6a: Using command "gcc AHT_use_driver.c -o AHT_use_driver" to commpile this file. 

Step 7a: Using command "sudo ./AHT_use_driver" to run file and see the result.


** If you want to use my library.

Step 6b: Using command "gcc -c -o aht_test_lib.o aht_test_lib.c" to compile file into object.

Step 7b: Using command "ar rcs libaht_test_lib.a aht_test_lib.o" to create static library.

Step 8b: Using command "gcc AHT_use_driver_lib.c -laht_test_lib -o AHT_use_driver_lib" to compile file with library.

Step 9b: Using command "sudo ./AHT_use_driver_lib" to run file and see the result.

****Note: If you want to remove the driver.
Step: Using command "echo 0x38 > delete_device"
Step: Using command "exit" to exit "sudo su" mode.
Step: Using command "sudo rmmod AHT_driver" to remove the driver.
Step: Using command "make clean" to delete files created after you use command "make".
