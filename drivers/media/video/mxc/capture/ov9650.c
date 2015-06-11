/*
 * Copyright 2005-2011 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */
//#define  DEBUG
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/regulator/consumer.h>
#include <linux/fsl_devices.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-int-device.h>
#include "mxc_v4l2_capture.h"

#include "ov9650.h"

#define OV9650_VOLTAGE_ANALOG               2500000
#define OV9650_VOLTAGE_DIGITAL_CORE         1800000
#define OV9650_VOLTAGE_DIGITAL_IO           2500000
#define OV9650_VOLTAGE_DIGITAL_GPO	    2800000

#define MIN_FPS 5
#define MAX_FPS 30
#define DEFAULT_FPS 15

#define OV9650_XCLK_MIN 6000000
#define OV9650_XCLK_MAX 27000000   //24000000





enum ov9650_mode {
	ov9650_mode_MIN = 0,
	ov9650_mode_VGA_640_480 = 0,
    ov9650_mode_QVGA_320_240 = 1,
	ov9650_mode_XGA_1024_768 = 2,
	ov9650_mode_QXGA_1280_1024 = 3,
	ov9650_mode_NTSC_720_480 = 4,
	ov9650_mode_PAL_720_576 = 5,
	ov9650_mode_MAX = 5
};

enum ov9650_frame_rate {
	ov9650_15_fps,
	ov9650_30_fps
};

struct reg_value {
	u8 u8RegAddr;
	u8 u8Val;
	u8 u8Mask;
	u32 u32Delay_ms;
};

struct ov9650_mode_info {
	enum ov9650_mode mode;
	u32 width;
	u32 height;
	struct reg_value *init_data_ptr;
	u32 init_data_size;
};

/*!
 * Maintains the information on the current state of the sesor.
 */
struct sensor {
	const struct ov9650_platform_data *platform_data;  //defined in linux/fsl_device.h
	struct v4l2_int_device *v4l2_int_device;
	struct i2c_client *i2c_client;
	struct v4l2_pix_format pix;     //defined in linux/videodev2.h
	struct v4l2_captureparm streamcap;  //defined in linux/videodev2.h
	bool on;

	/* control settings */
	int brightness;
	int hue;
	int contrast;
	int saturation;
	int red;
	int green;
	int blue;
	int ae_mode;

	u32 mclk;
	int csi;
} ov9650_data;

static struct reg_value ov9650_rotate_normal[] = {
	{0x307c, 0x10, 0x00, 0x00}, {0x3090, 0xc0, 0x00, 0x00},
};

static struct reg_value ov9650_rotate_mirror[] = {
	{0x307c, 0x12, 0x00, 0x00}, {0x3090, 0xc8, 0x00, 0x00},
};

static struct reg_value ov9650_rotate_flip[] = {
	{0x307c, 0x11, 0x00, 0x00}, {0x3090, 0xc0, 0x00, 0x00},
};

static struct reg_value ov9650_rotate_180[] = {
	{0x307c, 0x13, 0x00, 0x00}, {0x3090, 0xc8, 0x00, 0x00},
};

static struct reg_value ov9650_setting_15fps_QXGA_1280_1024[] = {
	{0x11, 0x80},
	{0x12, 0x00},
	{0x0c, 0x00},
	{0x0d, 0x00},
	{0x18, 0xbe},
	{0x17, 0x1e},
	{0x32, 0xbf},
	{0x03, 0x12},
	{0x1a, 0x81},
	{0x19, 0x01},
	{0x2a, 0x10},//24M clock
	{0x2b, 0x34},//24M clock
	{0x6a, 0x41}//24M clock
};

static struct reg_value ov9650_setting_15fps_XGA_1024_768[] = {
};
static struct reg_value ov9650_setting_30fps_XGA_1024_768[] = {
	{0x0, 0x0, 0}
};

static struct reg_value ov9650_setting_15fps_VGA_640_480[] = {
	{0x11, 0x81},
	{0x12, 0x40},
	{0x0c, 0x04},
	{0x0d, 0x80},
	{0x18, 0xc7},
	{0x17, 0x27},
	{0x32, 0xad},
	{0x03, 0x00},
	{0x1a, 0x3d},
	{0x19, 0x01},
	{0x6a, 0x3e},
	{0x11, 0x81},//for 24M clock
	{0x2a, 0x10},//for 24M clock
	{0x2b, 0x40}//for 24M clock
};

static struct reg_value ov9650_setting_30fps_VGA_640_480[] = {
};

static struct reg_value ov9650_setting_15fps_QVGA_320_240[] = {
};

static struct reg_value ov9650_setting_30fps_QVGA_320_240[] = {
};

static struct reg_value ov9650_setting_15fps_NTSC_720_480[] = {
};

static struct reg_value ov9650_setting_15fps_PAL_720_576[] = {
};

static struct reg_value ov9650_setting_30fps_NTSC_720_480[] = {
};

static struct reg_value ov9650_setting_30fps_PAL_720_576[] = {
};

static struct ov9650_mode_info ov9650_mode_info_data[2][ov9650_mode_MAX + 1] = {
	{
		{ov9650_mode_VGA_640_480,    640,  480,
		ov9650_setting_15fps_VGA_640_480,
		ARRAY_SIZE(ov9650_setting_15fps_VGA_640_480)},
               

                {ov9650_mode_QVGA_320_240,   320,  240,
		ov9650_setting_15fps_QVGA_320_240,
		ARRAY_SIZE(ov9650_setting_15fps_QVGA_320_240)},

		
		{ov9650_mode_XGA_1024_768,   1024, 768,
		ov9650_setting_15fps_XGA_1024_768,
		ARRAY_SIZE(ov9650_setting_15fps_XGA_1024_768)},
		{ov9650_mode_QXGA_1280_1024, 1280, 1024,
		ov9650_setting_15fps_QXGA_1280_1024,
		ARRAY_SIZE(ov9650_setting_15fps_QXGA_1280_1024)},
		{ov9650_mode_NTSC_720_480,   720, 480,
		ov9650_setting_15fps_NTSC_720_480,
		ARRAY_SIZE(ov9650_setting_15fps_NTSC_720_480)},
                 {ov9650_mode_PAL_720_576,    720, 576,
		ov9650_setting_15fps_PAL_720_576,
		ARRAY_SIZE(ov9650_setting_15fps_PAL_720_576) },
               
		
	},
	{

		{ov9650_mode_VGA_640_480,    640,  480,
		ov9650_setting_30fps_VGA_640_480,
		ARRAY_SIZE(ov9650_setting_30fps_VGA_640_480)},

                

                {ov9650_mode_QVGA_320_240,   320,  240,
		ov9650_setting_30fps_QVGA_320_240,
		ARRAY_SIZE(ov9650_setting_30fps_QVGA_320_240)},

		{ov9650_mode_XGA_1024_768,   1024, 768,
		ov9650_setting_30fps_XGA_1024_768,
		ARRAY_SIZE(ov9650_setting_30fps_XGA_1024_768)},
		{ov9650_mode_QXGA_1280_1024, 0, 0, NULL, 0},
		{ov9650_mode_NTSC_720_480,   720, 480,
		ov9650_setting_30fps_NTSC_720_480,
		ARRAY_SIZE(ov9650_setting_30fps_NTSC_720_480)},
                {ov9650_mode_PAL_720_576,   720, 576,
		ov9650_setting_30fps_PAL_720_576,
		ARRAY_SIZE(ov9650_setting_30fps_PAL_720_576) },
               
	},
};

static struct regulator *io_regulator;
static struct regulator *core_regulator;
static struct regulator *analog_regulator;
static struct regulator *gpo_regulator;
static struct mxc_camera_platform_data *camera_plat;

static int ov9650_probe(struct i2c_client *adapter,
				const struct i2c_device_id *device_id);
static int ov9650_remove(struct i2c_client *client);

static u8 ov9650_read_reg(u8 reg);
static u8 ov9650_write_reg(u8 reg, u8 val);

static const struct i2c_device_id ov9650_id[] = {
	{"ov9650", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, ov9650_id);

static struct i2c_driver ov9650_i2c_driver = {
	.driver = {
		  .owner = THIS_MODULE,
		  .name  = "ov9650",
		  },
	.probe  = ov9650_probe,
	.remove = ov9650_remove,
	.id_table = ov9650_id,
};

extern void gpio_sensor_active(unsigned int csi_index);
extern void gpio_sensor_inactive(unsigned int csi);

/***********************************************************************
 * I2C transfert.
 ***********************************************************************/

/*! Read one register from a OV9650 i2c slave device.
 *	@param *page	page in the device we wish to access.
 *  @param *reg		register in the device we wish to access.
 *	@return		    reg value if success, an error code otherwise.
 */
static u8 ov9650_read_reg(u8 reg)
{
	u8 au8RegBuf = 0;
	u8 u8RdVal = 0;

	au8RegBuf = reg;

	if (1 != i2c_master_send(ov9650_data.i2c_client, au8RegBuf, 1)) {
		pr_err("%s:write reg error:reg=%x\n",
				__func__, reg);
		return -1;
	}

	if (1 != i2c_master_recv(ov9650_data.i2c_client, &u8RdVal, 1)) {
		pr_err("%s:read reg error:reg=%x,val=%x\n",
				__func__, reg, u8RdVal);
		return -1;
	}

	return u8RdVal;
}

/*! Write one register of a OV9650 i2c slave device.
 *	@param *page	page in the device we wish to access.
 *  @param *reg		register in the device we wish to access.
 *	@param *reg		value we wish to wirte.
 *  @return		    0 if success, an error code otherwise.
 */
static u8 ov9650_write_reg(u8 reg, u8 val)
{
	u8 au8Buf[2] = {0};

	au8Buf[0] = reg;
	au8Buf[1] = val;

    if(reg == CHIP_DELAY)
    {
        msleep(val);
        goto ret;
    }

	if (i2c_master_send(ov9650_data.i2c_client, au8Buf, 2) < 0) {
		pr_err("%s:write reg error:reg=%x,val=%x\n",
			__func__, reg, val);
		return -1;
	}

	ret : 
        return 0;
}

///////////////////////////////////////////////////////////////////

 
void  OV9650_config(void)
{
	int i;

	for (i = 0; i < OV9650_REGS0; i++) {
		
			ov9650_write_reg(ov9650_reg0[i].subaddr
					, ov9650_reg0[i].value);
	}
	
}


static int ov9650_set_rotate_mode(struct reg_value *rotate_mode)
{
/*
    s32 i = 0;
	s32 iModeSettingArySize = \
	sizeof(ov9650_rotate_normal)/sizeof(ov9650_rotate_normal[0]);
	register u32 Delay_ms = 0;
	register u16 RegAddr = 0;
	register u8 Mask = 0;
	register u8 Val = 0;
	u8 RegVal = 0;
	int retval = 0;
	for (i = 0; i < iModeSettingArySize; ++i, ++rotate_mode) {
		Delay_ms = rotate_mode->u32Delay_ms;
		RegAddr = rotate_mode->u16RegAddr;
		Val = rotate_mode->u8Val;
		Mask = rotate_mode->u8Mask;
		if (Mask) {
			retval = ov3640_read_reg(RegAddr, RegVal);
			if (retval < 0)
				goto err;

			RegVal &= ~(u8)Mask;
			Val &= Mask;
			Val |= RegVal;
		}

		retval = ov3640_write_reg(RegAddr, Val);
		if (retval < 0)
			goto err;

		if (Delay_ms)
			msleep(Delay_ms);
	}
err:
	return retval;
*/
    printk("ov9650_set_rotate_mode not in use\n");
    return 0;
}

static int ov9650_init_mode(enum ov9650_frame_rate frame_rate,
			    enum ov9650_mode mode)
{

	struct reg_value *pModeSetting = NULL;
	s32 i = 0;
	s32 iModeSettingArySize = 0;
	register u32 Delay_ms = 0;
	register u16 RegAddr = 0;
	register u8 Mask = 0;
	register u8 Val = 0;
	u8 RegVal = 0;
	int retval = 0;

	u8 temp_reg1, temp_reg2, gain;
	u16 preview_shutter, preview_extra_lines, preview_gain, capture_exposure;
	u16 capture_line_width, capture_banding_filter, capture_gain, capture_maximum_shutter;
	u16 capture_gain16, capture_pclk_frequency, capture_max_gain16;
	u32 gain_exposure;

	if (mode > ov9650_mode_MAX || mode < ov9650_mode_MIN) {
		pr_err("Wrong ov9650 mode detected!\n");
		return -1;
	}
	if (mode == ov9650_mode_QXGA_1280_1024 && frame_rate == ov9650_15_fps) {
		/*
        //stop AEC/AGC 
		ov3640_read_reg(0x3013, &temp_reg1);
		temp_reg1 = temp_reg1 & 0xfa;
		ov3640_write_reg(0x3013, temp_reg1);

		//read out preview_shutter
		ov3640_read_reg(0x3003, &temp_reg1);
		ov3640_read_reg(0x3002, &temp_reg2);
		preview_shutter = (temp_reg1) | (temp_reg2 << 8);
		//read out preview_extra_lines
		ov3640_read_reg(0x302E, &temp_reg1);
		ov3640_read_reg(0x302D, &temp_reg2);
		preview_extra_lines = (temp_reg1) | (temp_reg2 << 8);

		//read out preview_gain
		ov3640_read_reg(0x3001, &temp_reg1);
		preview_gain = (((temp_reg1 & 0xF0) >> 4) + 1) * (16 + (temp_reg1 & 0x0F));
        */
	}

	pModeSetting = ov9650_mode_info_data[frame_rate][mode].init_data_ptr;
	iModeSettingArySize =
		ov9650_mode_info_data[frame_rate][mode].init_data_size;

	ov9650_data.pix.width = ov9650_mode_info_data[frame_rate][mode].width;
	ov9650_data.pix.height = ov9650_mode_info_data[frame_rate][mode].height;

	if (ov9650_data.pix.width == 0 || ov9650_data.pix.height == 0 ||
	    pModeSetting == NULL || iModeSettingArySize == 0)
		return -EINVAL;

	for (i = 0; i < iModeSettingArySize; ++i, ++pModeSetting) {
		RegAddr = pModeSetting->u8RegAddr;
		Val = pModeSetting->u8Val;

		retval = ov9650_write_reg(RegAddr, Val);
		if (retval < 0)
			goto err;
	}
	if (mode == ov9650_mode_QXGA_1280_1024 && frame_rate == ov9650_15_fps) {
		capture_max_gain16 = 64;
	
        /*
        //Set Capture_Maximum_Shutter
		capture_maximum_shutter = 1640;

		capture_line_width = 2376;

		capture_pclk_frequency = 28;

		// 50Hz banding filter calculate
		capture_banding_filter = capture_pclk_frequency * 1000000 / 100 / (2 * capture_line_width);

		//capture exposure calculate
		capture_exposure = (preview_shutter + preview_extra_lines) / 2;
		//redistribute gain and exposure
		gain_exposure =  preview_gain * capture_exposure;

		if (gain_exposure < capture_banding_filter * 16) {
			capture_exposure = gain_exposure / 16;
			if (0 == capture_exposure) {
				capture_exposure = 1;
			}
			capture_gain = (gain_exposure*2 + 1) / capture_exposure / 2;
		} else {
			if (gain_exposure > capture_maximum_shutter * 16) {
				capture_exposure = capture_maximum_shutter;
				capture_gain = (gain_exposure*2 + 1)/capture_maximum_shutter/2;
				if (capture_gain > capture_max_gain16) {
					capture_exposure = gain_exposure*11/10/capture_max_gain16;
					if (0 == capture_exposure) {
						capture_exposure = 1;
					}
					capture_exposure = capture_exposure / capture_banding_filter;
					capture_exposure = capture_exposure * capture_banding_filter;
					capture_gain = (gain_exposure*2 + 1) / capture_exposure/2;
				}
			} else {
				capture_exposure = gain_exposure / 16 / capture_banding_filter;
				capture_exposure = capture_exposure * capture_banding_filter;
				if (0 == capture_exposure) {
					capture_exposure = 1;
				}
				capture_gain = (gain_exposure*2 + 1) / capture_exposure/2;
			}
		}
		capture_gain = capture_gain * 500/2048;
		capture_gain16 = capture_gain * 16;

		//write back Capture_Exposure
		ov960_write_reg(0x3003, capture_exposure & 0xFF);
		ov9650_write_reg(0x3002, (capture_exposure & 0xFF00) >> 8);

		if (capture_gain16 > 31) {
			capture_gain16 = capture_gain16/2;
			gain = 0x10;
		}
		if (capture_gain16 > 31) {
			capture_gain16 = capture_gain16/2;
			gain = gain | 0x20;
		}
		if (capture_gain16 > 31) {
			capture_gain16 = capture_gain16/2;
			gain = gain | 0x40;
		}
		if (capture_gain16 > 31) {
			capture_gain16 = capture_gain16/2;
			gain = gain | 0x80;
		}
		if (capture_gain16 > 16) {
			gain = gain | (capture_gain16 - 16);
		}
		ov9650_write_reg(0x3001, gain);
		ov9650_read_reg(0x3021, &gain);
		ov9650_read_reg(0x3023, &gain);

        */
	}

err:
	return retval;

//    printk("ov9650_init_mode not in use\n");
//    return 0;
}

/* --------------- IOCTL functions from v4l2_int_ioctl_desc --------------- */

static int ioctl_g_ifparm(struct v4l2_int_device *s, struct v4l2_ifparm *p)
{
        pr_debug("ioctl_g_ifparm\n");
	if (s == NULL) {
		pr_err("   ERROR!! no slave device set!\n");
		return -1;
	}
        
	memset(p, 0, sizeof(*p));
	p->u.bt656.clock_curr = ov9650_data.mclk;
	pr_debug("   clock_curr=mclk=%d\n", ov9650_data.mclk);
	p->if_type = V4L2_IF_TYPE_BT656;
	p->u.bt656.mode = V4L2_IF_TYPE_BT656_MODE_NOBT_8BIT;
	p->u.bt656.clock_min = OV9650_XCLK_MIN;
	p->u.bt656.clock_max = OV9650_XCLK_MAX;
	p->u.bt656.bt_sync_correct = 1;  /* Indicate external vsync */

	return 0;
}

/*!
 * ioctl_s_power - V4L2 sensor interface handler for VIDIOC_S_POWER ioctl
 * @s: pointer to standard V4L2 device structure
 * @on: indicates power mode (on or off)
 *
 * Turns the power on or off, depending on the value of on and returns the
 * appropriate error code.
 */
static int ioctl_s_power(struct v4l2_int_device *s, int on)
{
	struct sensor *sensor = s->priv;

	if (on && !sensor->on) {
		gpio_sensor_active(ov9650_data.csi);
		if (io_regulator)
			if (regulator_enable(io_regulator) != 0)
				return -EIO;
		if (core_regulator)
			if (regulator_enable(core_regulator) != 0)
				return -EIO;
		if (gpo_regulator)
			if (regulator_enable(gpo_regulator) != 0)
				return -EIO;
		if (analog_regulator)
			if (regulator_enable(analog_regulator) != 0)
				return -EIO;
		/* Make sure power on */
		if (camera_plat->pwdn)
			camera_plat->pwdn(0);

	} else if (!on && sensor->on) {
		if (analog_regulator)
			regulator_disable(analog_regulator);
		if (core_regulator)
			regulator_disable(core_regulator);
		if (io_regulator)
			regulator_disable(io_regulator);
		if (gpo_regulator)
			regulator_disable(gpo_regulator);
		gpio_sensor_inactive(ov9650_data.csi);
	}

	sensor->on = on;

	return 0;
}

/*!
 * ioctl_g_parm - V4L2 sensor interface handler for VIDIOC_G_PARM ioctl
 * @s: pointer to standard V4L2 device structure
 * @a: pointer to standard V4L2 VIDIOC_G_PARM ioctl structure
 *
 * Returns the sensor's video CAPTURE parameters.
 */
static int ioctl_g_parm(struct v4l2_int_device *s, struct v4l2_streamparm *a)
{
	struct sensor *sensor = s->priv;
	struct v4l2_captureparm *cparm = &a->parm.capture;
	int ret = 0;

	switch (a->type) {
	/* This is the only case currently handled. */
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		memset(a, 0, sizeof(*a));
		a->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		cparm->capability = sensor->streamcap.capability;
		//cparm->timeperframe = sensor->streamcap.timeperframe;
                cparm->timeperframe.denominator = 5;
                cparm->timeperframe.numerator = 1;
		cparm->capturemode = sensor->streamcap.capturemode;
		ret = 0;
		break;

	/* These are all the possible cases. */
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		ret = -EINVAL;
		break;

	default:
		pr_debug("   type is unknown - %d\n", a->type);
		ret = -EINVAL;
		break;
	}

	return ret;
}

/*!
 * ioctl_s_parm - V4L2 sensor interface handler for VIDIOC_S_PARM ioctl
 * @s: pointer to standard V4L2 device structure
 * @a: pointer to standard V4L2 VIDIOC_S_PARM ioctl structure
 *
 * Configures the sensor to use the input parameters, if possible.  If
 * not possible, reverts to the old parameters and returns the
 * appropriate error code.
 */
static int ioctl_s_parm(struct v4l2_int_device *s, struct v4l2_streamparm *a)
{
	struct sensor *sensor = s->priv;
	struct v4l2_fract *timeperframe = &a->parm.capture.timeperframe;
	u32 tgt_fps;	/* target frames per secound */
	enum ov9650_frame_rate frame_rate;
	int ret = 0;

	/* Make sure power on */
	if (camera_plat->pwdn)
		camera_plat->pwdn(0);

	switch (a->type) {
	/* This is the only case currently handled. */
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		/* Check that the new frame rate is allowed. */
		if ((timeperframe->numerator == 0) ||
		    (timeperframe->denominator == 0)) {
			timeperframe->denominator = DEFAULT_FPS;
			timeperframe->numerator = 1;
		}

		tgt_fps = timeperframe->denominator /
			  timeperframe->numerator;

		if (tgt_fps > MAX_FPS) {
			timeperframe->denominator = MAX_FPS;
			timeperframe->numerator = 1;
		} else if (tgt_fps < MIN_FPS) {
			timeperframe->denominator = MIN_FPS;
			timeperframe->numerator = 1;
		}

		/* Actual frame rate we use */
		tgt_fps = timeperframe->denominator /
			  timeperframe->numerator;

		if (tgt_fps == 15)
			frame_rate = ov9650_15_fps;
		else if (tgt_fps == 30)
			frame_rate = ov9650_30_fps;
		else {
			pr_err(" The camera frame rate is not supported!\n");
			return -EINVAL;
		}

		sensor->streamcap.timeperframe = *timeperframe;
		sensor->streamcap.capturemode =
				(u32)a->parm.capture.capturemode;

		ret = ov9650_init_mode(frame_rate,
				       sensor->streamcap.capturemode);
		break;

	/* These are all the possible cases. */
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		pr_debug("   type is not " \
			"V4L2_BUF_TYPE_VIDEO_CAPTURE but %d\n",
			a->type);
		ret = -EINVAL;
		break;

	default:
		pr_debug("   type is unknown - %d\n", a->type);
		ret = -EINVAL;
		break;
	}

	return ret;
}

/*!
 * ioctl_g_fmt_cap - V4L2 sensor interface handler for ioctl_g_fmt_cap
 * @s: pointer to standard V4L2 device structure
 * @f: pointer to standard V4L2 v4l2_format structure
 *
 * Returns the sensor's current pixel format in the v4l2_format
 * parameter.
 */
static int ioctl_g_fmt_cap(struct v4l2_int_device *s, struct v4l2_format *f)
{
	struct sensor *sensor = s->priv;

	f->fmt.pix = sensor->pix;

	return 0;
}

/*!
 * ioctl_g_ctrl - V4L2 sensor interface handler for VIDIOC_G_CTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @vc: standard V4L2 VIDIOC_G_CTRL ioctl structure
 *
 * If the requested control is supported, returns the control's current
 * value from the video_control[] array.  Otherwise, returns -EINVAL
 * if the control is not supported.
 */
static int ioctl_g_ctrl(struct v4l2_int_device *s, struct v4l2_control *vc)
{
	int ret = 0;

	switch (vc->id) {
	case V4L2_CID_BRIGHTNESS:
		vc->value = ov9650_data.brightness;
		break;
	case V4L2_CID_HUE:
		vc->value = ov9650_data.hue;
		break;
	case V4L2_CID_CONTRAST:
		vc->value = ov9650_data.contrast;
		break;
	case V4L2_CID_SATURATION:
		vc->value = ov9650_data.saturation;
		break;
	case V4L2_CID_RED_BALANCE:
		vc->value = ov9650_data.red;
		break;
	case V4L2_CID_BLUE_BALANCE:
		vc->value = ov9650_data.blue;
		break;
	case V4L2_CID_EXPOSURE:
		vc->value = ov9650_data.ae_mode;
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

/*!
 * ioctl_s_ctrl - V4L2 sensor interface handler for VIDIOC_S_CTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @vc: standard V4L2 VIDIOC_S_CTRL ioctl structure
 *
 * If the requested control is supported, sets the control's current
 * value in HW (and updates the video_control[] array).  Otherwise,
 * returns -EINVAL if the control is not supported.
 */
static int ioctl_s_ctrl(struct v4l2_int_device *s, struct v4l2_control *vc)
{
	int retval = 0;

	pr_debug("In ov9650:ioctl_s_ctrl %d\n",
		 vc->id);

	switch (vc->id) {
	case V4L2_CID_BRIGHTNESS:
		break;
	case V4L2_CID_CONTRAST:
		break;
	case V4L2_CID_SATURATION:
		break;
	case V4L2_CID_HUE:
		break;
	case V4L2_CID_AUTO_WHITE_BALANCE:
		break;
	case V4L2_CID_DO_WHITE_BALANCE:
		break;
	case V4L2_CID_RED_BALANCE:
		break;
	case V4L2_CID_BLUE_BALANCE:
		break;
	case V4L2_CID_GAMMA:
		break;
	case V4L2_CID_EXPOSURE:
		break;
	case V4L2_CID_AUTOGAIN:
		break;
	case V4L2_CID_GAIN:
		break;
	case V4L2_CID_HFLIP:
		break;
	case V4L2_CID_VFLIP:
		break;
	case V4L2_CID_MXC_ROT:
	case V4L2_CID_MXC_VF_ROT:
		switch (vc->value) {
		case V4L2_MXC_CAM_ROTATE_NONE:
			if (ov9650_set_rotate_mode(ov9650_rotate_normal))
				retval = -EPERM;
			break;
		case V4L2_MXC_CAM_ROTATE_VERT_FLIP:
			if (ov9650_set_rotate_mode(ov9650_rotate_flip))
				retval = -EPERM;
			break;
		case V4L2_MXC_CAM_ROTATE_HORIZ_FLIP:
			if (ov9650_set_rotate_mode(ov9650_rotate_mirror))
				retval = -EPERM;
			break;
		case V4L2_MXC_CAM_ROTATE_180:
			if (ov9650_set_rotate_mode(ov9650_rotate_180))
				retval = -EPERM;
			break;
		default:
			retval = -EPERM;
			break;
		}
		break;
	default:
		retval = -EPERM;
		break;
	}

	return retval;
}

/*!
 * ioctl_enum_framesizes - V4L2 sensor interface handler for
 *			   VIDIOC_ENUM_FRAMESIZES ioctl
 * @s: pointer to standard V4L2 device structure
 * @fsize: standard V4L2 VIDIOC_ENUM_FRAMESIZES ioctl structure
 *
 * Return 0 if successful, otherwise -EINVAL.
 */
static int ioctl_enum_framesizes(struct v4l2_int_device *s,
				 struct v4l2_frmsizeenum *fsize)
{
	if (fsize->index > ov9650_mode_MAX)
		return -EINVAL;

	fsize->pixel_format = ov9650_data.pix.pixelformat;
	fsize->discrete.width =max(ov9650_mode_info_data[0][fsize->index].width,
			   ov9650_mode_info_data[1][fsize->index].width);
	fsize->discrete.height =max(ov9650_mode_info_data[0][fsize->index].height,
			    ov9650_mode_info_data[1][fsize->index].height);
	return 0;
}

/*!
 * ioctl_g_chip_ident - V4L2 sensor interface handler for
 *			VIDIOC_DBG_G_CHIP_IDENT ioctl
 * @s: pointer to standard V4L2 device structure
 * @id: pointer to int
 *
 * Return 0.
 */
static int ioctl_g_chip_ident(struct v4l2_int_device *s, int *id)
{
	((struct v4l2_dbg_chip_ident *)id)->match.type =
					V4L2_CHIP_MATCH_I2C_DRIVER;
	strcpy(((struct v4l2_dbg_chip_ident *)id)->match.name, "ov3640_camera");

	return 0;
}

/*!
 * ioctl_init - V4L2 sensor interface handler for VIDIOC_INT_INIT
 * @s: pointer to standard V4L2 device structure
 */
static int ioctl_init(struct v4l2_int_device *s)
{
	return 0;
}

/*!
 * ioctl_enum_fmt_cap - V4L2 sensor interface handler for VIDIOC_ENUM_FMT
 * @s: pointer to standard V4L2 device structure
 * @fmt: pointer to standard V4L2 fmt description structure
 *
 * Return 0.
 */
static int ioctl_enum_fmt_cap(struct v4l2_int_device *s,
			      struct v4l2_fmtdesc *fmt)
{
	if (fmt->index > ov9650_mode_MAX)
		return -EINVAL;

	fmt->pixelformat = ov9650_data.pix.pixelformat;

	return 0;
}

/*!
 * ioctl_dev_init - V4L2 sensor interface handler for vidioc_int_dev_init_num
 * @s: pointer to standard V4L2 device structure
 *
 * Initialise the device when slave attaches to the master.
 */
static int ioctl_dev_init(struct v4l2_int_device *s)
{
	struct sensor *sensor = s->priv;
	u32 tgt_xclk;	/* target xclk */
	u32 tgt_fps;	/* target frames per secound */
	enum ov9650_frame_rate frame_rate;

	gpio_sensor_active(ov9650_data.csi);
	ov9650_data.on = true;

	/* mclk */
	tgt_xclk = ov9650_data.mclk;
	tgt_xclk = min(tgt_xclk, (u32)OV9650_XCLK_MAX);
	tgt_xclk = max(tgt_xclk, (u32)OV9650_XCLK_MIN);
	ov9650_data.mclk = tgt_xclk;

	pr_debug("   Setting mclk to %d MHz\n", tgt_xclk / 1000000);
	set_mclk_rate(&ov9650_data.mclk, ov9650_data.csi);

	/* Default camera frame rate is set in probe */
	tgt_fps = sensor->streamcap.timeperframe.denominator /
		  sensor->streamcap.timeperframe.numerator;
        pr_debug("   tgt_fps= %d \n", tgt_fps);
	if (tgt_fps == 15)
		frame_rate = ov9650_15_fps;
	else if (tgt_fps == 30)
		frame_rate = ov9650_30_fps;
	else
		return -EINVAL; /* Only support 15fps or 30fps now. */
        pr_debug("streamcap.capturemode= %d \n", sensor->streamcap.capturemode);
	return ov9650_init_mode(frame_rate,
				sensor->streamcap.capturemode);
}

/*!
 * ioctl_dev_exit - V4L2 sensor interface handler for vidioc_int_dev_exit_num
 * @s: pointer to standard V4L2 device structure
 *
 * Delinitialise the device when slave detaches to the master.
 */
static int ioctl_dev_exit(struct v4l2_int_device *s)
{
	gpio_sensor_inactive(ov9650_data.csi);

	return 0;
}

/*!
 * This structure defines all the ioctls for this module and links them to the
 * enumeration.
 */
static struct v4l2_int_ioctl_desc ov9650_ioctl_desc[] = {
	{vidioc_int_dev_init_num, (v4l2_int_ioctl_func*)ioctl_dev_init},
	{vidioc_int_dev_exit_num, ioctl_dev_exit},
	{vidioc_int_s_power_num, (v4l2_int_ioctl_func*)ioctl_s_power},
	{vidioc_int_g_ifparm_num, (v4l2_int_ioctl_func*)ioctl_g_ifparm},
/*	{vidioc_int_g_needs_reset_num,
				(v4l2_int_ioctl_func *)ioctl_g_needs_reset}, */
/*	{vidioc_int_reset_num, (v4l2_int_ioctl_func *)ioctl_reset}, */
	{vidioc_int_init_num, (v4l2_int_ioctl_func*)ioctl_init},
	{vidioc_int_enum_fmt_cap_num,
				(v4l2_int_ioctl_func *)ioctl_enum_fmt_cap},
/*	{vidioc_int_try_fmt_cap_num,
				(v4l2_int_ioctl_func *)ioctl_try_fmt_cap}, */
	{vidioc_int_g_fmt_cap_num, (v4l2_int_ioctl_func*)ioctl_g_fmt_cap},
/*	{vidioc_int_s_fmt_cap_num, (v4l2_int_ioctl_func *)ioctl_s_fmt_cap}, */
	{vidioc_int_g_parm_num, (v4l2_int_ioctl_func*)ioctl_g_parm},
	{vidioc_int_s_parm_num, (v4l2_int_ioctl_func*)ioctl_s_parm},
/*	{vidioc_int_queryctrl_num, (v4l2_int_ioctl_func *)ioctl_queryctrl}, */
	{vidioc_int_g_ctrl_num, (v4l2_int_ioctl_func*)ioctl_g_ctrl},
	{vidioc_int_s_ctrl_num, (v4l2_int_ioctl_func*)ioctl_s_ctrl},
	{vidioc_int_enum_framesizes_num,
				(v4l2_int_ioctl_func *)ioctl_enum_framesizes},
	{vidioc_int_g_chip_ident_num,
				(v4l2_int_ioctl_func *)ioctl_g_chip_ident},
};

static struct v4l2_int_slave ov9650_slave = {
	.ioctls = ov9650_ioctl_desc,
	.num_ioctls = ARRAY_SIZE(ov9650_ioctl_desc),
};

static struct v4l2_int_device ov9650_int_device = {  //defined in /media/v4l2int-device.h
	.module = THIS_MODULE,
	.name = "ov9650",
	.type = v4l2_int_type_slave,
	.u = {
		.slave = &ov9650_slave,
	},
};

/*!
 * ov9650 I2C probe function
 *
 * @param adapter            struct i2c_adapter *
 * @return  Error code indicating success or failure
 */
static int ov9650_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	int retval;
	struct mxc_camera_platform_data *plat_data = client->dev.platform_data;
    //defined in linux/fsl_device.h    
       
	/* Set initial values for the sensor struct. */
	memset(&ov9650_data, 0, sizeof(ov9650_data));
	ov9650_data.mclk = 25000000; /* 6 - 54 MHz, typical 24MHz  Xrock*/
	ov9650_data.mclk = plat_data->mclk;
	ov9650_data.csi = plat_data->csi;

	ov9650_data.i2c_client = client;
	ov9650_data.pix.pixelformat = V4L2_PIX_FMT_UYVY;
	ov9650_data.pix.width = 640;    //Xrock
	ov9650_data.pix.height = 480;  //Xrock
	ov9650_data.streamcap.capability = V4L2_MODE_HIGHQUALITY |
					   V4L2_CAP_TIMEPERFRAME;
	ov9650_data.streamcap.capturemode = 0;
	ov9650_data.streamcap.timeperframe.denominator = DEFAULT_FPS; 
	ov9650_data.streamcap.timeperframe.numerator = 1;

	if (plat_data->io_regulator) {
		io_regulator = regulator_get(&client->dev,
					     plat_data->io_regulator);
		if (!IS_ERR(io_regulator)) {
			regulator_set_voltage(io_regulator,
					      OV9650_VOLTAGE_DIGITAL_IO,
					      OV9650_VOLTAGE_DIGITAL_IO);
			if (regulator_enable(io_regulator) != 0) {
				pr_err("%s:io set voltage error\n", __func__);
				goto err1;
			} else {
				dev_dbg(&client->dev,
					"%s:io set voltage ok\n", __func__);
			}
		} else
			io_regulator = NULL;
	}

	if (plat_data->core_regulator) {
		core_regulator = regulator_get(&client->dev,
					       plat_data->core_regulator);
		if (!IS_ERR(core_regulator)) {
			regulator_set_voltage(core_regulator,
					      OV9650_VOLTAGE_DIGITAL_CORE,
					      OV9650_VOLTAGE_DIGITAL_CORE);
			if (regulator_enable(core_regulator) != 0) {
				pr_err("%s:core set voltage error\n", __func__);
				goto err2;
			} else {
				dev_dbg(&client->dev,
					"%s:core set voltage ok\n", __func__);
			}
		} else
			core_regulator = NULL;
	}

	if (plat_data->analog_regulator) {
		analog_regulator = regulator_get(&client->dev,
						 plat_data->analog_regulator);
		if (!IS_ERR(analog_regulator)) {
			regulator_set_voltage(analog_regulator,
					      OV9650_VOLTAGE_ANALOG,
					      OV9650_VOLTAGE_ANALOG);
			if (regulator_enable(analog_regulator) != 0) {
				pr_err("%s:analog set voltage error\n",
					__func__);
				goto err3;
			} else {
				dev_dbg(&client->dev,
					"%s:analog set voltage ok\n", __func__);
			}
		} else
			analog_regulator = NULL;
	}

	if (plat_data->gpo_regulator) {
		gpo_regulator = regulator_get(&client->dev,
					      plat_data->gpo_regulator);
		if (!IS_ERR(gpo_regulator)) {
			regulator_set_voltage(gpo_regulator,
					      OV9650_VOLTAGE_DIGITAL_GPO,
					      OV9650_VOLTAGE_DIGITAL_GPO);
			if (regulator_enable(gpo_regulator) != 0) {
				pr_err("%s:gpo enable error\n", __func__);
				goto err4;
			} else {
				dev_dbg(&client->dev,
					"%s:gpo enable ok\n", __func__);
			}
		} else
			gpo_regulator = NULL;
	}

	if (plat_data->pwdn)
		plat_data->pwdn(0);

	camera_plat = plat_data;

	ov9650_int_device.priv = &ov9650_data;
	retval = v4l2_int_device_register(&ov9650_int_device);

	return retval;

err4:
	if (analog_regulator) {
		regulator_disable(analog_regulator);
		regulator_put(analog_regulator);
	}
err3:
	if (core_regulator) {
		regulator_disable(core_regulator);
		regulator_put(core_regulator);
	}
err2:
	if (io_regulator) {
		regulator_disable(io_regulator);
		regulator_put(io_regulator);
	}
err1:
	return -1;
}

/*!
 * ov9650 I2C detach function
 *
 * @param client            struct i2c_client *
 * @return  Error code indicating success or failure
 */
static int ov9650_remove(struct i2c_client *client)
{
	v4l2_int_device_unregister(&ov9650_int_device);

	if (gpo_regulator) {
		regulator_disable(gpo_regulator);
		regulator_put(gpo_regulator);
	}

	if (analog_regulator) {
		regulator_disable(analog_regulator);
		regulator_put(analog_regulator);
	}

	if (core_regulator) {
		regulator_disable(core_regulator);
		regulator_put(core_regulator);
	}

	if (io_regulator) {
		regulator_disable(io_regulator);
		regulator_put(io_regulator);
	}

	return 0;
}

/*!
 * ov9650 init function
 * Called by insmod ov9650_camera.ko.
 *
 * @return  Error code indicating success or failure
 */
static __init int ov9650_init(void)
{
	u8 err;

	err = i2c_add_driver(&ov9650_i2c_driver);
        
	if (err != 0)
		pr_err("%s:driver registration failed, error=%d \n",
			__func__, err);
			
	 OV9650_config();
     printk("********************Xrock:i2c init config done*****************\n");
        
	return err;
}

/*!
 * OV9650 cleanup function
 * Called on rmmod ov9650_camera.ko
 *
 * @return  Error code indicating success or failure
 */
static void __exit ov9650_clean(void)
{
	i2c_del_driver(&ov9650_i2c_driver);
}

module_init(ov9650_init);
module_exit(ov9650_clean);

MODULE_AUTHOR("Freescale Semiconductor, Inc.");
MODULE_DESCRIPTION("OV9650 Camera Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_ALIAS("CSI");
