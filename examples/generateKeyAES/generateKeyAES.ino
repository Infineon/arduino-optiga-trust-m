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

#define SYMMETRIC_KEY_SIZE 100

#define ASSERT(err)   if (ret) { printlnRed("Failed"); while (true); }

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

void loop() 
{
  /**
   * Enable V3 capabilities in src/optiga_trustm/optiga_lib_config.h
   */
  #ifdef OPTIGA_TRUST_M_V3

  optiga_lib_status_t return_status = !OPTIGA_LIB_SUCCESS;
  optiga_util_t * util_me = NULL;

  /**
   * Sample metadata of 0xE200 
   */
  const uint8_t E200_metadata[] = { 0x20, 0x06, 0xD0, 0x01, 0x00, 0xD3, 0x01, 0x00 }; 

    do
    {
        /**
         * 1. Create OPTIGA Util Instance
         */
        util_me = optiga_util_create(0, optiga_util_callback, NULL);
        if (NULL == util_me)
        {
            break;
        }
        
        /**
         * Write metadata of a data object (e.g. key data object 0xE200)
         */        
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_write_metadata(util_me,
                                                   OPTIGA_KEY_ID_SECRET_BASED,
                                                   E200_metadata,
                                                   sizeof(E200_metadata));

        WAIT_AND_CHECK_STATUS(return_status, optiga_lib_status);
        
        return_status = OPTIGA_LIB_SUCCESS;
    } while (FALSE);

  /* Generate symmetric key using AES 128 and store in OPTIGA session oid */
  generateKeyAES_oid();
  
  /* Generate symmetric key using AES and export to host */
  generateKeyAES_export();

    if (util_me)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_util_destroy(util_me);
        if(OPTIGA_LIB_SUCCESS != return_status)
        {
            //lint --e{774} suppress This is a generic macro
            OPTIGA_EXAMPLE_LOG_STATUS(return_status);
        }
    }

  #endif /* OPTIGA_TRUST_M_V3 */ 

  /* 
   * Execute the loop just once :)
   */
  while(1){}
}

/**
 * Enable V3 capabilities in src/optiga_trustm/optiga_lib_config.h
 */
#ifdef OPTIGA_TRUST_M_V3

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
  ret = trustM_V3.generateSymmetricKeyAES(OPTIGA_SYMMETRIC_AES_128, FALSE, OPTIGA_KEY_ID_SECRET_BASED);
  ts = millis() - ts;
  if (ret) {
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
  ret = trustM_V3.generateSymmetricKeyAES(OPTIGA_SYMMETRIC_AES_256, TRUE, symmetric_key);
  ts = millis() - ts;
  if (ret) {
    printlnRed("Failed");
    while (true);
  }
  
  printGreen("[OK] | Command executed in "); 
  Serial.print(ts); 
  Serial.println(" ms");

}

static void optiga_util_callback(void * context, optiga_lib_status_t return_status)
{
    optiga_lib_status = return_status;
    if (NULL != context)
    {
        // callback to upper layer here
    }
}

#endif /* OPTIGA_TRUST_M_V3 */ 
