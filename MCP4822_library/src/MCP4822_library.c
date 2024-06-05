/*
 * MCP4822_library.c
 *
 *  Created on: June 4, 2024
 *      Author: Ben Francis
 */

#include "MCP4822_library.h"

static MCP4822_Config_t *get_DAC_config(MCP4822_Chan_Configs_t *out_configs, MCP4822_DAC_SELECT dac_channel);

static uint16_t volts_to_DAC_units(float volts, MCP4822_OUTPUT_GAIN gain);

void init_MCP4822_device(MCP4822_Chan_Configs_t *out_configs, MCP4822_Handle_t *handle, GPIO_TypeDef *cs_port, uint16_t cs_pin, SPI_HandleTypeDef *hspi){

	MCP4822_handle_init(handle, cs_port, cs_pin, hspi);

	out_configs->chan_A_config.dac_channel = MCP4822_CHANNEL_A;
	out_configs->chan_A_config.gain = MCP4822_GAIN_1X;
	out_configs->chan_A_config.shutdown = MCP4822_ACTIVE_MODE;

	out_configs->chan_B_config.dac_channel = MCP4822_CHANNEL_B;
	out_configs->chan_B_config.gain = MCP4822_GAIN_1X;
	out_configs->chan_B_config.shutdown = MCP4822_ACTIVE_MODE;
}

MCP4822_STATUS shutdown_chan(MCP4822_Chan_Configs_t *out_configs, MCP4822_Handle_t *handle, MCP4822_DAC_SELECT dac_channel){

	MCP4822_Config_t *curr_chan_config = get_DAC_config(out_configs, dac_channel);
	curr_chan_config->shutdown = MCP4822_SHUTDOWN_MODE;

	const uint16_t dummy_value = 0x0000;

	MCP4822_STATUS status = MCP4822_write(handle, dummy_value, curr_chan_config);

	return status;
}

MCP4822_STATUS activate_chan(MCP4822_Chan_Configs_t *out_configs, MCP4822_Handle_t *handle, MCP4822_DAC_SELECT dac_channel){

	MCP4822_Config_t *curr_chan_config = get_DAC_config(out_configs, dac_channel);
	curr_chan_config->shutdown = MCP4822_ACTIVE_MODE;

	const uint16_t dummy_value = 0x0000;

	MCP4822_STATUS status = MCP4822_write(handle, dummy_value, curr_chan_config);

	return status;
}

void set_chan_gain(MCP4822_Chan_Configs_t *out_configs, MCP4822_DAC_SELECT dac_channel, MCP4822_OUTPUT_GAIN gain_update){

	MCP4822_Config_t *curr_chan_config = get_DAC_config(out_configs, dac_channel);

	curr_chan_config->gain = gain_update;

}

MCP4822_STATUS write_to_chan(MCP4822_Chan_Configs_t *out_configs, MCP4822_Handle_t *handle, uint16_t value, MCP4822_DAC_SELECT dac_channel){

	MCP4822_Config_t *curr_chan_config = get_DAC_config(out_configs, dac_channel);

	MCP4822_STATUS status = MCP4822_write(handle, value, curr_chan_config);

	return status;
}

MCP4822_STATUS write_to_both_chans(MCP4822_Chan_Configs_t *out_configs, MCP4822_Handle_t *handle, uint16_t value){

	MCP4822_STATUS status = write_to_chan(out_configs, handle, value, MCP4822_CHANNEL_A);

	status = write_to_chan(out_configs, handle, value, MCP4822_CHANNEL_B);

	return status;
}

MCP4822_STATUS write_volts_to_chan(MCP4822_Chan_Configs_t *out_configs, MCP4822_Handle_t *handle, float volts, MCP4822_DAC_SELECT dac_channel){

	MCP4822_Config_t *curr_chan_config = get_DAC_config(out_configs, dac_channel);

	uint16_t DAC_value = volts_to_DAC_units(volts, curr_chan_config->gain);

	MCP4822_STATUS status = MCP4822_write(handle, DAC_value, curr_chan_config);

	return status;
}

MCP4822_STATUS write_volts_to_both_chans(MCP4822_Chan_Configs_t *out_configs, MCP4822_Handle_t *handle, float volts){

	MCP4822_STATUS status = write_volts_to_chan(out_configs, handle, volts, MCP4822_CHANNEL_A);

	status = write_volts_to_chan(out_configs, handle, volts, MCP4822_CHANNEL_B);

	return status;
}

static MCP4822_Config_t *get_DAC_config(MCP4822_Chan_Configs_t *out_configs, MCP4822_DAC_SELECT dac_channel){

	MCP4822_Config_t *chan_config;
	if(dac_channel == MCP4822_CHANNEL_A){
		chan_config = &(out_configs->chan_A_config);
	}
	else{
		chan_config = &(out_configs->chan_B_config);
	}

	return chan_config;
}

static uint16_t volts_to_DAC_units(float volts, MCP4822_OUTPUT_GAIN gain){

	uint8_t gain_mult = (gain == MCP4822_GAIN_2X) ? 2 : 1;
    return (uint16_t)(volts * (MCP4822_DAC_MAX + 1)/(MCP4822_VREF * gain_mult));
}
