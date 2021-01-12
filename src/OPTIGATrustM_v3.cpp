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
#include "OPTIGATrustM_v3.h"

#ifdef OPTIGA_TRUST_M_V3

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
    //#define OPTIGA_LIB_ENABLE_ARDUINO_LOGGING
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


/**
 * Preinstantiated object
 */
IFX_OPTIGA_TrustM_V3 trustM_V3 = IFX_OPTIGA_TrustM_V3();


static volatile optiga_lib_status_t optiga_lib_status;
static void optiga_util_callback(void * context, optiga_lib_status_t return_status)
{
    optiga_lib_status = return_status; 
    if (NULL != context)
    {
        // callback to upper layer here
    }
};

IFX_OPTIGA_TrustM_V3::IFX_OPTIGA_TrustM_V3()
{ 
    v3_active = false;
}

IFX_OPTIGA_TrustM_V3::~IFX_OPTIGA_TrustM_V3()
{

}

/*
 * Global Functions
 */

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
				Serial.println((char*)"Encryption Failure");
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
				Serial.print((char*)"Decryption Failure. Return Code ");
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

/*************************************************************************************
 *                              COMMANDS API TRUST E COMPATIBLE
 **************************************************************************************/
 
int32_t IFX_OPTIGA_TrustM_V3::generateSymmetricKeyAES(optiga_symmetric_key_type_t sym_key_type, 
                                                   bool_t export_symmetric_key, 
                                                   optiga_key_id_t symmetricKey_oid)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
        /**
         *  Generate symmetric key using AES
         *       - Use key size of 128/192/256
         *       - Specify the Key Usage (Key Agreement or Sign based on requirement)
         *       - Store the Private key in OPTIGA Key store or export the key
         */
        if (symmetricKey_oid == 0)
            symmetricKey_oid = OPTIGA_KEY_ID_SECRET_BASED;
 
        optiga_lib_status = OPTIGA_LIB_BUSY;

        return_status = optiga_crypt_symmetric_generate_key(me_crypt,
                                                            sym_key_type,
                                                            (uint8_t)(OPTIGA_KEY_USAGE_SIGN | OPTIGA_KEY_USAGE_AUTHENTICATION | OPTIGA_KEY_USAGE_ENCRYPTION),
                                                            FALSE,
                                                            (void *)OPTIGA_KEY_ID_SECRET_BASED);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);
    
    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM_V3::generateSymmetricKeyAES(optiga_symmetric_key_type_t sym_key_type, 
                                                   bool_t export_symmetric_key, 
                                                   void * symmetric_key)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
        /**
         *  Generate symmetric key using AES
         *       - Use key size of 128/192/256
         *       - Specify the Key Usage (Key Agreement or Sign based on requirement)
         *       - Store the Private key in OPTIGA Key store or export the key
         */
 
        optiga_lib_status = OPTIGA_LIB_BUSY;

        optiga_crypt_symmetric_generate_key(me_crypt,
                                            sym_key_type,
                                            (uint8_t)(OPTIGA_KEY_USAGE_SIGN | OPTIGA_KEY_USAGE_AUTHENTICATION | OPTIGA_KEY_USAGE_ENCRYPTION),
                                            TRUE,
                                            symmetric_key);
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);
    
    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM_V3::generateHMAC(optiga_hmac_type_t type, 
                     					uint16_t secret, 
                     					const uint8_t * input_data, 
                     					uint32_t input_data_length, 
                     					uint8_t * mac, 
                     					uint32_t * mac_length)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;

    const uint8_t input_secret[] = {0x8d,0xe4,0x3f,0xff,
                                    0x65,0x2d,0xa0,0xa7,
                                    0xf0,0x4e,0x8f,0x22,
                                    0x84,0xa4,0x28,0x3b};

    const uint8_t input_secret_oid_metadata[] = {0x20, 0x06, 0xD3, 0x01, 0x00, 0xE8, 0x01, 0x21};

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
        /**
         * Write metadata 
         * Precondition 1 :
         * Metadata for 0xF1D0 :
         * Execute access condition = Always
         * Data object type  =  Pre-shared secret
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_write_metadata(me_util,
                                                   secret,
                                                   input_secret_oid_metadata,
                                                   sizeof(input_secret_oid_metadata));
	    OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

        /**
        *  Precondition 2 :
        *  Write secret in OID 0xF1D0
        */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_write_data(me_util,
                                               secret,
                                               OPTIGA_UTIL_ERASE_AND_WRITE,
                                               0,
                                               input_secret,
                                               sizeof(input_secret));
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

        /**
         *  Generate HMAC using secret in the OPTIGA
         */ 
        optiga_lib_status = OPTIGA_LIB_BUSY;

        return_status = optiga_crypt_hmac(me_crypt,
                                          type,
                                          secret,
                                          input_data,
                                          input_data_length,
                                          mac,
                                          mac_length);

        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);
    
    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM_V3::generateHMAC(optiga_hmac_type_t type, 
                     					uint16_t secret, 
                     					const uint8_t * input_data, 
                     					uint32_t input_data_length)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;

    const uint8_t input_secret[] = {0x8d,0xe4,0x3f,0xff,
                                    0x65,0x2d,0xa0,0xa7,
                                    0xf0,0x4e,0x8f,0x22,
                                    0x84,0xa4,0x28,0x3b};

    const uint8_t input_secret_oid_metadata[] = {0x20, 0x06, 0xD3, 0x01, 0x00, 0xE8, 0x01, 0x21};

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
        /**
         * Write metadata 
         * Precondition 1 :
         * Metadata for 0xF1D0 :
         * Execute access condition = Always
         * Data object type  =  Pre-shared secret
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_write_metadata(me_util,
                                                   0xF1D0,
                                                   input_secret_oid_metadata,
                                                   sizeof(input_secret_oid_metadata));
	    OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

        /**
        *  Precondition 2 :
        *  Write secret in OID 0xF1D0
        */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_write_data(me_util,
                                               secret,
                                               OPTIGA_UTIL_ERASE_AND_WRITE,
                                               0,
                                               input_secret,
                                               sizeof(input_secret));
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

        /**
         *  start HMAC using the secret present in the OPTIGA
         */ 
        optiga_lib_status = OPTIGA_LIB_BUSY;

        return_status = optiga_crypt_hmac_start(me_crypt,
                                                type,
                                                secret,
                                                input_data,
                                                input_data_length);
                                          
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);
    
    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM_V3::generateHMAC(const uint8_t * input_data, 
                     					uint32_t input_data_length)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
        /**
         *  Continue HMAC on the input and update the HMAC
         */ 
        optiga_lib_status = OPTIGA_LIB_BUSY;

        return_status = optiga_crypt_hmac_update(me_crypt,
                                                 input_data,
                                                 input_data_length);
                                          
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);
    
    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM_V3::generateHMAC(const uint8_t * input_data, 
                     					uint32_t input_data_length, 
                     					uint8_t * mac, 
                     					uint32_t * mac_length)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
        /**
         *  Finalize the generation of HMAC
         */ 
        optiga_lib_status = OPTIGA_LIB_BUSY;

        return_status = optiga_crypt_hmac_finalize(me_crypt,
                                                   input_data,
                                                   input_data_length,
                                                   mac,
                                                   mac_length);
                                          
        OPTIGA_ASSERT_WAIT_WHILE_BUSY(return_status);

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);
    
    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

#endif /* OPTIGA_TRUST_M_V3 */
