/*
 * MCP4822_library.h
 *
 *  Created on: June 4, 2024
 *      Author: Ben Francis
 */

#ifndef __MCP4822_LIBRARY_H_
#define __MCP4822_LIBRARY_H_

#include <stdbool.h>
#include "MCP4822.h"

typedef struct
{

	MCP4822_Config_t chan_A_config;

	MCP4822_Config_t chan_B_config;

}MCP4822_Chan_Configs_t;

void init_MCP4822_device(MCP4822_Chan_Configs_t *out_configs, MCP4822_Handle_t *handle, GPIO_TypeDef *cs_port, uint16_t cs_pin, SPI_HandleTypeDef *hspi);

MCP4822_STATUS shutdown_chan(MCP4822_Chan_Configs_t *out_configs, MCP4822_Handle_t *handle, MCP4822_DAC_SELECT dac_channel);

MCP4822_STATUS activate_chan(MCP4822_Chan_Configs_t *out_configs, MCP4822_Handle_t *handle, MCP4822_DAC_SELECT dac_channel);

void set_chan_gain(MCP4822_Chan_Configs_t *out_configs, MCP4822_DAC_SELECT dac_channel, MCP4822_OUTPUT_GAIN gain_update);

MCP4822_STATUS write_to_chan(MCP4822_Chan_Configs_t *out_configs, MCP4822_Handle_t *handle, uint16_t value, MCP4822_DAC_SELECT dac_channel);

MCP4822_STATUS write_to_both_chans(MCP4822_Chan_Configs_t *out_configs, MCP4822_Handle_t *handle, uint16_t value);

MCP4822_STATUS write_volts_to_chan(MCP4822_Chan_Configs_t *out_configs, MCP4822_Handle_t *handle, float volts, MCP4822_DAC_SELECT dac_channel);

MCP4822_STATUS write_volts_to_both_chans(MCP4822_Chan_Configs_t *out_configs, MCP4822_Handle_t *handle, float volts);

#endif /* __MCP4822_LIBRARY_H_ */
