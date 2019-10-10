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

#include <OPTIGATrustM.h>

#define CERT_MAXLENGTH   1024

#define SUPPRESSCOLLORS
#include "fprint.h"

uint8_t *cert = new uint8_t[CERT_MAXLENGTH];

void setup() 
{
  uint32_t ret = 0;
  
  /*
   * Initialise a serial port for debug output
   */
  Serial.begin(115200);
  delay(1000);
  Serial.println("Initializing ... ");

  /*
   * Initialise an OPTIGA™ Trust M Board
   */
 Serial.println("Begin to trust ... ");
  ret = trustM.begin();
  if (ret) {
    printlnRed("Failed");
    while (true);
  }
  Serial.println("OK");

  // /*
  //  * Speedup the board (from 6 mA to 15 mA)
  //  */
  // printGreen("Limit the Current ... ");
  // ret = trustM.setCurrentLimit(15);
  // if (ret) {
  //   printlnRed("Failed");
  //   while (true);
  // }
  // printlnGreen("OK");

}

void loop()
{
  uint32_t ret = 0;
  uint8_t  cntr = 10;
  uint16_t certLen = 0;

  /*
   * Calculate a hash of the given data
   */
  printlnGreen("\r\nGetting Certificate ... ");
  ret = trustM.getCertificate(cert, certLen);
  if (ret) {
    printlnRed("Failed");
    while (true);
  }

  printlnGreen("[OK]"); 
  printMagenta("Certificate Length: ");
  Serial.println(certLen); 
  printlnMagenta("Certificate:");
  HEXDUMP(cert, certLen);

  /* 
   * Execute the loop just once :)
   */
  while(1){}
}
