/*
 * MIT License
 *
 * Copyright (c) 201 Infineon Technologies AG
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

#ifndef IFXOPTIGATRUSTM_H_
#define IFXOPTIGATRUSTM_H_

#include <Arduino.h>
#include <Wire.h>
// #include "optiga_trustx/ifx_i2c_transport_layer.h"
// #include "optiga_trustx/pal_ifx_i2c_config.h"
#include <string.h> // memcpy
#include "optiga_trustm/optiga_util.h"
#include "optiga_trustm/optiga_crypt.h"

// #include "optiga_trustx/ErrorCodes.h"
// #include "optiga_trustx/AuthLibSettings.h"
// #include "optiga_trustx/BaseErrorCodes.h"
// #include "optiga_trustx/Util.h"
// #include "optiga_trustx/Version.h"

/*************************************************************************

 *  fundamental typedefs

 *************************************************************************/

/**
 * \brief  Typedef for OIDs
 */
typedef enum eOID_d {
    /// Global Life Cycle State
    eLCS_G = 0xE0C0,
    /// Global Security Status
    eSECURITY_STATUS_G = 0xE0C1,
    /// Coprocessor UID
    eCOPROCESSOR_UID = 0xE0C2,
    /// Global Life Cycle State
    eSLEEP_MODE_ACTIVATION_DELAY = 0xE0C3,
    /// Current limitation
    eCURRENT_LIMITATION = 0xE0C4,
    /// Security Event Counter
    eSECURITY_EVENT_COUNTER = 0xE0C5,
    /// Device Public Key Certificate issued by IFX
    eDEVICE_PUBKEY_CERT_IFX = 0xE0E0,
    /// Project-Specific device Public Key Certificate
    eDEVICE_PUBKEY_CERT_PRJSPC_1 = 0xE0E1,
    /// Project-Specific device Public Key Certificate
    eDEVICE_PUBKEY_CERT_PRJSPC_2 = 0xE0E2,
    /// Project-Specific device Public Key Certificate
    eDEVICE_PUBKEY_CERT_PRJSPC_3 = 0xE0E3,
    /// First Device Private Key
    eFIRST_DEVICE_PRIKEY_1 = 0xE0F0,
    /// First Device Private Key
    eFIRST_DEVICE_PRIKEY_2 = 0xE0F1,
    /// First Device Private Key
    eFIRST_DEVICE_PRIKEY_3 = 0xE0F2,
    /// First Device Private Key
    eFIRST_DEVICE_PRIKEY_4 = 0xE0F3,
    /// Application Life Cycle Status
    eLCS_A = 0xF1C0,
    /// Application Security Status
    eSECURITY_STATUS_A = 0xF1C1,
    /// Error codes
    eERROR_CODES = 0xF1C2
} eOID_d;

/**
 * \brief  Typedef for OIDs
 */
typedef enum eSessionCtxId_d {
    ///Session context id 1
    eSESSION_ID_1 = 0xE100,
    ///Session context id 2
    eSESSION_ID_2 = 0xE101,
    ///Session context id 3
    eSESSION_ID_3 = 0xE102,
    ///Session context id 4
    eSESSION_ID_4 = 0xE103,
} eSessionCtxId_d;



/**
 * @defgroup ifx_optiga_library Infineon OPTIGA Trust M Command Library
 * @{
 * @ingroup ifx_optiga
 *
 * @brief Module for application-level commands for Infineon OPTIGA Trust M.
 */
class IFX_OPTIGA_TrustM
{
public:
    //constructor
    IFX_OPTIGA_TrustM();

    //deconstructor
    ~IFX_OPTIGA_TrustM();

    /**
     *
     * This function initializes the Infineon OPTIGA Trust X command library and
     * sends the 'open application' command to the device. This opens the communicatino
     * channel to the Optiga Trust X, so that you can carry out different operations
     *
     * @retval  0    If function was successful.
     * @retval  1    If the operation failed.
     */
    int32_t begin(void);

    /**
     *
     * This function initializes the Infineon OPTIGA Trust M command library and
     * sends the 'open application' command to the device. This opens the communicatino
     * channel to the Optiga Trust X, so that you can carry out different operations
     *
     * @param[in]  CustomWire       Reference to a custom TwoWire object used with the Optiga.
     *
     * @retval  0  If function was successful.
     * @retval  1  If the operation failed.
     */
    int32_t begin(TwoWire& CustomWire);

	
	int32_t checkChip(void);
	
    /**
     *
     * This function resets the Infineon OPTIGA Trust M. This helps to recover the connection
     * to the optiga once it got lost. (Indicator: 1 is returned by any other function)
     *
     * @retval  0  If function was successful.
     * @retval  1  If the operation failed.
     */
    int32_t reset(void);

    /**
     *
     * @brief  Ends communication with the Optiga Trust M.
     *
     * @retval  0  If function was successful.
     * @retval  1  If the operation failed.
     */
    void end(void);

    /**
     * @brief Get the Infineon OPTIGA Trust M device certificate.
     *
     * The function retrieves the public X.509 certificate stored in the
     * Infineon OPTIGA Trust M device.
     * This certificate and the contained public key can be used to verify a signature from the device.
     * In addition, the receiver of the certificate can verify the chain of trust
     * by validating the issuer of the certificate and the issuer's signature on it.
     *
     * @param[out] certificate        Pointer to the buffer that will contain the output.
     * @param[out] certificateLength  Pointer to the variable that will contain the length.
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t getCertificate(uint8_t certificate[], uint16_t& certificateLength);
	
	/**
	 * @brief Get the Infineon OPTIGA Trust M device certificate public key.
	 *
	 * The function retrieves the public X.509 certificate stored in the
	 * Infineon OPTIGA Trust M device and extracts the public key from it.
	 * Work for Certificates based on NIST P256 curve
	 *
	 * @param[out] publickey  	 Pointer to the buffer where the public key will be stored. 
	 *                           Should 68 bytes long. 64 bytes for the key and 4 bytes for the encoding
	 *                           BitString Format (0x03, 0x42, 0x00) + Compression format (0x04) + Public Key (64 bytes)
	 *
	 * @retval  0 If the function was successful.
	 * @retval  1 If the operation failed.
	 */
    int32_t getPublicKey(uint8_t publickey[68]);
    
	/**
     * This function returns the Coprocessor UID value. Length is 27, where
     * First 25 bytes is the unique hardware identifier
     * Last 2 bytes is the Embedded Software Build Number BCD Coded
     *
     * @param[out] uniqueID      Pointer where the value will be stored
     * @param[in]  ulen          Pointer where the length of the value is stored
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t getUniqueID(uint8_t uniqueID[], uint16_t& uidLength) { return uidLength != 0?getGenericData(eCOPROCESSOR_UID, uniqueID, uidLength):1; }

    /**
     * @brief Get a random number.
     *
     * The function retrieves a cryptographic-quality random number
     * from the OPTIGA device. This function can be used as entropy
     * source for various security schemes.
     *
     * @param[in]  length           Length of the random number (range 8 to 256).
     * @param[out] random           Buffer to store the data.
     *
     * @retval  0  If function was successful.
     * @retval  1    If the operation failed.
     */
    int32_t getRandom(uint16_t length, uint8_t random[]);

    /**
     * This function returns the current limitation, which holds the maximum value of current allowed to be consumed by the OPTIGA™
     *  Trust M across all operating conditions.
     *
     *  Default value 0x06
     *
     * @param[out] currentLim       Reference where the value will be stored
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    // int32_t getCurrentLimit(uint8_t& currentLim) { return getState(eCURRENT_LIMITATION, currentLim); }
    
    /**
     * This function sets the sleep mode activation delay. Valid values are 0x06 - 0x0F or 6 mA - 15mA
     *
     * @param[in] currentLim        The value that will be set
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t setCurrentLimit(uint8_t currentLim) { return setGenericData(eCURRENT_LIMITATION, &currentLim, 1); }
	
    /**
     * This function returns the last error code.
     *
     * @param[out] errorCodes       Pointer where the value will be stored
     * @param[out] errorCodesLength Pointer where the length of the value is stored
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    // int32_t getLastErrorCodes(uint8_t errorCodes[], uint16_t& errorCodesLength) { return errorCodesLength != 0?getGenericData(eERROR_CODES, errorCodes, errorCodesLength):1; }

    /**
     * This function calculates SHA256 hash of the given data.
     *
     * @param[in] dataToHash        Pointer to the data
     * @param[in] dlen              Length of the input data
     * @param[out] p_out            Pointer to the data array where the final result should be stored. Must be defined.
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t sha256(uint8_t dataToHash[], uint16_t dlen, uint8_t hash[32]);
    
    /**
     * This function generates an ECDSA FIPS 186-3 w/o hash signature.
     *
     * @param[in] dataToSign        Pointer to the data
     * @param[in] dlen              Length of the input data
     * @param[in] privateKey_oid    [Optional] Object ID defines which private key slot will be used to generate the signature. Default is the first slot.
     *                              Use either one of:
     *                              @ref eFIRST_DEVICE_PRIKEY_1 (Default)
     *                              @ref eFIRST_DEVICE_PRIKEY_2 
     *                              @ref eFIRST_DEVICE_PRIKEY_3 
     *                              @ref eFIRST_DEVICE_PRIKEY_4 
     *                              slots define below or @ref eSessionCtxId_d session contexts
     * @param[out] result           Pointer to the data array where the final result should be stored.
     * @param[out] rlen             Length of the output data. Will be modified in case of success.
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t calculateSignature(uint8_t dataToSign[], uint16_t dlen, uint16_t privateKey_oid, uint8_t result[], uint16_t& rlen);
    int32_t calculateSignature(uint8_t dataToSign[], uint16_t dlen, uint8_t result[], uint16_t& rlen) { 
        return calculateSignature(dataToSign, dlen, eFIRST_DEVICE_PRIKEY_1, result, rlen);
	}
    
    /**
     * This function encodes generated signature in ASN.1 format
     *
     * @param[in]  signature        Pointer to signature in DER format
     * @param[in]  signatureLength  Length of the input data
     * @param[out] result           Pointer to the data array where the final result should be stored.
     * @param[out] rlen             Length of the output data. Will be modified in case of success.
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t formatSignature(uint8_t signature[], uint16_t signatureLength, uint8_t result[], uint16_t& rlen);
    
    /**
     * This function verifies an ECDSA FIPS 186-3 w/o hash signature.
     * This functions works in two modes, either use internal OID where a public key is stored
     * or you can give your own public key as an input
     *
     * @param[in] hash              Pointer to the hash
     * @param[in] hashLength        Length of the input data
     * @param[in] publicKey_oid     [Optional] Object ID defines which slot will be used to verify the signature. 
     *                              The slot should contain a public key certificate starting with internat 0xC0 byte. 
     *                              For more information please refere to the datasheet documents. Default is the first slot.
     *                              Possible values are:
     *                              @ref eDEVICE_PUBKEY_CERT_IFX (Default)
     *                              @ref eDEVICE_PUBKEY_CERT_PRJSPC_1
     *                              @ref eDEVICE_PUBKEY_CERT_PRJSPC_2
     *                              @ref eDEVICE_PUBKEY_CERT_PRJSPC_3
     * @param[in] signature         Pointer to the data array where the final result should be stored.
     * @param[in] signatureLength   Length of the output data. Will be modified in case of success.
     * @param[in] pubKey            A pointer to the public key to be used for the verification
     * @param[in] plen              Length of the public key to be used for the verification
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t verifySignature(uint8_t hash[], uint16_t hashLength, uint8_t signature[], uint16_t signatureLength, uint16_t publicKey_oid);
    int32_t verifySignature(uint8_t hash[], uint16_t hashLength, uint8_t signature[], uint16_t signatureLength ) {
		verifySignature(hash, hashLength, signature, signatureLength, eDEVICE_PUBKEY_CERT_IFX);
	}
    int32_t verifySignature(uint8_t hash[], uint16_t hashLength, uint8_t signature[], uint16_t signatureLength, uint8_t pubKey[], uint16_t plen);
    
    /**
     * This function generates a shared secret based on Elliptic Curve Diffie-Hellman Key Exchange Algorithm
     * This functions works in several modes. In general for such functions you need to specify following:
     * elliptic curve type, private key, public key, result shared secret. Different functions listed below
     * assume you don't need various parts of this input as you use internally stored values
     * #1 sharedSecret(p_pubkey) - Private Key is taken from the first private keys slot. NISTP256 Curve is used
     * #2 sharedSecret(priv_oid, p_pubkey) - Works like #1, but you can specifiy which slot to use.
     * #3 sharedSecret(priv_oid, ssec_oid, p_pubkey) - Works like #1, but you can specifiy which slot to use for the private key and for the shared secret.
     * #4 sharedSecret(curve_type, p_pubkey) - Works like #1, but you can define a curve type: "secp256r1" or "secp384r1"
     * #5 sharedSecret(curve_type, priv_oid, p_pubkey) - Works like #2, but you can define a curve type: "secp256r1" or "secp384r1"
     * #6 sharedSecretWithExport(p_pubkey, p_out) - Works like #1, but exports the result in p_out
     * #7 sharedSecretWithExport(curve_type, p_pubkey, p_out) - Works like #5, but additionally you can define curve type of the publick key
     *
     * This Shared secret can be used until the Session Context will be flashed, either after an application restart or a reset
     * @param[in] curveName         Curve name. The following are supported:
     *                              "secp256r1" (Deafult)
     *                              "secp384r1"
     * @param[in] oid               Object ID defines which slot will be used as input and output
     *                              Use one of the following slots:
     *                              @ref eSESSION_ID_1
     *                              @ref eSESSION_ID_2 (Default)
     *                              @ref eSESSION_ID_3
     *                              @ref eSESSION_ID_4
     * @param[in] publicKey         A pointer to a public key
     * @param[in] plen              Length of a public key
     * @param[out]sharedSecret      Pointer to the data array where the final result should be stored.
     * @param[in] shlen             Length of the output data. Will be modified in case of success.

     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    // int32_t sharedSecret(uint8_t publicKey[], uint16_t plen) { 
	// 	return calculateSharedSecretGeneric(0x03, eSESSION_ID_2, publicKey, plen, eSESSION_ID_2);
	// }
    // int32_t sharedSecret(uint16_t oid, uint8_t publicKey[], uint16_t plen) {
	// 	return calculateSharedSecretGeneric(0x03, oid, publicKey, plen, oid);
	// }
    // int32_t sharedSecret(uint16_t in_oid, uint16_t out_oid, uint8_t publicKey[], uint16_t plen) {
	// 	return calculateSharedSecretGeneric(0x03, in_oid, publicKey, plen, out_oid);
	// }
    // int32_t sharedSecret(String curveName, uint8_t publicKey[], uint16_t plen) {
	// 	return calculateSharedSecretGeneric(str2cur(curveName),eSESSION_ID_2, publicKey, plen, eSESSION_ID_2);
	// }
    // int32_t sharedSecret(String curveName, uint16_t oid, uint8_t publicKey[], uint16_t plen) { 
	// 	return calculateSharedSecretGeneric(str2cur(curveName),oid, publicKey, plen, oid);
	// }
    // int32_t sharedSecretWithExport(uint8_t publicKey[], uint16_t plen, uint8_t sharedSecret[], uint16_t shlen) {
	// 	return calculateSharedSecretGeneric(0x03, eSESSION_ID_2, publicKey, plen, 0x0000, sharedSecret, shlen);
	// }
    // int32_t sharedSecretWithExport(String curveName, uint8_t publicKey[], uint16_t plen, uint8_t sharedSecret[], uint16_t shlen) {
	// 	return calculateSharedSecretGeneric(str2cur(curveName), eSESSION_ID_2, publicKey, plen, 0x0000, sharedSecret, shlen);
	// }  
    
    /**
     * This function generates a public private keypair. You can store the private key internally or export it for your usage
     *
     * @param[out] publicKey        Pointer to the data array where the result public key should be stored.
     * @param[out] plen             Length of the public key
     * @param[in] privateKey_oid    an Object ID of a slot, where the newly generated key should be stored:
     *                              Use one of the following slots:
     *                              @ref eSESSION_ID_1
     *                              @ref eSESSION_ID_2 (Default)
     *                              @ref eSESSION_ID_3
     *                              @ref eSESSION_ID_4
     *                              @ref eFIRST_DEVICE_PRIKEY_1
     *                              @ref eFIRST_DEVICE_PRIKEY_2 
     *                              @ref eFIRST_DEVICE_PRIKEY_3 
     *                              @ref eFIRST_DEVICE_PRIKEY_4      
     * @param[out] privateKey       [Optional] Pointer to the data array where the result private key should be stored.
     * @param[out] prlen            [Optional] Length of the private key.

     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
	int32_t generateKeypair(uint8_t publicKey[], uint16_t& plen ) { return generateKeypair(publicKey, plen, 0); }
    int32_t generateKeypair(uint8_t publicKey[], uint16_t& plen, uint16_t privateKey_oid);
    int32_t generateKeypair(uint8_t publicKey[], uint16_t& plen, uint8_t privateKey[], uint16_t& prlen);

private:
    uint8_t return_value = 0;
    optiga_util_t * me_util;


	// bool active;
    // int32_t getGlobalSecurityStatus(uint8_t& status);
    // int32_t setGlobalSecurityStatus(uint8_t status);
    // int32_t getAppSecurityStatus(uint8_t* p_data, uint16_t& hashLength);
    // int32_t setAppSecurityStatus(uint8_t status);
    int32_t getGenericData(uint16_t oid, uint8_t* p_data, uint16_t& hashLength);
    int32_t getState(uint16_t oid, uint8_t& p_data);
    int32_t setGenericData(uint16_t oid, uint8_t* p_data, uint16_t hashLength);
    // int32_t str2cur(String curve_name);
	// int32_t calculateSharedSecretGeneric( int32_t curveID, uint16_t priv_oid, uint8_t* p_pubkey, uint16_t plen, uint16_t out_oid) {
	// 	uint16_t dummy_len; 
	// 	return calculateSharedSecretGeneric(0x03, priv_oid, p_pubkey, plen, out_oid, NULL, dummy_len);
	// }
    // int32_t calculateSharedSecretGeneric( int32_t curveID, uint16_t priv_oid, uint8_t* p_pubkey, uint16_t plen, uint16_t out_oid, uint8_t* p_out, uint16_t& olen);
    // int32_t ecp_gen_keypair_generic(uint8_t* p_pubkey, uint16_t& plen, uint16_t& ctx, uint8_t* p_privkey, uint16_t& prlen);
    // int32_t deriveKey(uint8_t hash[], uint16_t hashLength, uint8_t publicKey[], uint16_t plen);
};
/**
 * @}
 */

//Preinstantiated object
extern IFX_OPTIGA_TrustM trustM;

/*************************************************************************

 *  Inline functions 

 *************************************************************************/
                         

#endif /* IFXOPTIGATRUST_H_ */
