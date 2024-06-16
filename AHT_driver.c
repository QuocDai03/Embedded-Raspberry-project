#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/time.h>

#define DRIVER_NAME "aht20_driver"
#define CLASS_NAME  "aht20_class"
#define DEVICE_NAME "aht20"

#define IOCTL_BASE 'W'
#define IOCTL_READ_HUMIDITY _IO(IOCTL_BASE, 0)
#define IOCTL_READ_TEMPERATURE _IO(IOCTL_BASE, 1)
#define IOCTL_READ_BOTH _IO(IOCTL_BASE, 2)

static struct i2c_client* aht20_client;
static struct class*  aht20_class  = NULL;
static struct device* aht20_device = NULL;
static int major_number;

// Hàm để bắt đầu giao tiếp với cảm biến
static void begin(void) {
    unsigned char buffer[1] = {0xBE};
    int ret;

    // Gửi lệnh khởi động cảm biến
    ret = i2c_master_send(aht20_client, buffer, 1);
    if (ret < 0) {
        printk(KERN_INFO "Failed to send begin command\n");
    }
}

// Hàm để bắt đầu cảm biến
static int start_sensor(void) {
    unsigned char buffer[3] = {0xAC, 0x33, 0x00};
    unsigned char status;
    struct timespec64 start, current_time;
    int ret;

    // Gửi lệnh khởi động cảm biến
    ret = i2c_master_send(aht20_client, buffer, 3);
    if (ret < 0) {
        printk(KERN_INFO "Failed to send start command\n");
        return 0;
    }

    // Lấy thời gian bắt đầu
    ktime_get_real_ts64(&start);

    while (1) {
        // Tính thời gian hiện tại
        ktime_get_real_ts64(&current_time);
        if ((current_time.tv_sec - start.tv_sec) * 1000 + 
            (current_time.tv_nsec - start.tv_nsec) / 1000000 > 200) {
            // Time out sau 200 ms
            return 0;
        }

        // Yêu cầu đọc 1 byte từ cảm biến
        ret = i2c_master_recv(aht20_client, &status, 1);
        if (ret < 0) {
            printk(KERN_INFO "Failed to read status\n");
            return 0;
        }

        // Kiểm tra bit busy
        if ((status & 0x80) == 0) {
            return 1; // Không busy, khởi động thành công
        }

        // Delay 20 ms
        msleep(20);
    }
}

// Hàm để đọc cảm biến và gán giá trị vào biến __humi và __temp
static int get_sensor(unsigned long *h, unsigned long *t) {
    int ret;
    unsigned char data[6];
    unsigned long humidity = 0, temperature = 0;

    if (!start_sensor()) {
        return 0; // Thất bại khi bắt đầu cảm biến
    }

    // Yêu cầu 6 byte dữ liệu từ cảm biến
    ret = i2c_master_recv(aht20_client, data, 6);
    if (ret < 0) {
        dev_err(&aht20_client->dev, "Failed to read data: %d\n", ret);
        return 0;
    }

    // Kiểm tra bit busy trong byte đầu tiên
    if (data[0] & 0x80) {
        return 0;
    }

    // Gán giá trị vào biến __humi
    if (h) {
        humidity = (data[1] << 12) | (data[2] << 4) | (data[3] >> 4);
        *h = humidity;
    }

    // Gán giá trị vào biến __temp
    if (t) {
        temperature = ((data[3] & 0x0F) << 16) | (data[4] << 8) | data[5];
        *t = temperature;
    }

    return 1;
}

// Hàm mở thiết bị
static int aht20_open(struct inode *inodep, struct file *filep){
    printk(KERN_INFO "AHT20 Device opened\n");
    return 0;
}

// Hàm đóng thiết bị
static int aht20_release(struct inode *inodep, struct file *filep){
    printk(KERN_INFO "AHT20: Device successfully closed\n");
    return 0;
}

// Hàm ioctl
static long aht20_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
    unsigned long humidity = 0, temperature = 0;
    int ret;
    char output[64];
    size_t output_len;

    switch (cmd) {
        case IOCTL_READ_HUMIDITY:
            ret = get_sensor(&humidity, NULL);  // Chỉ đọc độ ẩm
            if (ret == 0) {
                return -EFAULT;
            }
            // Định dạng độ ẩm vào buffer output
            output_len = snprintf(output, sizeof(output), "Humidity raw: %lu\n", humidity);
            break;

        case IOCTL_READ_TEMPERATURE:
            ret = get_sensor(NULL, &temperature);  // Chỉ đọc nhiệt độ
            if (ret == 0) {
                return -EFAULT;
            }
            // Định dạng nhiệt độ vào buffer output
            output_len = snprintf(output, sizeof(output), "Temperature raw: %lu\n", temperature);
            break;

        case IOCTL_READ_BOTH:
            ret = get_sensor(&humidity, &temperature);  // Đọc cả độ ẩm và nhiệt độ
            if (ret == 0) {
                return -EFAULT;
            }
            // Định dạng cả độ ẩm và nhiệt độ vào buffer output
            output_len = snprintf(output, sizeof(output), "Humidity raw: %lu, Temperature raw: %lu\n",
                                  humidity, temperature);
            break;

        default:
            return -EINVAL;
    }

    // Đảm bảo không copy quá dữ liệu vào buffer của user
    if (copy_to_user((char *)arg, output, output_len)) {
        return -EFAULT;
    }

    return output_len;
}

// Cấu trúc file_operations
static struct file_operations fops =
{
    .open = aht20_open,
    .unlocked_ioctl = aht20_ioctl,
    .release = aht20_release,
};

// Hàm probe được gọi khi thiết bị được kết nối
static int aht20_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    aht20_client = client;

    // Đăng ký char device
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0){
        printk(KERN_ERR "Failed to register a major number\n");
        return major_number;
    }

    // Tạo lớp thiết bị
    aht20_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(aht20_class)){
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "Failed to register device class\n");
        return PTR_ERR(aht20_class);
    }

    // Tạo thiết bị
    aht20_device = device_create(aht20_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(aht20_device)){
        class_destroy(aht20_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "Failed to create the device\n");
        return PTR_ERR(aht20_device);
    }

    // Gọi hàm begin để khởi động cảm biến
    begin();

    printk(KERN_INFO "AHT20 driver installed\n");
    return 0;
}

// Hàm remove được gọi khi thiết bị bị ngắt kết nối
static void aht20_remove(struct i2c_client *client)
{
    device_destroy(aht20_class, MKDEV(major_number, 0));
    class_unregister(aht20_class);
    class_destroy(aht20_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "AHT20 driver removed\n");
}

static const struct i2c_device_id aht20_id[] = {
    { "aht20", 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, aht20_id);

static struct i2c_driver aht20_driver = {
    .driver = {
        .name = DRIVER_NAME,
        .owner  = THIS_MODULE,
    },
    .probe = aht20_probe,
    .remove = aht20_remove,
    .id_table = aht20_id,
};

static int __init aht20_init(void)
{
    printk(KERN_INFO "Initializing AHT20 driver\n");
    return i2c_add_driver(&aht20_driver);
}

static void __exit aht20_exit(void)
{
    printk(KERN_INFO "Exiting AHT20 driver\n");
    i2c_del_driver(&aht20_driver);
}

module_init(aht20_init);
module_exit(aht20_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tran Quoc Dai");
MODULE_DESCRIPTION("AHT20 Measuring Temperature and Humidity Sensor Device Driver");
MODULE_VERSION("2.0");


