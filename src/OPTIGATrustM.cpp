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
#include "third_crypto/uECC.h"

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
///msb bit mask
#define     MASK_MSB                            0x80
// ///TLS Identity Tag
#define     TLS_TAG                             0xC0
// ///IFX Private Key Slot
// #define     OID_PRIVATE_KEY                     0xE0F0
// ///Power Limit OID
// #define     OID_CURRENT_LIMIT                   0xE0C4
///Length of R and S vector
#define     LENGTH_RS_VECTOR                    0x40

// ///Length of maximum additional bytes to encode sign in DER
// #define     MAXLENGTH_SIGN_ENCODE               0x08

// ///Length of Signature
// #define     LENGTH_SIGNATURE                    (LENGTH_RS_VECTOR + MAXLENGTH_SIGN_ENCODE)


/**
 * OPTIGA metadata TLV types
 */
/// Metatada ttype
#define     METADATA_TYPE                       0x20
/// Life cycle state (LcsO)
#define     LCSO_TYPE                           0xC0
/// Data or key object version
#define     DATA_VERSION_TYPE                   0xC1
/// Max. size of the data
#define     MAX_DATA_SIZE_TYPE                  0xC4
/// Max. size of the data
#define     MAX_DATA_SIZE_TYPE                  0xC4
/// Used size of the data object
#define     USED_DATA_SIZE_TYPE                 0xC5

/// Change Access Condition descriptor
#define     CHA_ACCESS_COND_TYPE                0xD0
/// Read Access Condition descriptor
#define     READ_ACCESS_COND_TYPE               0xD1
/// Execute Access Condition descriptor
#define     EXE_ACCESS_COND_TYPE                0xD3
/// Algorithm associated with key container
#define     KEY_ALGORITHM_TYPE                  0xE0
/// Key usage associated with key container
#define     KEY_USAGE_TYPE                      0xE1
/// Data object type
#define     DATA_OBJ_TYPE_TYPE                  0xE8



/// Maximum objetct metada length
#define     MAXLENGTH_OBJ_METADATA              0x101

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
	int32_t err = 1;
	uint8_t p_rnd[32];
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
		
		if (uECC_verify(p_pubkey+4, p_rnd, rlen, p_unformSign, uECC_secp256r1())) {
		  err = 0;
		}
	} while(0);
	
	return err;
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
        offset      = 0x0000; 

        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_write_data(me_util,
                                               oid,
                                               OPTIGA_UTIL_ERASE_AND_WRITE,
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
    {   /**
         * Get the key data
         */
        //getObjectSize(publicKey_oid, keyLength);
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

    //To store the generated public key as part of Generate key pair
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

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_crypt_ecc_generate_keypair operation is completed
            pal_os_event_process();
        }

        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            return_status = optiga_lib_status;
            break;
        }

        /**
         *  Perform ECDH using the Peer Public key
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_ecdh(me_crypt,
                                          (optiga_key_id_t)priv_oid,
                                          &peer_public_key_details,
                                          exprt,
                                          p_out);
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

int32_t IFX_OPTIGA_TrustM::deriveKey(uint8_t* p_data, uint16_t hashLength, 
                                     uint8_t* p_key, uint16_t klen)
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

int32_t IFX_OPTIGA_TrustM::generateKeypairRSA(uint8_t* p_pubkey, uint16_t& plen, 
                                              uint16_t privateKey_oid, optiga_rsa_key_type_t rsa_key_type)
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
        if (privateKey_oid == 0)
            privateKey_oid = OPTIGA_KEY_ID_E0FC;
 
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_rsa_generate_keypair(me_crypt,
                                                          rsa_key_type,
                                                          (uint8_t)(OPTIGA_KEY_USAGE_SIGN | OPTIGA_KEY_USAGE_AUTHENTICATION | OPTIGA_KEY_USAGE_KEY_AGREEMENT) ,
                                                          FALSE,
                                                          &privateKey_oid,
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
    OPTIGA_ARDUINO_LOG_STATUS(return_status);
    
    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::generateKeypairRSA(uint8_t* p_pubkey, uint16_t& plen, 
                                              uint8_t* p_privkey, uint16_t& prlen, 
                                              optiga_rsa_key_type_t rsa_key_type)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
       /**
         * 1. Generate RSA Key pair
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
                                                          (uint8_t)(OPTIGA_KEY_USAGE_SIGN | OPTIGA_KEY_USAGE_AUTHENTICATION | OPTIGA_KEY_USAGE_KEY_AGREEMENT) ,
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
    OPTIGA_ARDUINO_LOG_STATUS(return_status);
    
    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::generateKeypairECC(uint8_t* p_pubkey, uint16_t& plen, 
                                              uint16_t privateKey_oid, optiga_ecc_curve_t ecc_key_type)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
        /**
         * 1. Generate ECC Key pair
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
    OPTIGA_ARDUINO_LOG_STATUS(return_status);
    
    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}

int32_t IFX_OPTIGA_TrustM::generateKeypairECC(uint8_t* p_pubkey, uint16_t& plen, 
                                              uint8_t* p_privkey, uint16_t& prlen,
                                              optiga_ecc_curve_t ecc_key_type)
{
    uint32_t ard_ret = 1;
    optiga_lib_status_t return_status = 0;

    OPTIGA_ARDUINO_LOG_MESSAGE(__FUNCTION__);
    do
    {
        /**
         * 1. Generate ECC Key pair
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
    OPTIGA_ARDUINO_LOG_STATUS(return_status);
    
    if(OPTIGA_LIB_SUCCESS == return_status)
    {
        ard_ret = 0;
    }
    return ard_ret;
}


