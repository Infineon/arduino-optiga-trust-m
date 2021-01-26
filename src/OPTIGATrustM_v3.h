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

/*
 * API Adaptions needed
 * 
 * Feature additions:
 * 1) Add feature for AES with all the mentioned key sizes and algotrithms - 128/192/256 (ECB, CBC, CBC-MAC, CMAC)
 * 2) Add feature for HMAC with the mentioned algorithms - SHA256/384/512
 * 3) Add feature for "Protected Data/key/metadata update (Integrity and/or confidentiality)"
 *    with the mentioned curve/algorithm - ECC NIST P256/384/521
                                           ECC Brainpool P256/384/512 r1
                                           RSA® 1024/2048
                                           Signature scheme as ECDSA FIPS 186-3/RSA SSA PKCS#1 v1.5 without hashing
 *
 * Algorithm support for existing features:
 * 1) Support 2 new curve/algorithm for ECC - ECC NIST P521, ECC Brainpool P256/384/512 r1
 * 2) Support 2 new curve/algorithm for ker derivation - TLS v1.2 PRF SHA 384/512; HKDF SHA-256/384/512
 * 3) Support new curve/algorithm for protected data(object), 
 *    update(integrity) - ECC NIST P521,
                          ECC Brainpool P256/384/512 r1
                          Signature scheme as ECDSA FIPS 186-3/RSA SSA PKCS#1 v1.5 without hashing
 */

#ifndef IFXOPTIGATRUSTM_V3_H_
#define IFXOPTIGATRUSTM_V3_H_

#include "optiga_trustm/optiga_lib_config.h"

#ifdef OPTIGA_TRUST_M_V3

#include <Arduino.h>
#include <Wire.h>
#include "optiga_trustm/optiga_lib_config.h"
#include "optiga_trustm/pal_i2c.h"
#include <string.h> // memcpy
#include "optiga_trustm/optiga_util.h"
#include "optiga_trustm/optiga_crypt.h"
#include "OPTIGATrustM.h"

#include "optiga_trustm/pal_os_event.h"
#include "optiga_trustm/pal_os_event_timer.h"
#include "optiga_trustm/pal_os_timer.h"
#include "optiga_trustm/optiga_lib_logger.h"
#include "optiga_trustm/pal_ifx_i2c_config.h"
#include "optiga_trustm/optiga_example.h"


/**
 * @defgroup ifx_optiga_library Infineon OPTIGA Trust M Command Library
 * @{
 * @ingroup ifx_optiga
 *
 * @brief Module for application-level commands for Infineon OPTIGA Trust M version 3.
 */
class IFX_OPTIGA_TrustM_V3 : public IFX_OPTIGA_TrustM
{
public:
    //constructor
    IFX_OPTIGA_TrustM_V3();

    //deconstructor
    ~IFX_OPTIGA_TrustM_V3();

    /**
     * This function generates an ECC NIST P521 public private keypair. You can store the private key internally or export it for your usage
     *
     * @param[out] publicKey        Pointer to the data array where the result public key should be stored.
     * @param[out] plen             Length of the public key
     * @param[in] privateKey_oid    an Object ID of a slot, where the newly generated key should be stored:
     *                              Use one of the following slots:
     *                              @ref eSESSION_ID_1
     *                              @ref eSESSION_ID_2 (Default)
     *                              @ref eSESSION_ID_3
     *                              @ref eSESSION_ID_4
     *                              @ref eDEVICE_PRIKEY_1
     *                              @ref eDEVICE_PRIKEY_2 
     *                              @ref eDEVICE_PRIKEY_3 
     *                              @ref eDEVICE_PRIKEY_4      
     * @param[out] privateKey       [Optional] Pointer to the data array where the result private key should be stored.
     * @param[out] prlen            [Optional] Length of the private key.
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t generateKeypairECCP521(uint8_t publicKey[], uint16_t& plen, uint8_t privateKey[], uint16_t& prlen) { return generateKeypairECC(publicKey, plen, privateKey, prlen, OPTIGA_ECC_CURVE_NIST_P_521); };
    int32_t generateKeypairECCP521(uint8_t publicKey[], uint16_t& plen, uint16_t privateKey_oid) { return generateKeypairECC(publicKey, plen, privateKey_oid, OPTIGA_ECC_CURVE_NIST_P_521); };
    
    /**
     * This function generates an ECC Brainpool P 256 r1 public private keypair. You can store the private key internally or export it for your usage
     * 
     * @param[out] publicKey        Pointer to the data array where the result public key should be stored.
     * @param[out] plen             Length of the public key
     * @param[in] privateKey_oid    an Object ID of a slot, where the newly generated key should be stored:
     *                              Use one of the following slots:
     *                              @ref eSESSION_ID_1
     *                              @ref eSESSION_ID_2 (Default)
     *                              @ref eSESSION_ID_3
     *                              @ref eSESSION_ID_4
     *                              @ref eDEVICE_PRIKEY_1
     *                              @ref eDEVICE_PRIKEY_2 
     *                              @ref eDEVICE_PRIKEY_3 
     *                              @ref eDEVICE_PRIKEY_4      
     * @param[out] privateKey       [Optional] Pointer to the data array where the result private key should be stored.
     * @param[out] prlen            [Optional] Length of the private key.
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t generateKeypairECCP256r1(uint8_t publicKey[], uint16_t& plen, uint8_t privateKey[], uint16_t& prlen) { return generateKeypairECC(publicKey, plen, privateKey, prlen, OPTIGA_ECC_CURVE_BRAIN_POOL_P_256R1); };
    int32_t generateKeypairECCP256r1(uint8_t publicKey[], uint16_t& plen, uint16_t privateKey_oid) { return generateKeypairECC(publicKey, plen, privateKey_oid, OPTIGA_ECC_CURVE_BRAIN_POOL_P_256R1); };

    /**
     * This function generates an ECC Brainpool P 384 r1 public private keypair. You can store the private key internally or export it for your usage
     * 
     * @param[out] publicKey        Pointer to the data array where the result public key should be stored.
     * @param[out] plen             Length of the public key
     * @param[in] privateKey_oid    an Object ID of a slot, where the newly generated key should be stored:
     *                              Use one of the following slots:
     *                              @ref eSESSION_ID_1
     *                              @ref eSESSION_ID_2 (Default)
     *                              @ref eSESSION_ID_3
     *                              @ref eSESSION_ID_4
     *                              @ref eDEVICE_PRIKEY_1
     *                              @ref eDEVICE_PRIKEY_2 
     *                              @ref eDEVICE_PRIKEY_3 
     *                              @ref eDEVICE_PRIKEY_4      
     * @param[out] privateKey       [Optional] Pointer to the data array where the result private key should be stored.
     * @param[out] prlen            [Optional] Length of the private key.
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t generateKeypairECCP384r1(uint8_t publicKey[], uint16_t& plen, uint8_t privateKey[], uint16_t& prlen) { return generateKeypairECC(publicKey, plen, privateKey, prlen, OPTIGA_ECC_CURVE_BRAIN_POOL_P_384R1); };
    int32_t generateKeypairECCP384r1(uint8_t publicKey[], uint16_t& plen, uint16_t privateKey_oid) { return generateKeypairECC(publicKey, plen, privateKey_oid, OPTIGA_ECC_CURVE_BRAIN_POOL_P_384R1); };
    
    /**
     * This function generates an ECC Brainpool P 512 r1 public private keypair. You can store the private key internally or export it for your usage
     * 
     * @param[out] publicKey        Pointer to the data array where the result public key should be stored.
     * @param[out] plen             Length of the public key
     * @param[in] privateKey_oid    an Object ID of a slot, where the newly generated key should be stored:
     *                              Use one of the following slots:
     *                              @ref eSESSION_ID_1
     *                              @ref eSESSION_ID_2 (Default)
     *                              @ref eSESSION_ID_3
     *                              @ref eSESSION_ID_4
     *                              @ref eDEVICE_PRIKEY_1
     *                              @ref eDEVICE_PRIKEY_2 
     *                              @ref eDEVICE_PRIKEY_3 
     *                              @ref eDEVICE_PRIKEY_4      
     * @param[out] privateKey       [Optional] Pointer to the data array where the result private key should be stored.
     * @param[out] prlen            [Optional] Length of the private key.
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t generateKeypairECCP512r1(uint8_t publicKey[], uint16_t& plen, uint8_t privateKey[], uint16_t& prlen) { return generateKeypairECC(publicKey, plen, privateKey, prlen, OPTIGA_ECC_CURVE_BRAIN_POOL_P_512R1); };
    int32_t generateKeypairECCP512r1(uint8_t publicKey[], uint16_t& plen, uint16_t privateKey_oid) { return generateKeypairECC(publicKey, plen, privateKey_oid, OPTIGA_ECC_CURVE_BRAIN_POOL_P_512R1); };

    /**
     * This function generates a symmetric key using AES. You can store the key in the optiga or export the key to the host.
     *
     * @param[in]       sym_key_type                            Key type of #optiga_symmetric_key_type_t.
     * @param[in,out]   symmetric_key                           Pointer to buffer of symmetric key.
     *                                                          - The size of the buffer must be sufficient enough to accommodate the key. 
     * @param[in]       oid                                     Object ID to story symmetric key
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t generateStoreSymmetricKeyAES(optiga_symmetric_key_type_t sym_key_type, optiga_key_id_t oid) 
                                        { return generateSymmetricKeyAES(sym_key_type, FALSE, (void *)&oid); };
    int32_t generateExportSymmetricKeyAES(optiga_symmetric_key_type_t sym_key_type, uint8_t symmetric_key[]) 
                                        { return generateSymmetricKeyAES(sym_key_type, TRUE, symmetric_key); };

    /**
     * This function generates a symmetric key using AES 128. You can store the key in the optiga or export the key to the host.
     *
     * @param[in]       sym_key_type                            Key type of #optiga_symmetric_key_type_t.
     * @param[in,out]   symmetric_key                           Pointer to buffer of symmetric key.
     *                                                          - The size of the buffer must be sufficient enough to accommodate the key. 
     * @param[in]       oid                                     Object ID to story symmetric key
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t generateStoreSymmetricKeyAES128(optiga_key_id_t oid) 
                                      { return generateStoreSymmetricKeyAES(OPTIGA_SYMMETRIC_AES_128, oid); };
    int32_t generateExportSymmetricKeyAES128(uint8_t symmetric_key[]) 
                                      { return generateExportSymmetricKeyAES(OPTIGA_SYMMETRIC_AES_128, symmetric_key); };

    /**
     * This function generates a symmetric key using AES 192. You can store the key in the optiga or export the key to the host.
     *
     * @param[in]       sym_key_type                            Key type of #optiga_symmetric_key_type_t.
     * @param[in,out]   symmetric_key                           Pointer to buffer of symmetric key.
     *                                                          - The size of the buffer must be sufficient enough to accommodate the key. 
     * @param[in]       oid                                     Object ID to story symmetric key
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
     int32_t generateStoreSymmetricKeyAES192(optiga_key_id_t oid)
                                       { return generateStoreSymmetricKeyAES(OPTIGA_SYMMETRIC_AES_192, oid); };
     int32_t generateExportSymmetricKeyAES192(uint8_t symmetric_key[])
                                       { return generateExportSymmetricKeyAES(OPTIGA_SYMMETRIC_AES_192, symmetric_key); };

    /**
     * This function generates a symmetric key using AES 256. You can store the key in the optiga or export the key to the host.
     *
     * @param[in]       sym_key_type                            Key type of #optiga_symmetric_key_type_t.
     * @param[in,out]   symmetric_key                           Pointer to buffer of symmetric key.
     *                                                          - The size of the buffer must be sufficient enough to accommodate the key. 
     * @param[in]       oid                                     Object ID to story symmetric key
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
     int32_t generateStoreSymmetricKeyAES256(optiga_key_id_t oid)
                                       { return generateStoreSymmetricKeyAES(OPTIGA_SYMMETRIC_AES_256, oid); };
     int32_t generateExportSymmetricKeyAES256(uint8_t symmetric_key[])
                                       { return generateExportSymmetricKeyAES(OPTIGA_SYMMETRIC_AES_256, symmetric_key); };


    /**
     * This function generates HMAC on the input data using secret key from the OPTIGA and exports the generated HMAC to the host. 
     *
     * @param[in]         type                                  HMAC type
     * @param[in]         secret                                OPTIGA OID with input secret
     *                                                          - Input secret must be available at the specified OID.<br> 
     *                                                          - To indicate session OID (already acquired by instance), specify #OPTIGA_KEY_ID_SESSION_BASED
     * @param[in]         input_data                            Pointer to input data for HMAC generation.
     * @param[in]         input_data_length                     Length of input data for HMAC generation.
     *                                                          - Must be greater than 0.<br>
     * @param[in,out]     mac                                   Pointer to buffer to store generated HMAC
     * @param[in,out]     mac_length                            Pointer to length of the <b>mac</b>. Initial value set as length of buffer, later updated as the actual length of generated HMAC.
     *                                                          - The size of the buffer must be sufficient enough to accommodate the HMAC.
     *                                                          - If the <b>mac_length</b> is lesser than the length of MAC received from OPTIGA, then first <b>mac_length</b> bytes are only returned.
     *                                                          - In case of any error, the value is set to 0.
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */

    int32_t generateHMAC(optiga_hmac_type_t type, uint16_t secret, const uint8_t * input_data, uint32_t input_data_length, uint8_t * mac, uint32_t * mac_length);
    int32_t generateHMACStart(optiga_hmac_type_t type, uint16_t secret, const uint8_t * input_data, uint32_t input_data_length);
    int32_t generateHMACUpdate(const uint8_t * input_data, uint32_t input_data_length);
    int32_t generateHMACFinalize(const uint8_t * input_data, uint32_t input_data_length, uint8_t * mac, uint32_t * mac_length);

    /**
     * This function generates HMAC on the input data using secret key from the OPTIGA with SHA-256. 
     *
     * @param[in]         type                                  HMAC type
     * @param[in]         secret                                OPTIGA OID with input secret
     *                                                          - Input secret must be available at the specified OID.<br> 
     *                                                          - To indicate session OID (already acquired by instance), specify #OPTIGA_KEY_ID_SESSION_BASED
     * @param[in]         input_data                            Pointer to input data for HMAC generation.
     * @param[in]         input_data_length                     Length of input data for HMAC generation.
     *                                                          - Must be greater than 0.<br>
     * @param[in,out]     mac                                   Pointer to buffer to store generated HMAC
     * @param[in,out]     mac_length                            Pointer to length of the <b>mac</b>. Initial value set as length of buffer, later updated as the actual length of generated HMAC.
     *                                                          - The size of the buffer must be sufficient enough to accommodate the HMAC.
     *                                                          - If the <b>mac_length</b> is lesser than the length of MAC received from OPTIGA, then first <b>mac_length</b> bytes are only returned.
     *                                                          - In case of any error, the value is set to 0.
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t generateHMACSHA256(optiga_key_id_t secret, const uint8_t * input_data, uint32_t input_data_length, uint8_t * mac, uint32_t * mac_length)
                              { return generateHMAC(OPTIGA_HMAC_SHA_256, secret, input_data, input_data_length, mac, mac_length); };
    int32_t generateHMACSHA256Start(optiga_key_id_t secret, const uint8_t * input_data, uint32_t input_data_length)
                              { return generateHMACStart(OPTIGA_HMAC_SHA_256, secret, input_data, input_data_length); };
    int32_t generateHMACSHA256Update(const uint8_t * input_data, uint32_t input_data_length)
                              { return generateHMACUpdate(input_data, input_data_length); };
    int32_t generateHMACSHA256Finalize(const uint8_t * input_data, uint32_t input_data_length, uint8_t * mac, uint32_t * mac_length)
                              { return generateHMACFinalize(input_data, input_data_length, mac, mac_length); };

    /**
     * This function generates HMAC on the input data using secret key from the OPTIGA with SHA-384.
     *
     * @param[in]         type                                  HMAC type
     * @param[in]         secret                                OPTIGA OID with input secret
     *                                                          - Input secret must be available at the specified OID.<br> 
     *                                                          - To indicate session OID (already acquired by instance), specify #OPTIGA_KEY_ID_SESSION_BASED
     * @param[in]         input_data                            Pointer to input data for HMAC generation.
     * @param[in]         input_data_length                     Length of input data for HMAC generation.
     *                                                          - Must be greater than 0.<br>
     * @param[in,out]     mac                                   Pointer to buffer to store generated HMAC
     * @param[in,out]     mac_length                            Pointer to length of the <b>mac</b>. Initial value set as length of buffer, later updated as the actual length of generated HMAC.
     *                                                          - The size of the buffer must be sufficient enough to accommodate the HMAC.
     *                                                          - If the <b>mac_length</b> is lesser than the length of MAC received from OPTIGA, then first <b>mac_length</b> bytes are only returned.
     *                                                          - In case of any error, the value is set to 0.
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t generateHMACSHA384(optiga_key_id_t secret, const uint8_t * input_data, uint32_t input_data_length, uint8_t * mac, uint32_t * mac_length)
                              { return generateHMAC(OPTIGA_HMAC_SHA_384, secret, input_data, input_data_length, mac, mac_length); };
    int32_t generateHMACSHA384(optiga_key_id_t secret, const uint8_t * input_data, uint32_t input_data_length)
                              { return generateHMACStart(OPTIGA_HMAC_SHA_384, secret, input_data, input_data_length); };
    int32_t generateHMACSHA384(const uint8_t * input_data, uint32_t input_data_length)
                              { return generateHMACUpdate(input_data, input_data_length); };
    int32_t generateHMACSHA384(const uint8_t * input_data, uint32_t input_data_length, uint8_t * mac, uint32_t * mac_length)
                              { return generateHMACFinalize(input_data, input_data_length, mac, mac_length); };

    /**
     * This function generates HMAC on the input data using secret key from the OPTIGA with SHA-512. 
     *
     * @param[in]         type                                  HMAC type
     * @param[in]         secret                                OPTIGA OID with input secret
     *                                                          - Input secret must be available at the specified OID.<br> 
     *                                                          - To indicate session OID (already acquired by instance), specify #OPTIGA_KEY_ID_SESSION_BASED
     * @param[in]         input_data                            Pointer to input data for HMAC generation.
     * @param[in]         input_data_length                     Length of input data for HMAC generation.
     *                                                          - Must be greater than 0.<br>
     * @param[in,out]     mac                                   Pointer to buffer to store generated HMAC
     * @param[in,out]     mac_length                            Pointer to length of the <b>mac</b>. Initial value set as length of buffer, later updated as the actual length of generated HMAC.
     *                                                          - The size of the buffer must be sufficient enough to accommodate the HMAC.
     *                                                          - If the <b>mac_length</b> is lesser than the length of MAC received from OPTIGA, then first <b>mac_length</b> bytes are only returned.
     *                                                          - In case of any error, the value is set to 0.
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t generateHMACSHA512(optiga_key_id_t secret, const uint8_t * input_data, uint32_t input_data_length, uint8_t * mac, uint32_t * mac_length)
                              { return generateHMAC(OPTIGA_HMAC_SHA_512, secret, input_data, input_data_length, mac, mac_length); };
    int32_t generateHMACSHA512(optiga_key_id_t secret, const uint8_t * input_data, uint32_t input_data_length)
                              { return generateHMACStart(OPTIGA_HMAC_SHA_512, secret, input_data, input_data_length); };
    int32_t generateHMACSHA512(const uint8_t * input_data, uint32_t input_data_length)
                              { return generateHMACUpdate(input_data, input_data_length); };
    int32_t generateHMACSHA512(const uint8_t * input_data, uint32_t input_data_length, uint8_t * mac, uint32_t * mac_length)
                              { return generateHMACFinalize(input_data, input_data_length, mac, mac_length); };
	
    private:

        bool v3_active;

        int32_t generateSymmetricKeyAES(optiga_symmetric_key_type_t sym_key_type, bool_t export_symmetric_key, void * symmetricKey_oid);
        int32_t generateSymmetricKeyAES128(bool_t export_symmetric_key, void * symmetricKey_oid)
                                            { return generateSymmetricKeyAES(OPTIGA_SYMMETRIC_AES_128, export_symmetric_key, symmetricKey_oid); };
        int32_t generateSymmetricKeyAES192(bool_t export_symmetric_key, void * symmetricKey_oid)
                                            { return generateSymmetricKeyAES(OPTIGA_SYMMETRIC_AES_192, export_symmetric_key, symmetricKey_oid); };
        int32_t generateSymmetricKeyAES256(bool_t export_symmetric_key, void * symmetricKey_oid)
                                            { return generateSymmetricKeyAES(OPTIGA_SYMMETRIC_AES_256, export_symmetric_key, symmetricKey_oid); };
};
/**
 * @}
 */

//Preinstantiated object
extern IFX_OPTIGA_TrustM_V3 trustM_V3;

/*************************************************************************

 *  Inline functions 

 *************************************************************************/
                         
#endif /* OPTIGA_TRUST_M_V3 */
#endif /* IFXOPTIGATRUSTM_H_ */
