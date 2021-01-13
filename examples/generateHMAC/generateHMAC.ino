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
uint16_t secret_oid = 0xF1D0;

volatile optiga_lib_status_t optiga_lib_status;

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
}

void loop() 
{
  uint32_t ret = 0;
  uint32_t ts = 0;

  // /*
  //  * Write input secret to OID
  //  */
  // printlnGreen("\r\nCalculate shared secret... ");
  // ts = millis();
  // ret = (uint32_t) write_input_secret_to_oid();
  // ts = millis() - ts;
  // if (ret) {
  //   printlnRed("Failed");
  //   while (true);
  // }
  
  printGreen("[OK] | Command executed in "); 
  Serial.print(ts); 
  Serial.println(" ms");

  /**
   * Enable V3 capabilities in src/optiga_trustm/optiga_lib_config.h
   */
  #ifdef OPTIGA_TRUST_M_V3

  /**
   * Start HMAC on the input data 
   */
  printlnGreen("\r\nStart to generate HMAC");
  ts = millis();
  ret = trustM_V3.generateHMACStart(OPTIGA_HMAC_SHA_256, secret_oid, input_data_buffer_start, (uint32_t)(sizeof(input_data_buffer_start)));
  ts = millis() - ts;
  if (ret) {
    printlnRed("Failed");
    while (true);
  }
  
  printGreen("[OK] | Command executed in "); 
  Serial.print(ts); 
  Serial.println(" ms");
  
  /**
   * Update HMAC on the input data 
   */
  printlnGreen("\r\nUpdate generate HMAC");
  ts = millis();
  ret = trustM_V3.generateHMACUpdate(input_data_buffer_update, (uint32_t)(sizeof(input_data_buffer_update)));
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
  printlnGreen("\r\nFinalize generate HMAC");
  ts = millis();
  ret = trustM_V3.generateHMACFinalize(input_data_buffer_final, (uint32_t)(sizeof(input_data_buffer_final)), mac_buffer, &mac_buffer_length);
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

// // Write metadata
// static optiga_lib_status_t write_metadata(optiga_util_t * me)
// {
//     optiga_lib_status_t return_status = OPTIGA_LIB_SUCCESS;
//     const uint8_t input_secret_oid_metadata[] = {0x20, 0x06, 0xD3, 0x01, 0x00, 0xE8, 0x01, 0x21};
//     do
//     {
//         optiga_lib_status = OPTIGA_LIB_BUSY;
//         return_status = optiga_util_write_metadata(me,
//                                                    secret_oid,
//                                                    input_secret_oid_metadata,
//                                                    sizeof(input_secret_oid_metadata));
//         WAIT_AND_CHECK_STATUS(return_status, optiga_lib_status);
//     } while (FALSE);

//     return(return_status);
// }

// // Write input secret to OID
// static optiga_lib_status_t write_input_secret_to_oid()
// {
//     optiga_lib_status_t return_status = OPTIGA_UTIL_ERROR;
//     optiga_util_t * me_util = NULL;
//     const uint8_t input_secret[] = {0x8d,0xe4,0x3f,0xff,
//                                     0x65,0x2d,0xa0,0xa7,
//                                     0xf0,0x4e,0x8f,0x22,
//                                     0x84,0xa4,0x28,0x3b};
//     do
//     {
//         me_util = optiga_util_create(0, optiga_util_crypt_callback, NULL);
//         if (NULL == me_util)
//         {
//             break;
//         }
//         /**
//          * Precondition 1 :
//          * Metadata for 0xF1D0 :
//          * Execute access condition = Always
//          * Data object type  =  Pre-shared secret
//          */
//         return_status = write_metadata(me_util);
//         if (OPTIGA_LIB_SUCCESS != return_status)
//         {
//             break;
//         }


//         /**
//         *  Precondition 2 :
//         *  Write secret in OID 0xF1D0
//         */
//         optiga_lib_status = OPTIGA_LIB_BUSY;
//         return_status = optiga_util_write_data(me_util,
//                                                secret_oid,
//                                                OPTIGA_UTIL_ERASE_AND_WRITE,
//                                                0,
//                                                input_secret,
//                                                sizeof(input_secret));

//         WAIT_AND_CHECK_STATUS(return_status, optiga_lib_status);
//     } while (FALSE);
//     if(me_util)
//     {
//         //Destroy the instance after the completion of usecase if not required.
//         return_status = optiga_util_destroy(me_util);
//         if(OPTIGA_LIB_SUCCESS != return_status)
//         {
//             //lint --e{774} suppress This is a generic macro
//             OPTIGA_EXAMPLE_LOG_STATUS(return_status);
//         }
//     }
//     return (return_status);
// }

// /* Call back function for  */
// static void optiga_util_crypt_callback(void * context, optiga_lib_status_t return_status)
// {
//     optiga_lib_status = return_status;
//     if (NULL != context)
//     {
//         // callback to upper layer here
//     }
// }
