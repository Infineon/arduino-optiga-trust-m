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

#define PUBKEY_LENGTH	300
#define KEY_MAXLENGTH 300

#define SUPPRESSCOLLORS
#include "fprint.h"

uint8_t pubKey[KEY_MAXLENGTH] = {0};
uint16_t pubKeyLen = KEY_MAXLENGTH;
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
  printGreen("Begin to trust ... ");
  ret = trustM.begin();
  if (ret) {
    printlnRed("Failed");
    while (true);
  }
  printlnGreen("OK");

  /*
   * Speedup the board (from 6 mA to 15 mA)
   */
  printGreen("Setting current limit to 15 mA ... ");
  ret = trustM.setCurrentLimit(15);
  if (ret) {
    printlnRed("Failed");
    while (true);
  }
  printlnGreen("OK");

}

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

void calculateSharedSecretKey_oid()
{
  uint32_t ret = 0;
  uint32_t ts = 0;

  /*
   * Generate public and private keypair and store the private key in device
   */
  printlnGreen("\r\nGenerate keypair and save the key in the device");
  ts = millis();
  ret = trustM.generateKeypairECC(pubKey, pubKeyLen);
  ts = millis() - ts;
  if (ret) {
    printlnRed("Failed");
    while (true);
  }
   
  output_result((char*)"My Public Key", ts, pubKey, pubKeyLen);

  /*
   * Calculate shared secret
   */
  printlnGreen("\r\nCalculate shared secret 1... ");
  ts = millis();
  ret = trustM.sharedSecret(pubKey, pubKeyLen);
  ts = millis() - ts;
  if (ret) {
    printlnRed("Failed");
    while (true);
  }
  
  printGreen("[OK] | Command executed in "); 
  Serial.print(ts); 
  Serial.println(" ms");

  /*
   * Calculate shared secret
   */
  printlnGreen("\r\nCalculate shared secret 2... ");
  ts = millis();
  ret = trustM.sharedSecret(eSESSION_ID_2, pubKey, pubKeyLen);
  ts = millis() - ts;
  if (ret) {
    printlnRed("Failed");
    while (true);
  }
  
  printGreen("[OK] | Command executed in "); 
  Serial.print(ts); 
  Serial.println(" ms");

  /*
   * Calculate shared secret
   */
  printlnGreen("\r\nCalculate shared secret 3... ");
  ts = millis();
  ret = trustM.sharedSecret("secp256r1", eSESSION_ID_2, pubKey, pubKeyLen);
  ts = millis() - ts;
  if (ret) {
    printlnRed("Failed");
    while (true);
  }
  
  printGreen("[OK] | Command executed in "); 
  Serial.print(ts); 
  Serial.println(" ms");
}

void calculateSharedSecretKey_export()
{
  uint32_t ret = 0;
  uint8_t  shared_secret[48];
  uint16_t shared_s_len = 0; 
  uint32_t ts = 0;

  /*
   * Generate public and private keypair and store the private key in device
   */
  printlnGreen("\r\nGenerate keypair and save the key in the device");
  ts = millis();
  ret = trustM.generateKeypairECC(pubKey, pubKeyLen);
  ts = millis() - ts;
  if (ret) {
    printlnRed("Failed");
    while (true);
  }
   
  output_result((char*)"My Public Key", ts, pubKey, pubKeyLen);

  /*
   * Calculate shared secret
   */
  printlnGreen("\r\nCalculate shared secret 4 with export... ");
  ts = millis();
  ret = trustM.sharedSecretWithExport(pubKey, pubKeyLen, shared_secret, shared_s_len);
  ts = millis() - ts;
  if (ret) {
    printlnRed("Failed");
    while (true);
  }
  
  output_result((char*)"My Shared Secret", ts, shared_secret, shared_s_len);
}


void loop()
{
  /* Calculate shared secret and store in optiga session object */
  calculateSharedSecretKey_oid();

  /* Calculate shared secret and export to host*/
  calculateSharedSecretKey_export();

 /* 
  * Execute the loop just once :)
  */
  while(1){};
}
