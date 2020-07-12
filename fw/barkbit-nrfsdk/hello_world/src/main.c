/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <drivers/sensor.h>
#include <sys/printk.h>
#include <stdio.h>
#include <drivers/i2c.h>
#include <logging/log.h>
//#define MMA8451Q DT_NODELABEL(mma8451q)
#define I2C_DEV DT_LABEL(DT_ALIAS(i2c_0))
void main(void)
{
	printk("Hello from MAIN! \n");
	struct sensor_value accel[3];
	struct device *dev = device_get_binding(I2C_DEV);

	if (dev == NULL)
	{
		printk("Could not get fxos8700 device\n");
		return;
	}
	printk("Hello from got it! \n");

	u32_t i2c_cfg = I2C_SPEED_SET(I2C_SPEED_STANDARD) | I2C_MODE_MASTER;
	if (i2c_configure(dev, i2c_cfg))
	{
		printk("I2C: config failed\n");
		return;
	}

	printk("Gah.\n");
	uint8_t buf[] = {42};
	int ret = i2c_write(dev, buf, sizeof(buf), 0x1c);
	if (ret > 0)
	{
		printk("found the thing");
	}
	else
	{
		printk("didn't found the thing");
	}

while(true)
{
	printk("Loop.\n");
	for (u8_t i = 4; i <= 0x77; i++)
	{
		uint8_t buf[] = {42};
		int ret = i2c_write(dev, buf, sizeof(buf), i);
		if (ret > 0)
		{
			printk("0x%2x FOUND\n", i);
		}
		else
		{
			printk("0x%2x NOT FOUND\n", i);
		}
	}

	k_msleep(1000);
}
	return;

	struct sensor_value attr = {
		.val1 = 6,
		.val2 = 250000,
	};

	if (sensor_attr_set(dev, SENSOR_CHAN_ALL,
						SENSOR_ATTR_SAMPLING_FREQUENCY, &attr))
	{
		printk("Could not set sampling frequency\n");
		return;
	}

	while (1)
	{
		sensor_channel_get(dev, SENSOR_CHAN_ACCEL_XYZ, accel);
		/* Print accel x,y,z data */
		printf("AX=%10.6f AY=%10.6f AZ=%10.6f ",
			   sensor_value_to_double(&accel[0]),
			   sensor_value_to_double(&accel[1]),
			   sensor_value_to_double(&accel[2]));

		printk("Hello World! %s\n", CONFIG_BOARD);
		k_msleep(1000);
	}
}
