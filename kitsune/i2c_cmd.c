#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "kit_assert.h"
#include <stdint.h>

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "i2c_if.h"
#include "uartstdio.h"
#include "i2c_cmd.h"

#include "hw_ver.h"

#include "hw_memmap.h"
#include "rom_map.h"
#include "wdt.h"

#include "stdbool.h"

#include "codec_debug_config.h"
#include "audio_codec_pps_driver.h"

#include "codec_runtime_update.h"

#define MAX_MEASURE_TIME			10

#define FAILURE                 	-1
#define SUCCESS                 	0

#define RETERR_IF_TRUE(condition) 	{if(condition) return FAILURE;}
#define RET_IF_ERR(Func)          	{int iRetVal = (Func); \
								  	  if (SUCCESS != iRetVal) \
									  	  return  iRetVal;}

extern xSemaphoreHandle i2c_smphr;

static int codec_after_init_test(void);

#include "stdbool.h"

#if 1
struct {
	uint16_t dig_T1;
	int16_t  dig_T2;
	int16_t  dig_T3;
	uint16_t dig_P1;
	int16_t  dig_P2;
	int16_t  dig_P3;
	int16_t  dig_P4;
	int16_t  dig_P5;
	int16_t  dig_P6;
	int16_t  dig_P7;
	int16_t  dig_P8;
	int16_t  dig_P9;
	uint8_t  dig_H1;
	int16_t  dig_H2;
	uint8_t  dig_H3;
	int16_t  dig_H4;
	int16_t  dig_H5;
	uint8_t  dig_H6;
} bme280_cal;

#define BME280_S32_t int32_t
#define BME280_U32_t uint32_t
#define BME280_S64_t int64_t
// Returns temperature in DegC, resolution is 0.01 DegC. Output value of �5123� equals 51.23 DegC. // t_fine carries fine temperature as global value
BME280_S32_t t_fine;
BME280_S32_t BME280_compensate_T_int32(BME280_S32_t adc_T) {
	BME280_S32_t var1, var2, T;
	var1 = ((((adc_T >> 3) -
			((BME280_S32_t)bme280_cal.dig_T1<<1))) *
			((BME280_S32_t)bme280_cal.dig_T2)) >> 11;
	var2 = (((((adc_T >> 4) -
			((BME280_S32_t)bme280_cal.dig_T1)) *
			((adc_T>>4) -
					((BME280_S32_t)bme280_cal.dig_T1))) >> 12) *
			((BME280_S32_t)bme280_cal.dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}
// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits). // Output value of �24674867� represents 24674867/256 = 96386.2 Pa = 963.862 hPa
BME280_U32_t BME280_compensate_P_int64(BME280_S32_t adc_P) {
	BME280_S64_t var1, var2, p;
	var1 = ((BME280_S64_t) t_fine)- 128000;
	var2 = var1 * var1 * (BME280_S64_t) bme280_cal.dig_P6;
	var2 = var2 + ((var1 * (BME280_S64_t) bme280_cal.dig_P5) << 17);
	var2 = var2 + (((BME280_S64_t) bme280_cal.dig_P4) << 35);
	var1 = ((var1 * var1 * (BME280_S64_t) bme280_cal.dig_P3) >> 8)
			+ ((var1 * (BME280_S64_t) bme280_cal.dig_P2) << 12);
	var1 = (((((BME280_S64_t) 1) << 47) + var1))
			* ((BME280_S64_t) bme280_cal.dig_P1) >> 33;
	if (var1 == 0) {
		return 0; // avoid exception caused by division by zero
	}
	p = 1048576 - adc_P;
	p = (((p << 31) - var2) * 3125) / var1;
	var1 = (((BME280_S64_t) bme280_cal.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
	var2 = (((BME280_S64_t) bme280_cal.dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((BME280_S64_t) bme280_cal.dig_P7) << 4);
	return (BME280_U32_t) p;
}
// Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits). // Output value of �47445� represents 47445/1024 = 46.333 %RH
// CAJ EDIT: returns 0.01 %RH
BME280_U32_t bme280_compensate_H_int32(BME280_S32_t adc_H) {
	BME280_S32_t v_x1_u32r;
	v_x1_u32r = (t_fine- ((BME280_S32_t)76800));
	v_x1_u32r = (((((adc_H << 14)- (((BME280_S32_t)bme280_cal.dig_H4) << 20) - (((BME280_S32_t)bme280_cal.dig_H5) * v_x1_u32r)) +
	((BME280_S32_t)16384)) >> 15) * (((((((v_x1_u32r * ((BME280_S32_t)bme280_cal.dig_H6)) >> 10) * (((v_x1_u32r *
											((BME280_S32_t)bme280_cal.dig_H3)) >> 11) + ((BME280_S32_t)32768))) >> 10) + ((BME280_S32_t)2097152)) *
	((BME280_S32_t)bme280_cal.dig_H2) + 8192) >> 14));
	v_x1_u32r = (v_x1_u32r- (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((BME280_S32_t)bme280_cal.dig_H1)) >> 4));
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
	return (100*(BME280_U32_t)(v_x1_u32r >> 12))>>10;
}
#endif
#define DBG_BME(...)
uint32_t BME_i2c;
int get_temp_press_hum(int32_t * temp, uint32_t * press, uint32_t * hum) {
	unsigned char cmd;
	int temp_raw, press_raw, hum_raw;

	unsigned char b[8] = {0};

	assert(xSemaphoreTakeRecursive(i2c_smphr, 30000));

	//humid oversample
	b[0] = 0xf2; //must come before 0xf4
	b[1] = 0b00000101;
	(I2C_IF_Write(BME_i2c, b, 2, 1));
	//temp/pressure oversample, force meas
	b[0] = 0xf4;
	b[1] = 0b10110101;
	(I2C_IF_Write(BME_i2c, b, 2, 1));

	vTaskDelay(95);

	b[0] = 1;
	b[2] = b[1] = 0;
	cmd = 0xf3;
	for(;;) {
		(I2C_IF_Write(BME_i2c, &cmd, 1, 1));
		(I2C_IF_Read(BME_i2c, b, 1));

		if (b[0] == 0 || b[0] == 4 ) break;
		DBG_BME("%x %x %x\n", b[0],b[1],b[2] );

		vTaskDelay(5);
	}
	cmd = 0xf7;
	(I2C_IF_Write(BME_i2c, &cmd, 1, 1));
	(I2C_IF_Read(BME_i2c, b, 8));

	xSemaphoreGiveRecursive(i2c_smphr);

    temp_raw = (b[3] << 16) | (b[4]<<8) | (b[5]);
    temp_raw >>= 4;
    *temp = BME280_compensate_T_int32(temp_raw);
    DBG_BME("%x %x %x %d %d\n", b[3],b[4],b[5], temp_raw, *temp);

	press_raw = (b[0] << 16) | (b[1]<<8) | (b[2]);
	press_raw >>= 4;
    *press = BME280_compensate_P_int64(press_raw);
    DBG_BME("%x %x %x %d %d\n", b[0],b[1],b[2], press_raw, *press);

	hum_raw = (b[6]<<8) | (b[7]);
    *hum = bme280_compensate_H_int32(hum_raw);

    DBG_BME("%x %x %d %d\n", b[6],b[7], hum_raw, *hum);

	return 0;
}

int init_temp_sensor()
{
	unsigned char id;
	unsigned char cmd = 0xd0;
	unsigned char b[25];


	assert(xSemaphoreTakeRecursive(i2c_smphr, 30000));
	vTaskDelay(5);
	(I2C_IF_Write(0x76, &cmd, 1, 1));
	vTaskDelay(5);
	(I2C_IF_Read(0x76, &id, 1));
	if (id == 0x60) {
		BME_i2c = 0x76;
	}
	else {
		vTaskDelay(5);
		(I2C_IF_Write(0x77, &cmd, 1, 1));
		vTaskDelay(5);
		(I2C_IF_Read(0x77, &id, 1));
		if (id == 0x60) {
			BME_i2c = 0x77;
		}
	}

	cmd =0x88;
	(I2C_IF_Write(BME_i2c, &cmd, 1, 1));
	(I2C_IF_Read(BME_i2c, b, 25));

	bme280_cal.dig_T1 = b[0] | (b[1]<<8);
	bme280_cal.dig_T2 = b[2] | (b[3]<<8);
	bme280_cal.dig_T3 = b[4] | (b[5]<<8);
	bme280_cal.dig_P1 = b[6] | (b[7]<<8);
	bme280_cal.dig_P2 = b[8] | (b[9]<<8);
	bme280_cal.dig_P3 = b[10] | (b[11]<<8);
	bme280_cal.dig_P4 = b[12] | (b[13]<<8);
	bme280_cal.dig_P5 = b[14] | (b[15]<<8);
	bme280_cal.dig_P6 = b[16] | (b[17]<<8);
	bme280_cal.dig_P7 = b[18] | (b[19]<<8);
	bme280_cal.dig_P8 = b[20] | (b[21]<<8);
	bme280_cal.dig_P9 = b[22] | (b[23]<<8);
	bme280_cal.dig_H1 = b[24];

	cmd =0xE1;
	(I2C_IF_Write(BME_i2c, &cmd, 1, 1));
	(I2C_IF_Read(BME_i2c, b, 7));

	bme280_cal.dig_H2 = b[0] | (b[1]<<8);
	bme280_cal.dig_H3 = b[2];
	bme280_cal.dig_H4 = (b[3]<<4) | (b[4]&0xf);
	bme280_cal.dig_H5 = (b[5]<<4) | ((b[4]>>4)&0xf);
	bme280_cal.dig_H6 = b[7];

	xSemaphoreGiveRecursive(i2c_smphr);
	DBG_BME("found %x\n", id);

	return id == 0x60;
}

int Cmd_read_temp_hum_press(int argc, char *argv[]) {
	int32_t temp;
	uint32_t hum,press;
	get_temp_press_hum(&temp, &press, &hum);
	LOGF("%d,%d,%d\n", temp, hum, press);
	return SUCCESS;
}

bool tvoc_wa = false;
int init_tvoc(int measmode) {
	unsigned char b[2];
	assert(xSemaphoreTakeRecursive(i2c_smphr, 30000));

	b[0] = 0;
	(I2C_IF_Write(0x5a, b, 1, 1));
	(I2C_IF_Read(0x5a, b, 1));

	if( !(b[0] & 0x10) ) {
		LOGE("no valid fw for TVOC\n");
		xSemaphoreGiveRecursive(i2c_smphr);
		return -1;
	}
	//boot
	b[0] = 0xf4;
	(I2C_IF_Write(0x5a, b, 1, 1));
	vTaskDelay(100);
	b[0] = 0;
	(I2C_IF_Write(0x5a, b, 1, 1));
	(I2C_IF_Read(0x5a, b, 1));
	if( !(b[0] & 0x90) ) {
		LOGE("fail to boot TVOC\n");
		xSemaphoreGiveRecursive(i2c_smphr);
		return -1;
	}
	b[0] = 1;
	b[1] = measmode;
	(I2C_IF_Write(0x5a, b, 2, 1));

	b[0] = 0x24;
	(I2C_IF_Write(0x5a, b, 1, 1));
	(I2C_IF_Read(0x5a, b, 2));

	LOGE("TVOC FW %d.%d.%d\n",(b[0]>>4) & 0xf,b[0] & 0xf, b[1]);
	if (b[0] == 0x02 && b[1] == 0x4) {
		LOGE("apply TVOC FW 0.2.4 workaround\n");
		tvoc_wa = true;
	}

	xSemaphoreGiveRecursive(i2c_smphr);
	return 0;
}
static int set_tvoc_env(int temp, unsigned int humid){
	unsigned char b[8];
	LOGI("setting tv temp %d humid %d\r\n", temp, humid);
	assert(xSemaphoreTakeRecursive(i2c_smphr, 30000));
	b[0] = 0x05;

	humid *= 10;
	// see cc-000803-an-4-ccs811_programming_and_interfacing_guide.pdf page 19, 20
	b[1] = ((humid % 1000) / 100) > 7 ? (humid/1000 + 1)<<1 : (humid/1000)<<1;
	if(((humid % 1000) / 100) > 2 && (((humid % 1000) / 100) < 8))
	{
		b[1] |= 1;
	}
	b[2] = 0;

	temp *= 10;
	temp += 25000;
	b[3] = ((temp % 1000) / 100) > 7 ? (temp / 1000 + 1) << 1 : (temp / 1000) << 1;
	if (((temp % 1000) / 100) > 2 && (((temp % 1000) / 100) < 8)) {
		b[3] |= 1;
	}
	b[4] = 0;
	(I2C_IF_Write(0x5a, b, 5, 1));
	vTaskDelay(10);
	xSemaphoreGiveRecursive(i2c_smphr);
	return 0;
}
int Cmd_set_tvenv(int argc, char * argv[]){
	return set_tvoc_env(atoi(argv[1]), atoi(argv[2]));
}
#define DBG_TVOC LOGI
int get_tvoc(int * tvoc, int * eco2, int * current, int * voltage, int temp, unsigned int humid ) {
	unsigned char b[8];
	static int last_tvoc, last_eco2;
	assert(xSemaphoreTakeRecursive(i2c_smphr, 30000));
	/*
	 * read status
	 */
	int tries = 3;
	bool data_is_ready = false;
	while(tries-- > 0){
		b[0] = 0;
		(I2C_IF_Write(0x5a, b, 1, 1));
		memset(b,0, sizeof(b));
		I2C_IF_Read(0x5a, b, 1);
		if(b[0] & 0x08){
			data_is_ready = true;
			break;
		}
		vTaskDelay(1);
	}
	if(!data_is_ready){
		LOGW("TVOC Data Not Ready\r\n");
		xSemaphoreGiveRecursive(i2c_smphr);
		*tvoc = last_tvoc;
		*eco2 = last_eco2;
		return 0;
	}
	/*
	 * read alg_result_data
	 */
	b[0] = 2;
	(I2C_IF_Write(0x5a, b, 1, 1));
	memset(b,0, sizeof(b));
	(I2C_IF_Read(0x5a, b, 8));

	DBG_TVOC("%x:%x:%x:%x:%x:%x:%x:%x\n",
			b[0],b[1],b[2],b[3],b[4],b[5],
		    b[6],b[7]);

	//Status and Error ID bytes swapped in TVOC FW 0.2.4
	if( tvoc_wa ) {
		unsigned char temp = b[4];
		b[4] = b[5];
		b[5] = temp;

		LOGE("OLD VOC\n");
	}
	if( b[4] & 0x01 ) {
		LOGE("TVOC error %x ", b[5] );
		b[0] = 0xe0;
		(I2C_IF_Write(0x5a, b, 1, 1));
		(I2C_IF_Read(0x5a, b, 1));
		LOGE("%x\n", b[0]);
		xSemaphoreGiveRecursive(i2c_smphr);
		return -1;
	}
	*eco2 = (b[1] | (b[0]<<8));
	*tvoc = (b[3] | (b[2]<<8));
	*current = (b[6]>>2);
	*voltage = (((b[6]&3)<<8) | (b[7]));
	last_tvoc = *tvoc;
	last_eco2 = *eco2;
	vTaskDelay(10);
	xSemaphoreGiveRecursive(i2c_smphr);
	set_tvoc_env(temp,humid);
	return 0;
}

int Cmd_meas_TVOC(int argc, char *argv[]) {
	int32_t temp;
	uint32_t hum,press;
	if( argc > 1 ) {
		init_tvoc(strtoul(argv[1], NULL, 16));
	}

	int tvoc, eco2, current, voltage;
	if( 0 == get_temp_press_hum(&temp, &press, &hum) &&
	    0 == get_tvoc( &tvoc, &eco2, &current, &voltage, temp, hum) ) {
		LOGF("voc %d eco2 %d %duA %dmv %d %d\n", tvoc, eco2, current, voltage, temp, hum);
		return 0;
	}
	return -1;
}
static bool haz_tmg4903() {
	unsigned char b[2]={0};
	b[0] = 0x92;
	if(xSemaphoreTakeRecursive(i2c_smphr, 1000)){
		(I2C_IF_Write(0x39, b, 1, 1));
		(I2C_IF_Read(0x39, b, 1));
		xSemaphoreGiveRecursive(i2c_smphr);

		if( b[0] != 0xb8 ) {
			LOGE("can't find TMG4903\n");
			return false;
		}
	}
	else{
		LOGW("failed to get i2c %d\n", __LINE__);
		return false;
	}

	return true;
}

#include "gesture.h"
int light_sensor_power(light_power_mode power_state) {
	unsigned char b[2];
	assert(xSemaphoreTakeRecursive(i2c_smphr, 2000));

	//max pulse length, number of pluses
	b[0] = 0x8E;
	if( power_state == HIGH_POWER ) {
		resume_gestures();
		b[1] = 0xcf;
	} else {
		pause_gestures();
		b[1] = 0;
	}
	(I2C_IF_Write(0x39, b, 2, 1));

	xSemaphoreGiveRecursive(i2c_smphr);

	return SUCCESS;

}

int init_light_sensor()
{
	unsigned char b[5];

	if( !haz_tmg4903() ) {
		LOGE("haz_tmg4903 fail\n");
		return FAILURE;
	}

	assert(xSemaphoreTakeRecursive(i2c_smphr, DEFAULT_I2C_SEM_TIMEOUT));
	b[0] = 0x80;
	b[1] = 0b0000111; //enable prox/als/power
	b[2] = 249; //20ms integration
	b[3] = 35; //100ms prox time
	b[4] = 249; //20ms als time
	(I2C_IF_Write(0x39, b, 5, 1));
	b[0] = 0x8d;
	b[1] = 0x0;
	(I2C_IF_Write(0x39, b, 2, 1));
	b[0] = 0x90;
	b[1] = 0x3;
	(I2C_IF_Write(0x39, b, 2, 1));

	//max pulse length, number of pluses
	b[0] = 0x8E;
	b[1] = 0xCF;
	(I2C_IF_Write(0x39, b, 2, 1));

	//gain and power
	b[0] = 0x8F;
	b[1] = 0xD2;
	(I2C_IF_Write(0x39, b, 2, 1));
	xSemaphoreGiveRecursive(i2c_smphr);

	vTaskDelay(50);
	return SUCCESS;
}


static int get_le_short( uint8_t * b ) {
	return (b[0] | (b[1]<<8));
}
#define DBG_TMG(...)
int get_rgb_prox( int * w, int * r, int * g, int * bl, int * p ) {
	unsigned char b[10];

	assert(xSemaphoreTakeRecursive(i2c_smphr, 2000));

	/*Red, green, blue, and clear data are stored as 16-bit values.
	The read sequence must read byte pairs (low followed by high)
	 starting on an even address boundary (0x94, 0x96, 0x98, or 0x9A)
	  inside the CRGB Data Register block. In addition, reading the
	  Clear channel data low byte (0x94) latches all 8 data bytes.
	   Reading these 8 bytes consecutively (0x94 - 0x9A) ensures that
		the data is concurrent.
	*/
	if( !haz_tmg4903() ) {
		LOGE("haz_tmg4903 fail\n");
		xSemaphoreGiveRecursive(i2c_smphr);
		return FAILURE;
	}

	b[0] = 0x94;
	(I2C_IF_Write(0x39, b, 1, 1));
	(I2C_IF_Read(0x39, b, 10));

	*w = get_le_short(b);
	*r = get_le_short(b+2);
	*g = get_le_short(b+4);
	*bl = get_le_short(b+6);
	*p = get_le_short(b+8);

	xSemaphoreGiveRecursive(i2c_smphr);
	return SUCCESS;

}

int Cmd_readlight(int argc, char *argv[]) {
	int r,g,b,w,p;
	if( SUCCESS == get_rgb_prox( &w, &r, &g, &b, &p ) ) {
		LOGF("%d,%d,%d,%d,%d\n", w,r,g,b,p );
	}else{
		LOGE("read light failed.\r\n");
	}
	return SUCCESS;
}

int get_ir( int * ir ) {
	unsigned char b[2];
	int w,r,g,bl,p;
	assert(xSemaphoreTakeRecursive(i2c_smphr, 2000));

	b[0] = 0xAB;
	b[1] = 0x40;
	(I2C_IF_Write(0x39, b, 2, 1));
	vTaskDelay(110);
	get_rgb_prox( &w, &r, &g, &bl, &p );
	*ir = (r+g+bl-w)/2;

	b[0] = 0xAB;
	b[1] = 0x00;
	(I2C_IF_Write(0x39, b, 2, 1));
	vTaskDelay(110);

	xSemaphoreGiveRecursive(i2c_smphr);

	return 0;
}
int init_uv(bool als) {
	unsigned char b[2];
	assert(xSemaphoreTakeRecursive(i2c_smphr, 2000));

#if 0 // checking the part id seems to make the mode setting fail
	//check the part id
	b[0] = 0x6;
	(I2C_IF_Write(0x53, b, 1, 1));
	(I2C_IF_Read(0x53, b, 1));
	if( b[1] != 0xb2 ) {
		xSemaphoreGiveRecursive(i2c_smphr);
		return -1;
	}
#endif
	//set mode
	b[0] = 0;
	if( als ) {
		b[1] = 2;
	} else {
		b[1] = 0xa;
	}
	(I2C_IF_Write(0x53, b, 2, 1));

	//set gain to 18
	b[0] = 0x5;
	b[1] = 0b100;
	(I2C_IF_Write(0x53, b, 2, 1));

	//set rate to 2 hz
	b[0] = 0x4;
	b[1] = 0b110100;
	(I2C_IF_Write(0x53, b, 2, 1));

	xSemaphoreGiveRecursive(i2c_smphr);

	return 0;
}
int read_zopt(zopt_mode selection) {
	static int use_als=-1;

	if( use_als != (selection==ZOPT_ALS) ) {
		use_als  = (selection==ZOPT_ALS);
		if( init_uv( use_als ) ) {
			LOGF("UV FAIL\n");
			return -1;
		}
	}

	int32_t v = 0;
	unsigned char b[2];
	assert(xSemaphoreTakeRecursive(i2c_smphr, 2000));

	b[0] = use_als ? 0xd : 0x10;
	(I2C_IF_Write(0x53, b, 1, 1));
	(I2C_IF_Read(0x53, (uint8_t*)&v, 3));
	xSemaphoreGiveRecursive(i2c_smphr);

	return v;
}

int Cmd_read_uv(int argc, char *argv[]) {
	LOGF("%d\n", read_zopt(atoi(argv[1])));
	return 0;
}

int Cmd_uvr(int argc, char *argv[]) {
	int i;
	int addr = strtol(argv[1], NULL, 16);
	int len = strtol(argv[2], NULL, 16);
	unsigned char b[2];
	assert(xSemaphoreTakeRecursive(i2c_smphr, DEFAULT_I2C_SEM_TIMEOUT));

	b[0] = addr;
	(I2C_IF_Write(0x53, b, 1, 1));
	(I2C_IF_Read(0x53, b, len));

	for(i=0;i<len;++i) {
		LOGF("%x:", b[i] );
	}
	LOGF("\n");
	xSemaphoreGiveRecursive(i2c_smphr);
	return SUCCESS;
}
int Cmd_uvw(int argc, char *argv[]) {
	int addr = strtol(argv[1], NULL, 16);
	int data = strtol(argv[2], NULL, 16);

	unsigned char b[2];
	assert(xSemaphoreTakeRecursive(i2c_smphr, DEFAULT_I2C_SEM_TIMEOUT));
	b[0] = addr;
	b[1] = data;
	(I2C_IF_Write(0x53, b, 2, 1));
	xSemaphoreGiveRecursive(i2c_smphr);

	return SUCCESS;
}

#include "simplelink.h"
#include "sl_sync_include_after_simplelink_header.h"

/********************************************************************************
 *                     AUDIO CODEC DRIVER CODE
 ********************************************************************************/
typedef enum{
	SPK_VOLUME_MUTE = 0,
	SPK_VOLUME_6dB = 1,
	SPK_VOLUME_12dB = 2,
	SPK_VOLUME_18dB = 3,
	SPK_VOLUME_24dB = 4,
	SPK_VOLUME_30dB = 5,
}spk_volume_t;

static void codec_sw_reset(void);

/*
 * This function updates the gain on the LOL output to the speaker driver
 * On the codec, this gain has 117 levels between 0db to -78.3db
 * The input v to the function varies from 0-64.
 */
volatile int sys_volume = 64;

#define VOL_LOC "/hello/vol"
int get_system_volume() {
	 if( fs_get(VOL_LOC, &sys_volume, sizeof(sys_volume), NULL) < 0 ) {
		 sys_volume = 64;
	 }

	 return sys_volume;
}
int32_t set_system_volume(int new_volume) {
	if( new_volume != sys_volume ) {
		 sys_volume = new_volume;
		 fs_save(VOL_LOC, &sys_volume, sizeof(sys_volume));
	}
	return set_volume(new_volume, portMAX_DELAY);
}
volatile int last_set_volume = 0;
static bool codec_muted = true;

static _set_volume_raw(int v, unsigned int dly) {
	char send_stop = 1;
	unsigned char cmd[2];

	v = 64-v;
	v <<= 10;
	v /= 560;

	if( xSemaphoreTakeRecursive(i2c_smphr, dly)) {

		codec_set_book(0);

		//	w 30 00 00 # Select Page 0
		codec_set_page(1);

		cmd[0] = 46;
		cmd[1] = v;
		I2C_IF_Write(Codec_addr, cmd, 2, send_stop);
		xSemaphoreGiveRecursive(i2c_smphr);
		return 0;
	} else {
		return -1;
	}
}

int32_t set_volume(int v, unsigned int dly) {
	LOGI("v %d\n", v);
	last_set_volume = v;

	if(v < 0) v = 0;
	if(v >64) v = 64;

	if( v == 0 && !codec_muted ) {
		codec_mute_spkr();
	}
	if( v != 0 && codec_muted ) {
		codec_unmute_spkr();
	}

	_set_volume_raw(v,dly);

}

void codec_watchdog() {
	unsigned char cmd[2];
	bool overcurrent, spkr_off;
	if( !codec_muted && xSemaphoreTakeRecursive(i2c_smphr, 0)) {

		codec_set_book(0);
		codec_set_page(0);

		cmd[0] = 44;
		cmd[1] = 0;
		I2C_IF_Write(Codec_addr, cmd, 1, 1);
		I2C_IF_Read(Codec_addr, &cmd[1], 1);

		uint8_t b0p0r44 = cmd[1];
		if( b0p0r44 ) {
			LOGW("B0_P0_R44 %x\n",b0p0r44);
		}
		overcurrent = b0p0r44 & 0xC0;

		codec_set_book(0);
		codec_set_page(1);

		cmd[0] = 45;
		cmd[1] = 0;
		I2C_IF_Write(Codec_addr, cmd, 1, 1);
		I2C_IF_Read(Codec_addr, &cmd[1], 1);
		uint8_t b0p1r45 = cmd[1];
		spkr_off = b0p1r45 == 4;

		if( overcurrent ) {
			LOGE("overcurrent\n");
		}
		if( spkr_off ) {
			LOGE("spkrdisabled\n");
			LOGW("B0_P1_R45 %x\n",b0p1r45);
		}
		if( overcurrent || spkr_off ) {
			LOGW("restart codec\n");
			cmd[0] = 45;
			cmd[1] = 6;
			I2C_IF_Write(Codec_addr, cmd, 2, 1);
		}

		xSemaphoreGiveRecursive(i2c_smphr);
	}
}

int cmd_codec(int argc, char *argv[]) {
	unsigned char cmd[2];
	if( xSemaphoreTakeRecursive(i2c_smphr, 1000)){

		codec_set_book(atoi(argv[2]));

		codec_set_page(atoi(argv[1]));

		cmd[0] = atoi(argv[3]);
		cmd[1] = atoi(argv[4]);
		I2C_IF_Write(Codec_addr, cmd, 2, 1);
		xSemaphoreGiveRecursive(i2c_smphr);
	}

	return 0;
}

int cmd_codec_read(int argc, char *argv[]) {
	unsigned char cmd[2]={0xFF,0xFF};
	if(argc < 4){
		LOGF("Usage: codecr <book> <page> <reg>\n");
		return -1;
	}
	if( xSemaphoreTakeRecursive(i2c_smphr, 1000)){

		codec_set_book(atoi(argv[1]));

		codec_set_page(atoi(argv[2]));

		cmd[0] = atoi(argv[3]);
		I2C_IF_Write(Codec_addr, &cmd[0],1,1);
		I2C_IF_Read(Codec_addr, &cmd[1], 1);
		xSemaphoreGiveRecursive(i2c_smphr);
	}

	LOGI("Codecr [%u][%u][%u] = 0x%x\n",atoi(argv[2]),atoi(argv[1]),cmd[0],cmd[1]);

	return 0;
}

static void codec_sw_reset(void)
{
	char send_stop = 1;
	unsigned char cmd[2];
	const TickType_t delay = 10 / portTICK_PERIOD_MS;
	int ret;

	//w 30 7f 00 # Initialize to Book 0
	codec_set_book(0);

	// w 30 00 00 # Initialize to Page 0
	codec_set_page(0);

	if( xSemaphoreTakeRecursive(i2c_smphr, 100)) {

		//w 30 01 01 # Software Reset
		cmd[0] = 0x01;
		cmd[1] = 0x01;
		if((ret=I2C_IF_Write(Codec_addr, cmd, 2, send_stop)))
		{
			UARTprintf("Codec sw reset fail:%d\n",ret);
		}
		xSemaphoreGiveRecursive(i2c_smphr);
	}
	vTaskDelay(delay);
}

void codec_set_page(uint32_t page)
{
	char send_stop = 1;
	unsigned char cmd[2];

	if( xSemaphoreTakeRecursive(i2c_smphr, 100)) {

		//	w 30 00 00 # Select Page 0
		cmd[0] = 0;
		cmd[1] = page;
		I2C_IF_Write(Codec_addr, cmd, 2, send_stop);

		xSemaphoreGiveRecursive(i2c_smphr);
	}
}

void codec_set_book(uint32_t book)
{
	char send_stop = 1;
	unsigned char cmd[2];

	codec_set_page(0);

	if( xSemaphoreTakeRecursive(i2c_smphr, 100)) {

		//	# Select Book 0
		cmd[0] = 0x7F;
		cmd[1] = book;
		I2C_IF_Write(Codec_addr, cmd, 2, send_stop);

		xSemaphoreGiveRecursive(i2c_smphr);
	}
}

// FOR board bring up - Tests I2C
#ifdef CODEC_1P5_TEST
int32_t codec_test_commands(void)
{
	unsigned char cmd[2] = {0};
	char send_stop = 1;

	// Send Software reset
	codec_sw_reset();

	if( xSemaphoreTakeRecursive(i2c_smphr, 100)) {

		// Read register in [0][0][00]
		cmd[0] = 0;
		cmd[1] = 0;

		I2C_IF_Write(Codec_addr, &cmd[0],1,send_stop);
		I2C_IF_Read(Codec_addr, &cmd[1], 1);

		UARTprintf("Codec Test read 1 %s: [0][0][%u]: %X \n", (cmd[1]==0)?"Pass":"Fail", cmd[0], cmd[1]);

		// Read register in [0][0][06]
		cmd[0] = 0x06;
		cmd[1] = 0;

		I2C_IF_Write(Codec_addr, &cmd[0],1,send_stop);
		I2C_IF_Read(Codec_addr, &cmd[1], 1);

		UARTprintf("Codec Test read 2 %s: [0][0][%u]: %X  \r\n", (cmd[1]==0x11)?"Pass":"Fail", cmd[0], cmd[1]);

		// Read register in [0][0][0A]
		cmd[0] = 0x0A;
		cmd[1] = 0;

		I2C_IF_Write(Codec_addr, &cmd[0],1,send_stop);
		I2C_IF_Read(Codec_addr, &cmd[1], 1);

		UARTprintf("Codec Test read 3 %s: [0][0][%u]: %X  \r\n", (cmd[1]==0x01)?"Pass":"Fail", cmd[0], cmd[1]);


		xSemaphoreGiveRecursive(i2c_smphr);
	}


	return 0;

}
#endif

void codec_mute_spkr(void)
{
	char send_stop = 1;
	unsigned char cmd[2];

	LOGI("codec: muting\n");

	codec_set_book(0);

	//	w 30 00 00 # Select Page 0
	codec_set_page(1);

	if( xSemaphoreTakeRecursive(i2c_smphr, 100)) {
		cmd[0] = 48;
		cmd[1] = 0x00;
		I2C_IF_Write(Codec_addr, cmd, 2, send_stop);

		codec_muted = true;
		xSemaphoreGiveRecursive(i2c_smphr);
	}

}

void codec_unmute_spkr(void)
{
	char send_stop = 1;
	unsigned char cmd[2];

	LOGI("codec: unmuting\n");

	_set_volume_raw(0, portMAX_DELAY);

	codec_set_book(0);

	//	w 30 00 00 # Select Page 0
	codec_set_page(1);

	if( xSemaphoreTakeRecursive(i2c_smphr, 100)) {
		cmd[0] = 48;
		cmd[1] = (SPK_VOLUME_6dB << 4);
		I2C_IF_Write(Codec_addr, cmd, 2, send_stop);

		codec_muted = false;
		xSemaphoreGiveRecursive(i2c_smphr);
	}

}

void codec_power_spkr(uint8_t power_up)
{
	char send_stop = 1;
	unsigned char cmd[2];

	codec_set_book(0);
	codec_set_page(1);

	if( xSemaphoreTakeRecursive(i2c_smphr, 100)) {
		cmd[0] = 45;

		if (power_up) {
			cmd[1] = 0x02;
			set_volume(0, portMAX_DELAY);
		}
		else {
			cmd[1] = 0x00;
		}

		I2C_IF_Write(Codec_addr, cmd, 2, send_stop);

		xSemaphoreGiveRecursive(i2c_smphr);
	}

}

int cmd_pwr_speaker(int argc, char * argv[]) {
	if (argc >= 2) {
		if (strcmp(argv[1],"up") == 0) {
			codec_power_spkr(1);
		}

		if (strcmp(argv[1],"down") == 0) {
			codec_power_spkr(0);
		}

		return 0;
	}

	return -1;
}




int32_t codec_runtime_prop_update(control_blocks_t type, uint32_t value);
int32_t codec_init(void)
{
	uint32_t i;
	char send_stop = 1;
	unsigned char cmd[2];

	assert(xSemaphoreTakeRecursive(i2c_smphr, 30000));

	/********************************************************************************
	 * Update clock config
	 ********************************************************************************
	 */
	uint32_t reg_array_size = sizeof(REG_Section_program)/2;

	// Write the registers
	for(i=0;i<reg_array_size;i++)
	{
		//	# Select Book 0
		cmd[0] = REG_Section_program[i].reg_off;
		cmd[1] = REG_Section_program[i].reg_val;
		I2C_IF_Write(Codec_addr, cmd, 2, send_stop);
	}

	xSemaphoreGiveRecursive(i2c_smphr);

	vTaskDelay(20);

	/********************************************************************************
	 * Update miniDSP A instructions and coefficients
	 ********************************************************************************
	 */
	reg_array_size = sizeof(miniDSP_A_reg_values)/2;

	assert(xSemaphoreTakeRecursive(i2c_smphr, 30000));
	// Write the registers
	for(i=0;i<reg_array_size;i++)
	{
		//	# Select Book 0
		cmd[0] = miniDSP_A_reg_values[i].reg_off;
		cmd[1] = miniDSP_A_reg_values[i].reg_val;
		I2C_IF_Write(Codec_addr, cmd, 2, send_stop);
	}
	xSemaphoreGiveRecursive(i2c_smphr);

	vTaskDelay(20);

	/********************************************************************************
	 * Update miniDSP D instructions and coefficients
	 ********************************************************************************
	 */
	reg_array_size = sizeof(miniDSP_D_reg_values)/2;

	assert(xSemaphoreTakeRecursive(i2c_smphr, 30000));
	// Write the registers
	for(i=0;i<reg_array_size;i++)
	{
		//	# Select Book 0
		cmd[0] = miniDSP_D_reg_values[i].reg_off;
		cmd[1] = miniDSP_D_reg_values[i].reg_val;
		I2C_IF_Write(Codec_addr, cmd, 2, send_stop);
	}
	xSemaphoreGiveRecursive(i2c_smphr);

	vTaskDelay(20);

	/********************************************************************************
	 * Power Up ADC, DAC and other modules
	 * IMPORTANT: REG_Section_program2 has to be written after minidsp coefficients
	 * and instructions have been written in order for playback to work.
	 ********************************************************************************
	 */
	reg_array_size = sizeof(REG_Section_program2)/2;

	assert(xSemaphoreTakeRecursive(i2c_smphr, 30000));
	// Write the registers
	for(i=0;i<reg_array_size;i++)
	{
		//	# Select Book 0
		cmd[0] = REG_Section_program2[i].reg_off;
		cmd[1] = REG_Section_program2[i].reg_val;
		I2C_IF_Write(Codec_addr, cmd, 2, send_stop);
	}
	xSemaphoreGiveRecursive(i2c_smphr);

	vTaskDelay(100);

#if 0
	codec_after_init_test();
#endif

	return 1;

}

static int codec_after_init_test(void){
	char send_stop = 1;
	unsigned char cmd[2];


	// Check if adaptive mode enabled

	codec_set_book(40);

	//	w 30 00 00 # Select Page 0
	codec_set_page(0);

	assert(xSemaphoreTakeRecursive(i2c_smphr, 30000));

	// Read register in [0][0][36]
	cmd[0] = 1;
	cmd[1] = 0;

	I2C_IF_Write(Codec_addr, &cmd[0],1,send_stop);
	I2C_IF_Read(Codec_addr, &cmd[1], 1);

	UARTprintf("CRAM Adaptive mode [40][0][%u]: %X \n",cmd[0], cmd[1]);

	xSemaphoreGiveRecursive(i2c_smphr);

	codec_set_book(0);

	//	w 30 00 00 # Select Page 0
	codec_set_page(0);

	assert(xSemaphoreTakeRecursive(i2c_smphr, 30000));

	// Read register in [0][0][36]
	cmd[0] = 0x24;
	cmd[1] = 0;

	I2C_IF_Write(Codec_addr, &cmd[0],1,send_stop);
	I2C_IF_Read(Codec_addr, &cmd[1], 1);

	UARTprintf("ADC Flag read [0][0][%u]: %X \n",cmd[0], cmd[1]);

	// Read register in [0][0][37]
	cmd[0] = 0x25;
	cmd[1] = 0;

	I2C_IF_Write(Codec_addr, &cmd[0],1,send_stop);
	I2C_IF_Read(Codec_addr, &cmd[1], 1);

	UARTprintf("DAC Flag read [0][0][%u]: %X  \r\n", cmd[0], cmd[1]);

	// Read register in [0][0][38]
	cmd[0] = 0x26;
	cmd[1] = 0;

	I2C_IF_Write(Codec_addr, &cmd[0],1,send_stop);
	I2C_IF_Read(Codec_addr, &cmd[1], 1);

	UARTprintf("DAC Flag read [0][0][%u]: %X  \r\n", cmd[0], cmd[1]);

	// Read register in [0][0][38]
	cmd[0] = 0x3C;
	cmd[1] = 0;

	I2C_IF_Write(Codec_addr, &cmd[0],1,send_stop);
	I2C_IF_Read(Codec_addr, &cmd[1], 1);

	UARTprintf("[0][0][%u]: %X  \r\n", cmd[0], cmd[1]);

	cmd[0] = 0x3D;
	cmd[1] = 0;

	I2C_IF_Write(Codec_addr, &cmd[0],1,send_stop);
	I2C_IF_Read(Codec_addr, &cmd[1], 1);

	UARTprintf("[0][0][%u]: %X  \r\n", cmd[0], cmd[1]);

	codec_set_book(0);

	//	w 30 00 00 # Select Page 1
	codec_set_page(1);

	// Read register in [0][1][51]
	cmd[0] = 51;
	cmd[1] = 0;

	I2C_IF_Write(Codec_addr, &cmd[0],1,send_stop);
	I2C_IF_Read(Codec_addr, &cmd[1], 1);

	UARTprintf("Mic Bias Control [0][1][%u]: %X  \r\n", cmd[0], cmd[1]);

	// Read register in [0][1][64]
	cmd[0] = 64;
	cmd[1] = 0;

	I2C_IF_Write(Codec_addr, &cmd[0],1,send_stop);
	I2C_IF_Read(Codec_addr, &cmd[1], 1);

	UARTprintf("DAC Analog Gain [0][1][%u]: %X  \r\n", cmd[0], cmd[1]);

	// Read register in [0][1][66]
	cmd[0] = 66;
	cmd[1] = 0;

	I2C_IF_Write(Codec_addr, &cmd[0],1,send_stop);
	I2C_IF_Read(Codec_addr, &cmd[1], 1);

	UARTprintf("Driver power up [0][1][%u]: %X  \r\n", cmd[0], cmd[1]);

	// Read register in [0][1][52]
	cmd[0] = 31;
	cmd[1] = 0;

	I2C_IF_Write(Codec_addr, &cmd[0],1,send_stop);
	I2C_IF_Read(Codec_addr, &cmd[1], 1);

	UARTprintf(" [0][1][%u]: %X  \r\n", cmd[0], cmd[1]);

	// Read register in [0][1][54]
	cmd[0] = 32;
	cmd[1] = 0;

	I2C_IF_Write(Codec_addr, &cmd[0],1,send_stop);
	I2C_IF_Read(Codec_addr, &cmd[1], 1);

	UARTprintf(" [0][1][%u]: %X  \r\n", cmd[0], cmd[1]);

	// Read register in [0][1][55]
	cmd[0] = 33;
	cmd[1] = 0;

	I2C_IF_Write(Codec_addr, &cmd[0],1,send_stop);
	I2C_IF_Read(Codec_addr, &cmd[1], 1);

	UARTprintf(" [0][1][%u]: %X  \r\n", cmd[0], cmd[1]);

	// Read register in [0][1][57]
	cmd[0] = 34;
	cmd[1] = 0;

	I2C_IF_Write(Codec_addr, &cmd[0],1,send_stop);
	I2C_IF_Read(Codec_addr, &cmd[1], 1);

	UARTprintf(" [0][1][%u]: %X  \r\n", cmd[0], cmd[1]);

	// Read register in [0][1][57]
	cmd[0] = 47;
	cmd[1] = 0;

	I2C_IF_Write(Codec_addr, &cmd[0],1,send_stop);
	I2C_IF_Read(Codec_addr, &cmd[1], 1);

	UARTprintf(" [0][1][%u]: %X  \r\n", cmd[0], cmd[1]);

	xSemaphoreGiveRecursive(i2c_smphr);

	//	w 30 00 00 # Select Page 0
	codec_set_page(0);


	return 0;
}



