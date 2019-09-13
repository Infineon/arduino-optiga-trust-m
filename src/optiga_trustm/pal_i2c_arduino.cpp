/**
* \copyright
* MIT License
*
* Copyright (c) 2019 Infineon Technologies AG
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE
*
* \endcopyright
*
* \author Infineon Technologies AG
*
* \file pal_i2c_arduino.cpp
*
* \brief   This file implements the platform abstraction layer(pal) APIs for I2C.
*
* \ingroup  grPAL
*
* @{
*/
#include <Arduino.h>
#include <Wire.h>
#include "pal_i2c.h"

#define PAL_I2C_MASTER_MAX_BITRATE  (400)

LIBRARY_EXPORTS pal_status_t pal_i2c_init(const pal_i2c_t * p_i2c_context)
{
	pal_status_t status = PAL_STATUS_FAILURE;

	if ((p_i2c_context != NULL) && (p_i2c_context->p_i2c_hw_config != NULL))
	{
		((TwoWire *)(p_i2c_context->p_i2c_hw_config))->begin();
		status = PAL_STATUS_SUCCESS;
	}

    return status;
}

LIBRARY_EXPORTS pal_status_t pal_i2c_set_bitrate(const pal_i2c_t * p_i2c_context, uint16_t bitrate)
{
	pal_status_t status = PAL_STATUS_FAILURE;

	if (bitrate > PAL_I2C_MASTER_MAX_BITRATE)
	{
		bitrate = PAL_I2C_MASTER_MAX_BITRATE;
	}

	if ((p_i2c_context != NULL) && (p_i2c_context->p_i2c_hw_config != NULL))
	{
		((TwoWire *)(p_i2c_context->p_i2c_hw_config))->setClock(bitrate*1000);
		status = PAL_STATUS_SUCCESS;
	}

	return status;
}

#define MAX_POLLING	50
LIBRARY_EXPORTS pal_status_t pal_i2c_write(pal_i2c_t * p_i2c_context, uint8_t * p_data, uint16_t length)
{
	upper_layer_callback_t upper_layer_handler;
	upper_layer_handler = (upper_layer_callback_t)p_i2c_context->upper_layer_event_handler;
    pal_status_t status = PAL_STATUS_FAILURE;
    TwoWire * i2c = NULL;
    uint16_t ack = 0;
    uint8_t tx_cntr = 0;
    //Serial.println("inside this thing  ");
    do {
    	if (p_i2c_context == NULL)
    		break;
		
    	if (p_i2c_context->p_i2c_hw_config == NULL)
    		break;
	    
    	i2c = (TwoWire *)p_i2c_context->p_i2c_hw_config;

		i2c->beginTransmission(p_i2c_context->slave_address);
		i2c->write(p_data, length);
		ack = i2c->endTransmission(true);

		if (ack == 0) {
			upper_layer_handler(p_i2c_context->p_upper_layer_ctx, PAL_I2C_EVENT_SUCCESS);
			status = PAL_STATUS_SUCCESS;
			Serial.println("ACK");
			// Serial.print("->  ");
			// for(int i = 0; i < length; i++)
			// {
			// 	Serial.print("0x");
			//     Serial.print(p_data[i], HEX);
			// 	Serial.print(" ");
			// }
			// Serial.print("\n");
			
		} else {
			upper_layer_handler(p_i2c_context->p_upper_layer_ctx, PAL_I2C_EVENT_ERROR);
			status = PAL_STATUS_FAILURE;
			Serial.println("NAK");
			// Serial.print("~->\n");
		}
    }while(0);


    return status;
}

LIBRARY_EXPORTS pal_status_t pal_i2c_read(pal_i2c_t * p_i2c_context, uint8_t * p_data, uint16_t length)
{
    upper_layer_callback_t upper_layer_handler;
	upper_layer_handler = (upper_layer_callback_t)p_i2c_context->upper_layer_event_handler;
    pal_status_t status = PAL_STATUS_FAILURE;
    TwoWire * i2c = NULL;
    uint16_t rx_len = 0;
    int 	 bytes = length;

    do {
    	if (p_i2c_context == NULL)
    		break;

    	if (p_i2c_context->p_i2c_hw_config == NULL)
    		break;

    	i2c = (TwoWire *)p_i2c_context->p_i2c_hw_config;

		bytes = i2c->requestFrom((int)p_i2c_context->slave_address, bytes);

		if (bytes == 0)
		{
			//Serial.print("~<\n");
			upper_layer_handler(p_i2c_context->p_upper_layer_ctx, PAL_I2C_EVENT_ERROR);
			break;
		}

		while (i2c->available() && (rx_len < length))
		{
			p_data[rx_len] = i2c->read();
			rx_len++;
		}

		if (rx_len == length)
		{
			upper_layer_handler(p_i2c_context->p_upper_layer_ctx, PAL_I2C_EVENT_SUCCESS);
			status = PAL_STATUS_SUCCESS;
			/*
			Serial.print("<-  ");
			for(int i = 0; i < rx_len; i++)
			{
				Serial.print("0x");
			    Serial.print(p_data[i], HEX);
				Serial.print(" ");
			}
			Serial.print("\n");
			*/
		}
		else {
			//Serial.print("~<-\n");
			upper_layer_handler(p_i2c_context->p_upper_layer_ctx, PAL_I2C_EVENT_ERROR);
		}

    }while(0);

    return status;
}

LIBRARY_EXPORTS pal_status_t pal_i2c_deinit(const pal_i2c_t * p_i2c_context)
{
    pal_status_t status = PAL_STATUS_FAILURE;

	if ((p_i2c_context != NULL) && (p_i2c_context->p_i2c_hw_config != NULL))
	{
		status = PAL_STATUS_SUCCESS;
	}

    return status;
}