#include <zephyr.h>
#include <drivers/sensor.h>
#include <sys/printk.h>
#include <stdio.h>
#include <drivers/i2c.h>
#include <logging/log.h>
#include <drivers/gpio.h>

//#define MMA8451Q DT_NODELABEL(mma8451q)
#define I2C_DEV DT_LABEL(DT_ALIAS(i2c_0))
#define GPIO DT_LABEL(DT_ALIAS(gpio_0))
void main(void)
{
	printk("Hello from MAIN! \n");
	//struct sensor_value accel[3];
	struct device *gpio = device_get_binding(GPIO);
	if (gpio == NULL)
	{
		printk("Could not get gpio device\n");
		return;
	}

	gpio_pin_configure(gpio, 0, GPIO_INPUT | GPIO_PULL_UP);
	gpio_pin_configure(gpio, 1, GPIO_INPUT | GPIO_PULL_UP);

	//gpio_pin_configure(gpio, 0, GPIO_INPUT);
	//gpio_pin_configure(gpio, 1, GPIO_INPUT);

/*
	struct device *mma = device_get_binding("MMA8451Q");

	if (mma == NULL)
	{
		printk("Could not get fxos8700 device\n");
		return;
	}
*/
	struct device *i2c = device_get_binding(I2C_DEV);

	if (i2c == NULL)
	{
		printk("Could not get i2c device\n");
		return;
	}
	printk("Hello from got it! \n");

	u32_t i2c_cfg = I2C_SPEED_SET(I2C_SPEED_STANDARD) | I2C_MODE_MASTER;
	if (i2c_configure(i2c, i2c_cfg))
	{
		printk("I2C: config failed\n");
		return;
	}

	printk("Gah.\n");
#define MMA8451_REG_WHOAMI 0x0D

	uint8_t write[] = {MMA8451_REG_WHOAMI};
	uint8_t buf[] = {0};
	int ret = i2c_write_read(i2c, 0x1c, write, 1, buf, 1);
	if (ret == 0)
	{
		printk("Write succeeded.\n");
		printk("Response: [%01x].\n", *buf);
	}
	return;

	struct sensor_value attr = {
		.val1 = 6,
		.val2 = 250000,
	};
/*
	if (sensor_attr_set(mma, SENSOR_CHAN_ALL,
						SENSOR_ATTR_SAMPLING_FREQUENCY, &attr))
	{
		printk("Could not set sampling frequency\n");
		return;
	}

	while (1)
	{
		sensor_channel_get(mma, SENSOR_CHAN_ACCEL_XYZ, accel);
		printk("AX=%10.6f AY=%10.6f AZ=%10.6f ",
			   sensor_value_to_double(&accel[0]),
			   sensor_value_to_double(&accel[1]),
			   sensor_value_to_double(&accel[2]));

		k_msleep(1000);
	}
*/
}
