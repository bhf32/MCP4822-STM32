/*
 * MCP4822.c
 *
 *  Created on: June 3, 2024
 *      Author: Ben Francis
 */

#include "MCP4822.h"

void MCP4822_handle_init(MCP4822_Handle_t *handle, GPIO_TypeDef *cs_port, uint16_t cs_pin, SPI_HandleTypeDef *hspi){

	handle->CS_Port = cs_port;
	handle->CS_Pin = cs_pin;

	handle->hspi = hspi;
}

MCP4822_STATUS MCP4822_write(MCP4822_Handle_t *handle, uint16_t value, MCP4822_Config_t *out_config){

	if(value > MCP4822_DAC_MAX){
		 return MCP4822_ERROR_INVALID_ARG;
	}

	uint8_t tx_data[2];

	tx_data[0] = ((uint8_t)((out_config->dac_channel) & FIRST_BIT_MASK) << SHIFT_7) |
				 ((uint8_t)((out_config->gain) & FIRST_BIT_MASK) << SHIFT_5) |
				 ((uint8_t)((out_config->shutdown) & FIRST_BIT_MASK) << SHIFT_4) |
				 (uint8_t)((value >> SHIFT_8) & LOW_HALF_BYTE_MASK);

	tx_data[1] = (uint8_t)(value & FIRST_BYTE_MASK);

	/*uint16_t tx_data = (((uint16_t)(out_config->dac_channel) & FIRST_BIT_MASK) << SHIFT_15) |
					   (((uint16_t)(out_config->gain) & FIRST_BIT_MASK) << SHIFT_13) |
					   (((uint16_t)(out_config->shutdown) & FIRST_BIT_MASK) << SHIFT_12) | value;*/

	HAL_GPIO_WritePin(handle->CS_Port, handle->CS_Pin, GPIO_PIN_RESET);
	HAL_StatusTypeDef spi_status = HAL_SPI_Transmit(handle->hspi, tx_data, sizeof(tx_data), MCP4822_SPI_TIMEOUT);
	HAL_GPIO_WritePin(handle->CS_Port, handle->CS_Pin, GPIO_PIN_SET);

	if(spi_status != HAL_OK){
		return MCP4822_ERROR_SPI;
	}

	return MCP4822_OK;

}
