/**
 * MIT License
 *
 * Copyright (c) 2019 Infineon Technologies AG
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

#define DATA_LENGTH       20
#define BIGDATA_LENGTH    1024
#define HASH_LENGTH       32

#define SUPPRESSCOLLORS
#include "fprint.h"

/* Arrays to store results of operations */
uint8_t  data[DATA_LENGTH];
uint8_t  bigdata[BIGDATA_LENGTH];
uint8_t  hash[HASH_LENGTH];

void setup() 
{
  uint32_t ret = 0;
  
  /*
   * Initialise a serial port for debug output
   */
  Serial.begin(38400);
  Serial.println("Initializing ... ");

  /*
   * Initialise an OPTIGA™ Trust X Board
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

void loop()
{
  // uint32_t ret = 0;
  // uint8_t  cntr = 10;
  // // Timestamp is used to measure the execution time of a command
  // uint32_t ts = 0;
  
  // /*
  //  * Calculate a hash of the given data
  //  */
  // printGreen("\r\nCalculate One-Time Hash for ");
  // printlnGreen((char *)data);
  // ts = millis();
  // ret = trustM.sha256(data, DATA_LENGTH, hash);
  // ts = millis() - ts;
  // if (ret) {
  //   printlnRed("Failed");
  //   while (true);
  // }
  // output_result("Hash", ts, hash, HASH_LENGTH);

  // /*
  //  * Calculate a hash of the given data (big input)
  //  */
  // printlnGreen("\r\nCalculate One-Time Hash for 1024 bytes... ");
  // ts = millis();
  // ret = trustM.sha256(bigdata, BIGDATA_LENGTH, hash);
  // ts = millis() - ts;
  // if (ret) {
  //   printlnRed("Failed");
  //   while (true);
  // }

  // output_result("Hash", ts, hash, HASH_LENGTH);
  // printGreen("Hashrate is "); 
  // Serial.print(1024/ts); 
  // Serial.println(" kB/sec");

  // /*
  //  * Benchmarking hash for the current microcontroller
  //  */
  // printlnGreen("\r\nBenchmarking SHA256 100 times for 20 bytes data ... ");
  // ts = millis();
  // for (int i = 0; i < 100; i++) {
  //    trustM.sha256(data, DATA_LENGTH, hash);
  // }
  // ts = millis() - ts;
  // if (ret) {
  //   printlnRed("Failed");
  //   while (true);
  // }

  // printGreen("Becnhmark executed in "); 
  // Serial.print(ts); 
  // Serial.println(" ms");
  // printGreen("Hashrate is "); 
  // Serial.print((1000 * 100)/ts); 
  // Serial.println(" H/sec");

  // /*
  //  * Count down 10 seconds and restart the application
  //  */
  // while(cntr) {
  //   Serial.print(cntr);
  //   Serial.println(" seconds untill restart.");
  //   delay(1000);
  //   cntr--;
  // }
}
