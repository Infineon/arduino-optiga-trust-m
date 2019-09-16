

#include "optiga_util.h"
#include "optiga_crypt.h"

#include "pal_os_event.h"
#include "pal_os_event_timer.h"
#include "pal_os_timer.h"
#include "optiga_lib_logger.h"
#include "optiga_example.h"

// extern void pal_os_event_process(void);

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

void example_optiga_crypt_hash (void)
{

}

void example_optiga_crypt_ecc_generate_keypair(void)
{

}

void example_optiga_crypt_ecdsa_sign(void)
{

}

void example_optiga_crypt_ecdsa_verify(void)
{

}

void example_optiga_crypt_ecdh(void)
{

}

void example_optiga_crypt_random(void)
{

}

void example_optiga_crypt_tls_prf_sha256(void)
{

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

}

void example_optiga_crypt_rsa_generate_keypair(void)
{

}

void example_optiga_crypt_rsa_sign(void)
{

}

void example_optiga_crypt_rsa_verify(void)
{

}

void example_optiga_crypt_rsa_decrypt_and_export(void)
{

}

void example_optiga_crypt_rsa_decrypt_and_store(void)
{

}

void example_optiga_crypt_rsa_encrypt_message(void)
{

}

void example_optiga_crypt_rsa_encrypt_session(void)
{

}

void example_optiga_util_update_count(void)
{

}

void example_optiga_util_protected_update(void)
{

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
		OPTIGA_UTIL_SET_COMMS_PROTOCOL_VERSION(me, OPTIGA_COMMS_PROTOCOL_VERSION_PRE_SHARED_SECRET);
		OPTIGA_UTIL_SET_COMMS_PROTECTION_LEVEL(me, OPTIGA_COMMS_RESPONSE_PROTECTION);

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
                break;
            }
            pal_os_timer_delay_in_milliseconds(1000);
        } while (0 != security_event_counter);

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
            break;
        }
        logging_status = 1;
    
    } while (FALSE);
    
    if (me)
    {
        //Destroy the instance after the completion of usecase if not required.
        return_status = optiga_util_destroy(me);
    }
    OPTIGA_EXAMPLE_LOG_STATUS(logging_status);
}

