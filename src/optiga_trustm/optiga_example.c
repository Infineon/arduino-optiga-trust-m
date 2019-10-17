

#include "optiga_util.h"
#include "optiga_crypt.h"

#include "pal_os_event.h"
#include "pal_os_event_timer.h"
#include "pal_os_timer.h"
#include "optiga_lib_logger.h"
#include "optiga_example.h"

static volatile optiga_lib_status_t optiga_lib_status;
//lint --e{818} suppress "argument "context" is not used in the sample provided"
static void optiga_util_callback(void * context, optiga_lib_status_t return_status)
{
    optiga_lib_status = return_status;
    if (NULL != context)
    {
        // callback to upper layer here
    }
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

void example_optiga_crypt_hash (void)
{
optiga_lib_status_t return_status = 0;

    uint8_t hash_context_buffer [130];
    optiga_hash_context_t hash_context;

    uint8_t data_to_hash [] = {"OPTIGA, Infineon Technologies AG"};
    hash_data_from_host_t hash_data_host;

    uint8_t digest [32];

    optiga_crypt_t * me = NULL;
    optiga_util_t  * me_util = NULL;
    OPTIGA_EXAMPLE_LOG_MESSAGE(__FUNCTION__);

    do
    {
    	/**
		 * Create OPTIGA Util Instance
		 */
		me_util = optiga_util_create(0, optiga_util_callback, NULL);
		if (NULL == me_util)
		{
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
            //optiga_util_open_application failed
            return_status = optiga_lib_status;
            break;
        }


        /**
         * 1. Create OPTIGA Crypt Instance
         */
        me = optiga_crypt_create(0, optiga_util_callback, NULL);
        if (NULL == me)
        {
            break;
        }

        /**
         * 2. Initialize the Hash context
         */
        OPTIGA_HASH_CONTEXT_INIT(hash_context,hash_context_buffer,  \
                                 sizeof(hash_context_buffer),(uint8_t)OPTIGA_HASH_TYPE_SHA_256);

        optiga_lib_status = OPTIGA_LIB_BUSY;

        /**
         * 3. Initialize the hashing context at OPTIGA
         */
        return_status = optiga_crypt_hash_start(me, &hash_context);
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
         * 4. Continue hashing the data
         */
        hash_data_host.buffer = data_to_hash;
        hash_data_host.length = sizeof(data_to_hash);

        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_hash_update(me,
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
         * 5. Finalize the hash
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_hash_finalize(me,
                                                   &hash_context,
                                                   digest);

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


        /**
         * Close the application on OPTIGA 
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
            //optiga_util_close_application failed
            return_status = optiga_lib_status;
            break;
        }
    
    } while (FALSE);
    OPTIGA_EXAMPLE_LOG_STATUS(return_status);


    if (me)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_crypt_destroy(me);
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
}

void example_optiga_crypt_ecc_generate_keypair(void)
{
    optiga_lib_status_t return_status = 0;
    optiga_key_id_t optiga_key_id;

    //To store the generated public key as part of Generate key pair
    uint8_t public_key [100];
    uint16_t public_key_length = sizeof(public_key);

    optiga_crypt_t * me = NULL;
    optiga_util_t  * me_util = NULL;
    OPTIGA_EXAMPLE_LOG_MESSAGE(__FUNCTION__);

    do
    {
        /**
		 * Create OPTIGA Util Instance
		 */
		me_util = optiga_util_create(0, optiga_util_callback, NULL);
		if (NULL == me_util)
		{
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
            //optiga_util_open_application failed
            return_status = optiga_lib_status;
            break;
        }
        /**
         * 1. Create OPTIGA Crypt Instance
         */
        me = optiga_crypt_create(0, optiga_util_callback, NULL);
        if (NULL == me)
        {
            break;
        }

        /**
         * 2. Generate ECC Key pair
         *       - Use ECC NIST P 256 Curve
         *       - Specify the Key Usage (Key Agreement or Sign based on requirement)
         *       - Store the Private key in OPTIGA Key store
         *       - Export Public Key
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        optiga_key_id = OPTIGA_KEY_ID_E0F1;
        //for Session based, use OPTIGA_KEY_ID_SESSION_BASED as key id as shown below.
        //optiga_key_id = OPTIGA_KEY_ID_SESSION_BASED;
        return_status = optiga_crypt_ecc_generate_keypair(me,
                                                          OPTIGA_ECC_CURVE_NIST_P_256,
                                                          (uint8_t)OPTIGA_KEY_USAGE_SIGN,
                                                          FALSE,
                                                          &optiga_key_id,
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
            //Key pair generation failed
            return_status = optiga_lib_status;
            break;
        }
        return_status = OPTIGA_LIB_SUCCESS;

        /**
         * Close the application on OPTIGA 
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
            //optiga_util_close_application failed
            return_status = optiga_lib_status;
            break;
        }

    } while (FALSE);
    OPTIGA_EXAMPLE_LOG_STATUS(return_status);

    if (me)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_crypt_destroy(me);
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
}

void example_optiga_crypt_ecdsa_sign(void)
{
    //SHA-256 Digest to be signed
    static uint8_t digest [] =
    {
        // Size of digest to be chosen based on Curve
        0x61, 0xC7, 0xDE, 0xF9, 0x0F, 0xD5, 0xCD, 0x7A,0x8B, 0x7A, 0x36, 0x41, 0x04, 0xE0, 0x0D, 0x82,
        0x38, 0x46, 0xBF, 0xB7, 0x70, 0xEE, 0xBF, 0x8F,0x40, 0x25, 0x2E, 0x0A, 0x21, 0x42, 0xAF, 0x9C,
    };

    //To store the signture generated
    uint8_t signature [80];
    uint16_t signature_length = sizeof(signature);

    optiga_crypt_t * me = NULL;
    optiga_util_t * me_util = NULL;
    optiga_lib_status_t return_status = 0;
    OPTIGA_EXAMPLE_LOG_MESSAGE(__FUNCTION__);

    do
    {
        /**
		 * Create OPTIGA Util Instance
		 */
		me_util = optiga_util_create(0, optiga_util_callback, NULL);
		if (NULL == me_util)
		{
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
            //optiga_util_open_application failed
            return_status = optiga_lib_status;
            break;
        }
        /**
         * 1. Create OPTIGA Crypt Instance
         */
        me = optiga_crypt_create(0, optiga_util_callback, NULL);
        if (NULL == me)
        {
            break;
        }

        /**
         * 2. Sign the digest using Private key from Key Store ID E0F0
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_ecdsa_sign(me,
                                                digest,
                                                sizeof(digest),
                                                OPTIGA_KEY_ID_E0F0,
                                                signature,
                                                &signature_length);

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_crypt_ecdsa_sign operation is completed
            pal_os_event_process();
        }

        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            return_status = optiga_lib_status;
            break;
        }
        return_status = OPTIGA_LIB_SUCCESS;

        /**
         * Close the application on OPTIGA 
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
            //optiga_util_close_application failed
            return_status = optiga_lib_status;
            break;
        }
    } while (FALSE);
    OPTIGA_EXAMPLE_LOG_STATUS(return_status);
    
    if (me)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_crypt_destroy(me);
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
}

void example_optiga_crypt_ecdsa_verify(void)
{
    static uint8_t public_key [] =
    {
        //BitString Format
        0x03,
            //Remaining length
            0x42,
                //unused bits
                0x00,
                //Compression format
                0x04,
                //NIST-256 Public Key
                0x8b,0x88,0x9c,0x1d,0xd6,0x07,0x58,0x2e,
                0xd6,0xf8,0x2c,0xc2,0xd9,0xbe,0xd0,0xfe,
                0x64,0xf3,0x24,0x5e,0x94,0x7d,0x54,0xcd,
                0x20,0xdc,0x58,0x98,0xcf,0x51,0x31,0x44,
                0x22,0xea,0x01,0xd4,0x0b,0x23,0xb2,0x45,
                0x7c,0x42,0xdf,0x3c,0xfb,0x0d,0x33,0x10,
                0xb8,0x49,0xb7,0xaa,0x0a,0x85,0xde,0xe7,
                0x6a,0xf1,0xac,0x31,0x31,0x1e,0x8c,0x4b
    };

    //SHA-256 Digest
    static uint8_t digest [] =
    {
        0xE9,0x5F,0xB3,0xB1,0x9F,0xA4,0xDD,0x27,0xFE,0xAE,0xB3,0x33,0x40,0x80,0xCE,0x35,
        0xDF,0x3E,0x08,0xF1,0x6F,0x36,0xF3,0x24,0x0E,0xB0,0xB3,0x2F,0xAB,0xD0,0x90,0xCA,
    };

    // ECDSA Signature
    static uint8_t signature [] =
    {
        //DER INTEGER Format
        0x02,
            //Length
            0x20,
                0x39,0xA4,0x70,0xE9,0x32,0x30,0xF5,0x5F,
                0xA4,0xDF,0x8A,0x07,0x36,0x58,0x65,0xC6,
                0xE6,0x1B,0x07,0x51,0xFB,0xC6,0x16,0x05,
                0xEB,0xDF,0x56,0x6D,0xA9,0x50,0x3B,0x24,
        //DER INTEGER Format
        0x02,
            //Length
            0x1E,
                0x49,0x33,0x6C,0x07,0x2B,0xD0,0x40,0x20,
                0x0F,0xD4,0xE0,0x7E,0x67,0x66,0xC4,0xF5,
                0x7F,0x98,0xEC,0x38,0xB8,0xEF,0x44,0x8F,
                0x6A,0xE1,0xFD,0x1E,0x92,0xB4,
    };

    public_key_from_host_t public_key_details = {
                                                 public_key,
                                                 sizeof(public_key),
                                                 (uint8_t)OPTIGA_ECC_CURVE_NIST_P_256
                                                };

    optiga_lib_status_t return_status = 0;

    optiga_crypt_t * me = NULL;
    optiga_util_t * me_util = NULL;
    OPTIGA_EXAMPLE_LOG_MESSAGE(__FUNCTION__);
    do
    {
   	    /**
		 * Create OPTIGA Util Instance
		 */
		me_util = optiga_util_create(0, optiga_util_callback, NULL);
		if (NULL == me_util)
		{
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
            //optiga_util_open_application failed
            return_status = optiga_lib_status;
            break;
        }

        /**
         * 1. Create OPTIGA Crypt Instance
         */
        me = optiga_crypt_create(0, optiga_util_callback, NULL);
        if (NULL == me)
        {
            break;
        }

        /**
         * 2. Verify ECDSA signature using public key from host
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_ecdsa_verify (me,
                                                   digest,
                                                   sizeof(digest),
                                                   signature,
                                                   sizeof(signature),
                                                   OPTIGA_CRYPT_HOST_DATA,
                                                   &public_key_details);

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_crypt_ecdsa_verify operation is completed
            pal_os_event_process();
        }

        if ((OPTIGA_LIB_SUCCESS != optiga_lib_status))
        {
            //Signature verification failed.
            return_status = optiga_lib_status;
            break;
        }

        /**
         * Close the application on OPTIGA 
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
            //optiga_util_close_application failed
            return_status = optiga_lib_status;
            break;
        }

        return_status = OPTIGA_LIB_SUCCESS;
    } while (FALSE);
    OPTIGA_EXAMPLE_LOG_STATUS(return_status);
    
    if (me)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_crypt_destroy(me);
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
}

void example_optiga_crypt_ecdh(void)
{
    // Peer public key details for the ECDH operation
    static uint8_t peer_public_key [] =
    {
        //Bit string format
        0x03,
            //Remaining length
            0x42,
                //Unused bits
                0x00,
                //Compression format
                0x04,
                //Public Key
                0x94, 0x89, 0x2F, 0x09, 0xEA, 0x4E, 0xCA, 0xBC, 0x6A, 0x4E, 0xF2, 0x06, 0x36, 0x26, 0xE0, 0x5D,
                0xE0, 0xD5, 0xF9, 0x77, 0xEA, 0xC3, 0xB2, 0x70, 0xAC, 0xE2, 0x19, 0x00, 0xF5, 0xDB, 0x56, 0xE7,
                0x37, 0xBB, 0xBE, 0x46, 0xE4, 0x49, 0x76, 0x38, 0x25, 0xB5, 0xF8, 0x94, 0x74, 0x9E, 0x1A, 0xB6,
                0x5A, 0xF1, 0x29, 0xD7, 0x3A, 0xB6, 0x9B, 0x80, 0xAC, 0xC5, 0xE1, 0xC3, 0x10, 0xF2, 0x16, 0xC6,
    };
    static public_key_from_host_t peer_public_key_details =
    {
        (uint8_t *)peer_public_key,
        sizeof(peer_public_key),
        (uint8_t)OPTIGA_ECC_CURVE_NIST_P_256,
    };

    optiga_lib_status_t return_status = 0;
    optiga_key_id_t optiga_key_id;

    //To store the generated public key as part of Generate key pair
    uint8_t public_key [100];
    uint16_t public_key_length = sizeof(public_key);

    //To store the generated shared secret as part of ECDH
    uint8_t shared_secret [32];

    optiga_crypt_t * me = NULL;
    optiga_util_t * me_util = NULL;
    OPTIGA_EXAMPLE_LOG_MESSAGE(__FUNCTION__);

    do
    {
       	/**
		 * Create OPTIGA Util Instance
		 */
		me_util = optiga_util_create(0, optiga_util_callback, NULL);
		if (NULL == me_util)
		{
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
            //optiga_util_open_application failed
            return_status = optiga_lib_status;
            break;
        }

        /**
         * 1. Create OPTIGA Crypt Instance
         */
        me = optiga_crypt_create(0, optiga_util_callback, NULL);
        if (NULL == me)
        {
            break;
        }

        /**
         * 2. Generate ECC Key pair - To use the private key with ECDH in the next step
         *       - Use ECC NIST P 256 Curve
         *       - Specify the Key Usage as Key Agreement
         *       - Store the Private key with in OPTIGA Session
         *       - Export Public Key
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        optiga_key_id = OPTIGA_KEY_ID_SESSION_BASED;

        // OPTIGA_CRYPT_SET_COMMS_PROTECTION_LEVEL(me, OPTIGA_COMMS_COMMAND_PROTECTION);
        // OPTIGA_CRYPT_SET_COMMS_PROTOCOL_VERSION(me, OPTIGA_COMMS_PROTOCOL_VERSION_PRE_SHARED_SECRET);
        return_status = optiga_crypt_ecc_generate_keypair(me,
                                                          OPTIGA_ECC_CURVE_NIST_P_256,
                                                          (uint8_t)OPTIGA_KEY_USAGE_KEY_AGREEMENT,
                                                          FALSE,
                                                          &optiga_key_id,
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
         * 3. Perform ECDH using the Peer Public key
         *       - Use ECC NIST P 256 Curve
         *       - Provide the peer public key details
         *       - Export the generated shared secret with protected I2C communication
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        // OPTIGA_CRYPT_SET_COMMS_PROTECTION_LEVEL(me, OPTIGA_COMMS_COMMAND_PROTECTION);
        // OPTIGA_CRYPT_SET_COMMS_PROTOCOL_VERSION(me, OPTIGA_COMMS_PROTOCOL_VERSION_PRE_SHARED_SECRET);
        return_status = optiga_crypt_ecdh(me,
                                          optiga_key_id,
                                          &peer_public_key_details,
                                          TRUE,
                                          shared_secret);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_crypt_ecdh operation is completed
            pal_os_event_process();
        }

        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            //ECDH Operation failed.
            return_status = optiga_lib_status;
            break;
        }

        /**
         * Close the application on OPTIGA 
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
            //optiga_util_close_application failed
            return_status = optiga_lib_status;
            break;
        }

        return_status = OPTIGA_LIB_SUCCESS;
    } while (FALSE);
    OPTIGA_EXAMPLE_LOG_STATUS(return_status);
    
    if (me)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_crypt_destroy(me);
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
}

void example_optiga_crypt_random(void)
{
    uint8_t random_data_buffer [32];
    optiga_crypt_t * me = NULL;
    optiga_util_t * me_util = NULL;
    optiga_lib_status_t return_status = 0;
    OPTIGA_EXAMPLE_LOG_MESSAGE(__FUNCTION__);

    do
    {
       	/**
		 * Create OPTIGA Util Instance
		 */
		me_util = optiga_util_create(0, optiga_util_callback, NULL);
		if (NULL == me_util)
		{
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
            //optiga_util_open_application failed
            return_status = optiga_lib_status;
            break;
        }

        /**
         * 1. Create OPTIGA Crypt Instance
         *
         */
        me = optiga_crypt_create(0, optiga_util_callback, NULL);
        if (NULL == me)
        {
            break;
        }

        /**
         * 2. Generate Random -
         *       - Specify the Random type as TRNG
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;

        return_status = optiga_crypt_random(me,
                                            OPTIGA_RNG_TYPE_TRNG,
                                            random_data_buffer,
                                            sizeof(random_data_buffer));

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

        /**
         * Close the application on OPTIGA 
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
            //optiga_util_close_application failed
            return_status = optiga_lib_status;
            break;
        }

        return_status = OPTIGA_LIB_SUCCESS;

    } while (FALSE);
    OPTIGA_EXAMPLE_LOG_STATUS(return_status);
    if (me)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_crypt_destroy(me);
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
}

void example_optiga_crypt_tls_prf_sha256(void)
{
    static uint8_t metadata [] = {
        //Metadata tag in the data object
        0x20, 0x06,
            //Data object type set to PRESSEC
            0xE8, 0x01, 0x21,
            0xD3, 0x01, 0x00,
    };

    /**
     * Default metadata
     */
    static uint8_t default_metadata [] = {
        //Metadata tag in the data object
        0x20, 0x06,
            //Data object type set to BSTR
            0xE8, 0x01, 0x00,
            0xD3, 0x01, 0xFF,
    };

    static uint8_t label [] = "Firmware update";

    static uint8_t random_seed [] = {
        0x61, 0xC7, 0xDE, 0xF9, 0x0F, 0xD5, 0xCD, 0x7A,
        0x8B, 0x7A, 0x36, 0x41, 0x04, 0xE0, 0x0D, 0x82,
        0x38, 0x46, 0xBF, 0xB7, 0x70, 0xEE, 0xBF, 0x8F,
        0x40, 0x25, 0x2E, 0x0A, 0x21, 0x42, 0xAF, 0x9C,
    };

    // Secret to be written to data object which will be
    // later used as part of TLS PRF SHA256 key derivation
    static uint8_t secret_to_be_written [] = {
        0xBF, 0xB7, 0x70, 0xEE, 0xBF, 0x8F, 0x61, 0xC7,
        0x04, 0xE0, 0x0D, 0x82, 0x8B, 0x7A, 0x36, 0x41,
        0xD5, 0xCD, 0x7A, 0x38, 0x46, 0xDE, 0xF9, 0x0F,
        0x21, 0x42, 0x40, 0x25, 0x0A, 0xAF, 0x9C, 0x2E,
    };

    uint8_t decryption_key [16] = {0};

    optiga_lib_status_t return_status = 0;

    optiga_crypt_t * me = NULL;
    optiga_util_t * me_util = NULL;
    OPTIGA_EXAMPLE_LOG_MESSAGE(__FUNCTION__);
    do
    {

        me_util = optiga_util_create(0, optiga_util_callback, NULL);
        if (NULL == me_util)
        {
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
            //optiga_util_open_application failed
            return_status = optiga_lib_status;
            break;
        }

        /**
         * 1. Write the shared secret to the Arbitrary data object F1D0
         *       - This is typically a one time activity and
         *       - use the this OID as input secret to derive keys further
         * 2. Use Erase and Write (OPTIGA_UTIL_ERASE_AND_WRITE) option,
         *    to clear the remaining data in the object
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_write_data(me_util,
                                               0xF1D0,
                                               OPTIGA_UTIL_ERASE_AND_WRITE ,
                                               0x00,
                                               secret_to_be_written,
                                               sizeof(secret_to_be_written));

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
            //Write to data object is failed.
            return_status = optiga_lib_status;
            break;
        }

        /**
         * 2. Change data object type to PRESSEC
         *
         */

        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_write_metadata(me_util,
                                                   0xF1D0,
                                                   metadata,
                                                   sizeof(metadata));

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_util_write_metadata operation is completed
            pal_os_event_process();
        }

        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            //writing metadata to a data object failed.
            return_status = optiga_lib_status;
            break;
        }

        /**
         * 3. Create OPTIGA Crypt Instance
         *
         */
        me = optiga_crypt_create(0, optiga_util_callback, NULL);
        if (NULL == me)
        {
            break;
        }

        /**
         * 4. Derive key (e.g. decryption key) using optiga_crypt_tls_prf_sha256 with protected I2C communication.
         *       - Use shared secret from F1D0 data object
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;

        OPTIGA_CRYPT_SET_COMMS_PROTOCOL_VERSION(me, OPTIGA_COMMS_PROTOCOL_VERSION_PRE_SHARED_SECRET);
        // Default protecition for this API is OPTIGA_COMMS_COMMAND_PROTECTION
        return_status = optiga_crypt_tls_prf_sha256(me,
                                                    0xF1D0, /* Input secret OID */
                                                    label,
                                                    sizeof(label),
                                                    random_seed,
                                                    sizeof(random_seed),
                                                    sizeof(decryption_key),
                                                    TRUE,
                                                    decryption_key);

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_crypt_tls_prf_sha256 operation is completed
            pal_os_event_process();
        }

        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            //Derive key operation failed.
            return_status = optiga_lib_status;
            break;
        }
        /**
         * 5. Change meta data to default value
         *
         */


        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_write_metadata(me_util,
                                                   0xF1D0,
                                                   default_metadata,
                                                   sizeof(default_metadata));

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_util_write_metadata operation is completed
            pal_os_event_process();
        }

        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            //writing metadata to a data object failed.
            return_status = optiga_lib_status;
            break;
        }

        /**
         * Close the application on OPTIGA 
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
            //optiga_util_close_application failed
            return_status = optiga_lib_status;
            break;
        }

        return_status = OPTIGA_LIB_SUCCESS;

    } while (FALSE);
    OPTIGA_EXAMPLE_LOG_STATUS(return_status);
    
    if (me)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_crypt_destroy(me);
        if(OPTIGA_LIB_SUCCESS != return_status)
        {
            //lint --e{774} suppress This is a generic macro
            OPTIGA_EXAMPLE_LOG_STATUS(return_status);
        }
    }

    if (me_util)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_util_destroy(me_util);
        if(OPTIGA_LIB_SUCCESS != return_status)
        {
            //lint --e{774} suppress This is a generic macro
            OPTIGA_EXAMPLE_LOG_STATUS(return_status);
        }
    }
}

void example_optiga_util_read_data(void)
{
    uint16_t offset, bytes_to_read;
    uint16_t optiga_oid;
    uint8_t read_data_buffer[1024];
    optiga_lib_status_t return_status = 0;
    optiga_util_t * me = NULL;
    OPTIGA_EXAMPLE_LOG_MESSAGE(__FUNCTION__);

    do
    {
        /**
         * 1. Create OPTIGA Util Instance
         */
        me = optiga_util_create(0, optiga_util_callback, NULL);
        if (NULL == me)
        {
            break;
        }

        /**
         * Open the application on OPTIGA which is a precondition to perform any other operations
         * using optiga_util_open_application
         */        
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_open_application(me, 0);

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


        //Read device end entity certificate from OPTIGA
        optiga_oid = 0xE0E0;
        offset = 0x00;
        bytes_to_read = 1024;

        // OPTIGA Comms Shielded connection settings to enable the protection
        //OPTIGA_UTIL_SET_COMMS_PROTOCOL_VERSION(me, OPTIGA_COMMS_PROTOCOL_VERSION_PRE_SHARED_SECRET);
        //OPTIGA_UTIL_SET_COMMS_PROTECTION_LEVEL(me, OPTIGA_COMMS_RESPONSE_PROTECTION);

        /**
         * 2. Read data from a data object (e.g. certificate data object)
         *    using optiga_util_read_data.
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_read_data(me,
                                              optiga_oid,
                                              offset,
                                              read_data_buffer,
                                              &bytes_to_read);

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

        OPTIGA_EXAMPLE_LOG_HEX_DATA(read_data_buffer,bytes_to_read);
        
        /**
         * Read metadata of a data object (e.g. certificate data object E0E0)
         * using optiga_util_read_data.
         */
        optiga_oid = 0xE0E0;
        bytes_to_read = 1024;
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_read_metadata(me,
                                                  optiga_oid,
                                                  read_data_buffer,
                                                  &bytes_to_read);

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_util_read_metadata operation is completed
            pal_os_event_process();
        }

        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            //Reading metadata data object failed.
            return_status = optiga_lib_status;
            break;
        }

        /**
         * Close the application on OPTIGA without hibernating
         * using optiga_util_close_application
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_close_application(me, 0);
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
            //optiga_util_close_application failed
            return_status = optiga_lib_status;
            break;
        }

        return_status = OPTIGA_LIB_SUCCESS;
    } while (FALSE);    
    OPTIGA_EXAMPLE_LOG_STATUS(return_status);

    if (me)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_util_destroy(me);
        if(OPTIGA_LIB_SUCCESS != return_status)
        {
            //lint --e{774} suppress This is a generic macro
            OPTIGA_UTIL_LOG_STATUS(return_status);
        }
    }
}

void example_optiga_util_write_data(void)
{
    static uint8_t trust_anchor [] = {
    //00    01    02    03    04    05    06    07    08    09    0A    0B    0C    0D    0E    0F
    0x30, 0x82, 0x02, 0x7E, 0x30, 0x82, 0x02, 0x05, 0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x09, 0x00,
    0x9B, 0x0C, 0x24, 0xB4, 0x5E, 0x7D, 0xE3, 0x73, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE,
    0x3D, 0x04, 0x03, 0x02, 0x30, 0x74, 0x31, 0x0B, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13,
    0x02, 0x44, 0x45, 0x31, 0x21, 0x30, 0x1F, 0x06, 0x03, 0x55, 0x04, 0x0A, 0x0C, 0x18, 0x49, 0x6E,
    0x66, 0x69, 0x6E, 0x65, 0x6F, 0x6E, 0x20, 0x54, 0x65, 0x63, 0x68, 0x6E, 0x6F, 0x6C, 0x6F, 0x67,
    0x69, 0x65, 0x73, 0x20, 0x41, 0x47, 0x31, 0x1B, 0x30, 0x19, 0x06, 0x03, 0x55, 0x04, 0x0B, 0x0C,
    0x12, 0x4F, 0x50, 0x54, 0x49, 0x47, 0x41, 0x28, 0x54, 0x4D, 0x29, 0x20, 0x54, 0x72, 0x75, 0x73,
    0x74, 0x20, 0x58, 0x31, 0x25, 0x30, 0x23, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x1C, 0x49, 0x6E,
    0x66, 0x69, 0x6E, 0x65, 0x6F, 0x6E, 0x20, 0x54, 0x65, 0x73, 0x74, 0x20, 0x53, 0x65, 0x72, 0x76,
    0x65, 0x72, 0x20, 0x52, 0x6F, 0x6F, 0x74, 0x20, 0x43, 0x41, 0x30, 0x1E, 0x17, 0x0D, 0x31, 0x36,
    0x31, 0x30, 0x31, 0x34, 0x30, 0x33, 0x35, 0x38, 0x33, 0x36, 0x5A, 0x17, 0x0D, 0x34, 0x31, 0x31,
    0x30, 0x30, 0x38, 0x30, 0x33, 0x35, 0x38, 0x33, 0x36, 0x5A, 0x30, 0x74, 0x31, 0x0B, 0x30, 0x09,
    0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x44, 0x45, 0x31, 0x21, 0x30, 0x1F, 0x06, 0x03, 0x55,
    0x04, 0x0A, 0x0C, 0x18, 0x49, 0x6E, 0x66, 0x69, 0x6E, 0x65, 0x6F, 0x6E, 0x20, 0x54, 0x65, 0x63,
    0x68, 0x6E, 0x6F, 0x6C, 0x6F, 0x67, 0x69, 0x65, 0x73, 0x20, 0x41, 0x47, 0x31, 0x1B, 0x30, 0x19,
    0x06, 0x03, 0x55, 0x04, 0x0B, 0x0C, 0x12, 0x4F, 0x50, 0x54, 0x49, 0x47, 0x41, 0x28, 0x54, 0x4D,
    0x29, 0x20, 0x54, 0x72, 0x75, 0x73, 0x74, 0x20, 0x58, 0x31, 0x25, 0x30, 0x23, 0x06, 0x03, 0x55,
    0x04, 0x03, 0x0C, 0x1C, 0x49, 0x6E, 0x66, 0x69, 0x6E, 0x65, 0x6F, 0x6E, 0x20, 0x54, 0x65, 0x73,
    0x74, 0x20, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x20, 0x52, 0x6F, 0x6F, 0x74, 0x20, 0x43, 0x41,
    0x30, 0x76, 0x30, 0x10, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x05, 0x2B,
    0x81, 0x04, 0x00, 0x22, 0x03, 0x62, 0x00, 0x04, 0x7B, 0x2E, 0xE6, 0xFB, 0xBD, 0x6F, 0x40, 0x0F,
    0x41, 0x9F, 0xE5, 0xF0, 0x8C, 0x97, 0x21, 0xB0, 0x07, 0xB5, 0xBB, 0xD2, 0xB8, 0x5A, 0x14, 0x3B,
    0x75, 0x54, 0x7E, 0xEA, 0xFE, 0xF2, 0x8D, 0x5A, 0xB8, 0x54, 0xE0, 0xC8, 0xAD, 0xED, 0xF1, 0xD5,
    0x8B, 0x97, 0xBA, 0x02, 0x3E, 0xD9, 0x25, 0xE0, 0x00, 0x86, 0x17, 0x35, 0xE6, 0xE6, 0xD9, 0x12,
    0x0F, 0x8A, 0x21, 0x1C, 0x62, 0xFA, 0xCE, 0xF6, 0x9E, 0xB1, 0xF8, 0x8C, 0xA3, 0xDC, 0x52, 0x04,
    0x83, 0xEB, 0xA0, 0xB3, 0xFA, 0xB0, 0xCA, 0x02, 0x30, 0xB1, 0xFE, 0x53, 0x4E, 0xAD, 0xFB, 0xE0,
    0x88, 0x05, 0x86, 0x4E, 0x5E, 0x67, 0xEB, 0x7B, 0xA3, 0x63, 0x30, 0x61, 0x30, 0x1D, 0x06, 0x03,
    0x55, 0x1D, 0x0E, 0x04, 0x16, 0x04, 0x14, 0x91, 0x4A, 0x4B, 0x07, 0x58, 0xB2, 0xC6, 0x4B, 0x37,
    0xFD, 0x91, 0x62, 0xD8, 0x8A, 0x17, 0x28, 0xAA, 0x94, 0x18, 0x62, 0x30, 0x1F, 0x06, 0x03, 0x55,
    0x1D, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0x91, 0x4A, 0x4B, 0x07, 0x58, 0xB2, 0xC6, 0x4B,
    0x37, 0xFD, 0x91, 0x62, 0xD8, 0x8A, 0x17, 0x28, 0xAA, 0x94, 0x18, 0x62, 0x30, 0x0F, 0x06, 0x03,
    0x55, 0x1D, 0x13, 0x01, 0x01, 0xFF, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xFF, 0x30, 0x0E, 0x06,
    0x03, 0x55, 0x1D, 0x0F, 0x01, 0x01, 0xFF, 0x04, 0x04, 0x03, 0x02, 0x02, 0x04, 0x30, 0x0A, 0x06,
    0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x03, 0x67, 0x00, 0x30, 0x64, 0x02, 0x30,
    0x20, 0x1C, 0x7A, 0x21, 0x50, 0x50, 0xC9, 0x15, 0x1C, 0xC5, 0x14, 0x8D, 0x46, 0x5C, 0xA6, 0xD3,
    0x81, 0xCE, 0x57, 0x06, 0x1A, 0xAE, 0x39, 0x10, 0x27, 0x51, 0x42, 0xEF, 0xCD, 0x64, 0x75, 0x99,
    0xDE, 0x0D, 0x3D, 0x01, 0x47, 0x69, 0xFC, 0x93, 0x6D, 0x99, 0xC7, 0xF0, 0xF8, 0x8C, 0xAA, 0xD1,
    0x02, 0x30, 0x68, 0xC3, 0x27, 0xD9, 0x0F, 0x52, 0xAD, 0x3A, 0xA8, 0xDB, 0xF8, 0x53, 0x11, 0x1D,
    0xF1, 0x30, 0x6B, 0x39, 0xF3, 0x3F, 0xEF, 0x65, 0x61, 0xBE, 0xC4, 0xDD, 0x19, 0x11, 0x1E, 0x83,
    0xF9, 0xE8, 0x3F, 0x41, 0x97, 0x45, 0xFC, 0x61, 0xE0, 0x06, 0xD0, 0xE6, 0xF7, 0x5C, 0x9F, 0xE2,
    0x57, 0xC2,
    };

    /**
     * Sample metadata
     */
    static uint8_t metadata [] = {
        //Metadata tag in the data object
        0x20, 0x05,
            //Read tag in the metadata
            0xD1, 0x03,
                //LcsO < Operation
                0xE1 , 0xFB, 0x03,
    };

     uint16_t optiga_oid;
    uint16_t offset;

    optiga_lib_status_t return_status = 0;
    optiga_util_t * me = NULL;
    OPTIGA_EXAMPLE_LOG_MESSAGE(__FUNCTION__);

    do
    {
        /**
         * 1. Create OPTIGA Util Instance
         */
        me = optiga_util_create(0, optiga_util_callback, NULL);
        if (NULL == me)
        {
            break;
        }

        /**
         * Open the application on OPTIGA which is a precondition to perform any other operations
         * using optiga_util_open_application
         */        
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_open_application(me, 0);

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

        /**
         * Write Trust Anchor to a Trust Anchor object (e.g. E0E8)
         * using optiga_util_write_data with no shielded connection protection.
         *
         * Use Erase and Write (OPTIGA_UTIL_ERASE_AND_WRITE) option,
         * to clear the remaining data in the object
         */
        optiga_oid = 0xE0E8;
        offset = 0x00;

        OPTIGA_UTIL_SET_COMMS_PROTECTION_LEVEL(me, OPTIGA_COMMS_NO_PROTECTION);

        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_write_data(me,
                                               optiga_oid,
                                               OPTIGA_UTIL_ERASE_AND_WRITE,
                                               offset,
                                               trust_anchor,
                                               sizeof(trust_anchor));

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

        /**
         *
         * Write metadata to Trust Anchor to a Trust Anchor object (e.g. E0E8)
         * using optiga_util_write_metadata.
         *
         * Sample metadata: write "Read" access condition as LcsO < Operational
         *
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_write_metadata(me,
                                                   0xE0E8,
                                                   metadata,
                                                   sizeof(metadata));

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_util_write_metadata operation is completed
            pal_os_event_process();
        }

        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            //writing metadata to a data object failed.
            return_status = optiga_lib_status;
            break;
        }

        /**
         * Close the application on OPTIGA without hibernating
         * using optiga_util_close_application
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_close_application(me, 0);
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
            //optiga_util_close_application failed
            return_status = optiga_lib_status;
            break;
        }
        return_status = OPTIGA_LIB_SUCCESS;

    } while (FALSE);
    OPTIGA_EXAMPLE_LOG_STATUS(return_status);
    
    if (me)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_util_destroy(me);
        if(OPTIGA_LIB_SUCCESS != return_status)
        {
            //lint --e{774} suppress This is a generic macro
            OPTIGA_EXAMPLE_LOG_STATUS(return_status);
        }
    }
}

void example_optiga_crypt_rsa_generate_keypair(void)
{
    optiga_lib_status_t return_status = 0;
    optiga_key_id_t optiga_key_id;

    //To store the generated public key as part of Generate key pair
    /*
    *
    * Give a buffer of minimum size required to store the exported public key.
    * E.g
    * For 1024 bit key : modulus(128 bytes) + public expoenent(3 bytes) + Encoding bytes length(approximately 15 bytes)
    * For 2048 bit key : modulus(256 bytes) + public expoenent(3 bytes) + Encoding bytes length(approximately 15 bytes)
    */
    uint8_t public_key [150];
    uint16_t public_key_length = sizeof(public_key);

    optiga_crypt_t * me = NULL;
    optiga_util_t * me_util = NULL;
    OPTIGA_EXAMPLE_LOG_MESSAGE(__FUNCTION__);

    do
    {
       	/**
		 * Create OPTIGA Util Instance
		 */
		me_util = optiga_util_create(0, optiga_util_callback, NULL);
		if (NULL == me_util)
		{
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
            //optiga_util_open_application failed
            return_status = optiga_lib_status;
            break;
        }

        /**
         * 1. Create OPTIGA Crypt Instance
         */
        me = optiga_crypt_create(0, optiga_util_callback, NULL);
        if (NULL == me)
        {
            break;
        }

        /**
         * 2. Generate RSA Key pair
         *       - Use 1024 or 2048 bit RSA key
         *       - Specify the Key Usage
         *       - Store the Private key in OPTIGA Key store
         *              (When exporting the private key, provide buffer of sufficient length (key size in bytes +
         *               encoding length))
         *       - Export Public Key
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        optiga_key_id = OPTIGA_KEY_ID_E0FC;
        return_status = optiga_crypt_rsa_generate_keypair(me,
                                                          OPTIGA_RSA_KEY_1024_BIT_EXPONENTIAL,
                                                          (uint8_t)OPTIGA_KEY_USAGE_SIGN,
                                                          FALSE,
                                                          &optiga_key_id,
                                                          public_key,
                                                          &public_key_length);
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

        /**
         * Close the application on OPTIGA 
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
            //optiga_util_close_application failed
            return_status = optiga_lib_status;
            break;
        }

        return_status = OPTIGA_LIB_SUCCESS;
    } while (FALSE);
    OPTIGA_EXAMPLE_LOG_STATUS(return_status);

    if (me)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_crypt_destroy(me);
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
}

void example_optiga_crypt_rsa_sign(void)
{
    // SHA-256 digest to be signed
    static uint8_t digest [] =
    {
        0x61, 0xC7, 0xDE, 0xF9, 0x0F, 0xD5, 0xCD, 0x7A,0x8B, 0x7A, 0x36, 0x41, 0x04, 0xE0, 0x0D, 0x82,
        0x38, 0x46, 0xBF, 0xB7, 0x70, 0xEE, 0xBF, 0x8F,0x40, 0x25, 0x2E, 0x0A, 0x21, 0x42, 0xAF, 0x9C,
    };

        //To store the signture generated
    uint8_t signature [200];
    uint16_t signature_length = sizeof(signature);

    //Crypt Instance
    optiga_crypt_t * me = NULL;
    optiga_util_t * me_util = NULL;
    optiga_lib_status_t return_status = 0;
    OPTIGA_EXAMPLE_LOG_MESSAGE(__FUNCTION__);

    do
    {
        /**
		 * Create OPTIGA Util Instance
		 */
		me_util = optiga_util_create(0, optiga_util_callback, NULL);
		if (NULL == me_util)
		{
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
            //optiga_util_open_application failed
            return_status = optiga_lib_status;
            break;
        }

        /**
         * 1. Create OPTIGA Crypt Instance
         *
         */
        me = optiga_crypt_create(0, optiga_util_callback, NULL);
        if (NULL == me)
        {
            break;
        }

        /**
         * 2. Sign the digest -
         *       - Use Private key from Key Store ID E0FC
         *       - Signature scheme is SHA256,
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_rsa_sign(me,
                                              OPTIGA_RSASSA_PKCS1_V15_SHA256,
                                              digest,
                                              sizeof(digest),
                                              OPTIGA_KEY_ID_E0FC,
                                              signature,
                                              &signature_length,
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

        /**
         * Close the application on OPTIGA 
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
            //optiga_util_close_application failed
            return_status = optiga_lib_status;
            break;
        }

        return_status = OPTIGA_LIB_SUCCESS;

    } while (FALSE);
    OPTIGA_EXAMPLE_LOG_STATUS(return_status);
    
    if (me)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_crypt_destroy(me);
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
}

void example_optiga_crypt_rsa_verify(void)
{
    static uint8_t public_key [] = {
        //BIT String
        0x03,
            //BIT String Length
            0x81, 0x8E,
            //UnusedBits
            0x00,
            //SEQUENCE
            0x30,
                //Length
                0x81, 0x8A,
                //INTEGER (pub key)
                0x02,
                //Pub key modulus length
                0x81, 0x81,
                //Public key modulus
                0x00,
                0xA1, 0xD4, 0x6F, 0xBA, 0x23, 0x18, 0xF8, 0xDC, 0xEF, 0x16, 0xC2, 0x80, 0x94, 0x8B, 0x1C, 0xF2,
                0x79, 0x66, 0xB9, 0xB4, 0x72, 0x25, 0xED, 0x29, 0x89, 0xF8, 0xD7, 0x4B, 0x45, 0xBD, 0x36, 0x04,
                0x9C, 0x0A, 0xAB, 0x5A, 0xD0, 0xFF, 0x00, 0x35, 0x53, 0xBA, 0x84, 0x3C, 0x8E, 0x12, 0x78, 0x2F,
                0xC5, 0x87, 0x3B, 0xB8, 0x9A, 0x3D, 0xC8, 0x4B, 0x88, 0x3D, 0x25, 0x66, 0x6C, 0xD2, 0x2B, 0xF3,
                0xAC, 0xD5, 0xB6, 0x75, 0x96, 0x9F, 0x8B, 0xEB, 0xFB, 0xCA, 0xC9, 0x3F, 0xDD, 0x92, 0x7C, 0x74,
                0x42, 0xB1, 0x78, 0xB1, 0x0D, 0x1D, 0xFF, 0x93, 0x98, 0xE5, 0x23, 0x16, 0xAA, 0xE0, 0xAF, 0x74,
                0xE5, 0x94, 0x65, 0x0B, 0xDC, 0x3C, 0x67, 0x02, 0x41, 0xD4, 0x18, 0x68, 0x45, 0x93, 0xCD, 0xA1,
                0xA7, 0xB9, 0xDC, 0x4F, 0x20, 0xD2, 0xFD, 0xC6, 0xF6, 0x63, 0x44, 0x07, 0x40, 0x03, 0xE2, 0x11,
                //INTEGER (publicExponent)
                0x02,
                //Pub key exponent length
                0x04,
                //Public Exponent
                0x00, 0x01, 0x00, 0x01
    };

    //SHA-256 Digest
    static uint8_t digest [] =
    {
        0x91, 0x70, 0x02, 0x48, 0x3F, 0xBD, 0x5F, 0xDD, 0xD5, 0x38, 0xEB, 0xDA, 0x9A, 0x5E, 0x1F, 0x46,
        0xFC, 0xAD, 0x8F, 0x1E, 0x2C, 0x75, 0xB0, 0x83, 0xD0, 0x71, 0x2B, 0x80, 0xD4, 0xAA, 0xC6, 0x9B
    };

    // RSA 1024 Signature ( no additional encoding needed )
    static uint8_t signature [] =
    {
        0x5B, 0xDE, 0x46, 0xE4, 0x35, 0x48, 0xF4, 0x81, 0x45, 0x7C, 0x72, 0x31, 0x54, 0x55, 0xE8, 0x9F,
        0x1D, 0xD0, 0x5D, 0x9D, 0xEC, 0x40, 0xE6, 0x6B, 0x89, 0xF3, 0xBC, 0x52, 0x68, 0xB1, 0xD8, 0x70,
        0x35, 0x05, 0xFC, 0x98, 0xF6, 0x36, 0x99, 0x24, 0x53, 0xF0, 0x17, 0xB8, 0x9B, 0xD4, 0xA0, 0x5F,
        0x12, 0x04, 0x8A, 0xA1, 0xA7, 0x96, 0xE6, 0x33, 0xCA, 0x48, 0x84, 0xD9, 0x00, 0xE4, 0xA3, 0x8E,
        0x2F, 0x6F, 0x3F, 0x6D, 0xE0, 0x1D, 0xF8, 0xEA, 0xE0, 0x95, 0xBA, 0x63, 0x15, 0xED, 0x7B, 0x6A,
        0xB6, 0x6E, 0x20, 0x17, 0xB5, 0x64, 0xDE, 0x49, 0x64, 0x97, 0xCA, 0x5E, 0x4D, 0x84, 0x63, 0xA0,
        0xF1, 0x00, 0x6C, 0xEE, 0x70, 0x89, 0xD5, 0x6E, 0xC5, 0x05, 0x31, 0x0D, 0xAA, 0xB7, 0xBA, 0xA0,
        0xAA, 0xBF, 0x98, 0xE8, 0x39, 0x93, 0x70, 0x07, 0x2D, 0xFF, 0x42, 0xF9, 0xA4, 0x6F, 0x1B, 0x00
    };

    public_key_from_host_t public_key_details =
    {
         public_key,
         sizeof(public_key),
         (uint8_t)OPTIGA_RSA_KEY_1024_BIT_EXPONENTIAL
    };

    optiga_lib_status_t return_status = 0;
    optiga_crypt_t * me = NULL;
    optiga_util_t * me_util = NULL;
    OPTIGA_EXAMPLE_LOG_MESSAGE(__FUNCTION__);
    do
    {
        /**
		 * Create OPTIGA Util Instance
		 */
		me_util = optiga_util_create(0, optiga_util_callback, NULL);
		if (NULL == me_util)
		{
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
            //optiga_util_open_application failed
            return_status = optiga_lib_status;
            break;
        }
    
        /**
         * 1. Create OPTIGA Crypt Instance
         */
        me = optiga_crypt_create(0, optiga_util_callback, NULL);
        if (NULL == me)
        {
            break;
        }

        /**
         * 2. Verify RSA signature using public key from host
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_rsa_verify (me,
                                                 OPTIGA_RSASSA_PKCS1_V15_SHA256,
                                                 digest,
                                                 sizeof(digest),
                                                 signature,
                                                 sizeof(signature),
                                                 OPTIGA_CRYPT_HOST_DATA,
                                                 &public_key_details,
                                                 0x0000);

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_crypt_rsa_verify operation is completed
            pal_os_event_process();
        }

        if ((OPTIGA_LIB_SUCCESS != optiga_lib_status))
        {
            //RSA Signature verification failed.
            return_status = optiga_lib_status;
            break;
        }

        /**
         * Close the application on OPTIGA 
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
            //optiga_util_close_application failed
            return_status = optiga_lib_status;
            break;
        }

        return_status = OPTIGA_LIB_SUCCESS;
    } while (FALSE);
    OPTIGA_EXAMPLE_LOG_STATUS(return_status);
    
    if (me)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_crypt_destroy(me);
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
}

void example_optiga_crypt_rsa_decrypt_and_export(void)
{

}

void example_optiga_crypt_rsa_decrypt_and_store(void)
{

}

void example_optiga_crypt_rsa_encrypt_message(void)
{
    static uint8_t public_key [] = {
        //BIT String
        0x03,
            //BIT String Length
            0x81, 0x8E,
                //UnusedBits
                0x00,
                //SEQUENCE
                0x30,
                    //Length
                    0x81, 0x8A,
                    //INTEGER (pub key)
                    0x02,
                    //Pub key modulus length
                    0x81, 0x81,
                        //Public key modulus
                        0x00,
                        0xA1, 0xD4, 0x6F, 0xBA, 0x23, 0x18, 0xF8, 0xDC,
                        0xEF, 0x16, 0xC2, 0x80, 0x94, 0x8B, 0x1C, 0xF2,
                        0x79, 0x66, 0xB9, 0xB4, 0x72, 0x25, 0xED, 0x29,
                        0x89, 0xF8, 0xD7, 0x4B, 0x45, 0xBD, 0x36, 0x04,
                        0x9C, 0x0A, 0xAB, 0x5A, 0xD0, 0xFF, 0x00, 0x35,
                        0x53, 0xBA, 0x84, 0x3C, 0x8E, 0x12, 0x78, 0x2F,
                        0xC5, 0x87, 0x3B, 0xB8, 0x9A, 0x3D, 0xC8, 0x4B,
                        0x88, 0x3D, 0x25, 0x66, 0x6C, 0xD2, 0x2B, 0xF3,
                        0xAC, 0xD5, 0xB6, 0x75, 0x96, 0x9F, 0x8B, 0xEB,
                        0xFB, 0xCA, 0xC9, 0x3F, 0xDD, 0x92, 0x7C, 0x74,
                        0x42, 0xB1, 0x78, 0xB1, 0x0D, 0x1D, 0xFF, 0x93,
                        0x98, 0xE5, 0x23, 0x16, 0xAA, 0xE0, 0xAF, 0x74,
                        0xE5, 0x94, 0x65, 0x0B, 0xDC, 0x3C, 0x67, 0x02,
                        0x41, 0xD4, 0x18, 0x68, 0x45, 0x93, 0xCD, 0xA1,
                        0xA7, 0xB9, 0xDC, 0x4F, 0x20, 0xD2, 0xFD, 0xC6,
                        0xF6, 0x63, 0x44, 0x07, 0x40, 0x03, 0xE2, 0x11,
                    //INTEGER (publicExponent)
                    0x02,
                        //Pub key exponent length
                        0x04,
                        //Public Exponent
                        0x00, 0x01, 0x00, 0x01
    };



    uint8_t message[] = {"RSA PKCS1_v1.5 Encryption of user message"};

    optiga_lib_status_t return_status = 0;
    optiga_rsa_encryption_scheme_t encryption_scheme;
    uint8_t encrypted_message[128];
    uint16_t encrypted_message_length = sizeof(encrypted_message);
    public_key_from_host_t public_key_from_host;

    optiga_crypt_t * me = NULL;
    optiga_util_t * me_util = NULL;
    OPTIGA_EXAMPLE_LOG_MESSAGE(__FUNCTION__);

    do
    {
       	/**
		 * Create OPTIGA Util Instance
		 */
		me_util = optiga_util_create(0, optiga_util_callback, NULL);
		if (NULL == me_util)
		{
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
            //optiga_util_open_application failed
            return_status = optiga_lib_status;
            break;
        }

        /**
         * 1. Create OPTIGA Crypt Instance
         *
         */
        me = optiga_crypt_create(0, optiga_util_callback, NULL);
        if (NULL == me)
        {
            break;
        }

        /**
         * 2. RSA encryption
         */
        encryption_scheme = OPTIGA_RSAES_PKCS1_V15;
        public_key_from_host.public_key = public_key;
        public_key_from_host.length = sizeof(public_key);
        public_key_from_host.key_type = (uint8_t)OPTIGA_RSA_KEY_1024_BIT_EXPONENTIAL;
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_rsa_encrypt_message(me,
                                                            encryption_scheme,
                                                            message,
                                                            sizeof(message),
                                                            NULL,
                                                            0,
                                                            OPTIGA_CRYPT_HOST_DATA,
                                                            &public_key_from_host,
                                                            encrypted_message,
                                                            &encrypted_message_length);

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_crypt_rsa_encrypt_message operation is completed
            pal_os_event_process();
        }

        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            //RSA Encryption failed
            return_status = optiga_lib_status;
            break;
        }

        /**
         * Close the application on OPTIGA 
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
            //optiga_util_close_application failed
            return_status = optiga_lib_status;
            break;
        }

        return_status = OPTIGA_LIB_SUCCESS;

    } while (FALSE);
    OPTIGA_EXAMPLE_LOG_STATUS(return_status);
    
    if (me)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_crypt_destroy(me);
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
}

void example_optiga_crypt_rsa_encrypt_session(void)
{
    optiga_lib_status_t return_status = 0;
    optiga_key_id_t optiga_key_id;
    optiga_rsa_encryption_scheme_t encryption_scheme;

    uint8_t public_key [150];
    uint8_t encrypted_message[128];
    uint16_t encrypted_message_length = sizeof(encrypted_message);
    uint16_t public_key_length = sizeof(public_key);
    public_key_from_host_t public_key_from_host;
    const uint8_t optional_data[2] = {0x01, 0x02};
    uint16_t optional_data_length = sizeof(optional_data);

    optiga_crypt_t * me = NULL;
    optiga_util_t * me_util = NULL;
    OPTIGA_EXAMPLE_LOG_MESSAGE(__FUNCTION__);

    do
    {
       	/**
		 * Create OPTIGA Util Instance
		 */
		me_util = optiga_util_create(0, optiga_util_callback, NULL);
		if (NULL == me_util)
		{
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
            //optiga_util_open_application failed
            return_status = optiga_lib_status;
            break;
        }

        /**
         * 1. Create OPTIGA Crypt Instance
         *
         */
        me = optiga_crypt_create(0, optiga_util_callback, NULL);
        if (NULL == me)
        {
            break;
        }

        /**
         * 2. Generate 1024 bit RSA Key pair
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        optiga_key_id = OPTIGA_KEY_ID_E0FC;
        OPTIGA_CRYPT_SET_COMMS_PROTECTION_LEVEL(me, OPTIGA_COMMS_NO_PROTECTION);
        return_status = optiga_crypt_rsa_generate_keypair(me,
                                                          OPTIGA_RSA_KEY_1024_BIT_EXPONENTIAL,
                                                          (uint8_t)OPTIGA_KEY_USAGE_SIGN,
                                                          FALSE,
                                                          &optiga_key_id,
                                                          public_key,
                                                          &public_key_length);
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
            //Key pair generation failed
            return_status = optiga_lib_status;
            break;
        }

        /**
         * 3. Generate 48 byte RSA Pre master secret in acquired session OID
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_rsa_generate_pre_master_secret(me,
                                                                    optional_data,
                                                                    optional_data_length,
                                                                    48);

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_crypt_rsa_generate_pre_master_secret operation is completed
            pal_os_event_process();
        }

        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            return_status = optiga_lib_status;
            break;
        }

        optiga_lib_status = OPTIGA_LIB_BUSY;

        /**
         * 4. Encrypt (RSA) the data in session OID
         */

        // OPTIGA Comms Shielded connection settings to enable the protection
        // OPTIGA_CRYPT_SET_COMMS_PROTOCOL_VERSION(me, OPTIGA_COMMS_PROTOCOL_VERSION_PRE_SHARED_SECRET);

        encryption_scheme = OPTIGA_RSAES_PKCS1_V15;
        public_key_from_host.public_key = public_key;
        public_key_from_host.length = public_key_length;
        public_key_from_host.key_type = (uint8_t)OPTIGA_RSA_KEY_1024_BIT_EXPONENTIAL;
        // Default protection level for this API is OPTIGA_COMMS_COMMAND_PROTECTION
        return_status = optiga_crypt_rsa_encrypt_session(me,
                                                         encryption_scheme,
                                                         NULL,
                                                         0,
                                                         OPTIGA_CRYPT_HOST_DATA,
                                                         &public_key_from_host,
                                                         encrypted_message,
                                                         &encrypted_message_length);

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_crypt_rsa_encrypt_session operation is completed
            pal_os_event_process();
        }

        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            //RSA Encryption failed
            return_status = optiga_lib_status;
            break;
        }

        /**
         * Close the application on OPTIGA 
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
            //optiga_util_close_application failed
            return_status = optiga_lib_status;
            break;
        }

        return_status = OPTIGA_LIB_SUCCESS;

    } while (FALSE);
    OPTIGA_EXAMPLE_LOG_STATUS(return_status);
    
    if (me)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_crypt_destroy(me);
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
}

void example_optiga_util_update_count(void)
{
    /**
     * Initialize the counter object with a threshold value 0x0A
     */
    static uint8_t initial_counter_object_data [] =
    {
        //Initial counter value
        0x00, 0x00, 0x00, 0x00,
        //Threshold value
        0x00, 0x00, 0x00, 0x0A,
    };

     uint16_t optiga_counter_oid;
    uint8_t offset;

    optiga_lib_status_t return_status = 0;
    optiga_util_t * me = NULL;
    OPTIGA_EXAMPLE_LOG_MESSAGE(__FUNCTION__);

    do
    {
        /**
         * 1. Create OPTIGA Util Instance
         */
        me = optiga_util_create(0, optiga_util_callback, NULL);
        if (NULL == me)
        {
            break;
        }
    
        /**
         * Open the application on OPTIGA which is a precondition to perform any other operations
         * using optiga_util_open_application
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_open_application(me, 0);
    
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
            //optiga_util_open_application failed
            return_status = optiga_lib_status;
            break;
        }
        /**
         * Pre-condition
         * Any data object can be converted to counter data object by changing metadata as mentioned below:
         * - As precondition  write access should be always or LCSO < OPERATIONAL
         * - Write metadata as "0xD0, 0x01, 0x01" using #optiga_util_write_metadata
        */

        /**
         * Write default count and threshold value to counter data object (e.g. E120)
         * using optiga_util_write_data.
         *
         * Use Erase and Write (OPTIGA_UTIL_ERASE_AND_WRITE) option,
         * in order to correctly update the used length of the object.
         */
        optiga_counter_oid = 0xE120;
        offset = 0x00;
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_write_data(me,
                                               optiga_counter_oid,
                                               OPTIGA_UTIL_ERASE_AND_WRITE,
                                               offset,
                                               initial_counter_object_data,
                                               sizeof(initial_counter_object_data));

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

        // In this example, the counter is update by 5 and the final count would be 15
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_update_count(me,
                                                 optiga_counter_oid,
                                                 0x05);

        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_util_update_count operation is completed
            pal_os_event_process();
        }

        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            //Updating count of a counter data object failed.
            return_status = optiga_lib_status;
            break;
        }

        /**
         * Close the application on OPTIGA without hibernating
         * using optiga_util_close_application
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_close_application(me, 0);
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
            //optiga_util_close_application failed
            return_status = optiga_lib_status;
            break;
        }

        return_status = OPTIGA_LIB_SUCCESS;
    } while (FALSE);
    OPTIGA_EXAMPLE_LOG_STATUS(return_status);
    
    if (me)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_util_destroy(me);
        if(OPTIGA_LIB_SUCCESS != return_status)
        {
            //lint --e{774} suppress This is a generic macro
            OPTIGA_EXAMPLE_LOG_STATUS(return_status);
        }
    }
}

void example_optiga_util_protected_update(void)
{
    static uint8_t trust_anchor [] = 
    {
        0x30, 0x82, 0x02, 0x58, 0x30, 0x82, 0x01, 0xFF, 0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x01, 0x2F,
        0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x30, 0x56, 0x31, 0x0B,
        0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x49, 0x4E, 0x31, 0x0D, 0x30, 0x0B, 0x06,
        0x03, 0x55, 0x04, 0x0A, 0x0C, 0x04, 0x49, 0x46, 0x49, 0x4E, 0x31, 0x0C, 0x30, 0x0A, 0x06, 0x03,
        0x55, 0x04, 0x0B, 0x0C, 0x03, 0x43, 0x43, 0x53, 0x31, 0x13, 0x30, 0x11, 0x06, 0x03, 0x55, 0x04,
        0x03, 0x0C, 0x0A, 0x49, 0x6E, 0x74, 0x43, 0x41, 0x20, 0x50, 0x32, 0x35, 0x36, 0x31, 0x15, 0x30,
        0x13, 0x06, 0x03, 0x55, 0x04, 0x2E, 0x13, 0x0C, 0x54, 0x72, 0x75, 0x73, 0x74, 0x20, 0x41, 0x6E,
        0x63, 0x68, 0x6F, 0x72, 0x30, 0x1E, 0x17, 0x0D, 0x31, 0x36, 0x30, 0x35, 0x32, 0x36, 0x30, 0x38,
        0x30, 0x31, 0x33, 0x37, 0x5A, 0x17, 0x0D, 0x31, 0x37, 0x30, 0x36, 0x30, 0x35, 0x30, 0x38, 0x30,
        0x31, 0x33, 0x37, 0x5A, 0x30, 0x5A, 0x31, 0x0B, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13,
        0x02, 0x49, 0x4E, 0x31, 0x0D, 0x30, 0x0B, 0x06, 0x03, 0x55, 0x04, 0x0A, 0x0C, 0x04, 0x49, 0x46,
        0x49, 0x4E, 0x31, 0x0C, 0x30, 0x0A, 0x06, 0x03, 0x55, 0x04, 0x0B, 0x0C, 0x03, 0x43, 0x43, 0x53,
        0x31, 0x17, 0x30, 0x15, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x0E, 0x65, 0x6E, 0x64, 0x45, 0x6E,
        0x74, 0x69, 0x74, 0x79, 0x20, 0x50, 0x32, 0x35, 0x36, 0x31, 0x15, 0x30, 0x13, 0x06, 0x03, 0x55,
        0x04, 0x2E, 0x13, 0x0C, 0x54, 0x72, 0x75, 0x73, 0x74, 0x20, 0x41, 0x6E, 0x63, 0x68, 0x6F, 0x72,
        0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A,
        0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x19, 0xB5, 0xB2, 0x17, 0x0D,
        0xF5, 0x98, 0x5E, 0xD4, 0xD9, 0x72, 0x16, 0xEF, 0x61, 0x39, 0x3F, 0x14, 0x58, 0xAF, 0x5C, 0x02,
        0x78, 0x07, 0xCA, 0x48, 0x8F, 0x2A, 0xE3, 0x90, 0xB9, 0x03, 0xA1, 0xD2, 0x46, 0x20, 0x09, 0x21,
        0x52, 0x98, 0xDC, 0x8E, 0x88, 0x84, 0x67, 0x8E, 0x83, 0xD1, 0xDE, 0x0F, 0x1C, 0xE5, 0x19, 0x1D,
        0x0C, 0x74, 0x60, 0x41, 0x58, 0x5B, 0x36, 0x55, 0xF8, 0x3D, 0xAB, 0xA3, 0x81, 0xB9, 0x30, 0x81,
        0xB6, 0x30, 0x09, 0x06, 0x03, 0x55, 0x1D, 0x13, 0x04, 0x02, 0x30, 0x00, 0x30, 0x1D, 0x06, 0x03,
        0x55, 0x1D, 0x0E, 0x04, 0x16, 0x04, 0x14, 0xB5, 0x97, 0xFD, 0xAB, 0x36, 0x1A, 0xA0, 0xA2, 0x23,
        0xA7, 0x68, 0x25, 0x25, 0xFB, 0x82, 0x55, 0xD0, 0x4F, 0xCF, 0xB8, 0x30, 0x7A, 0x06, 0x03, 0x55,
        0x1D, 0x23, 0x04, 0x73, 0x30, 0x71, 0x80, 0x14, 0x1A, 0xBB, 0x56, 0x44, 0x65, 0x8C, 0x4D, 0x4F,
        0xCD, 0x29, 0xA2, 0x3F, 0x4C, 0xC6, 0xBC, 0xA8, 0x8B, 0xA4, 0x0A, 0xDA, 0xA1, 0x56, 0xA4, 0x54,
        0x30, 0x52, 0x31, 0x0B, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x49, 0x4E, 0x31,
        0x0D, 0x30, 0x0B, 0x06, 0x03, 0x55, 0x04, 0x0A, 0x0C, 0x04, 0x49, 0x46, 0x49, 0x4E, 0x31, 0x0C,
        0x30, 0x0A, 0x06, 0x03, 0x55, 0x04, 0x0B, 0x0C, 0x03, 0x43, 0x43, 0x53, 0x31, 0x0F, 0x30, 0x0D,
        0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x06, 0x52, 0x6F, 0x6F, 0x74, 0x43, 0x41, 0x31, 0x15, 0x30,
        0x13, 0x06, 0x03, 0x55, 0x04, 0x2E, 0x13, 0x0C, 0x54, 0x72, 0x75, 0x73, 0x74, 0x20, 0x41, 0x6E,
        0x63, 0x68, 0x6F, 0x72, 0x82, 0x01, 0x2E, 0x30, 0x0E, 0x06, 0x03, 0x55, 0x1D, 0x0F, 0x01, 0x01,
        0xFF, 0x04, 0x04, 0x03, 0x02, 0x00, 0x81, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D,
        0x04, 0x03, 0x02, 0x03, 0x47, 0x00, 0x30, 0x44, 0x02, 0x20, 0x68, 0xFD, 0x9C, 0x8F, 0x35, 0x33,
        0x0B, 0xB8, 0x32, 0x8C, 0xAF, 0x1C, 0x81, 0x4E, 0x41, 0x29, 0x26, 0xCB, 0xB7, 0x10, 0xA0, 0x75,
        0xFC, 0x89, 0xAE, 0xC5, 0x1D, 0x92, 0x8E, 0x72, 0xEF, 0x5C, 0x02, 0x20, 0x7D, 0xC1, 0xEB, 0x58,
        0x21, 0xF1, 0xFD, 0xFB, 0x5E, 0xD7, 0xDE, 0x06, 0xC9, 0xB4, 0xFF, 0x59, 0x8D, 0x37, 0x8C, 0x7A,
        0x48, 0xCD, 0x2D, 0x99, 0x74, 0x77, 0x58, 0x9D, 0x95, 0x51, 0x8F, 0x5D
    };

    /**
     * Manifest for ECC-256
     */
    static uint8_t manifest[] = 
    {
        // COSE Sign1 Trust
        0x84,
            // Protected signed header trust
            0x43,
                // Trust Sign Alogorithm
                0xA1, 
                // ECDSA
                0x01, 0x26, 
            // Unprotected signed header trust
            0xA1, 
                // Root of trust
                0x04, 0x42, 
                // Trust Anchor OID
                0xE0, 0xE3, 
            // Payload info Byte string of single byte length
            0x58, 
                // Byte string length of manifest            
                0x3D, 
                // Trust manifest, array of 6 entries
                0x86, 
                // Version(Major Type 0)
                0x01,
                // NULL
                0xF6,
                // NULL
                0xF6,
                // Resources, array of 4 entries
                0x84, 
                    // Trust Payload Type
                    0x20,
                    // 2 byte length
                    0x19,
                    // Payload Length
                    0x02, 0x92, 
                    // Trust Payload version
                    0x03, 
                    // Trust Add info data 
                    0x82, 
                        // Offset( Major Type 0)
                        0x00, 
                        // Trust add info write type
                        0x01, 
                    // Trust Processors, array of 2 entries 
                    0x82, 
                        // Processing step integrity, array of 2 entries
                        0x82, 
                            // Process( Major Type 1)
                            0x20, 
                            // Parameters, byte string with single byte length
                            0x58, 
                            // Byte string length 
                            0x25, 
                            // IFX Digest info, array of 2 entries
                            0x82, 
                            // Digest Algorithm
                            0x18,
                            // SHA-256
                            0x29, 
                            // Digest
                            0x58, 
                            // Byte string length
                            0x20, 
                            // Digest data
                            0xA0, 0xAE, 0xD2, 0x75, 0x75, 0xB8, 0x77, 0xED, 
                            0x0F, 0xEA, 0xB6, 0x3C, 0x74, 0x35, 0x58, 0xEA,
                            0xE3, 0xA2, 0x26, 0x4C, 0x8C, 0xEC, 0xD5, 0x8F,
                            0x8F, 0x4E, 0x12, 0xAD, 0xA0, 0xDB, 0x73, 0x9A, 
                        // NULL
                        0xF6, 
                    // Trsust Target
                    0x82, 
                        // Component identifier
                        0x40, 
                        // Storage identifier
                        0x42, 
                        // Optiga target OID
                        0xE0, 0xE1, 
            // Signature info, byte string of single byte length
            0x58, 
                // Byte string length for ECC 256
                0x40, 
                // Signature data
                0x8B, 0x87, 0xAE, 0x23, 0x11, 0x4D, 0x44, 0xC4, 0xE8, 0x93, 0xFA, 0x70, 0x99, 0xD0, 0x32, 0xFE,
                0x70, 0x9D, 0xF9, 0x7C, 0x81, 0x98, 0x05, 0x73, 0xA9, 0x61, 0x8A, 0x3D, 0xD7, 0xCE, 0x8B, 0xA4,
                0xC8, 0xC2, 0x70, 0x19, 0x8E, 0x74, 0xE8, 0x58, 0xDC, 0x22, 0x63, 0x9E, 0x38, 0x52, 0x8C, 0x7D,
                0x95, 0xE2, 0x5E, 0x28, 0xC7, 0x71, 0xED, 0xDF, 0xFE, 0x79, 0xC4, 0x62, 0x77, 0xB8, 0xC6, 0x5C                     

    };

    /**
     * Fragment array for continue with 608 bytes of payload and 32 bytes of digest of next fragment
     */
    static uint8_t continue_fragment_array[] = 
    {
        // Payload of 608 bytes
        0x21, 0x40, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
        0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,
        0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x3B,
        0x0D, 0x0A, 0x21, 0x40, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D,
        0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D,
        0x3E, 0x3F, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E,
        0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E,
        0x5F, 0x3B, 0x0D, 0x0A, 0x21, 0x40, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B,
        0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
        0x3C, 0x3D, 0x3E, 0x3F, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C,
        0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C,
        0x5D, 0x5E, 0x5F, 0x3B, 0x0D, 0x0A, 0x21, 0x40, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
        0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
        0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A,
        0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A,
        0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x3B, 0x0D, 0x0A, 0x21, 0x40, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
        0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
        0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
        0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x3B, 0x0D, 0x0A, 0x21, 0x40, 0x22, 0x23, 0x24, 0x25,
        0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
        0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56,
        0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x3B, 0x0D, 0x0A, 0x21, 0x40, 0x22, 0x23,
        0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33,
        0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x41, 0x42, 0x43, 0x44,
        0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54,
        0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x3B, 0x0D, 0x0A, 0x21, 0x40,
        0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31,
        0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x41, 0x42,
        0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52,
        0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x3B, 0x0D, 0x0A,
        0x21, 0x40, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
        0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,
        0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x3B,
        0x0D, 0x0A, 0x21, 0x40, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D,
        // Digest of final fragment data    
        0xDF, 0x34, 0x57, 0x42, 0xB8, 0xB8, 0xB3, 0xC1, 0x21, 0xCC, 0x4D, 0x06, 0x88, 0x6F, 0xFE, 0x09,
        0x1F, 0xBC, 0xA0, 0x07, 0xD5, 0xDA, 0x81, 0xB4, 0x2D, 0xDC, 0xE0, 0x50, 0xCE, 0xBA, 0x1E, 0x13
    };

    /**
    * Fragment array for final with 50 bytes of payload
    */
    static uint8_t final_fragment_array[] = 
    {
        // Payload of 50 bytes  
        0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D,
        0x3E, 0x3F, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E,
        0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E,
        0x5F, 0x3B    
    };

    /**
    * Sample metadata of 0xE0E1 to change access condition to integrity protected and reseting the version tag
    */
    static uint8_t E0E1_metadata[] = { 0x20, 0x09, 0xD0, 0x03, 0x21, 0xE0, 0xE3, 0xC1, 0x02, 0x00, 0x00 };

    /**
    * Sample metadata of 0xE0E3 for execute access condition
    */
    static uint8_t E0E3_metadata[] = { 0x20, 0x06, 0xD3, 0x01, 0x00, 0xe8, 0x01, 0x11 };
    
    optiga_lib_status_t return_status = 0;

    optiga_util_t * me = NULL;
    OPTIGA_EXAMPLE_LOG_MESSAGE(__FUNCTION__);

    do
    {
        /**
         *  Create OPTIGA util Instance
         *
         */
        me = optiga_util_create(0, optiga_util_callback, NULL);
        if (NULL == me)
        {
            break;
        }

        /**
         * Open the application on OPTIGA which is a precondition to perform any other operations
         * using optiga_util_open_application
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_open_application(me, 0);
    
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
            //optiga_util_open_application failed
            return_status = optiga_lib_status;
            break;
        }
        
        /**
        *  Precondition 1 : Write Metadata for 0xE0E1 and 0xE0E3
         */
        //return_status = write_metadata( me);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }

        /**
        *  Precondition 2 : Write ECC-256 certificate for 0xE0E3
         */
        //return_status = write_ecc_256_certificate( me);
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }
        /**
        *   Send the manifest using optiga_util_protected_update_start
        */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_protected_update_start(me,
                                                           0x01,
                                                           manifest,
                                                           sizeof(manifest));

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

        /**
        *   Send the first fragment using optiga_util_protected_update_continue
        */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_protected_update_continue( me,
                                                               continue_fragment_array,
                                                               sizeof(continue_fragment_array));

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

        /**
        *   Send the last fragment using optiga_util_protected_update_final
        */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_protected_update_final(me,
                                                           final_fragment_array,
                                                           sizeof(final_fragment_array));

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

        /**
         * Close the application on OPTIGA without hibernating
         * using optiga_util_close_application
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_close_application(me, 0);
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
            //optiga_util_close_application failed
            return_status = optiga_lib_status;
            break;
        }

        return_status = OPTIGA_LIB_SUCCESS;
    } while (FALSE);
    OPTIGA_EXAMPLE_LOG_STATUS(return_status);
    
    if (me)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_util_destroy(me);
        if(OPTIGA_LIB_SUCCESS != return_status)
        {
            //lint --e{774} suppress This is a generic macro
            OPTIGA_EXAMPLE_LOG_STATUS(return_status);
        }
    }
}

void example_optiga_util_read_uuid(void)
{
    uint16_t offset, bytes_to_read;
    uint16_t optiga_oid;
    uint8_t read_data_buffer[1024];
    optiga_lib_status_t return_status;
    optiga_util_t * me = NULL;
    uint8_t logging_status = 0;
    OPTIGA_EXAMPLE_LOG_MESSAGE(__FUNCTION__);

    do
    {
    	/**
		 * 1. Create OPTIGA Util Instance
		 */
		me = optiga_util_create(0, optiga_util_callback, NULL);
		if (NULL == me)
		{
			break;
		}

        /**
         * Open the application on OPTIGA which is a precondition to perform any other operations
         * using optiga_util_open_application
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_open_application(me, 0);
    
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
            //optiga_util_open_application failed
            return_status = optiga_lib_status;
            break;
        }

		//Read device end entity certificate from OPTIGA
		optiga_oid = 0xE0C2;
		offset = 0x00;
		bytes_to_read = sizeof(read_data_buffer);

		// OPTIGA Comms Shielded connection settings to enable the protection
		//OPTIGA_UTIL_SET_COMMS_PROTOCOL_VERSION(me, OPTIGA_COMMS_PROTOCOL_VERSION_PRE_SHARED_SECRET);
		//OPTIGA_UTIL_SET_COMMS_PROTECTION_LEVEL(me, OPTIGA_COMMS_RESPONSE_PROTECTION);

		/**
		 * 2. Read data from a data object (e.g. certificate data object)
		 *    using optiga_util_read_data.
		 */
		optiga_lib_status = OPTIGA_LIB_BUSY;
		return_status = optiga_util_read_data(me,
											  optiga_oid,
											  offset,
											  read_data_buffer,
											  &bytes_to_read);

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

		logging_status = 1;

		// #ifdef OPTIGA_MINI_SHELL
		        OPTIGA_EXAMPLE_LOG_MESSAGE("OPTIGA Trust Read Unique Identifier (UID):");
		        OPTIGA_EXAMPLE_LOG_HEX_DATA(read_data_buffer, bytes_to_read);
		        OPTIGA_EXAMPLE_LOG_MESSAGE("\n");

		        //OPTIGA_EXAMPLE_LOG_MESSAGE("Read device end entity certificate from OPTIGA:");
		        OPTIGA_EXAMPLE_LOG_MESSAGE("CIM Identifier:");
		        OPTIGA_EXAMPLE_LOG_HEX_DATA(&read_data_buffer[0], 1);

		        OPTIGA_EXAMPLE_LOG_MESSAGE("Platform Identifier:");
		        OPTIGA_EXAMPLE_LOG_HEX_DATA(&read_data_buffer[1], 1);

		        OPTIGA_EXAMPLE_LOG_MESSAGE("Model identifier:");
		        OPTIGA_EXAMPLE_LOG_HEX_DATA(&read_data_buffer[2], 1);

		        OPTIGA_EXAMPLE_LOG_MESSAGE("ID of ROM mask:");
		        OPTIGA_EXAMPLE_LOG_HEX_DATA(&read_data_buffer[3], 2);

		        OPTIGA_EXAMPLE_LOG_MESSAGE("Chip type:");
				OPTIGA_EXAMPLE_LOG_HEX_DATA(&read_data_buffer[5], 6);

				OPTIGA_EXAMPLE_LOG_MESSAGE("Batch number:");
				OPTIGA_EXAMPLE_LOG_HEX_DATA(&read_data_buffer[11], 6);

				OPTIGA_EXAMPLE_LOG_MESSAGE("Wafer Position:");
				OPTIGA_EXAMPLE_LOG_HEX_DATA(&read_data_buffer[17], 4);

				OPTIGA_EXAMPLE_LOG_MESSAGE("Firmware Identifier:");
				OPTIGA_EXAMPLE_LOG_HEX_DATA(&read_data_buffer[21], 4);

				OPTIGA_EXAMPLE_LOG_MESSAGE("ESW build number, BCD coded:");
				OPTIGA_EXAMPLE_LOG_HEX_DATA(&read_data_buffer[25], 2);
		// #endif

        /**
         * Close the application on OPTIGA without hibernating
         * using optiga_util_close_application
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_close_application(me, 0);
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
            //optiga_util_close_application failed
            return_status = optiga_lib_status;
            break;
        }

    }while(0);
    OPTIGA_EXAMPLE_LOG_STATUS(return_status);

    if (me)
	{
		//Destroy the instance after the completion of usecase if not required.
		return_status = optiga_util_destroy(me);
	}
	OPTIGA_EXAMPLE_LOG_STATUS(return_status);
}

void example_pair_host_and_optiga_using_pre_shared_secret(void)
{

}

void example_optiga_util_hibernate_restore(void)
{
    optiga_lib_status_t return_status;
    optiga_util_t * me = NULL;
    uint8_t logging_status = 0;
    uint8_t security_event_counter = 0;
    uint16_t bytes_to_read = 1;
    OPTIGA_EXAMPLE_LOG_MESSAGE(__FUNCTION__);
    
    do
    {
        //Create an instance of optiga_util to open the application on OPTIGA.
        me = optiga_util_create(0, optiga_util_callback, NULL);
    
        /**
         * Open the application on OPTIGA which is a precondition to perform any other operations
         * using optiga_util_open_application
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_open_application(me, 0);
    
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
            //optiga_util_open_application failed
            return_status = optiga_lib_status;
            break;
        }

                /**
         * To perform the hibernate, Security Event Counter(SEC) must be 0.
                 * Read SEC data object (0xE0C5) and wait until SEC = 0
         */
        do
        {
            optiga_lib_status = OPTIGA_LIB_BUSY;
            return_status = optiga_util_read_data(me,
                                                  0xE0C5,
                                                  0x00,
                                                  &security_event_counter,
                                                  &bytes_to_read);

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
            pal_os_timer_delay_in_milliseconds(1000);
        } while (0 != security_event_counter);
        OPTIGA_EXAMPLE_LOG_STATUS(return_status);
        /**
         * Hibernate the application on OPTIGA
         * using optiga_util_close_application with perform_hibernate parameter as true
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_close_application(me, 1);
    
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
            //optiga_util_close_application failed
            return_status = optiga_lib_status;
            break;
        }
    
        /**
         * Restore the application on OPTIGA
         * using optiga_util_open_application with perform_restore parameter as true
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_open_application(me, 1);
    
        if (OPTIGA_LIB_SUCCESS != return_status)
        {
            break;
        }
        while (OPTIGA_LIB_BUSY == optiga_lib_status)
        {
            //Wait until the optiga_util_open_application is completed
            pal_os_event_process();
        }
        if (OPTIGA_LIB_SUCCESS != optiga_lib_status)
        {
            //optiga_util_open_application failed
            return_status = optiga_lib_status;
            break;
        }
    
        /**
         * Close the application on OPTIGA without hibernating
         * using optiga_util_close_application
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_close_application(me, 0);
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
            //optiga_util_close_application failed
            return_status = optiga_lib_status;
            break;
        }
        logging_status = 1;
        
    
    } while (FALSE);
    OPTIGA_EXAMPLE_LOG_STATUS(return_status);
    
    if (me)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_util_destroy(me);
    }
    OPTIGA_EXAMPLE_LOG_STATUS(return_status);
}

