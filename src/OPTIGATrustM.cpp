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
// #include "third_crypto/uECC.h"

// ///OID of IFX Certificate
// #define     OID_IFX_CERTIFICATE                 0xE0E0
// ///OID of the Coprocessor UID
// #define     OID_IFX_UID                         0xE0C2
// #define     LENGTH_UID                          27
// ///Length of certificate
#define     LENGTH_CERTIFICATE                  1728
// ///ASN Tag for sequence
#define     ASN_TAG_SEQUENCE                    0x30
// ///ASN Tag for integer
#define     ASN_TAG_INTEGER                     0x02
// ///msb bit mask
// #define     MASK_MSB                            0x80
// ///TLS Identity Tag
#define     TLS_TAG                             0xC0
// ///IFX Private Key Slot
// #define     OID_PRIVATE_KEY                     0xE0F0
// ///Power Limit OID
// #define     OID_CURRENT_LIMIT                   0xE0C4
// ///Length of R and S vector
// #define     LENGTH_RS_VECTOR                    0x40

// ///Length of maximum additional bytes to encode sign in DER
// #define     MAXLENGTH_SIGN_ENCODE               0x08

// ///Length of Signature
// #define     LENGTH_SIGNATURE                    (LENGTH_RS_VECTOR + MAXLENGTH_SIGN_ENCODE)

/**
 * optiga logger arduino library level 
 */

#define OPTIGA_ARDUINO_SERVICE                     "[optiga arduino]  : "
#define OPTIGA_ARDUINO_SERVICE_COLOR               OPTIGA_LIB_LOGGER_COLOR_MAGENTA

#ifdef OPTIGA_LIB_ENABLE_LOGGING
    /** @brief Macro to enable logger for Util service */
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
 * Preinstantiated object
 */
IFX_OPTIGA_TrustM trustM = IFX_OPTIGA_TrustM();

static volatile optiga_lib_status_t optiga_lib_status;
static void optiga_util_callback(void * context, optiga_lib_status_t return_status)
{
    optiga_lib_status = return_status; 
    if (NULL != context)
    {
        // callback to upper layer here
    }
};

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

int32_t IFX_OPTIGA_TrustM::begin(void)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do 
    {
        //Create an instance of optiga_util to open the application on OPTIGA.
        me_util = optiga_util_create(OPTIGA_INSTANCE_ID_0, optiga_util_callback, NULL);
        if (NULL == me_util)
        {
            return_status = OPTIGA_UTIL_ERROR;
            break;
        }

        /**
         *  Create OPTIGA Crypt Instance
         *
         */
        me_crypt = optiga_crypt_create(OPTIGA_INSTANCE_ID_0, optiga_util_callback, NULL);
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

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }
        while (optiga_lib_status == OPTIGA_LIB_BUSY)
        {
            //Wait until the optiga_util_open_application is completed
            pal_os_event_process();
        }
        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            //optiga util open application failed
            return_status = optiga_lib_status;
            break;
        }

    	active = true;
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
	// int32_t err = CMD_LIB_ERROR;
	// uint8_t p_rnd[32];
	// uint16_t rlen = 32;
	// uint8_t p_cert[512];
	// uint16_t clen = 0;
	// uint8_t p_pubkey[68];
	// uint8_t p_sign[69];
	// uint8_t p_unformSign[64];
	// uint16_t slen = 0;
	// uint8_t r_off = 0;
	// uint8_t s_off = 0;
	
	// do {
	// 	randomSeed(analogRead(0));
		
	// 	for (uint8_t i = 0; i < rlen; i++) {
	// 	  p_rnd[i] = random(0xff);
	// 	  randomSeed(analogRead(0));
	// 	}
		
	// 	err = getCertificate(p_cert, clen);
	// 	if (err)
	// 	  break;
		
	// 	getPublicKey(p_pubkey);
		
	// 	err = calculateSignature(p_rnd, rlen, p_sign, slen);
	// 	if (err)
	// 	  break;
		
	// 	// Checking the size of r,s components withing the signature
	// 	// It is represented as follows
	// 	// R: 0x02 0x21 0x00 0xXX or 0x02 0x20 0xXX
	// 	// L: 0x02 0x21 0x00 0xXX or 0x02 0x20 0xXX
	// 	if (p_sign[1] == 0x21) { r_off = 3; } 
	// 	else                   { r_off = 2; }
		
	// 	if (p_sign[r_off + (LENGTH_RS_VECTOR/2) + 1] == 0x21) { s_off = r_off + (LENGTH_RS_VECTOR/2) + 3; }
	// 	else                                                  { s_off = r_off + (LENGTH_RS_VECTOR/2) + 2; }
		
	// 	memcpy(p_unformSign, &p_sign[r_off], LENGTH_RS_VECTOR/2);
	// 	memcpy(&p_unformSign[LENGTH_RS_VECTOR/2], &p_sign[s_off], LENGTH_RS_VECTOR/2);
		
	// 	if (uECC_verify(p_pubkey+4, p_rnd, rlen, p_unformSign, uECC_secp256r1())) {
	// 	  err = 0;
	// 	}
	// } while(0);
	
	// return err;
}

int32_t IFX_OPTIGA_TrustM::begin(TwoWire& CustomWire)
{
    /**
     * @todo Set the corresponding i2c context first
     */

    return begin();
}

int32_t IFX_OPTIGA_TrustM::reset(void)
{
    /**
     * @todo Soft reset -->not neccesarily ?? I2C-reset?
     */
    end();
    return begin(Wire);
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
        
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (optiga_lib_status == OPTIGA_LIB_BUSY)
        {
            //Wait until the optiga_util_close_application is completed
            pal_os_event_process();
        }
        
        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            //optiga util close application failed
            return_status = optiga_lib_status;
            break;
        }

        active = false;
    }while (FALSE);

    if (me_crypt)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_crypt_destroy(me_crypt);
        if(OPTIGA_LIB_SUCCESS != return_status)
        {
            //lint --e{774} suppress This is a generic macro
            OPTIGA_EXAMPLE_LOG_STATUS(return_status);
        }
    }

    if(me_util)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_util_destroy(me_util);
        if(OPTIGA_LIB_SUCCESS != return_status)
        {
            //lint --e{774} suppress This is a generic macro
            OPTIGA_EXAMPLE_LOG_STATUS(return_status);
        }
    }

    OPTIGA_ARDUINO_LOG_STATUS(return_status);
    //TODO: original function prototypes does returns void in Optiga X.
}

int32_t IFX_OPTIGA_TrustM::getGenericData(uint16_t oid, uint8_t* p_data, uint16_t& hashLength)
{   
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

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_util_read_data operation is completed
            pal_os_event_process();
        }

        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            //Reading the data object failed.
            return_status = optiga_lib_status;
            break;
        }
    } while (FALSE);

    OPTIGA_ARDUINO_LOG_STATUS(return_status);
    return return_status;
}

int32_t IFX_OPTIGA_TrustM::getState(uint16_t oid, uint8_t& byte)
{

    // uint16_t length = 1;
    // int32_t  ret = (int32_t)CMD_LIB_ERROR;
	// uint8_t  bt = 0;
	// sGetData_d sGDVector;
    // sCmdResponse_d sCmdResponse;
	
	// sGDVector.wOID = oid;
	// sGDVector.wLength = 1;
	// sGDVector.wOffset = 0;
	// sGDVector.eDataOrMdata = eDATA;
	
	// sCmdResponse.prgbBuffer = &bt;
	// sCmdResponse.wBufferLength = 1;
	// sCmdResponse.wRespLength = 0;

	// ret = CmdLib_GetDataObject(&sGDVector,&sCmdResponse);
	// if(CMD_LIB_OK == ret)
	// {
	// 	byte = bt;
	// 	ret = 0;
	// }

    // return ret;
}

int32_t IFX_OPTIGA_TrustM::setGenericData(uint16_t oid, uint8_t* p_data, uint16_t hashLength)
{
    uint32_t ard_ret = 1;
    uint16_t offset;
    optiga_lib_status_t return_status = 0;
    
    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
        offset      = 0x0000; 

        return_status = optiga_util_write_data(me_util,
                                               oid,
                                               OPTIGA_UTIL_WRITE_ONLY,
                                               offset,
                                               p_data,
                                               hashLength);

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_util_write_data operation is completed
            pal_os_event_process();
        }

        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            //writing data to a data object failed.
            return_status = optiga_lib_status;
            break;
        }
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
                OPTIGA_UTIL_LOG_MESSAGE("check lenghts");
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

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_crypt_random operation is completed
            pal_os_event_process();
        }

        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            return_status = optiga_lib_status;
            break;
        }

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);

        if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

/**
 * Prepare the hash context
 */
#define OPTIGA_HASH_CONTEXT_INIT(hash_context,p_context_buffer,context_buffer_size,hash_type) \
{                                                               \
    hash_context.context_buffer = p_context_buffer;             \
    hash_context.context_buffer_length = context_buffer_size;   \
    hash_context.hash_algo = hash_type;                         \
}

int32_t IFX_OPTIGA_TrustM::sha256(uint8_t dataToHash[], uint16_t ilen, uint8_t out[32])
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;
    optiga_hash_context_t hash_context;
    uint8_t hash_context_buffer [130];
    hash_data_from_host_t hash_data_host;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);

    do
    {
        /**
         * 1. Initialize the Hash context
         */
        OPTIGA_HASH_CONTEXT_INIT(hash_context,hash_context_buffer,  \
                                 sizeof(hash_context_buffer),(uint8_t)OPTIGA_HASH_TYPE_SHA_256);

        optiga_lib_status = OPTIGA_LIB_BUSY;

        /**
         * 2. Initialize the hashing context at OPTIGA
         */
        return_status = optiga_crypt_hash_start(me_crypt, &hash_context);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }
        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_crypt_hash_start operation is completed
            pal_os_event_process();
        }

        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            return_status = optiga_lib_status;
            break;
        }
        
        /**
         * 3. Continue hashing the data
         */
        hash_data_host.buffer = dataToHash;
        hash_data_host.length = ilen;

        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_hash_update(me_crypt,
                                                 &hash_context,
                                                 OPTIGA_CRYPT_HOST_DATA,
                                                 &hash_data_host);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_crypt_hash_update operation is completed
            pal_os_event_process();
        }

        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            return_status = optiga_lib_status;
            break;
        }

        /**
         * 4. Finalize the hash
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_hash_finalize(me_crypt,
                                                   &hash_context,
                                                   out);

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_crypt_hash_finalize operation is completed
            pal_os_event_process();
        }

        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            return_status = optiga_lib_status;
            break;
        }
        return_status = OPTIGA_LIB_SUCCESS;

    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);


    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::calculateSignature(uint8_t dataToSign[], uint16_t ilen, uint16_t privateKey_oid, uint8_t* out, uint16_t& olen)
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
         * 1. Sign the digest -
         *       - Use Private key from Key Store ID E0FC
         *       - Signature scheme is SHA256,
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_rsa_sign(me_crypt,
                                              OPTIGA_RSASSA_PKCS1_V15_SHA256,
                                              dataToSign,
                                              ilen,
                                              (optiga_key_id_t)privateKey_oid,
                                              out,
                                              &olen,
                                              0x0000);

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_crypt_rsa_sign operation is completed
            pal_os_event_process();
        }

        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            //RSA Signature generation failed.
            return_status = optiga_lib_status;
            break;
        }
    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);

    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
//     uint16_t ret = (int32_t)INT_LIB_ERROR;
//     sCalcSignOptions_d calsign_opt;
//     sbBlob_d sign_blob;
// #define MAX_SIGN_LEN    80

//     do
//     {
//         if (dataToSign == NULL || out == NULL)
//         {
//             break;
//         }

//         //
//         // Example to demonstrate the calc sign using the private key object
//         //
//         calsign_opt.eSignScheme = eECDSA_FIPS_186_3_WITHOUT_HASH;
//         calsign_opt.sDigestToSign.prgbStream = dataToSign;
//         calsign_opt.sDigestToSign.wLen = ilen;

//         //Choose the key OID from the device private keys or session private keys.
//         //Note: Make sure the private key is available in the OID
//         calsign_opt.wOIDSignKey = (uint16_t)ctx;

//         sign_blob.prgbStream = out;
//         sign_blob.wLen = MAX_SIGN_LEN;

//         //Initiate CmdLib API for the Calculation of signature
//         if(CMD_LIB_OK == CmdLib_CalculateSign(&calsign_opt,&sign_blob))
//         {
//             olen = sign_blob.wLen;
//             ret = 0;
//             //Print_Stringline("Calculation of Signature is successful");
//         }
//     }while(FALSE);

//     return ret;
}

int32_t IFX_OPTIGA_TrustM::formatSignature(uint8_t* inSign, uint16_t signLen, uint8_t* outSign, uint16_t& outSignLen)
{
    // int32_t ret = (int32_t)INT_LIB_ERROR;
    // uint8_t bIndex = 0;
    // do
    // {
    //     if((NULL == outSign) || (NULL == inSign))
    //     {
    //         ret = (int32_t)INT_LIB_NULL_PARAM;
    //         break;
    //     }
    //     if((0 == outSignLen)||(0 == signLen))
    //     {
    //         ret = (int32_t)INT_LIB_ZEROLEN_ERROR;
    //         break;
    //     }
    //     //check to see oif input buffer is short,
    //     // or signture plus 6 byte considering der encoding  is more than 0xff
    //     if((outSignLen < signLen)||(0xFF < (signLen + 6)))
    //     {
    //         //send lib error
    //         break;
    //     }
    //     //Encode ASN sequence
    //     *(outSign + 0) = ASN_TAG_SEQUENCE;
    //     //Length of RS and encoding bytes
    //     *(outSign + 1) = LENGTH_RS_VECTOR + 4;
    //     //Encode integer
    //     *(outSign + 2) = ASN_TAG_INTEGER;
    //     //Check if the integer is negative
    //     bIndex = 4;
    //     *(outSign + 3) = 0x20;
    //     if(outSign[0] & MASK_MSB)
    //     {
    //         *(outSign + 3) = 0x21;
    //         *(outSign + bIndex++) = 0x00;
    //     }

    //     //copy R
    //     memmove(outSign + bIndex, inSign, (LENGTH_RS_VECTOR/2));
    //     bIndex+=(LENGTH_RS_VECTOR/2);
    //     //Encode integer
    //     *(outSign + bIndex++) = ASN_TAG_INTEGER;
    //     //Check if the integer is negative
    //     *(outSign + bIndex) = 0x20;
    //     if(outSign[LENGTH_RS_VECTOR/2] & MASK_MSB)
    //     {
    //         *(outSign + bIndex) = 0x21;
    //         bIndex++;
    //         *(outSign + bIndex) = 0x00;
    //     }
    //     bIndex++;

    //     //copy S
    //     memcpy(outSign + bIndex, inSign+(LENGTH_RS_VECTOR/2), (LENGTH_RS_VECTOR/2));
    //     bIndex += (LENGTH_RS_VECTOR/2);
    //     //Sequence length is "index-2"
    //     *(outSign + 1) = (bIndex-2);
    //     //Total length is equal to index
    //     outSignLen = bIndex;

    //     ret = 0;

    // }while(FALSE);

    // return ret;
}

int32_t IFX_OPTIGA_TrustM::verifySignature( uint8_t* digest, uint16_t hashLength,
                                         uint8_t* sign, uint16_t signatureLength,
                                         uint8_t* pubKey, uint16_t plen)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;
    public_key_from_host_t public_key_details =
    {
         pubKey,
         plen,
         (uint8_t)OPTIGA_ECC_CURVE_NIST_P_256
    };

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
          /**
         * Verify RSA signature using public key from host
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_rsa_verify (me_crypt,
                                                 OPTIGA_RSASSA_PKCS1_V15_SHA256,
                                                 digest,
                                                 hashLength,
                                                 sign,
                                                 signatureLength,
                                                 OPTIGA_CRYPT_HOST_DATA,
                                                 &public_key_details,
                                                 0x0000);

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_crypt_rsa_sign operation is completed
            pal_os_event_process();
        }

        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            //RSA Signature generation failed.
            return_status = optiga_lib_status;
            break;
        }
    } while (FALSE);
    OPTIGA_ARDUINO_LOG_STATUS(return_status);

    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
    // int32_t ret = (int32_t)INT_LIB_ERROR;
    // sVerifyOption_d versign_opt;
    // sbBlob_d sign_blob, digest_blob;

    // //
    // // Example to demonstrate the verifySignature using the Public Key from Host
    // //
    // versign_opt.eSignScheme = eECDSA_FIPS_186_3_WITHOUT_HASH;
    // versign_opt.eVerifyDataType = eDataStream;
    // versign_opt.sPubKeyInput.eAlgId = eECC_NIST_P256;
    // versign_opt.sPubKeyInput.sDataStream.prgbStream = pubKey;
    // versign_opt.sPubKeyInput.sDataStream.wLen = plen;

    // digest_blob.prgbStream = digest;
    // digest_blob.wLen = hashLength;

    // sign_blob.prgbStream = sign;
    // sign_blob.wLen = signatureLength;

    // //Initiate CmdLib API for the Verification of signature
    // ret = CmdLib_VerifySign(&versign_opt, &digest_blob, &sign_blob);

    // if(CMD_LIB_OK == ret)
    // {
    //     ret = 0;
    // }

    // return ret;
}

int32_t IFX_OPTIGA_TrustM::verifySignature( uint8_t* digest, uint16_t hashLength, 
											uint8_t* sign, uint16_t signatureLength,
											uint16_t publicKey_oid )
{
    // int32_t ret = (int32_t)INT_LIB_ERROR;
    // sVerifyOption_d versign_opt;
    // sbBlob_d sign_blob;
    // sbBlob_d digest_blob;

    // do
    // {
    //     //
    //     // Example to demonstrate the verifySignature using the Public Key from Host
    //     //
    //     versign_opt.eSignScheme = eECDSA_FIPS_186_3_WITHOUT_HASH;
    //     versign_opt.eVerifyDataType = eOIDData;
    //     versign_opt.wOIDPubKey = publicKey_oid;

    //     digest_blob.prgbStream = digest;
    //     digest_blob.wLen = hashLength;

    //     sign_blob.prgbStream = sign;
    //     sign_blob.wLen = signatureLength;

    //     //Initiate CmdLib API for the Verification of signature
    //     ret = CmdLib_VerifySign(&versign_opt, &digest_blob, &sign_blob);

    //     if(CMD_LIB_OK == ret)
    //     {
    //         ret = 0;
    //     }
    // }while(FALSE);

    // return ret;
}

int32_t IFX_OPTIGA_TrustM::calculateSharedSecretGeneric(int32_t curveID, uint16_t priv_oid, uint8_t* p_pubkey, uint16_t plen, uint16_t out_oid, uint8_t* p_out, uint16_t& olen)
{
//     int32_t             ret = IFX_I2C_STACK_ERROR;
//     sCalcSSecOptions_d  shsec_opt;
//     sbBlob_d            shsec;
//     uint8_t             out[32];

//     //
//     // Example to demonstrate the calculation of shared secret
//     //

//     //Mention the Key Agreement protocol
//     shsec_opt.eKeyAgreementType = eECDH_NISTSP80056A;

//     //Provide the public key information
//     shsec_opt.ePubKeyAlgId          = (eAlgId_d)curveID;
//     shsec_opt.sPubKey.prgbStream    = p_pubkey;
//     shsec_opt.sPubKey.wLen          = plen;

//     //Provide the ID of the private key to be used
//     //Make sure the private key is present in the OID. Use CmdLib_GenerateKeyPair
//     shsec_opt.wOIDPrivKey = priv_oid;

//     //Mentioned where should the generated shared secret be stored.
//     //1.To store the shared secret in session oid,provide the session oid value
//     //or
//     //2.To export the shared secret, set the value to 0x0000
//     shsec_opt.wOIDSharedSecret = out_oid;

//     //Buffer to export the generated shared secret
//     //Shared secret is returned if sCalcSSecOptions.wOIDSharedSecret is 0x0000.
//     shsec.prgbStream = out;
//     shsec.wLen = olen;

//     //Initiate CmdLib API for the Calculate shared secret
//     if(CMD_LIB_OK == CmdLib_CalculateSharedSecret(&shsec_opt, &shsec))
//     {
//         olen = shsec.wLen;
//         ret = 0;
//     }

//     return ret;
}

int32_t IFX_OPTIGA_TrustM::str2cur(String curve_name)
{
    // int32_t ret;
    
    // if (curve_name == "secp256r1") {
    //     ret = eECC_NIST_P256;
    // } else if (curve_name == "secp384r1") {
    //     ret = eECC_NIST_P384;
    // } else {
    //     ret = eECC_NIST_P256;
    // }
    
    // return ret;
}

int32_t IFX_OPTIGA_TrustM::deriveKey(uint8_t* p_data, uint16_t hashLength, uint8_t* p_key, uint16_t klen)
{
//     int32_t             ret = INT_LIB_ERROR;
//     sDeriveKeyOptions_d key_opt;
//     sbBlob_d            key;

//     //
//     // Example to demonstrate the derive key
//     //

//     //Mention the Key derivation method
//     key_opt.eKDM = eTLS_PRF_SHA256;

//     //Provide the seed information
//     key_opt.sSeed.prgbStream = p_data;
//     key_opt.sSeed.wLen =  hashLength;

//     //Provide the ID of the share secret to be used
//     //Make sure the shared secret is present in the OID. Use CmdLib_CalculateSharedSecret
//     // OID Master Secret
//     key_opt.wOIDSharedSecret = 0xe100;

//     if (p_key)
//     {
//         //Mentioned where should the generated derive key be stored.
//         //1.To export the derive key, set the value to 0x0000
//         key_opt.wOIDDerivedKey = 0x0000;
//     }
//     else
//     {
//         //or
//         //2.To store the shared secret in session oid,provide the session oid value
//         key_opt.wOIDDerivedKey = 0xe101;
//     }

//     //Provide the expected length of the derive secret
//     key_opt.wDerivedKeyLen = klen;

//     //Buffer to export the generated derive key
//     //Shared secret is returned if sDeriveKeyOptions.wOIDDerivedKey is 0x0000.
//     key.prgbStream = p_key;
//     key.wLen = klen;

//     //Initiate CmdLib API for the Calculate shared secret
//     if(CMD_LIB_OK == CmdLib_DeriveKey(&key_opt, &key))
//     {
//         klen = key.wLen;
//         ret = 0;
//     }

//     return ret;
}

int32_t IFX_OPTIGA_TrustM::generateKeypair(uint8_t* p_pubkey, uint16_t& plen, uint16_t privkey_oid)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
       /**
         * 2. Generate RSA Key pair
         *       - Use 1024 or 2048 bit RSA key
         *       - Specify the Key Usage
         *       - Store the Private key in OPTIGA Key store
         *              (When exporting the private key, provide buffer of sufficient length (key size in bytes +
         *               encoding length))
         *       - Export Public Key
         */
        if (privkey_oid == 0)
            privkey_oid = OPTIGA_KEY_ID_E0FC;
            //privkey_oid= (uint16_t)eSESSION_ID_2;
 
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_rsa_generate_keypair(me_crypt,
                                                          OPTIGA_RSA_KEY_1024_BIT_EXPONENTIAL,
                                                          (uint8_t)OPTIGA_KEY_USAGE_SIGN,
                                                          FALSE,
                                                          &privkey_oid,
                                                          p_pubkey,
                                                          &plen);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_crypt_rsa_generate_keypair operation is completed
            pal_os_event_process();
        }

        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            //RSA Key pair generation failed
            return_status = optiga_lib_status;
            break;
        }


    } while (FALSE);
    OPTIGA_EXAMPLE_LOG_STATUS(return_status);
    
    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::generateKeypair(uint8_t* p_pubkey, uint16_t& plen, uint8_t* p_privkey, uint16_t& prlen)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
       /**
         * 2. Generate RSA Key pair
         *       - Use 1024 or 2048 bit RSA key
         *       - Specify the Key Usage
         *       - Export the Private key in OPTIGA Key store
         *              (When exporting the private key, provide buffer of sufficient length (key size in bytes +
         *               encoding length))
         *       - Export Public Key
         */
 
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_rsa_generate_keypair(me_crypt,
                                                          OPTIGA_RSA_KEY_1024_BIT_EXPONENTIAL,
                                                          (uint8_t)(OPTIGA_KEY_USAGE_SIGN |OPTIGA_KEY_USAGE_AUTHENTICATION | OPTIGA_KEY_USAGE_KEY_AGREEMENT) ,
                                                          TRUE,
                                                          p_privkey,
                                                          p_pubkey,
                                                          &plen);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_crypt_rsa_generate_keypair operation is completed
            pal_os_event_process();
        }

        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            //RSA Key pair generation failed
            return_status = optiga_lib_status;
            break;
        }


    } while (FALSE);
    OPTIGA_EXAMPLE_LOG_STATUS(return_status);
    
    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}
