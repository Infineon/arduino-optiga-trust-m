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
* \file pal_gpio_arduino.cpp
*
* \brief   This file implements the platform abstraction layer APIs for GPIO.
*
* \ingroup  grPAL
*
* @{
*/

#include <Arduino.h>
#include "pal_gpio.h"

LIBRARY_EXPORTS void pal_gpio_set_high(const pal_gpio_t * p_gpio_context)
{
   if ((p_gpio_context != NULL) && (p_gpio_context->p_gpio_hw != NULL))
   {
       digitalWrite(*(uint8_t*)p_gpio_context->p_gpio_hw, HIGH);
   }
}

LIBRARY_EXPORTS void pal_gpio_set_low(const pal_gpio_t * p_gpio_context)
{
    if ((p_gpio_context != NULL) && (p_gpio_context->p_gpio_hw != NULL))
    {
        digitalWrite(*(uint8_t*)p_gpio_context->p_gpio_hw, LOW);
    }
}

LIBRARY_EXPORTS pal_status_t pal_gpio_init(const pal_gpio_t * p_gpio_context)
{
    if ((p_gpio_context != NULL) && (p_gpio_context->p_gpio_hw != NULL))
    {
        pinMode(*(uint8_t*)p_gpio_context->p_gpio_hw, OUTPUT);
    }
    return PAL_STATUS_SUCCESS;
}

LIBRARY_EXPORTS pal_status_t pal_gpio_deinit(const pal_gpio_t * p_gpio_context)
{
    return PAL_STATUS_SUCCESS;
}