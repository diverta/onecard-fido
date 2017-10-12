/*
 * UC8156.h
 *
 *  Created on: 2016/05/11
 *      Author: K.Shoji
 */

#ifndef UC8156_H_
#define UC8156_H_

#include <stdint.h>

#define GATE_LINES_MAX 160
#define SOURCE_LINES_MAX 240

enum UPDATE_MODES {FULL_UPDATE=0x00, PARTIAL_UPDATE=0x04, INIT_UPDATE=0x10};

#define WAVEFORM_FROM_LUT 0x00
#define WAVEFORM_FROM_MTP 0x02
//#define UPDATE_WAVEFORMSOURCESELECT WAVEFORM_FROM_LUT
#define UPDATE_WAVEFORMSOURCESELECT WAVEFORM_FROM_MTP

#define CMD_REVISION							0x00
#define CMD_PANEL_SETTING						0x01
#define CMD_DRIVER_VOLTAGE_SETTING				0x02
#define CMD_POWER_CONTROL_SETTING				0x03
#define CMD_BOOST_SETTING						0x04
#define CMD_VCOM_AND_DATA_INTERVAL_SETTING		0x05
#define CMD_TCOM_TIMING_SETTING					0x06
#define CMD_TEMP_SENSOR_CONFIG					0x07
#define CMD_TEMP_VALUE_REGISTER					0x08
#define CMD_GPIO_CONGIG_REGISTER				0x09
#define CMD_GPIO_INTERRUPT_REGISTER				0x0A
#define CMD_GPIO_PORT_TYPE_REGISTER				0x0B
#define CMD_PANEL_RESOLUTION_SETTING			0x0C
#define CMD_WHITE_PIXEL_RECT_SETTING			0x0D
#define CMD_PIXEL_ACCESS_POSITION_SETTING		0x0E
#define CMD_DATA_ENTRY_MODE_SETTING				0x0F
#define CMD_WRITE_RAM							0x10
#define CMD_READ_RAM							0x11
#define CMD_BYPASS_UPDATE_SETTING				0x12
#define CMD_INITIAL_UPDATE_SETTING				0x13
#define CMD_DISPLAY_ENGINE_CONTROL_REGISTER		0x14
#define	CMD_STATUS_REGISTER						0x15
#define CMD_INTERRUPT_ENABLE_REGISTER			0x16
#define CMD_INTERRUPT_STATUS_REGISTER			0x17
#define CMD_VCOM_CONFIG_REGISTER				0x18
#define CMD_AUTO_MEASURE_VCOM					0x19
#define CMD_VCOM_MEASURE_VALUE					0x1A
#define CMD_VCOM_DC_SETTING						0x1B
#define CMD_WAVEFORM_LUT_SETTING				0x1C
#define CMD_VBORDER_SETTING						0x1D
#define CMD_POWER_SEQUENCE_SETTING				0x1F
#define CMD_SOFTWARE_RESET						0x20
#define CMD_SLEEP_MODE							0x21
#define CMD_PROGRAM_WS_MTP						0x40
#define CMD_MTP_ADDRESS_SETTING					0x41
#define CMD_MTP_ONE_BYTE_PROGRAM				0x42
#define CMD_MTP_READ							0x43

#define DATA_ENTRY_MODE_BIT_SPIOEN				0x20
#define DATA_ENTRY_MODE_BIT_RAMSEL				0x10
#define DATA_ENTRY_MODE_BIT_DEM					0x07



#define WAVEFORM_LENGTH 120
#define TS_LENGTH 10

void UC8156_configure_interface(void);
void UC8156_power_on(void);
void UC8156_power_off(void);
void UC8156_hardware_reset(void);
void UC8156_wait_for_BUSY_inactive(void);
void UC8156_init_registers(void);
void UC8156_HVs_on(void);
void UC8156_HVs_off(void);

uint8_t UC8156_read_RevID(void);

void UC8156_send_waveform(uint8_t *waveform);
void UC8156_set_Vcom(int Vcom_mv_value);
void UC8156_send_image_data(uint8_t *image_data);
void UC8156_send_repeated_image_data(uint8_t image_data);
void UC8156_update_display(uint8_t mode);
void UC8156_send_data_to_image_RAM_for_MTP_program(uint8_t *waveform_data, uint16_t size);
uint8_t UC8156_read_status_register(void);
//void UC8156_read_image_data(uint8_t *image_data);

#endif /* UC8156_H_ */
