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
 * Infineon Technologies AG OPTIGA™ Trust M Arduino library
 */

#include "OPTIGATrustM.h"
#include "OPTIGATrustM_v3.h"

#define SUPPRESSCOLLORS
#include "fprint.h"

#define ASSERT(err)   if (ret) { printlnRed("Failed"); while (true); }

/**
 * Input data for generating HMAC
 */
const uint8_t input_data_buffer_start[] = {0x6b, 0xc1, 0xbe, 0xe2,
                                           0x2e, 0x40, 0x9f, 0x96,
                                           0xe9, 0x3d, 0x7e, 0x11,
                                           0x73, 0x93, 0x17, 0x2a};
const uint8_t input_data_buffer_update[] = {0x7c,0xd2,0xcf,0xf3,
                                            0x3f,0x51,0xa0,0xa7,
                                            0xf0,0x4e,0x8f,0x22,
                                            0x84,0xa4,0x28,0x3b};
const uint8_t input_data_buffer_final[] = {0x5a,0xb0,0xad,0xd1,
                                           0x1d,0x3f,0x8e,0x85,
                                           0xd8,0x2c,0x6d,0xf1,
                                           0x62,0x82,0x06,0x19};

/**
 * Buffer for using in loop
 */
uint8_t mac_buffer[32] = {0};
uint32_t mac_buffer_length = sizeof(mac_buffer);

static void output_result(char* tag, uint8_t* in, uint16_t in_len)
{
  printlnGreen("OK"); 
  printMagenta(tag); 
  printMagenta(" Length: ");
  Serial.println(in_len);
  printMagenta(tag); 
  printlnMagenta(":");
  HEXDUMP(in, in_len);
}

void loop() 
{
  uint32_t ret = 0;
  uint32_t ts = 0;
  uint8_t ifxPublicKey[68];

  /*
   * Extract public key of the device certificate
   */
  printlnGreen("\r\nGet IFX public key ... ");
  trustM.getPublicKey(ifxPublicKey);
   
  output_result("My Public Key", ifxPublicKey, sizeof(ifxPublicKey));

  /*
   * Calculate shared secret
   */
  printlnGreen("\r\nCalculate shared secret... ");
  ts = millis();
  ret = trustM.sharedSecret(eSESSION_ID_1, ifxPublicKey, sizeof(ifxPublicKey));
  ts = millis() - ts;
  if (ret) {
    printlnRed("Failed");
    while (true);
  }
  
  printGreen("[OK] | Command executed in "); 
  Serial.print(ts); 
  Serial.println(" ms");

  /**
   * Start HMAC on the input data 
   */
  printlnGreen("\r\nStart to generate HMAC");
  ts = millis();
  ret = trustM_V3.generateHMAC(OPTIGA_HMAC_SHA_256, OPTIGA_KEY_ID_E0F0, input_data_buffer_start, (uint32_t)(sizeof(input_data_buffer_start)));
  ts = millis() - ts;
  if (ret) {
    printlnRed("Failed");
    while (true);
  }
  
  printGreen("[OK] | Command executed in "); 
  Serial.print(ts); 
  Serial.println(" ms");

  /**
   * Enable V3 capabilities in src/optiga_trustm/optiga_lib_config.h
   */
  #ifdef OPTIGA_TRUST_M_V3
  
  /**
   * Update HMAC on the input data 
   */
  printlnGreen("\r\nStart to generate HMAC");
  ts = millis();
  ret = trustM_V3.generateHMAC(input_data_buffer_update, (uint32_t)(sizeof(input_data_buffer_update)));
  ts = millis() - ts;
  if (ret) {
    printlnRed("Failed");
    while (true);
  }
  
  printGreen("[OK] | Command executed in "); 
  Serial.print(ts); 
  Serial.println(" ms");

  /**
   * Finalize HMAC on the input data 
   */
  printlnGreen("\r\nStart to generate HMAC");
  ts = millis();
  ret = trustM_V3.generateHMAC(input_data_buffer_final, (uint32_t)(sizeof(input_data_buffer_final)), mac_buffer, &mac_buffer_length);
  ts = millis() - ts;
  if (ret) {
    printlnRed("Failed");
    while (true);
  }

 #endif /* OPTIGA_TRUST_M_V3 */ 

  printGreen("[OK] | Command executed in "); 
  Serial.print(ts); 
  Serial.println(" ms");

  /* 
   * Execute the loop just once :)
   */
  while(1){}
}

void setup()
{
	uint32_t ret = 0;

  /*
   * Initialise serial output
   */
	Serial.begin(115200);
	Serial.println("Initializing ... ");

  /*
   * Initialise OPTIGA™ Trust M board
   */
	printGreen("Begin Trust ... ");
	ret = trustM.begin();
	ASSERT(ret);
	printlnGreen("OK");

  /*
   * Speed up the chip (min is 6ma, maximum is 15ma)
   */
  printGreen("Setting Current Limit... ");
	ret = trustM.setCurrentLimit(15);
	ASSERT(ret);
	printlnGreen("OK");

  /*
   * Check the return value which we just set
   */
  printGreen("Checking Power Limit... ");
  uint8_t current_lim = 0;
  ret = trustM.getCurrentLimit(current_lim);
  ASSERT(ret);
  if (current_lim == 15) {
    printlnGreen("OK");
  } else {
    printlnRed("Failed");
    while(1);
  }
}
