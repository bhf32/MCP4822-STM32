/*
 * MCP4822.c
 *
 *  Created on: June 3, 2024
 *      Author: Ben Francis
 */

#include "MCP4822.h"

static inline MCP4822_Config_t *choose_DAC_chan(MCP4822_Handle_t *handle, MCP4822_DAC_SELECT dac_channel);

static inline uint16_t volts_to_DAC_units(float volts, MCP4822_OUTPUT_GAIN gain);

void MCP4822_handle_init(MCP4822_Handle_t *handle, GPIO_TypeDef *cs_port, uint16_t cs_pin, SPI_HandleTypeDef *hspi){

	handle->CS_Port = cs_port;
	handle->CS_Pin = cs_pin;

	handle->hspi = hspi;

	handle->chan_configs.chan_sel = MCP4822_CHANNEL_A;

	handle->chan_configs.chan_A_config.gain = MCP4822_GAIN_1X;
	handle->chan_configs.chan_A_config.shutdown = MCP4822_ACTIVE_MODE;

	handle->chan_configs.chan_B_config.gain = MCP4822_GAIN_1X;
	handle->chan_configs.chan_B_config.shutdown = MCP4822_ACTIVE_MODE;
}

MCP4822_STATUS MCP4822_transmit_data(MCP4822_Handle_t *handle, uint16_t value){

	if(value > MCP4822_DAC_MAX){
		 return MCP4822_ERROR_INVALID_ARG;
	}

	MCP4822_Config_t *curr_chan_config = choose_DAC_chan(handle, handle->chan_configs.chan_sel);

	uint8_t tx_data[2];

	tx_data[0] = ((uint8_t)((handle->chan_configs.chan_sel) & FIRST_BIT_MASK) << SHIFT_7) |
				 ((uint8_t)((curr_chan_config->gain) & FIRST_BIT_MASK) << SHIFT_5) |
				 ((uint8_t)((curr_chan_config->shutdown) & FIRST_BIT_MASK) << SHIFT_4) |
				 (uint8_t)((value >> SHIFT_8) & LOW_HALF_BYTE_MASK);

	tx_data[1] = (uint8_t)(value & FIRST_BYTE_MASK);

	HAL_GPIO_WritePin(handle->CS_Port, handle->CS_Pin, GPIO_PIN_RESET);
	HAL_StatusTypeDef spi_status = HAL_SPI_Transmit(handle->hspi, tx_data, sizeof(tx_data), MCP4822_SPI_TIMEOUT);
	HAL_GPIO_WritePin(handle->CS_Port, handle->CS_Pin, GPIO_PIN_SET);

	if(spi_status != HAL_OK){
		return MCP4822_ERROR_SPI;
	}

	return MCP4822_OK;

}

MCP4822_STATUS MCP4822_shutdown_chan(MCP4822_Handle_t *handle, MCP4822_DAC_SELECT dac_channel){

	MCP4822_Config_t *curr_chan_config = choose_DAC_chan(handle, dac_channel);
	curr_chan_config->shutdown = MCP4822_SHUTDOWN_MODE;

	const uint16_t dummy_value = 0x0000;

	MCP4822_STATUS status = MCP4822_transmit_data(handle, dummy_value);

	return status;
}

MCP4822_STATUS MCP4822_activate_chan(MCP4822_Handle_t *handle, MCP4822_DAC_SELECT dac_channel){

	MCP4822_Config_t *curr_chan_config = choose_DAC_chan(handle, dac_channel);
	curr_chan_config->shutdown = MCP4822_ACTIVE_MODE;

	const uint16_t dummy_value = 0x0000;

	MCP4822_STATUS status = MCP4822_transmit_data(handle, dummy_value);

	return status;
}

void MCP4822_set_chan_gain(MCP4822_Handle_t *handle, MCP4822_DAC_SELECT dac_channel, MCP4822_OUTPUT_GAIN gain_update){

	MCP4822_Config_t *curr_chan_config = choose_DAC_chan(handle, dac_channel);

	curr_chan_config->gain = gain_update;

}

MCP4822_STATUS MCP4822_write_to_chan(MCP4822_Handle_t *handle, uint16_t value, MCP4822_DAC_SELECT dac_channel){

	choose_DAC_chan(handle, dac_channel);

	MCP4822_STATUS status = MCP4822_transmit_data(handle, value);

	return status;
}

MCP4822_STATUS MCP4822_write_to_both_chans(MCP4822_Handle_t *handle, uint16_t value){

	MCP4822_STATUS status = MCP4822_write_to_chan(handle, value, MCP4822_CHANNEL_A);

	status = MCP4822_write_to_chan(handle, value, MCP4822_CHANNEL_B);

	return status;
}

MCP4822_STATUS MCP4822_write_volts_to_chan(MCP4822_Handle_t *handle, float volts, MCP4822_DAC_SELECT dac_channel){

	MCP4822_Config_t *curr_chan_config = choose_DAC_chan(handle, dac_channel);

	uint16_t DAC_value = volts_to_DAC_units(volts, curr_chan_config->gain);

	MCP4822_STATUS status = MCP4822_transmit_data(handle, DAC_value);

	return status;
}

MCP4822_STATUS MCP4822_write_volts_to_both_chans(MCP4822_Handle_t *handle, float volts){

	MCP4822_STATUS status = MCP4822_write_volts_to_chan(handle, volts, MCP4822_CHANNEL_A);

	status = MCP4822_write_volts_to_chan(handle, volts, MCP4822_CHANNEL_B);

	return status;
}

static inline MCP4822_Config_t *choose_DAC_chan(MCP4822_Handle_t *handle, MCP4822_DAC_SELECT dac_channel){

	handle->chan_configs.chan_sel = dac_channel;
	MCP4822_Config_t *chan_config;
	if(dac_channel == MCP4822_CHANNEL_A){
		chan_config = &(handle->chan_configs.chan_A_config);
	}
	else{
		chan_config = &(handle->chan_configs.chan_B_config);
	}

	return chan_config;
}

static inline uint16_t volts_to_DAC_units(float volts, MCP4822_OUTPUT_GAIN gain){

	uint8_t gain_mult = (gain == MCP4822_GAIN_2X) ? 2 : 1;
    return (uint16_t)(volts * (MCP4822_DAC_MAX + 1)/(MCP4822_VREF * gain_mult));
}
