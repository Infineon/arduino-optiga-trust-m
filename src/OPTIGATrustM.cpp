/*
 * MIT License
 *
 * Copyright (c) 2019 Infineon Technologies AG
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
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
 * Arduino library for OPTIGA™ Trust M.
 */

#include "OPTIGATrustM.h"
#include "optiga_trustm/optiga_util.h"
#include "optiga_trustm/optiga_lib_types.h"
#include "optiga_trustm/optiga_comms.h"
#include "optiga_trustm/ifx_i2c_config.h"
#include "optiga_trustm/pal_os_event_timer.h"
#include "optiga_trustm/Util.h"
//#include "uecc/uECC.h"
#include "fprint.h"


// ///Length of certificate
#define     LENGTH_CERTIFICATE                  1728
// ///ASN Tag for sequence
#define     ASN_TAG_SEQUENCE                    0x30
// ///ASN Tag for integer
#define     ASN_TAG_INTEGER                     0x02
///msb bit mask
#define     MASK_MSB                            0x80
// ///TLS Identity Tag
#define     TLS_TAG                             0xC0
///Length of R and S vector
#define     LENGTH_RS_VECTOR                    0x40

//Currently set to Initialization state. At the real time / customer side this needs to be OPERATIONAL (0x07)
#define 	LCSO_STATE_FINAL          			0x03

// Value of Operational state
#define 	LCSO_STATE_OPERATIONAL    			0x07

/**
 * optiga logger arduino library level 
 */

#define OPTIGA_ARDUINO_SERVICE                     "[optiga arduino]  : "
#define OPTIGA_ARDUINO_SERVICE_COLOR               OPTIGA_LIB_LOGGER_COLOR_MAGENTA

#ifdef OPTIGA_LIB_ENABLE_LOGGING
    /** @brief Macro to enable logger for Arduino logging service */
    #define OPTIGA_LIB_ENABLE_ARDUINO_LOGGING
#endif

//  #if defined (OPTIGA_LIB_ENABLE_LOGGING) && defined (OPTIGA_LIB_ENABLE_ARDUINO_LOGGING)
 #if defined (OPTIGA_LIB_ENABLE_ARDUINO_LOGGING)
/**
 * \brief Logs the message provided from Util layer
 *
 * \details
 * Logs the message provided from Util layer
 *
 * \pre
 *
 * \note
 * - None
 *
 * \param[in]      msg      Valid pointer to string to be logged
 *
 */
#define OPTIGA_ARDUINO_LOG_MESSAGE(msg) \
{\
    optiga_lib_print_message(msg,OPTIGA_ARDUINO_SERVICE,OPTIGA_ARDUINO_SERVICE_COLOR);\
}

/**
 * \brief Logs the byte array buffer provided from Util layer in hexadecimal format
 *
 * \details
 * Logs the byte array buffer provided from Util layer in hexadecimal format
 *
 * \pre
 *
 * \note
 * - None
 *
 * \param[in]      array      Valid pointer to array to be logged
 * \param[in]      array_len  Length of array buffer
 *
 */
#define OPTIGA_ARDUINO_LOG_HEX_DATA(array,array_len) \
{\
    optiga_lib_print_array_hex_format(array,array_len,OPTIGA_UNPROTECTED_DATA_COLOR);\
}

/**
 * \brief Logs the status info provided from Util layer
 *
 * \details
 * Logs the status info provided from Util layer
 *
 * \pre
 *
 * \note
 * - None
 *
 * \param[in]      return_value      Status information Util service
 *
 */
#define OPTIGA_ARDUINO_LOG_STATUS(return_value) \
{ \
    if (OPTIGA_LIB_SUCCESS != return_value) \
    { \
        optiga_lib_print_status(OPTIGA_ARDUINO_SERVICE,OPTIGA_ERROR_COLOR,return_value); \
    } \
    else\
    { \
        optiga_lib_print_status(OPTIGA_ARDUINO_SERVICE,OPTIGA_ARDUINO_SERVICE_COLOR,return_value); \
    } \
}
#else

#define OPTIGA_ARDUINO_LOG_MESSAGE(msg) {}
#define OPTIGA_ARDUINO_LOG_HEX_DATA(array, array_len) {}
#define OPTIGA_ARDUINO_LOG_STATUS(return_value) {}

#endif

/**
 * \brief Asserts the Util or crypt API call and wait till completion
 *
 * \details
 * Asserts the util or crypt API call and wait till completion
 *
 * \pre
 *
 * \note
 * - None
 *
 * \param[in]      return_value      Status information from util or crypt service
 *
 */
#define OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status) \
{ \
        if (OPTIGA_LIB_SUCCESS != return_status)  \
        {  \
            break;  \
        }  \
        \
        while (OPTIGA_LIB_BUSY == optiga_lib_status)  \
        { \
            pal_os_event_process(); \
        } \
        \
        if (OPTIGA_LIB_SUCCESS != optiga_lib_status) \
        { \
            return_status = optiga_lib_status;  \
            break;  \
        }  \
}

uint8_t platformBindingSharedSecretMetadataFinal [] = {
	//Metadata to be updated
	0x20, 0x17,
		// LcsO
		0xC0, 0x01,
					LCSO_STATE_FINAL,       // Refer Macro to see the value or some more notes
		// Change/Write Access tag
		0xD0, 0x07,
					// This allows updating the binding secret during the runtime using shielded connection
					// If not required to update the secret over the runtime, set this to NEV and
					// update Metadata length accordingly
					0xE1, 0xFC, LCSO_STATE_OPERATIONAL,   // LcsO < Operational state
					0xFE,
					0x20, 0xE1, 0x40,
		// Read Access tag
		0xD1, 0x03,
					0xE1, 0xFC, LCSO_STATE_OPERATIONAL,   // LcsO < Operational state
		// Execute Access tag
		0xD3, 0x01,
					0x00,   // Always
		// Data object Type
		0xE8, 0x01,
					0x22,   // Platform binding secret type
};


/**
 * Preinstantiated object
 */
IFX_OPTIGA_TrustM trustM = IFX_OPTIGA_TrustM();

IFX_OPTIGA_TrustM::IFX_OPTIGA_TrustM()
{ 
    active = false;
}

IFX_OPTIGA_TrustM::~IFX_OPTIGA_TrustM()
{

}

/*
 * Global Functions
 */

int32_t IFX_OPTIGA_TrustM::begin(uint8_t pairDevice)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do 
    {
        //Create an instance of optiga_util to open the application on OPTIGA.
        me_util = optiga_util_create(OPTIGA_INSTANCE_ID_0, utilCryptCallback, &optiga_lib_status);
        if (NULL == me_util)
        {
            return_status = OPTIGA_UTIL_ERROR;
            break;
        }

        /**
         *  Create OPTIGA Crypt Instance
         *
         */
        me_crypt = optiga_crypt_create(OPTIGA_INSTANCE_ID_0, utilCryptCallback, &optiga_lib_status);
        if (NULL == me_crypt)
        {
            return_status = OPTIGA_CRYPT_ERROR;
            break;
        }

        /**
         * Open the application on OPTIGA which is a precondition to perform any other operations
         * using optiga_util_open_application
         */        
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_open_application(me_util, 0);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    	active = true;
		
		if (pairDevice)
		{
			if (!pairMCU())
			{
				return_status = OPTIGA_CRYPT_ERROR;
				break;
			}
		}
		
    }while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);

    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::checkChip(void)
{
	int32_t err = 1;
	/*uint8_t p_rnd[32];
	uint16_t rlen = 32;
	uint8_t p_cert[512];
	uint16_t clen = 0;
	uint8_t p_pubkey[68];
	uint8_t p_sign[69];
	uint8_t p_unformSign[64];
	uint16_t slen = 0;
	uint8_t r_off = 0;
	uint8_t s_off = 0;
	
	do {
		randomSeed(analogRead(0));
		
		for (uint8_t i = 0; i < rlen; i++) {
		  p_rnd[i] = random(0xff);
		  randomSeed(analogRead(0));
		}
		
		err = getCertificate(p_cert, clen);
		if (err)
		  break;
		
		getPublicKey(p_pubkey);
		
		err = calculateSignature(p_rnd, rlen, p_sign, slen);
		if (err)
		  break;
		
		// Checking the size of r,s components withing the signature
		// It is represented as follows
		// R: 0x02 0x21 0x00 0xXX or 0x02 0x20 0xXX
		// L: 0x02 0x21 0x00 0xXX or 0x02 0x20 0xXX
		if (p_sign[1] == 0x21) { r_off = 3; } 
		else                   { r_off = 2; }
		
		if (p_sign[r_off + (LENGTH_RS_VECTOR/2) + 1] == 0x21) { s_off = r_off + (LENGTH_RS_VECTOR/2) + 3; }
		else                                                  { s_off = r_off + (LENGTH_RS_VECTOR/2) + 2; }
		
		memcpy(p_unformSign, &p_sign[r_off], LENGTH_RS_VECTOR/2);
		memcpy(&p_unformSign[LENGTH_RS_VECTOR/2], &p_sign[s_off], LENGTH_RS_VECTOR/2);
		
		if (uECC_verify(p_pubkey+4, p_rnd, rlen, p_unformSign, uECC_secp256r1())) {*/
		  err = 0;
//		}
//	} while(0);
	
	return err;
}

#include "optiga_trustm/pal_crypt.h"
static void output_result(char* tag, uint8_t* in, uint16_t in_len)
{
  Serial.println(" ms");
  printMagenta(tag); 
  printMagenta(" Length: ");
  Serial.println(in_len);
  printMagenta(tag); 
  printlnMagenta(":");
  HEXDUMP(in, in_len);
}

static uint8_t example_secret[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

static uint8_t example_label[] = {
	0x42, 0x61, 0x62, 0x79, 0x6c, 0x6f, 0x6e, 0x20,
	0x50, 0x52, 0x46, 0x20, 0x41, 0x70, 0x70, 0x4e,
	0x6f, 0x74, 0x65
};

static uint8_t example_seed[] = {
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};

static uint8_t example_result[] = {
	0xbf, 0x88, 0xeb, 0xde, 0xfa, 0x78, 0x46, 0xa1,
	0x10, 0x55, 0x91, 0x88, 0xd4, 0x22, 0xf3, 0xf7,
	0xfa, 0xfe, 0xf4, 0xa5, 0x49, 0xbd, 0xaa, 0xce,
	0x37, 0x39, 0xc9, 0x44, 0x65, 0x7f, 0x2d, 0xd9,
	0xbc, 0x30, 0x83, 0x14, 0x47, 0xd0, 0xed, 0x1c,
	0x89, 0xf6, 0x58, 0x23, 0xb2, 0xec, 0xe0, 0x52,
	0xf3, 0xb7, 0x95, 0xed, 0xe8, 0x6c, 0xad, 0x59,
	0xca, 0x47, 0x3b, 0x3a, 0x78, 0x98, 0x63, 0x69,
	0x44, 0x65, 0x62, 0xc9, 0xa4, 0x0d, 0x6a, 0xac,
	0x59, 0xa2, 0x04, 0xfa, 0x0e, 0x44, 0xb7, 0xd7
};

// Test Vector from website www.ietf.org/mail-archive/web/tls/current/msg03416.html
// Secret
static const uint8_t tls_at_ietf_secret[] = 
{
    0x9b,0xbe,0x43,0x6b,0xa9,0x40,0xf0,0x17,    
    0xb1,0x76,0x52,0x84,0x9a,0x71,0xdb,0x35      
};

// Seed
static const uint8_t tls_at_ietf_seed[] = 
{
    0xa0,0xba,0x9f,0x93,0x6c,0xda,0x31,0x18,    
    0x27,0xa6,0xf7,0x96,0xff,0xd5,0x19,0x8c       
};

// Label test label (0x74,0x65,0x73,0x74,0x20,0x6c,0x61,0x62,0x65,0x6c)
static const uint8_t tls_at_ietf_label[] = 
{
    0x74,0x65,0x73,0x74,0x20,0x6c,0x61,0x62,
    0x65,0x6c
};

// Output
static uint8_t tls_at_ietf_output[] = 
{
    0xe3,0xf2,0x29,0xba,0x72,0x7b,0xe1,0x7b,    
    0x8d,0x12,0x26,0x20,0x55,0x7c,0xd4,0x53,    
    0xc2,0xaa,0xb2,0x1d,0x07,0xc3,0xd4,0x95,    
    0x32,0x9b,0x52,0xd4,0xe6,0x1e,0xdb,0x5a,    
    0x6b,0x30,0x17,0x91,0xe9,0x0d,0x35,0xc9,    
    0xc9,0xa4,0x6b,0x4e,0x14,0xba,0xf9,0xaf,    
    0x0f,0xa0,0x22,0xf7,0x07,0x7d,0xef,0x17,    
    0xab,0xfd,0x37,0x97,0xc0,0x56,0x4b,0xab,    
    0x4f,0xbc,0x91,0x66,0x6e,0x9d,0xef,0x9b,    
    0x97,0xfc,0xe3,0x4f,0x79,0x67,0x89,0xba,    
    0xa4,0x80,0x82,0xd1,0x22,0xee,0x42,0xc5,    
    0xa7,0x2e,0x5a,0x51,0x10,0xff,0xf7,0x01,    
    0x87,0x34,0x7b,0x66    
};

static int32_t testTLSPRF256(void)
{
	int32_t ret = 1;
	uint8_t p_derived_key[sizeof(tls_at_ietf_output)];
	memset(p_derived_key, 0x00, sizeof(tls_at_ietf_output));

	do 
	{
		pal_crypt_tls_prf_sha256(NULL,
                                 tls_at_ietf_secret,
                                 sizeof(tls_at_ietf_secret),
                                 tls_at_ietf_label,
                                 sizeof(tls_at_ietf_label),
                                 tls_at_ietf_seed,
                                 sizeof(tls_at_ietf_seed),
                                 p_derived_key,
                                 sizeof(tls_at_ietf_output));
								 
		if (memcmp(p_derived_key, tls_at_ietf_output, sizeof(tls_at_ietf_output)) != 0)
		{
			output_result((char*)"Derived Key: ", p_derived_key, sizeof(tls_at_ietf_output));
			output_result((char*)"Expected Key: ", tls_at_ietf_output, sizeof(tls_at_ietf_output));
			break;
		}
		
		ret = 0;
	}while(0);
	
	return ret;
}

#define NB_TESTS 3
#define CCM_SELFTEST_PT_MAX_LEN 24
#define CCM_SELFTEST_CT_MAX_LEN 32
/*
 * The data is the same for all tests, only the used length changes
 */
static const uint8_t key[] = {
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f
};

static const uint8_t iv[] = {
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b
};

static const uint8_t ad[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13
};

static uint8_t msg[CCM_SELFTEST_PT_MAX_LEN] = {
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
};

static const size_t iv_len [NB_TESTS] = { 7, 8,  12 };
static const size_t add_len[NB_TESTS] = { 8, 16, 20 };
static const size_t msg_len[NB_TESTS] = { 4, 16, 24 };
static const size_t tag_len[NB_TESTS] = { 4, 6,  8  };

static  uint8_t res[NB_TESTS][CCM_SELFTEST_CT_MAX_LEN] = {
    {   0x71, 0x62, 0x01, 0x5b, 0x4d, 0xac, 0x25, 0x5d },
    {   0xd2, 0xa1, 0xf0, 0xe0, 0x51, 0xea, 0x5f, 0x62,
        0x08, 0x1a, 0x77, 0x92, 0x07, 0x3d, 0x59, 0x3d,
        0x1f, 0xc6, 0x4f, 0xbf, 0xac, 0xcd },
    {   0xe3, 0xb2, 0x01, 0xa9, 0xf5, 0xb7, 0x1a, 0x7a,
        0x9b, 0x1c, 0xea, 0xec, 0xcd, 0x97, 0xe7, 0x0b,
        0x61, 0x76, 0xaa, 0xd9, 0xa4, 0x42, 0x8a, 0xa5,
        0x48, 0x43, 0x92, 0xfb, 0xc1, 0xb0, 0x99, 0x51 }
};


static int32_t testAES128CCMEncrypt(void)
{
	int32_t ret = 1;
	uint8_t plaintext[CCM_SELFTEST_PT_MAX_LEN];
    uint8_t ciphertext[CCM_SELFTEST_CT_MAX_LEN];

	do 
	{
		for( int i = 0; i < NB_TESTS; i++ )
		{
			memset( plaintext, 0, CCM_SELFTEST_PT_MAX_LEN );
			memset( ciphertext, 0, CCM_SELFTEST_CT_MAX_LEN );
			memcpy( plaintext, msg, msg_len[i] );
			
			pal_crypt_encrypt_aes128_ccm(NULL,
										 plaintext,
										 CCM_SELFTEST_PT_MAX_LEN,
										 key,
										 iv,
										 iv_len[i],
										 ad,
										 add_len[i],
										 tag_len[i],
										 ciphertext);
			if (memcmp(ciphertext, res[i], msg_len[i] + tag_len[i])!= 0)
			{
				Serial.println("Encryption Failure");
				output_result((char*)"Result Cipher: ", ciphertext, msg_len[i] + tag_len[i]);
				output_result((char*)"Expected Cipher: ", res[i], msg_len[i] + tag_len[i]);
			}
		}
		
		ret = 0;
	}while(0);
	
	return ret;
}

static int32_t testAES128CCMDecrypt(void)
{
	int32_t ret = 1;
	uint8_t plaintext[CCM_SELFTEST_PT_MAX_LEN];

	do 
	{
		for( int i = 0; i < NB_TESTS; i++ )
		{
			memset( plaintext, 0, CCM_SELFTEST_PT_MAX_LEN );
			
			ret = pal_crypt_decrypt_aes128_ccm( NULL,
									  res[i],
									  msg_len[i] + tag_len[i],
									  key,
									  iv,
									  iv_len[i],
									  ad,
									  add_len[i],
									  tag_len[i],
									  plaintext);
			if (memcmp(plaintext, msg, msg_len[i]) != 0)
			{
				Serial.print("Decryption Failure. Return Code ");
				Serial.println(ret);
				output_result((char*)"Encrypted message: ", res[i], msg_len[i] + tag_len[i]);
				output_result((char*)"Result Plain Text: ", plaintext, msg_len[i]);
				output_result((char*)"Expected Plain Text: ", msg, msg_len[i]);
				ret = 1;
			}
		}
		
		ret = 0;
	}while(0);
	
	return ret;
}

static int32_t testPalCryptTinyCrypt(void)
{
	int32_t ret = 1;
	do 
	{
		//testTLSPRF256();
		//testAES128CCMEncrypt();
		testAES128CCMDecrypt();
		ret = 0;
	}while(0);
	
	return ret;
}

int32_t IFX_OPTIGA_TrustM::pairMCU(void)
{
	int32_t ard_ret = 0;
	optiga_lib_status_t return_status;
	pal_status_t pal_return_status;
	uint16_t bytes_to_read;
	uint8_t platform_binding_secret[64];
    uint8_t platform_binding_secret_metadata[44];
	
	do 
	{
		if (me_crypt == NULL || me_util == NULL)
			break;
		
		testPalCryptTinyCrypt();
		
		/**
         * 1. Initialize the protection level and protocol version for the instances
         */
        OPTIGA_UTIL_SET_COMMS_PROTECTION_LEVEL(me_util,OPTIGA_COMMS_NO_PROTECTION);
        OPTIGA_UTIL_SET_COMMS_PROTOCOL_VERSION(me_util,OPTIGA_COMMS_PROTOCOL_VERSION_PRE_SHARED_SECRET);

        OPTIGA_CRYPT_SET_COMMS_PROTECTION_LEVEL(me_crypt,OPTIGA_COMMS_NO_PROTECTION);
        OPTIGA_CRYPT_SET_COMMS_PROTOCOL_VERSION(me_crypt,OPTIGA_COMMS_PROTOCOL_VERSION_PRE_SHARED_SECRET);
		
		/**
         * 2. Read Platform Binding Shared secret (0xE140) data object metadata from OPTIGA
         *    using optiga_util_read_metadata.
         */
        bytes_to_read = sizeof(platform_binding_secret_metadata);
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_read_metadata(me_util,
                                                  0xE140,
                                                  platform_binding_secret_metadata,
                                                  &bytes_to_read);
		OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);
		
		/**
         * 3. Validate LcsO in the metadata.
         *    Skip the rest of the procedure if LcsO is greater than or equal to operational state(0x07)
         */
        if (platform_binding_secret_metadata[4] >= LCSO_STATE_OPERATIONAL)
        {
            // The LcsO is already greater than or equal to operational state
            break;
        }
		
		/**
         * 4. Generate Random using optiga_crypt_random
         *       - Specify the Random type as TRNG
         *    a. The recommended size of secret is 64 bytes which is maximum supported.
         *       the minimum recommended is 32 bytes.
         *    b. If the host platform doesn't support random generation,
         *       use OPTIGA to generate the maximum size chosen.
         *       else choose the appropriate length of random to be generted by OPTIGA
         *
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_random(me_crypt,
                                            OPTIGA_RNG_TYPE_TRNG,
                                            platform_binding_secret,
                                            sizeof(platform_binding_secret));
		OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);
		
		/**
         * 5. Write random(secret) to OPTIGA platform Binding shared secret data object (0xE140)
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        OPTIGA_UTIL_SET_COMMS_PROTECTION_LEVEL(me_util,OPTIGA_COMMS_NO_PROTECTION);
        return_status = optiga_util_write_data(me_util,
                                               0xE140,
                                               OPTIGA_UTIL_ERASE_AND_WRITE,
                                               0,
                                               platform_binding_secret,
                                               sizeof(platform_binding_secret));
		OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);
		
		/**
         * 6. Write/store the random(secret) on the Host platform
         *
         */
        pal_return_status = pal_os_datastore_write(OPTIGA_PLATFORM_BINDING_SHARED_SECRET_ID,
                                                   platform_binding_secret,
                                                   sizeof(platform_binding_secret));

        if (PAL_STATUS_SUCCESS != pal_return_status)
        {
            //Storing of Pre-shared secret on Host failed.
            optiga_lib_status = pal_return_status;
            break;
        }
		
		/**
         * 7. Update metadata of OPTIGA Platform Binding shared secret data object (0xE140)
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        OPTIGA_UTIL_SET_COMMS_PROTECTION_LEVEL(me_util,OPTIGA_COMMS_NO_PROTECTION);
        return_status = optiga_util_write_metadata(me_util,
                                                   0xE140,
                                                   platformBindingSharedSecretMetadataFinal,
                                                   sizeof(platformBindingSharedSecretMetadataFinal));
	    OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);
	} while(0);
	
	OPTIGA_ARDUINO_LOG_STATUS(return_status);

    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 1;
    }
		
	return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::begin(TwoWire& CustomWire)
{
    /**
     * Set the corresponding i2c context first
     */
    optiga_pal_i2c_context_0.p_i2c_hw_config = &CustomWire;
    
    return begin();
}

int32_t IFX_OPTIGA_TrustM::reset(void)
{
    end();
    return begin();
}

void IFX_OPTIGA_TrustM::end(void)
{
    optiga_lib_status_t return_status;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {  
        /* Close the application on OPTIGA after all the operations are executed
         * using optiga_util_close_application
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_close_application(me_util, 0);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

        active = false;
    }while (FALSE);

    if (me_crypt)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_crypt_destroy(me_crypt);
        if(OPTIGA_LIB_SUCCESS != return_status)
        {
            //lint --e{774} suppress This is a generic macro
            OPTIGA_ARDUINO_LOG_STATUS(return_status);
        }
    }

    if(me_util)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_util_destroy(me_util);
        if(OPTIGA_LIB_SUCCESS != return_status)
        {
            //lint --e{774} suppress This is a generic macro
            OPTIGA_ARDUINO_LOG_STATUS(return_status);
        }
    }

    OPTIGA_ARDUINO_LOG_STATUS(return_status);
}

void IFX_OPTIGA_TrustM::utilCryptCallback(void * context, optiga_lib_status_t return_status)
{
    // optiga_lib_status = return_status; 
    if (NULL != context)
    {
        optiga_lib_status_t * ptr = (optiga_lib_status_t*) context;
        *ptr =  return_status;
    }
};

int32_t IFX_OPTIGA_TrustM::getGenericData(uint16_t oid, uint8_t* p_data, uint16_t& hashLength)
{  
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;
    
    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
        hashLength = 0xFFFF;
        uint16_t offset = 0x00;

        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_read_data(me_util,
                                              oid,
                                              offset,
                                              p_data,
                                              &hashLength);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    } while (FALSE);

    OPTIGA_ARDUINO_LOG_STATUS(return_status);

    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::getState(uint16_t oid, uint8_t& byte)
{
    uint16_t length = 1;
    int32_t  ret = 1;

    ret = getGenericData(oid, &byte, length);

    return ret;
}

int32_t IFX_OPTIGA_TrustM::setGenericData(uint16_t oid, uint8_t* p_data, uint16_t hashLength)
{
    uint32_t ard_ret = 1;
    uint16_t offset;
    optiga_lib_status_t return_status = 0;
    
    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
        offset            = 0x0000; 
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_write_data(me_util,
                                               oid,
                                               OPTIGA_UTIL_ERASE_AND_WRITE,
                                               offset,
                                               p_data,
                                               hashLength);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);

    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::getGenericMetadata(uint16_t oid, uint8_t* p_data, uint16_t& length)
{   
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;
    
    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
        /**
         * Read metadata of a data object (e.g. certificate data object E0E0)
         * using optiga_util_read_data.
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_read_metadata(me_util,
                                                  oid,
                                                  p_data,
                                                  &length);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);

    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}


/*************************************************************************************
 *                              COMMANDS API TRUST E COMPATIBLE
 **************************************************************************************/
 
int32_t IFX_OPTIGA_TrustM:: getCertificate(uint8_t* p_cert, uint16_t& clen)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;
    uint16_t tag_len;
    uint32_t cert_len = 0;
     
    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do{
        #define LENGTH_CERTLIST_LEN     3
        #define LENGTH_CERTLEN          3
        #define LENGTH_TAGlEN_PLUS_TAG  3
        #define LENGTH_MINIMUM_DATA     10

        if ((p_cert == NULL)  || (active == false)) {
            break;
        }

        return_status = getGenericData(eDEVICE_PUBKEY_CERT_IFX, p_cert, clen);

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        OPTIGA_ARDUINO_LOG_HEX_DATA(p_cert,clen);

        //Validate TLV
        if((TLS_TAG != p_cert[0]) && (ASN_TAG_SEQUENCE != p_cert[0]))
        {
            return_status = OPTIGA_UTIL_ERROR;
            OPTIGA_ARDUINO_LOG_MESSAGE("tls tag validate");
            break;
        }

        if(TLS_TAG == p_cert[0])
        {
            //Check minimum length must be 10
            if(clen < LENGTH_MINIMUM_DATA)
            {
                OPTIGA_ARDUINO_LOG_MESSAGE("tls tag minimum length");
                return_status = OPTIGA_UTIL_ERROR;
                break;
            }
            tag_len = Utility_GetUint16(&p_cert[1]);
            cert_len = Utility_GetUint24(&p_cert[6]);
            //Length checks
            if((tag_len != (clen - LENGTH_TAGlEN_PLUS_TAG)) ||           \
                (Utility_GetUint24(&p_cert[3]) != (uint32_t)(tag_len - LENGTH_CERTLIST_LEN)) ||   \
                ((cert_len > (uint32_t)(tag_len - (LENGTH_CERTLIST_LEN  + LENGTH_CERTLEN))) || (cert_len == 0x00)))
            {
                OPTIGA_ARDUINO_LOG_MESSAGE("check lenghts");
                return_status = OPTIGA_UTIL_ERROR;
                break;

            }
        }

        memmove(&p_cert[0], &p_cert[9], cert_len);
        clen = (uint16_t)cert_len;

    }while(FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);

    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::getPublicKey(uint8_t p_pubkey[64])
{
	uint32_t ard_ret = 1;
	uint8_t p_cert[512];
	uint16_t clen = 0;
	
	do{
		ard_ret = getCertificate(p_cert, clen);
		if (ard_ret)
			break;
	
		if ((p_cert != NULL) || (p_pubkey != NULL)) {
			  for (uint16_t i=0; i < clen; i++) {
				if (p_cert[i] != 0x03)
				  continue;
				if (p_cert[i+1] != 0x42)
				  continue;
				if (p_cert[i+2] != 0x00)
				  continue;
				if (p_cert[i+3] != 0x04)
				  continue;
			  
				memcpy(p_pubkey, &p_cert[i], 68);
			  }
		}
		
		ard_ret = 0;
	} while (FALSE);
	
	return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::getRandom(uint16_t length, uint8_t* p_random)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;
     
    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
        /**
         * Generate Random -
         *  - Specify the Random type as TRNG
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_random(me_crypt,
                                            OPTIGA_RNG_TYPE_TRNG,
                                            p_random,
                                            length);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);

        if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::sha256(uint8_t dataToHash[], uint16_t ilen, uint8_t out[32])
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;
    hash_data_from_host_t hash_data_host;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);

    do
    {
        hash_data_host.buffer = dataToHash;
        hash_data_host.length = ilen;
		
		optiga_lib_status = OPTIGA_LIB_BUSY;
		return_status = optiga_crypt_hash(me_crypt, OPTIGA_HASH_TYPE_SHA_256, OPTIGA_CRYPT_HOST_DATA, &hash_data_host, out);
		OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

        return_status = OPTIGA_LIB_SUCCESS;

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);


    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::calculateSignatureRSA(uint8_t dataToSign[], uint16_t ilen, 
                                                 uint16_t privateKey_oid, uint8_t* out, 
                                                 uint16_t& olen)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
        if (dataToSign == NULL || out == NULL)
        {
            break;
        }

        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_rsa_sign(me_crypt,
                                        OPTIGA_RSASSA_PKCS1_V15_SHA256,
                                        dataToSign,
                                        ilen,
                                        (optiga_key_id_t)privateKey_oid,
                                        out,
                                        &olen,
                                        0x0000);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);

    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::calculateSignatureECDSA(uint8_t dataToSign[], uint16_t ilen, 
                                                   uint16_t privateKey_oid, uint8_t* out, 
                                                   uint16_t& olen)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
        if (dataToSign == NULL || out == NULL)
        {
            break;
        }

        /**
         * Sign the digest -
         *       - Signature scheme is SHA256,
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_ecdsa_sign(me_crypt,
                                                dataToSign,
                                                ilen,
                                                (optiga_key_id_t)privateKey_oid,
                                                out,
                                                &olen);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);

    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}


int32_t IFX_OPTIGA_TrustM::formatSignature(uint8_t* inSign, uint16_t signLen, 
                                           uint8_t* outSign, uint16_t& outSignLen)
{
    int32_t ret = 1;
    uint8_t bIndex = 0;
    do
    {
        if((NULL == outSign) || (NULL == inSign))
        {
            ret = 1;
            break;
        }
        if((0 == outSignLen)||(0 == signLen))
        {
            ret = 1;
            break;
        }
        //check to see oif input buffer is short,
        // or signture plus 6 byte considering der encoding  is more than 0xff
        if((outSignLen < signLen)||(0xFF < (signLen + 6)))
        {
            //send lib error
            break;
        }
        //Encode ASN sequence
        *(outSign + 0) = ASN_TAG_SEQUENCE;
        //Length of RS and encoding bytes
        *(outSign + 1) = LENGTH_RS_VECTOR + 4;
        //Encode integer
        *(outSign + 2) = ASN_TAG_INTEGER;
        //Check if the integer is negative
        bIndex = 4;
        *(outSign + 3) = 0x20;
        if(outSign[0] & MASK_MSB)
        {
            *(outSign + 3) = 0x21;
            *(outSign + bIndex++) = 0x00;
        }

        //copy R
        memmove(outSign + bIndex, inSign, (LENGTH_RS_VECTOR/2));
        bIndex+=(LENGTH_RS_VECTOR/2);
        //Encode integer
        *(outSign + bIndex++) = ASN_TAG_INTEGER;
        //Check if the integer is negative
        *(outSign + bIndex) = 0x20;
        if(outSign[LENGTH_RS_VECTOR/2] & MASK_MSB)
        {
            *(outSign + bIndex) = 0x21;
            bIndex++;
            *(outSign + bIndex) = 0x00;
        }
        bIndex++;

        //copy S
        memcpy(outSign + bIndex, inSign+(LENGTH_RS_VECTOR/2), (LENGTH_RS_VECTOR/2));
        bIndex += (LENGTH_RS_VECTOR/2);
        //Sequence length is "index-2"
        *(outSign + 1) = (bIndex-2);
        //Total length is equal to index
        outSignLen = bIndex;

        ret = 0;

    }while(FALSE);

    return ret;
}

int32_t  IFX_OPTIGA_TrustM::verifySignatureRSA(uint8_t hash[], uint16_t hashLength, 
                                               uint8_t signature[], uint16_t signatureLength, 
                                               uint16_t publicKey_oid)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;
    uint16_t keyLength = 0;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {   
        /**
         * Verify RSA signature using public key from host
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_rsa_verify(me_crypt,
                                                OPTIGA_RSASSA_PKCS1_V15_SHA256,
                                                hash,
                                                hashLength,
                                                signature,
                                                signatureLength,
                                                OPTIGA_CRYPT_OID_DATA,
                                                &publicKey_oid,
                                                0x0000);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);

    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t  IFX_OPTIGA_TrustM::verifySignatureRSA(uint8_t hash[], uint16_t hashLength, 
                                               uint8_t signature[], uint16_t signatureLength, 
                                               uint8_t pubKey[], uint16_t plen, optiga_rsa_key_type_t rsa_key_type)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;
    public_key_from_host_t public_key_details =
    {
         pubKey,
         plen,
         (uint8_t)rsa_key_type
    };

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
        /**
         * Verify RSA signature using public key from host
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_rsa_verify(me_crypt,
                                                OPTIGA_RSASSA_PKCS1_V15_SHA256,
                                                hash,
                                                hashLength,
                                                signature,
                                                signatureLength,
                                                OPTIGA_CRYPT_HOST_DATA,
                                                &public_key_details,
                                                0x0000);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);
        
    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);

    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::verifySignatureECDSA( uint8_t hash[], uint16_t hashLength, 
											     uint8_t signature[], uint16_t signatureLength,
											     uint16_t publicKey_oid)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;
    uint16_t keyLength = 0;


    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {  
        /**
         * Verify RSA signature using public key from host
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_ecdsa_verify (me_crypt,
                                                   hash,
                                                   hashLength,
                                                   signature,
                                                   signatureLength,
                                                   OPTIGA_CRYPT_OID_DATA,
                                                   &publicKey_oid);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);

    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::verifySignatureECDSA( uint8_t hash[], uint16_t hashLength,
                                                 uint8_t signature[], uint16_t signatureLength,
                                                 uint8_t pubKey[], uint16_t plen, optiga_ecc_curve_t ecc_key_type)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;
    public_key_from_host_t public_key_details =
    {
         pubKey,
         plen,
         (uint8_t)ecc_key_type
    };

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
        /**
         * Verify RSA signature using public key from host
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_ecdsa_verify (me_crypt,
                                                   hash,
                                                   hashLength,
                                                   signature,
                                                   signatureLength,
                                                   OPTIGA_CRYPT_HOST_DATA,
                                                   &public_key_details);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);

    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::calculateSharedSecretGeneric(int32_t curveID, uint16_t priv_oid, 
                                                        uint8_t* p_pubkey, uint16_t plen, 
                                                        uint16_t out_oid, uint8_t* p_out, 
                                                        uint16_t& olen)
{
    #define SHARED_SECRET_SIZE_PRIVATE_NIST_P256    32
    #define SHARED_SECRET_SIZE_PRIVATE_NIST_P384    48
    uint32_t ard_ret = 1;
    // optiga_lib_status_t return_status = 0;
    bool_t exprt = FALSE;

    // To store the generated public key as part of Generate key pair
    uint8_t public_key [100];
    uint16_t public_key_length = sizeof(public_key);

    // Peer public key details for the ECDH operation
    public_key_from_host_t peer_public_key_details =
    {
         p_pubkey,
         plen,
         (uint8_t)curveID   //Not working for NIST P 384??
    };

    optiga_lib_status_t return_status = 0;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
        // If out_oid is set to 0x0000 the shared secret is exported
        if (p_out != NULL && 0x0000 == out_oid)
        {
            exprt = TRUE;
        }
        else
        {
            if(p_out == NULL && 0x0000 == out_oid)
            {
                return_status = OPTIGA_CRYPT_ERROR_INVALID_INPUT;
                break;
            }
        }

        if(OPTIGA_ECC_CURVE_NIST_P_256 == curveID)
        {
            olen = SHARED_SECRET_SIZE_PRIVATE_NIST_P256;
        }
        else if (OPTIGA_ECC_CURVE_NIST_P_384 == curveID)
        {
            olen = SHARED_SECRET_SIZE_PRIVATE_NIST_P384;
        }
        else
        {
            return_status = OPTIGA_CRYPT_ERROR_INVALID_INPUT;
            break;
        }

        /**
         * Generate ECC Key pair - To use the private key with ECDH in the next step
         *       - Specify the Key Usage as Key Agreement
         *       - Store the Private key with in OPTIGA Session
         *       - Export Public Key
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;

        //optiga_key_id = OPTIGA_KEY_ID_SESSION_BASED;
        return_status = optiga_crypt_ecc_generate_keypair(me_crypt,
                                                          (optiga_ecc_curve_t)curveID,
                                                          (uint8_t)OPTIGA_KEY_USAGE_KEY_AGREEMENT,
                                                          FALSE,
                                                          &priv_oid,
                                                          public_key,
                                                          &public_key_length);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);
        

        /**
         *  Perform ECDH using the Peer Public key
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_ecdh(me_crypt,
                                          (optiga_key_id_t)priv_oid,
                                          &peer_public_key_details,
                                          exprt,
                                          p_out);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);

    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::str2cur(String curve_name)
{
    int32_t ret;
    
    if (curve_name == "secp256r1") {
        ret = OPTIGA_ECC_CURVE_NIST_P_256;
    } else if (curve_name == "secp384r1") {
        ret = OPTIGA_ECC_CURVE_NIST_P_384;
    } else {
        ret = OPTIGA_ECC_CURVE_NIST_P_256;
    }
    
    return ret;
}

int32_t IFX_OPTIGA_TrustM::generateKeypairRSA(uint8_t p_pubkey[], uint16_t& plen, 
                                              uint16_t privateKey_oid, optiga_rsa_key_type_t rsa_key_type)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
       /**
         * Generate RSA Key pair
         *       - Use 1024 or 2048 bit RSA key
         *       - Specify the Key Usage
         *       - Store the Private key in OPTIGA Key store
         *              (When exporting the private key, provide buffer of sufficient length (key size in bytes +
         *               encoding length))
         *       - Export Public Key
         */
        if (privateKey_oid == 0)
            privateKey_oid = OPTIGA_KEY_ID_E0FC;
 
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_rsa_generate_keypair(me_crypt,
                                                          rsa_key_type,
                                                          (uint8_t)(OPTIGA_KEY_USAGE_SIGN | OPTIGA_KEY_USAGE_AUTHENTICATION | OPTIGA_KEY_USAGE_KEY_AGREEMENT | OPTIGA_KEY_USAGE_ENCRYPTION ) ,
                                                          FALSE,
                                                          &privateKey_oid,
                                                          p_pubkey,
                                                          &plen);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);
    
    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::generateKeypairRSA(uint8_t publicKey[], uint16_t& plen, 
                                              uint8_t privateKey[], uint16_t& prlen, 
                                              optiga_rsa_key_type_t rsa_key_type)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
       /**
         * Generate RSA Key pair
         *       - Use 1024 or 2048 bit RSA key
         *       - Specify the Key Usage
         *       - Export the Private key in OPTIGA Key store
         *              (When exporting the private key, provide buffer of sufficient length (key size in bytes +
         *               encoding length))
         *       - Export Public Key
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_rsa_generate_keypair(me_crypt,
                                                          rsa_key_type,
                                                          (uint8_t)(OPTIGA_KEY_USAGE_SIGN | OPTIGA_KEY_USAGE_AUTHENTICATION | OPTIGA_KEY_USAGE_KEY_AGREEMENT | OPTIGA_KEY_USAGE_ENCRYPTION ) ,
                                                          TRUE,
                                                          privateKey,
                                                          publicKey,
                                                          &plen);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);
    
    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::generateKeypairECC(uint8_t  publicKey[], uint16_t& plen, 
                                              uint16_t privateKey_oid, optiga_ecc_curve_t ecc_key_type)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
        /**
         *  Generate ECC Key pair
         *       - Use ECC NIST P 256  or P384 Curve
         *       - Specify the Key Usage (Key Agreement or Sign based on requirement)
         *       - Store the Private key in OPTIGA Key store
         *       - Export Public Key
         */
        if (privateKey_oid == 0)
            privateKey_oid = OPTIGA_KEY_ID_E0F1;
 
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_ecc_generate_keypair(me_crypt,
                                                          ecc_key_type,
                                                          (uint8_t)(OPTIGA_KEY_USAGE_SIGN | OPTIGA_KEY_USAGE_AUTHENTICATION | OPTIGA_KEY_USAGE_KEY_AGREEMENT),
                                                          FALSE,
                                                          &privateKey_oid,
                                                          publicKey,
                                                          &plen);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);
    
    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::generateKeypairECC(uint8_t publicKey[], uint16_t& plen, 
                                              uint8_t privateKey[], uint16_t& prlen,
                                              optiga_ecc_curve_t ecc_key_type)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
        /**
         * Generate ECC Key pair
         *       - Use ECC NIST P 256 or P384 Curve
         *       - Specify the Key Usage (Key Agreement or Sign based on requirement)
         *       - Store the Private key in OPTIGA Key store
         *       - Export Public Key
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_ecc_generate_keypair(me_crypt,
                                                          ecc_key_type,
                                                          (uint8_t)(OPTIGA_KEY_USAGE_SIGN |OPTIGA_KEY_USAGE_AUTHENTICATION | OPTIGA_KEY_USAGE_KEY_AGREEMENT),
                                                          TRUE,
                                                          privateKey,
                                                          publicKey,
                                                          &plen);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);
		
		prlen = plen;

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);
    
    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::encrypt( uint8_t dataToEncrypt[], uint16_t dlen,  
									uint8_t pubkeyFromUser[], uint16_t pklen, 
									uint8_t result[], uint16_t& rlen)
{
	uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;
	public_key_from_host_t public_key_from_host;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
        public_key_from_host.public_key = pubkeyFromUser;
        public_key_from_host.length = pklen;
		
		if (pklen < 160)
			public_key_from_host.key_type = (uint8_t)OPTIGA_RSA_KEY_1024_BIT_EXPONENTIAL;
		else
			public_key_from_host.key_type = (uint8_t)OPTIGA_RSA_KEY_2048_BIT_EXPONENTIAL;
		
		OPTIGA_CRYPT_SET_COMMS_PROTOCOL_VERSION(me_crypt, OPTIGA_COMMS_PROTOCOL_VERSION_PRE_SHARED_SECRET);
        OPTIGA_CRYPT_SET_COMMS_PROTECTION_LEVEL(me_crypt, OPTIGA_COMMS_COMMAND_PROTECTION );
		
        optiga_lib_status = OPTIGA_LIB_BUSY;

        return_status = optiga_crypt_rsa_encrypt_message(me_crypt,
                                                         OPTIGA_RSAES_PKCS1_V15,
                                                         dataToEncrypt,
                                                         dlen,
                                                         NULL,
                                                         0,
                                                         OPTIGA_CRYPT_HOST_DATA,
                                                         &public_key_from_host,
                                                         result,
                                                         &rlen);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);
    
    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::encrypt( uint8_t dataToEncrypt[], uint16_t dlen,  
									uint16_t certOID,
									uint8_t result[], uint16_t& rlen)
{
	uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
		OPTIGA_CRYPT_SET_COMMS_PROTOCOL_VERSION(me_crypt, OPTIGA_COMMS_PROTOCOL_VERSION_PRE_SHARED_SECRET);
        OPTIGA_CRYPT_SET_COMMS_PROTECTION_LEVEL(me_crypt, OPTIGA_COMMS_COMMAND_PROTECTION );
		
        optiga_lib_status = OPTIGA_LIB_BUSY;

        return_status = optiga_crypt_rsa_encrypt_message(me_crypt,
                                                         OPTIGA_RSAES_PKCS1_V15,
                                                         dataToEncrypt,
                                                         dlen,
                                                         NULL,
                                                         0,
                                                         OPTIGA_CRYPT_OID_DATA,
                                                         &certOID,
                                                         result,
                                                         &rlen);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);
    
    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::decrypt( uint8_t dataToDecrypt[], uint16_t dlen,
									uint16_t keyOID,
									uint8_t result[], uint16_t& rlen)
{
	uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
		// OPTIGA Comms Shielded connection settings to enable the protection
        OPTIGA_CRYPT_SET_COMMS_PROTOCOL_VERSION(me_crypt, OPTIGA_COMMS_PROTOCOL_VERSION_PRE_SHARED_SECRET);
        OPTIGA_CRYPT_SET_COMMS_PROTECTION_LEVEL(me_crypt, OPTIGA_COMMS_RESPONSE_PROTECTION);
		
        optiga_lib_status = OPTIGA_LIB_BUSY;

        return_status = optiga_crypt_rsa_decrypt_and_export(me_crypt,
                                                            OPTIGA_RSAES_PKCS1_V15,
                                                            dataToDecrypt,
                                                            dlen,
                                                            NULL,
                                                            0,
                                                            (optiga_key_id_t)keyOID,
                                                            result,
                                                            &rlen);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);
    
    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}


