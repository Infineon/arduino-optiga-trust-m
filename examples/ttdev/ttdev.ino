/**
 * MIT License
 *
 * Copyright (c) 2018 Infineon Technologies AG
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
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
 * Demonstrates use of the 
 * Infineon Technologies AG OPTIGA™ Trust X Arduino library
 */

#include <OPTIGATrustM.h>

void setup() 
{
    /*
     * Initialise a serial port for debug output
     */
    Serial.begin(100000);
    delay(100);
    Serial.println("Initializing ... ");

    //getState();
    example_optiga_util_read_data();
    //example_optiga_util_read_uuid();
    //example_optiga_util_hibernate_restore();
}

void loop()
{

}



void upeh(void *, pal_status_t status)
{
   if (status == PAL_I2C_EVENT_ERROR)
   {
       Serial.println("NAK");
   }
   if(status == PAL_I2C_EVENT_SUCCESS)
   {
       Serial.println("ACK");
   }
}

pal_status_t getState()
{
    uint8_t reg = 0x82;
    uint8_t value[6] = {0};
    optiga_pal_i2c_context_0.upper_layer_event_handler = upeh;
 

    pal_i2c_init(&optiga_pal_i2c_context_0);
    Serial.println("initialized");
    pal_i2c_set_bitrate(&optiga_pal_i2c_context_0,50);

    while(pal_i2c_write(&optiga_pal_i2c_context_0, &reg, 1) == PAL_STATUS_FAILURE){};
    while(pal_i2c_read(&optiga_pal_i2c_context_0, value, 4) == PAL_STATUS_FAILURE){};
    Serial.print("value: ");
    for(int i = 0; i < 6; i++)
    {
        Serial.print(value[i]);
        Serial.print(".");
    }
        Serial.println(" ");
}
