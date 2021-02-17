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

#include "OPTIGATrustM_v3.h"

#define KEY_MAXLENGTH   300

#define SUPPRESSCOLLORS
#include "fprint.h"

#define ASSERT(err)   if (ret) { printlnRed("Failed"); while (true); }

uint8_t pubKey[KEY_MAXLENGTH] = {0};
uint16_t pubKeyLen = KEY_MAXLENGTH;

#ifdef OPTIGA_TRUST_M_V3
IFX_OPTIGA_TrustM_V3 * trustm = &trustM_V3;
#elif defined(OPTIGA_TRUST_M_V1)
IFX_OPTIGA_TrustM * trustm = &trustM;
#endif

#define ENABLE_PAIRING 0

/**
 * Input data for generating HMAC
 */
const uint8_t input_data_buffer[] = {0x6b, 0xc1, 0xbe, 0xe2,
                                     0x2e, 0x40, 0x9f, 0x96,
                                     0xe9, 0x3d, 0x7e, 0x11,
                                     0x73, 0x93, 0x17, 0x2a};
uint32_t input_data_buffer_length = sizeof(input_data_buffer);

/**
 * Buffer for using in loop
 */
uint8_t mac_buffer[32] = {0};
uint32_t mac_buffer_length = sizeof(mac_buffer);

volatile optiga_lib_status_t optiga_lib_status;

static void output_result(char* tag, uint32_t tstamp, uint8_t* in, uint16_t in_len)
{
  printGreen("[OK] | Command executed in "); 
  Serial.print(tstamp); 
  Serial.println(" ms");
  printMagenta(tag); 
  printMagenta(" Length: ");
  Serial.println(in_len);
  printMagenta(tag); 
  printlnMagenta(":");
  HEXDUMP(in, in_len);
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
	ret = trustm->begin(ENABLE_PAIRING);
	ASSERT(ret);
	printlnGreen("OK");

  /*
   * Speed up the chip (min is 6ma, maximum is 15ma)
   */
  printGreen("Setting Current Limit... ");
	ret = trustm->setCurrentLimit(15);
	ASSERT(ret);
	printlnGreen("OK");

  /*
   * Check the return value which we just set
   */
  printGreen("Checking Power Limit... ");
  uint8_t current_lim = 0;
  ret = trustM_V3.getCurrentLimit(current_lim);
  ASSERT(ret);
  if (current_lim == 15) {
    printlnGreen("OK");
  } else {
    printlnRed("Failed");
    while(1);
  }
}

void loop() 
{
  uint32_t ret = 0;
  uint32_t ts = 0;

  /**
   * Enable V3 capabilities in src/optiga_trustm/optiga_lib_config.h
   */
  #ifdef OPTIGA_TRUST_M_V3

 /**
   * Generate public private keypair
   */
  printlnGreen("\r\nGenerate Key Pair ECC NIST P 256. Store Private Key on Board ... ");
  ts = millis();
  ret = trustm->generateKeypairECC(pubKey, pubKeyLen);
  ts = millis() - ts;
  if (ret) {
    printlnRed("Failed");
    while (true);
  }

  output_result((char*)"Public Key ", ts, pubKey, pubKeyLen);

  /*
   * Calculate shared secret
   */
  printlnGreen("\r\nCalculate shared secret... ");
  ts = millis();
  ret = trustm->sharedSecret(eSESSION_ID_2, pubKey, pubKeyLen);
  ts = millis() - ts;
  if (ret) {
    printlnRed("Failed");
    while (true);
  }
  
  printGreen("[OK] | Command executed in "); 
  Serial.print(ts); 
  Serial.println(" ms");

  /**
   * Generate HMAC on the input data 
   */
  printlnGreen("\r\nGenerate HMAC");
  ts = millis();
  ret = trustm->generateHMACSHA256(input_data_buffer, input_data_buffer_length, mac_buffer, mac_buffer_length );
  ts = millis() - ts;
  if (ret) {
    printlnRed("Failed");
    while (true);
  }
 
  output_result((char*)"HMAC ", ts, mac_buffer, mac_buffer_length);

 #endif /* OPTIGA_TRUST_M_V3 */ 

  /* 
   * Execute the loop just once :)
   */
  while(1){}
}
