/*
 * MCP4822.c
 *
 *  Created on: June 3, 2024
 *      Author: Ben Francis
 */

#include "MCP4822.h"

static inline MCP4822_Config_t *get_chan_config(MCP4822_Handle_t *handle, MCP4822_DAC_SELECT dac_channel);

static inline uint16_t volts_to_DAC_units(float volts, MCP4822_OUTPUT_GAIN gain);

/**
 * Initialize the MCP4822 driver handle and enable the outputs
 *
 * @param handle - handle for MCP4822 driver
 * @param cs_port - CS pin GPIO port
 * @param cs_pin - CS GPIO pin number
 * @param hspi - STM32x SPI peripheral handle
 *
 * @return None
 */
void MCP4822_handle_init(MCP4822_Handle_t *handle, GPIO_TypeDef *cs_port, uint16_t cs_pin, SPI_HandleTypeDef *hspi){

	//Assign the port and pins for the SPI CS pin
	handle->CS_Port = cs_port;
	handle->CS_Pin = cs_pin;

	handle->hspi = hspi;

	//Initialize both channel configurations
	handle->chan_configs.chan_A_config.gain = MCP4822_GAIN_1X;
	handle->chan_configs.chan_A_config.shutdown = MCP4822_ACTIVE_MODE;

	handle->chan_configs.chan_B_config.gain = MCP4822_GAIN_1X;
	handle->chan_configs.chan_B_config.shutdown = MCP4822_ACTIVE_MODE;
}

/**
 * Write new DAC data to one of the MCP4822 device channels using SPI
 *
 * @param handle - handle for MCP4822 driver
 * @param value - digital value to be sent to DAC
 * @param dac_channel - DAC channel to be written to
 *
 * @return MCP4822_OK in case of success, MCP4822_ERROR_INVALID_ARG or MCP4822_ERROR_SPI otherwise
 */
MCP4822_STATUS MCP4822_write_to_chan(MCP4822_Handle_t *handle, uint16_t value, MCP4822_DAC_SELECT dac_channel){

	//Limit value to the max input for MCP4822
	if(value > MCP4822_DAC_MAX){
		 return MCP4822_ERROR_INVALID_ARG;
	}

	//Receive the correct DAC channel configuration
	MCP4822_Config_t *curr_chan_config = get_chan_config(handle, dac_channel);

	uint8_t tx_data[2];

	//Prepare the tx buffer to be transmitted over SPI to the device
	tx_data[0] = ((uint8_t)((dac_channel) & FIRST_BIT_MASK) << SHIFT_7) |
				 ((uint8_t)((curr_chan_config->gain) & FIRST_BIT_MASK) << SHIFT_5) |
				 ((uint8_t)((curr_chan_config->shutdown) & FIRST_BIT_MASK) << SHIFT_4) |
				 (uint8_t)((value >> SHIFT_8) & LOW_HALF_BYTE_MASK);

	tx_data[1] = (uint8_t)(value & FIRST_BYTE_MASK);

	//Transmit the tx buffer over SPI
	HAL_GPIO_WritePin(handle->CS_Port, handle->CS_Pin, GPIO_PIN_RESET);
	HAL_StatusTypeDef spi_status = HAL_SPI_Transmit(handle->hspi, tx_data, sizeof(tx_data), MCP4822_SPI_TIMEOUT);
	HAL_GPIO_WritePin(handle->CS_Port, handle->CS_Pin, GPIO_PIN_SET);

	if(spi_status != HAL_OK){
		return MCP4822_ERROR_SPI;
	}

	return MCP4822_OK;
}

/**
 * Shutdown one of the DAC channels
 *
 * @param handle - handle for MCP4822 driver
 * @param dac_channel - DAC channel to be shutdown
 *
 * @return MCP4822_OK in case of success, MCP4822_ERROR_SPI otherwise
 */
MCP4822_STATUS MCP4822_shutdown_chan(MCP4822_Handle_t *handle, MCP4822_DAC_SELECT dac_channel){

	//Receive the correct DAC channel configuration
	MCP4822_Config_t *curr_chan_config = get_chan_config(handle, dac_channel);

	//Set the channel and its circuitry to be shutdown
	curr_chan_config->shutdown = MCP4822_SHUTDOWN_MODE;

	//Write the channel shutdown condition to the device
	const uint16_t dummy_value = 0x0000;
	MCP4822_STATUS status = MCP4822_write_to_chan(handle, dummy_value, dac_channel);

	return status;
}

/**
 * Activate one of the DAC channels
 *
 * @param handle - handle for MCP4822 driver
 * @param dac_channel - DAC channel to be activated
 *
 * @return MCP4822_OK in case of success, MCP4822_ERROR_SPI otherwise
 */
MCP4822_STATUS MCP4822_activate_chan(MCP4822_Handle_t *handle, MCP4822_DAC_SELECT dac_channel){

	//Receive the correct DAC channel configuration
	MCP4822_Config_t *curr_chan_config = get_chan_config(handle, dac_channel);

	//Set the channel and its circuitry to be activated
	curr_chan_config->shutdown = MCP4822_ACTIVE_MODE;

	//Write the channel activation condition to the device
	const uint16_t dummy_value = 0x0000;
	MCP4822_STATUS status = MCP4822_write_to_chan(handle, dummy_value, dac_channel);

	return status;
}

/**
 * Set the gain for one of the DAC channels
 *
 * @param handle - handle for MCP4822 driver
 * @param dac_channel - DAC channel that's gain will be changed
 * @param gain_update - updated gain value (1X or 2X)
 *
 * @return None
 */
void MCP4822_set_chan_gain(MCP4822_Handle_t *handle, MCP4822_DAC_SELECT dac_channel, MCP4822_OUTPUT_GAIN gain_update){

	//Receive the correct DAC channel configuration
	MCP4822_Config_t *curr_chan_config = get_chan_config(handle, dac_channel);

	//Update the DAC channel gain
	curr_chan_config->gain = gain_update;
}

/**
 * Write new DAC data to both of the MCP4822 device channels using SPI
 *
 * @param handle - handle for MCP4822 driver
 * @param value - digital value to be sent to the DACs
 *
 * @return MCP4822_OK in case of success, MCP4822_ERROR_INVALID_ARG or MCP4822_ERROR_SPI otherwise
 */
MCP4822_STATUS MCP4822_write_to_both_chans(MCP4822_Handle_t *handle, uint16_t value){

	//Write the value to channel A
	MCP4822_STATUS status = MCP4822_write_to_chan(handle, value, MCP4822_CHANNEL_A);
	if(status != MCP4822_OK){
			return status;
	}

	//Write the value to channel B
	status = MCP4822_write_to_chan(handle, value, MCP4822_CHANNEL_B);

	return status;
}

/**
 * Write new DAC data, after converting from volts, to one of the MCP4822 device channels using SPI
 *
 * @param handle - handle for MCP4822 driver
 * @param volts - voltage value to be converted and sent to the DAC
 * @param dac_channel - DAC channel to be written to
 *
 * @return MCP4822_OK in case of success, MCP4822_ERROR_INVALID_ARG or MCP4822_ERROR_SPI otherwise
 */
MCP4822_STATUS MCP4822_write_volts_to_chan(MCP4822_Handle_t *handle, float volts, MCP4822_DAC_SELECT dac_channel){

	//Receive the correct DAC channel configuration
	MCP4822_Config_t *curr_chan_config = get_chan_config(handle, dac_channel);

	//Convert the voltage value to DAC units
	uint16_t DAC_value = volts_to_DAC_units(volts, curr_chan_config->gain);

	//Write the converted voltage to the correct channel
	MCP4822_STATUS status = MCP4822_write_to_chan(handle, DAC_value, dac_channel);

	return status;
}

/**
 * Write new DAC data, after converting from volts, to both of the MCP4822 device channels using SPI
 *
 * @param handle - handle for MCP4822 driver
 * @param volts - voltage value to be converted and sent to the DACs
 *
 * @return MCP4822_OK in case of success, MCP4822_ERROR_INVALID_ARG or MCP4822_ERROR_SPI otherwise
 */
MCP4822_STATUS MCP4822_write_volts_to_both_chans(MCP4822_Handle_t *handle, float volts){

	//Convert and write the voltage to channel A
	MCP4822_STATUS status = MCP4822_write_volts_to_chan(handle, volts, MCP4822_CHANNEL_A);
	if(status != MCP4822_OK){
		return status;
	}

	//Convert and write the voltage to channel A
	status = MCP4822_write_volts_to_chan(handle, volts, MCP4822_CHANNEL_B);

	return status;
}

/**
 * Retrieve the pointer to the correct channel configuration
 *
 * @param handle - handle for MCP4822 driver
 * @param dac_channel - channel configuration to be returned
 *
 * @return Pointer to DAC channel configuration struct
 */
static inline MCP4822_Config_t *get_chan_config(MCP4822_Handle_t *handle, MCP4822_DAC_SELECT dac_channel){

	//Assign pointer to the dac_channel configuration
	MCP4822_Config_t *chan_config;
	if(dac_channel == MCP4822_CHANNEL_A){
		chan_config = &(handle->chan_configs.chan_A_config);
	}
	else{
		chan_config = &(handle->chan_configs.chan_B_config);
	}

	return chan_config;
}

/**
 * Convert voltage units to DAC digital units
 *
 * @param volts - voltage value to be converted to DAC digital units
 * @param gain - DAC channel gain
 *
 * @return Converted voltage value
 */
static inline uint16_t volts_to_DAC_units(float volts, MCP4822_OUTPUT_GAIN gain){

	//Calculate the DAC value based on the voltage
	uint8_t gain_mult = (gain == MCP4822_GAIN_2X) ? 2 : 1;

    return (uint16_t)(volts * (MCP4822_DAC_MAX + 1)/(MCP4822_VREF * gain_mult));
}
