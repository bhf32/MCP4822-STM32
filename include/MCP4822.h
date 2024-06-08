/*
 * MCP4822.h
 *
 *  Created on: June 3, 2024
 *      Author: Ben Francis
 */

#ifndef __MCP4822_H_
#define __MCP4822_H_

#include <stdlib.h>
#include "stm32l5xx_hal.h"

/** Bit Manipulation Macros */
#define SHIFT_4    			       	   4
#define SHIFT_5    			           5
#define SHIFT_7    			           7
#define SHIFT_8    			           8
#define FIRST_BIT_MASK  			   0x01
#define LOW_HALF_BYTE_MASK			   0x000F
#define FIRST_BYTE_MASK 			   0x00FF
//#define SHIFT_12    			       12
//#define SHIFT_13    			       13
//#define SHIFT_15    			       15

/** Limits and Commonly used conversion Macro*/
#define MCP4822_RES      	   		   12
#define MCP4822_DAC_MAX				   4095
#define MCP4822_VREF				   2.048f
#define MCP4822_SPI_TIMEOUT			   1    //1 msec timeout

typedef enum
{
	MCP4822_CHANNEL_A			     = 0,
	MCP4822_CHANNEL_B			   	 = 1

}MCP4822_DAC_SELECT;

typedef enum
{
	MCP4822_GAIN_2X			   	     = 0,
	MCP4822_GAIN_1X			   	     = 1

}MCP4822_OUTPUT_GAIN;

typedef enum
{
	MCP4822_SHUTDOWN_MODE		   	 = 0,
	MCP4822_ACTIVE_MODE		   	     = 1

}MCP4822_OUTPUT_MODE;

/**
 * @brief Error mapping for driver functions
 */
typedef enum
{
	MCP4822_OK						 =  0,
    MCP4822_ERROR_INVALID_ARG   	 = -1,
    MCP4822_ERROR_SPI 				 = -2

}MCP4822_STATUS;

typedef struct
{

	MCP4822_OUTPUT_GAIN gain;

	MCP4822_OUTPUT_MODE shutdown;

}MCP4822_Config_t;

typedef struct
{

	MCP4822_Config_t chan_A_config;

	MCP4822_Config_t chan_B_config;

	MCP4822_DAC_SELECT chan_sel;

}MCP4822_Chan_Configs_t;

typedef struct
{

	MCP4822_Chan_Configs_t chan_configs;

	GPIO_TypeDef *CS_Port;

	uint16_t CS_Pin;

    SPI_HandleTypeDef *hspi;

}MCP4822_Handle_t;

void MCP4822_handle_init(MCP4822_Handle_t *handle, GPIO_TypeDef *cs_port, uint16_t cs_pin, SPI_HandleTypeDef *hspi);

MCP4822_STATUS MCP4822_transmit_data(MCP4822_Handle_t *handle, uint16_t value);

MCP4822_STATUS MCP4822_shutdown_chan(MCP4822_Handle_t *handle, MCP4822_DAC_SELECT dac_channel);

MCP4822_STATUS MCP4822_activate_chan(MCP4822_Handle_t *handle, MCP4822_DAC_SELECT dac_channel);

void MCP4822_set_chan_gain(MCP4822_Handle_t *handle, MCP4822_DAC_SELECT dac_channel, MCP4822_OUTPUT_GAIN gain_update);

MCP4822_STATUS MCP4822_write_to_chan(MCP4822_Handle_t *handle, uint16_t value, MCP4822_DAC_SELECT dac_channel);

MCP4822_STATUS MCP4822_write_to_both_chans(MCP4822_Handle_t *handle, uint16_t value);

MCP4822_STATUS MCP4822_write_volts_to_chan(MCP4822_Handle_t *handle, float volts, MCP4822_DAC_SELECT dac_channel);

MCP4822_STATUS MCP4822_write_volts_to_both_chans(MCP4822_Handle_t *handle, float volts);

#endif /* __MCP4822_H_ */
