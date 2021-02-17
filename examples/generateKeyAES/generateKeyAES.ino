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

#define SUPPRESSCOLLORS
#include "fprint.h"

#define SYMMETRIC_KEY_SIZE 100

#define ASSERT(err)   if (ret) { printlnRed("Failed"); while (true); }

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

volatile optiga_lib_status_t optiga_lib_status;

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
	ret = trustM_V3.begin();
	ASSERT(ret);
	printlnGreen("OK");

  /*
   * Speed up the chip (min is 6ma, maximum is 15ma)
   */
  printGreen("Setting Current Limit... ");
	ret = trustM_V3.setCurrentLimit(15);
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

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() 
{

  /* Generate symmetric key using AES 128 and store in OPTIGA session oid */
  generateKeyAES_oid();
  
  /* Generate symmetric key using AES and export to host */
  generateKeyAES_export();

  /* 
   * Execute the loop just once :)
   */
  while(1){}
}

/**
 * Generate symmetric key using AES and store in OPTIGA Trust M
 */
void generateKeyAES_oid()
{
  uint32_t ret = 0;
  uint32_t ts = 0;

  /*
   * Generate symetric key and store in oid
   */
  printlnGreen("\r\nGenerating symmetric key... ");
  ts = millis();
  ret = trustM_V3.generateStoreSymmetricKeyAES(OPTIGA_SYMMETRIC_AES_128, OPTIGA_KEY_ID_SECRET_BASED);
  ts = millis() - ts;
  if (ret) {
    digitalWrite(LED_BUILTIN, HIGH);   // Make the LED high to indicate failure
    printlnRed("Failed");
    while (true);
  }
  
  printGreen("[OK] | Command executed in "); 
  Serial.print(ts); 
  Serial.println(" ms");

}

/**
 * Generate symmetric key using AES with export
 */
void generateKeyAES_export()
{
  uint32_t ret = 0;
  uint32_t ts = 0;

  /* buffer for storing symmetric key */
  uint8_t symmetric_key[SYMMETRIC_KEY_SIZE];

  /*
   * Generate symetric key with export
   */
  printlnGreen("\r\nGenerating symmetric key and export... ");
  ts = millis();
  ret = trustM_V3.generateExportSymmetricKeyAES(OPTIGA_SYMMETRIC_AES_256, symmetric_key);
  ts = millis() - ts;
  if (ret) {
    printlnRed("Failed");
    while (true);
  }
  
  output_result((char*)"Symmetric Key ", ts, symmetric_key, SYMMETRIC_KEY_SIZE);

}

