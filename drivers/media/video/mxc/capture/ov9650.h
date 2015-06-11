#ifndef _OV9650_H_
#define _OV9650_H_

#define CHIP_DELAY 0xff
#define DELAY_TIME 3

/* Register definitions */
#define	OV9650_GAIN	0x00
#define	OV9650_BLUE	0x01
#define	OV9650_RED	0x02
#define	OV9650_VFER	0x03
#define	OV9650_COM1	0x04
#define	OV9650_BAVE	0x05
#define	OV9650_GEAVE	0x06
#define	OV9650_RSID	0x07
#define	OV9650_RAVE	0x08
#define	OV9650_COM2	0x09
#define	OV9650_PID	0x0a
#define	OV9650_VER	0x0b
#define	OV9650_COM3	0x0c
#define	OV9650_COM4	0x0d
#define	OV9650_COM5	0x0e
#define	OV9650_COM6	0x0f
#define	OV9650_AECH	0x10
#define	OV9650_CLKRC	0x11
#define	OV9650_COM7	0x12
#define	OV9650_COM8	0x13
#define	OV9650_COM9	0x14
#define	OV9650_COM10	0x15
/* 0x16 - RESERVED */
#define	OV9650_HSTART	0x17
#define	OV9650_HSTOP	0x18
#define	OV9650_VSTART	0x19
#define	OV9650_VSTOP	0x1a
#define	OV9650_PSHFT	0x1b
#define	OV9650_MIDH	0x1c
#define	OV9650_MIDL	0x1d
#define	OV9650_MVFP	0x1e
#define	OV9650_LAEC	0x1f
#define	OV9650_BOS	0x20
#define	OV9650_GBOS	0x21
#define	OV9650_GROS	0x22
#define	OV9650_ROS	0x23
#define	OV9650_AEW	0x24
#define	OV9650_AEB	0x25
#define	OV9650_VPT	0x26
#define	OV9650_BBIAS	0x27
#define	OV9650_GBBIAS	0x28
/* 0x29 - RESERVED   for 9650 Gr_COM*/
#define	OV9650_EXHCH	0x2a
#define	OV9650_EXHCL	0x2b
#define	OV9650_RBIAS	0x2c
#define	OV9650_ADVFL	0x2d
#define	OV9650_ADVFH	0x2e
#define	OV9650_YAVE	0x2f
#define	OV9650_HSYST	0x30
#define	OV9650_HSYEN	0x31
#define	OV9650_HREF	0x32
#define	OV9650_CHLF	0x33
#define	OV9650_ARBLM	0x34
/* 0x35..0x36 - RESERVED */
#define	OV9650_ADC	0x37
#define	OV9650_ACOM	0x38
#define	OV9650_OFON	0x39
#define	OV9650_TSLB	0x3a
#define	OV9650_COM11	0x3b
#define	OV9650_COM12	0x3c
#define	OV9650_COM13	0x3d
#define	OV9650_COM14	0x3e
#define	OV9650_EDGE	0x3f
#define	OV9650_COM15	0x40
#define	OV9650_COM16	0x41
#define	OV9650_COM17	0x42
/* 0x43..0x4e - RESERVED */
#define	OV9650_MTX1	0x4f
#define	OV9650_MTX2	0x50
#define	OV9650_MTX3	0x51
#define	OV9650_MTX4	0x52
#define	OV9650_MTX5	0x53
#define	OV9650_MTX6	0x54
#define	OV9650_MTX7	0x55
#define	OV9650_MTX8	0x56
#define	OV9650_MTX9	0x57
#define	OV9650_MTXS	0x58
/* 0x59..0x61 - RESERVED */
#define	OV9650_LCC1	0x62
#define	OV9650_LCC2	0x63
#define	OV9650_LCC3	0x64
#define	OV9650_LCC4	0x65
#define	OV9650_LCC5	0x66
#define	OV9650_MANU	0x67
#define	OV9650_MANV	0x68
#define	OV9650_HV	0x69
#define	OV9650_MBD	0x6a
#define	OV9650_DBLV	0x6b
#define	OV9650_GSP	0x6c	/* ... till 0x7b */
#define	OV9650_GST	0x7c	/* ... till 0x8a */
//Xrock add for ov9650 20140829
#define OV9650_COM21 0x8b
#define OV9650_COM22 0x8c
#define OV9650_COM23 0x8d
#define OV9650_COM24 0x8e
#define OV9650_DBLC1 0X8f
#define OV9650_DBLC_B 0x90
#define OV9650_DBLC_R 0x91
#define OV9650_DM_LNL 0x92
#define OV9650_DM_LNH 0x93
#define OV9650_RSVD 0x94  /* ... till 0x9c*/
#define OV9650_LCCFB 0x9d
#define OV9650_LCCFR 0x9e
#define OV9650_DBLC_Gb 0x9f
#define OV9650_DBLC_Gr 0xa0
#define OV9650_AECHM 0xa1
/* 0xa2..0xa5 - RESERVED*/
#define OV9650_G_GAIN 0xa6
#define OV9650_VGA_ST 0xa7
/* 0xa8...0xaa - RESERVED*/
//end add

#define	OV9650_CLKRC_DPLL_EN	0x80
#define	OV9650_CLKRC_DIRECT	0x40
#define	OV9650_CLKRC_DIV(x)	((x) & 0x3f)

#define	OV9650_PSHFT_VAL(x)	((x) & 0xff) //delay timing for the data relative to HREF in pixel unit

#define	OV9650_ACOM_2X_ANALOG	0x80  //9650 reserved
#define	OV9650_ACOM_RSVD	0x12      //9650 reserved

#define	OV9650_MVFP_V		0x10    //VFlip enable
#define	OV9650_MVFP_H		0x20    //mirror image

#define	OV9650_COM1_HREF_NOSKIP	0x00    //no skip 
#define	OV9650_COM1_HREF_2SKIP	0x04    //HREF skip: YUV/RGB skip every other row for YUV/RGB skip 2 rowws for every 4 rows for RAW data
#define	OV9650_COM1_HREF_3SKIP	0x08    //HREF skip:skip 3 rows for every 4 rows for YUV/RGB skip 6 rows for every 8 rows for RAW data
#define	OV9650_COM1_QQFMT	0x20

#define	OV9650_COM2_SSM		0x10     //soft sleep mode

#define	OV9650_COM3_VP		0x04    //VarioPixel for VGA and CIF

#define	OV9650_COM4_QQ_VP	0x80    //VarioPixel 
#define	OV9650_COM4_RSVD	0x40    //default value

#define	OV9650_COM5_SYSCLK	0x80    //system clock selection
#define	OV9650_COM5_LONGEXP	0x01    //exposure step can be set longer than VSYNC time

#define	OV9650_COM6_OPT_BLC	0x40    //use optical blank line as BLC signal
#define	OV9650_COM6_ADBLC_BIAS	0x08    //HREF is high from optical blank line
#define	OV9650_COM6_FMT_RST	0x82    //enable Href at optical blank reset all timing when faomat changes
#define	OV9650_COM6_ADBLC_OPTEN	0x01    //enable ADBLC option

#define	OV9650_COM7_RAW_RGB	0x01    //output format:RAW RGB
#define	OV9650_COM7_RGB		0x04    //output format:RGB
#define	OV9650_COM7_QCIF	0x08    //output format:QCIF
#define	OV9650_COM7_QVGA	0x10    //output format:QVGA
#define	OV9650_COM7_CIF		0x20    //output format:CIF
#define	OV9650_COM7_VGA		0x40    //output format:VGA
#define	OV9650_COM7_SCCB_RESET	0x80    //reset all SCCB registers to default values

#define	OV9650_TSLB_YVYU_YUYV	0x04    //output sequence is YVYU
#define	OV9650_TSLB_YUYV_UYVY	0x08    //output sequence is YUYV

#define OV9650_COM10_VSYNC_NEG 0x02   //VSYNC negative

#define	OV9650_COM12_YUV_AVG	0x04    //enable YUV average
#define	OV9650_COM12_RSVD	0x40        //reserved

#define	OV9650_COM13_GAMMA_NONE	0x00    //no gamma function 
#define	OV9650_COM13_GAMMA_Y	0x40    //gamma used for Y channel only
#define	OV9650_COM13_GAMMA_RAW	0x80    //gamma used for Raw data before interpolation
#define	OV9650_COM13_RGB_AVG	0x20    //reserved in 9650
#define	OV9650_COM13_MATRIX_EN	0x10    //enable color matrix for RGB/YUV
#define	OV9650_COM13_Y_DELAY_EN	0x08    //enable Y channel delay option
#define	OV9650_COM13_YUV_DLY(x)	((x) & 0x07) //output Y/UV delay

//Xrock modify
#define	OV9650_COM15_OR_00FF	0xc0    //0x00 before    
#define	OV9650_COM15_OR_01FE	0x80    //0x40 before
#define	OV9650_COM15_OR_10F0	0x00    //0xc0 before
#define	OV9650_COM15_RGB_NORM	0x00    //normal RGB output
#define	OV9650_COM15_RGB_565	0x10    //RGB 565
#define	OV9650_COM15_RGB_555	0x30    //RGB 555

#define	OV9650_COM16_RB_AVG	0x01        //not used in 9650

/* IDs */
#define	OV9650_V2		0x9648
#define	OV9650_V3		0x9649
#define	VERSION(pid, ver)	(((pid) << 8) | ((ver) & 0xFF))

/* supported resolutions */
enum {
	W_QQCIF	= 88,
	W_QQVGA	= 160,
	W_QCIF	= 176,
	W_QVGA	= 320,
	W_CIF	= 352,
	W_VGA	= 640,
	W_SXGA	= 1280
};
#define	H_SXGA	960

struct OV9650_Data {
	char subaddr;
	char value;
}; 

static struct OV9650_Data ov9650_reg0[] =   //ov9650_regs_dflt
{
{ OV9650_COM7,OV9650_COM7_SCCB_RESET},  //RESET ALL REGISTER
{CHIP_DELAY,DELAY_TIME},
{0x11, 0x81},
{0x6b, 0x0a},
{0x6a, 0x3e},
{0x3b, 0x09},
{0x13, 0xe0},
{0x01, 0x80},
{0x02, 0x80},
{0x00, 0x00},
{0x10, 0x00},
{0x13, 0xe5},
{0x39, 0x43}, //50 for 30fps
{0x38, 0x12}, //92 for 30fps
{0x37, 0x00},
{0x35, 0x91}, //81 for 30fps

{0x0e, 0x20},
{0x1e, 0x04},
{0xa8, 0x80},
{0x12, 0x40},
{0x04, 0x00},
{0x0c, 0x04},
{0x0d, 0x80},
{0x18, 0xc6},
{0x17, 0x26},
{0x32, 0xad},
{0x03, 0x00},
{0x1a, 0x3d},
{0x19, 0x01},
{0x3f, 0xa6},
{0x14, 0x2e},
{0x41, 0x02},
{0x42, 0x08},
{0x1b, 0x00},
{0x16, 0x06},
{0x33, 0xe2},//2f for internal regulator
{0x34, 0xbf},
{0x96, 0x04},
{0x3a, 0x00},
{0x8e, 0x00},

{0x3c, 0x77},
{0x8b, 0x06},
{0x94, 0x88},
{0x95, 0x88},
{0x40, 0xc1},
{0x29, 0x3f}, //2f for internal regulator
{0x0f, 0x42},

{0x3d, 0x92},
{0x69, 0x40},
{0x5c, 0xb9},
{0x5d, 0x96},
{0x5e, 0x10},
{0x59, 0xc0},
{0x5a, 0xaf},
{0x5b, 0x55},
{0x43, 0xf0},
{0x44, 0x10},
{0x45, 0x68},
{0x46, 0x96},
{0x47, 0x60},
{0x48, 0x80},
{0x5f, 0xe0},
{0x60, 0x8c}, //0c for advanced AWB (related to lens)
{0x61, 0x20},
{0xa5, 0xd9},
{0xa4, 0x74},
{0x8d, 0x02},
{0x13, 0xe7},
{0x4f, 0x3a},
{0x50, 0x3d},
{0x51, 0x03},
{0x52, 0x12},
{0x53, 0x26},
{0x54, 0x38},
{0x55, 0x40},
{0x56, 0x40},
{0x57, 0x40},
{0x58, 0x0d},
{0x8c, 0x23},
{0x3e, 0x02},
{0xa9, 0xb8},
{0xaa, 0x92},
{0xab, 0x0a},
{0x8f, 0xdf},
{0x90, 0x00},
{0x91, 0x00},
{0x9f, 0x00},
{0xa0, 0x00},
{0x3a, 0x0c},
{0x24, 0x70},
{0x25, 0x64},
{0x26, 0xc3},
//
{0x2a, 0x00}, //10 for 50Hz
{0x2b, 0x00}, //40 for 50Hz
//
//gamma
{0x6c, 0x40},
{0x6d, 0x30},
{0x6e, 0x4b},
{0x6f, 0x60},
{0x70, 0x70},
{0x71, 0x70},
{0x72, 0x70},
{0x73, 0x70},
{0x74, 0x60},
{0x75, 0x60},
{0x76, 0x50},
{0x77, 0x48},
{0x78, 0x3a},
{0x79, 0x2e},
{0x7a, 0x28},
{0x7b, 0x22},
{0x7c, 0x04},
{0x7d, 0x07},
{0x7e, 0x10},
{0x7f, 0x28},
{0x80, 0x36},
{0x81, 0x44},
{0x82, 0x52},
{0x83, 0x60},
{0x84, 0x6c},
{0x85, 0x78},
{0x86, 0x8c},
{0x87, 0x9e},
{0x88, 0xbb},
{0x89, 0xd2},
{0x8a, 0xe6}

};


#define OV9650_REGS0 (sizeof(ov9650_reg0)/sizeof(ov9650_reg0[0]))
#endif /* _OV9650_H_ */






