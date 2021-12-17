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

#define SUPPRESSCOLLORS
#include "fprint.h"

#define KEY_MAXLENGTH 200
#define MSG_MAXLENGTH 128
#define ENABLE_PAIRING 1

uint8_t *pubKey = new uint8_t[KEY_MAXLENGTH];
uint8_t *privKey = new uint8_t[KEY_MAXLENGTH];
  
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
   * Initialise an OPTIGA™ Trust X Board
   */
  printGreen("Begin to trust ... ");
  ret = trustM.begin(ENABLE_PAIRING);
  if (ret) {
    printlnRed("Failed");
    while (true);
  }
  printlnGreen("OK");

  /*
   * Speedup the board (from 6 mA to 15 mA)
   */
  printGreen("Limit the Current ... ");
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

void loop()
{
  uint32_t ret = 0;
  uint8_t  cntr = 10;
  uint32_t ts = 0;
  uint16_t keyID = eRSA_DEVICE_PRIKEY_1;
  uint16_t pubKeyLen = KEY_MAXLENGTH;
  uint16_t privKeyLen = KEY_MAXLENGTH;
  uint8_t message[] = "Encrypt and Decrypt Data using OPTIGA(TM) Trust M";
  uint8_t secret[MSG_MAXLENGTH];
  uint16_t secretLen = MSG_MAXLENGTH;
  uint8_t restoredMessage[MSG_MAXLENGTH];
  uint16_t restoredMessageLen = MSG_MAXLENGTH;

  /*
   * Generate a keypair#1 RSA 1024
   */
  printlnGreen("\r\nGenerate Key Pair RSA 1024. Store Private Key on Board ... ");
  ts = millis();
  ret = trustM.generateKeypair(pubKey, pubKeyLen, keyID);
  ts = millis() - ts;
  if (ret) {
    printlnRed("Failed");
    while (true);
  }

  output_result((char*)"Public Key ", ts, pubKey, pubKeyLen);

  /*
   * Encrypt Data
   */
  printGreen("\r\nMessage to Encrypt: ");
  printlnGreen((char*)message);
  printlnGreen("");
  ts = millis();
  ret = trustM.encrypt(message, sizeof(message), pubKey, pubKeyLen, secret, secretLen);
  ts = millis() - ts;
  if (ret) {
    printlnRed("Failed");
    while (true);
  }

  output_result((char*)"\r\nEncrypted message ", ts, secret, secretLen);
  printlnGreen("");
  /*
   * Decrypt Data
   */
  ts = millis();
  ret = trustM.decrypt(secret, secretLen, keyID, restoredMessage, restoredMessageLen);
  ts = millis() - ts;
  if (ret) {
    printlnRed("Failed");
    while (true);
  }

  output_result((char*)"\r\nDecrypted message ", ts, restoredMessage, restoredMessageLen);
  
  /* 
   * Execute the loop just once :)
   */
  while(1){};
}