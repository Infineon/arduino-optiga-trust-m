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

#define DATA_LENGTH		10
#define HASH_LENGTH		32
#define SIGN_LENGTH		200
#define PUBKEY_LENGTH	70

#define SUPPRESSCOLLORS
#include "fprint.h"

uint8_t *hash = new uint8_t[HASH_LENGTH];
uint8_t *rawSign = new uint8_t[SIGN_LENGTH];
uint8_t *formSign = new uint8_t[SIGN_LENGTH ];
uint8_t *pubKey = new uint8_t[PUBKEY_LENGTH];

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

  // /*
  //  * Speedup the board (from 6 mA to 15 mA)
  //  */
  // ret = trustM.setCurrentLimit(15);
  // if (ret) {
  //   printlnRed("Failed");
  //   while (true);
  // }
  // printlnGreen("OK");

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

void calculateSignVerifySign_ownkey()
{
  uint32_t ret = 0;
  uint8_t  data[DATA_LENGTH] = {'T', 'R', 'U', 'S', 'T', 'M', 'T', 'E', 'S', 'T'};
  uint8_t  ifxPublicKey[68];
  uint32_t ts = 0;
  
  uint16_t hashLen = HASH_LENGTH;
  uint16_t signLen = SIGN_LENGTH;
  uint16_t pubKeyLen = PUBKEY_LENGTH;

  /*
   * Extract public key of the device certificate
   */
  trustM.getPublicKey(ifxPublicKey);
   
  output_result("My Public Key", ts, ifxPublicKey, sizeof(ifxPublicKey));

  /*
   * Calculate a hash of the given data
   */
  printlnGreen("\r\nCalculate Hash ... ");
  ts = millis();
  ret = trustM.sha256(data, DATA_LENGTH, hash);
  ts = millis() - ts;
  hashLen = HASH_LENGTH;
  if (ret) {
    printlnRed("Failed");
    while (true);
  }

  output_result("Hash", ts, hash, hashLen);

  /*
   * Sign hash
   */
  printlnGreen("\r\nGenerate Signature ... ");
  ts = millis();
  ret = trustM.calculateSignature(hash, hashLen, formSign, signLen);
  ts = millis() - ts;
  if (ret) {
    printlnRed("Failed");
    while (true);
  }

  output_result("Signature #1", ts, formSign, signLen);

  /*
   * Verify a signature generated before
   */
  printlnGreen("\r\nVerify Signature ... ");
  ts = millis();
  ret = trustM.verifySignature(hash, hashLen, formSign, signLen, ifxPublicKey, sizeof(ifxPublicKey));
  ts = millis() - ts;
  if (ret) {
    printlnRed("Failed");
    while (true);
  }
  
  printGreen("[OK] | Command executed in "); 
  Serial.print(ts); 
  Serial.println(" ms");
}

void calculateSignVerifySign_newkey()
{
  // uint32_t ret = 0;
  // uint8_t  data[DATA_LENGTH] = {'T', 'R', 'U', 'S', 'T', 'M', 'T', 'E', 'S', 'T'};
  // uint8_t  ifxPublicKey[68];
  // uint32_t ts = 0;
  
  // uint16_t hashLen = HASH_LENGTH;
  // uint16_t signLen = SIGN_LENGTH;
  // uint16_t pubKeyLen = PUBKEY_LENGTH;

  // /*
  //  * Calculate a hash of the given data
  //  */
  // printlnGreen("\r\nCalculate Hash ... ");
  // ts = millis();
  // ret = trustM.sha256(data, DATA_LENGTH, hash);
  // ts = millis() - ts;
  // hashLen = HASH_LENGTH;
  // if (ret) {
  //   printlnRed("Failed");
  //   while (true);
  // }

  // output_result("Hash", ts, hash, hashLen);

  // /*
  //  * Generate a key pair and store private key inside the security chip
  //  */
  // printGreen("Generate Key Pair ... ");
  // ts = millis();
  // ret = trustM.generateKeypair(pubKey, pubKeyLen, eSESSION_ID_2);
  // ts = millis() - ts;
  // if (ret) {
  //   printlnRed("Failed");
  //   while (true);
  // }
  // output_result("Public key", ts, pubKey, pubKeyLen);

  // /*
  //  * Sign hash with the newly generated private key
  //  */
  // printlnGreen("\r\nGenerate Signature ... ");
  // ts = millis();
  // ret = trustM.calculateSignature(hash, hashLen, eSESSION_ID_2, formSign, signLen);
  // ts = millis() - ts;
  // if (ret) {
  //   printlnRed("Failed");
  //   while (true);
  // }

  // output_result("Signature #2", ts, formSign, signLen);

  // /*
  //  * Verify a signature generated before
  //  */
  // printlnGreen("\r\nVerify Signature ... ");
  // ts = millis();
  // ret = trustM.verifySignature(hash, hashLen, formSign, signLen, pubKey, pubKeyLen);
  // ts = millis() - ts;
  // if (ret) {
  //   printlnRed("Failed");
  //   while (true);
  // }

  // printGreen("[OK] | Command executed in "); 
  // Serial.print(ts); 
  // Serial.println(" ms");
}

void loop()
{
  /* Sign data and verify a signature with the embedded certificate */
  calculateSignVerifySign_ownkey();

  /* Sign data and verify a signature with a newly generated keypair */
  calculateSignVerifySign_newkey();

 /* 
   * Execute the loop just once :)
   */
  while(1){};
}
