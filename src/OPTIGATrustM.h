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

#ifndef IFXOPTIGATRUSTM_H_
#define IFXOPTIGATRUSTM_H_

#include <Arduino.h>
#include <Wire.h>
#include "optiga_trustm/optiga_lib_config.h"
#include "optiga_trustm/pal_i2c.h"
#include <string.h> // memcpy
#include "optiga_trustm/optiga_util.h"
#include "optiga_trustm/optiga_crypt.h"

#include "optiga_trustm/pal_os_event.h"
#include "optiga_trustm/pal_os_event_timer.h"
#include "optiga_trustm/pal_os_timer.h"
#include "optiga_trustm/optiga_lib_logger.h"
#include "optiga_trustm/pal_ifx_i2c_config.h"
#include "optiga_trustm/optiga_example.h"


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
    eDEVICE_PRIKEY_1 = 0xE0F0,
    /// Second Device Private Key
    eDEVICE_PRIKEY_2 = 0xE0F1,
    /// Third Device Private Key
    eDEVICE_PRIKEY_3 = 0xE0F2,
    /// Fourth Device Private Key
    eDEVICE_PRIKEY_4 = 0xE0F3,
	/// First RSA Device Private Key
    eRSA_DEVICE_PRIKEY_1 = 0xE0FC,
	/// Second RSA Device Private Key
    eRSA_DEVICE_PRIKEY_2 = 0xE0FD,
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
     * channel to the Optiga Trust M, so that you can carry out different operations
	 * 
	 * @param[in]  pairDevice       Define whether you want to pair MCU and OPTIGA Trust M devices.
     *
     * @retval  0    If function was successful.
     * @retval  1    If the operation failed.
     */
    int32_t begin(uint8_t pairDevice=1);

    /**
     *
     * This function initializes the Infineon OPTIGA Trust M command library and
     * sends the 'open application' command to the device. This opens the communicatino
     * channel to the Optiga Trust M, so that you can carry out different operations
     *
     * @param[in]  CustomWire       Reference to a custom TwoWire object used with the Optiga.
     *
     * @retval  0  If function was successful.
     * @retval  1  If the operation failed.
     */
    int32_t begin(TwoWire& CustomWire);

	
    /**
     *
     * This function checks whether the chip posseses the genuine certificate
     *
     * @retval  0  If function was successful.
     * @retval  1  If the operation failed.
     */
	int32_t checkChip(void);
	
	/**
     *
     * This function pairs the Host MCU and the OPTIGA Trust M
	 * This function by default works only with samples which have 
	 * Pres-shared secret data object unlocked for modification
	 * YOu can though pair devices and then lock this dataobject afterwards.
     *
     * @retval  0  If function was successful.
     * @retval  1  If the operation failed.
     */
	int32_t pairMCU(void);
	
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
    int32_t getUniqueID(uint8_t uniqueID[], uint16_t& uidLength) { return uidLength != 0? getGenericData(eCOPROCESSOR_UID, uniqueID, uidLength):1; }

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
    int32_t getCurrentLimit(uint8_t& currentLim) { return getState(eCURRENT_LIMITATION, currentLim); }
    
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
    int32_t getLastErrorCodes(uint8_t errorCodes[], uint16_t& errorCodesLength) { return errorCodesLength != 0?getGenericData(eERROR_CODES, errorCodes, errorCodesLength):1; }

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
     * This function generates an ECDSA signature.
     *
     * @param[in] dataToSign        Pointer to the data
     * @param[in] dlen              Length of the input data
     * @param[in] privateKey_oid    [Optional] Object ID defines which private key slot will be used to generate the signature. Default is the first slot.
     *                              Use either one of:
     *                              @ref eDEVICE_PRIKEY_1 (Default)
     *                              @ref eDEVICE_PRIKEY_2 
     *                              @ref eDEVICE_PRIKEY_3 
     *                              @ref eDEVICE_PRIKEY_4 
     *                              slots define below or @ref eSessionCtxId_d session contexts
     * @param[out] result           Pointer to the data array where the final result should be stored.
     * @param[out] rlen             Length of the output data. Will be modified in case of success.
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t calculateSignature(uint8_t dataToSign[], uint16_t dlen, uint16_t privateKey_oid, uint8_t result[], uint16_t& rlen)
    {   return(calculateSignatureECDSA(dataToSign, dlen, privateKey_oid, result, rlen));};
    int32_t calculateSignature(uint8_t dataToSign[], uint16_t dlen, uint8_t result[], uint16_t& rlen) { 
        return calculateSignatureECDSA(dataToSign, dlen, OPTIGA_KEY_ID_E0F0, result, rlen);
	}
    
    /**
     * This function generates an RSA signature.
     *
     * @param[in] dataToSign        Pointer to the data
     * @param[in] dlen              Length of the input data
     * @param[in] privateKey_oid    [Optional] Object ID defines which private key slot will be used to generate the signature. Default is the first slot.
     *                              Use either one of:
     *                              @ref eDEVICE_PRIKEY_1 (Default)
     *                              @ref eDEVICE_PRIKEY_2 
     *                              @ref eDEVICE_PRIKEY_3 
     *                              @ref eDEVICE_PRIKEY_4 
     *                              slots define below or @ref eSessionCtxId_d session contexts
     * @param[out] result           Pointer to the data array where the final result should be stored.
     * @param[out] rlen             Length of the output data. Will be modified in case of success.
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t calculateSignatureRSA(uint8_t dataToSign[], uint16_t dlen, uint16_t privateKey_oid, uint8_t result[], uint16_t& rlen);
    int32_t calculateSignatureRSA(uint8_t dataToSign[], uint16_t dlen, uint8_t result[], uint16_t& rlen) { 
        return calculateSignatureRSA(dataToSign, dlen, eRSA_DEVICE_PRIKEY_1, result, rlen);
	}
	    
    /**
     * This function generates an ECDSA signature.
     *
     * @param[in] dataToSign        Pointer to the data
     * @param[in] dlen              Length of the input data
     * @param[in] privateKey_oid    [Optional] Object ID defines which private key slot will be used to generate the signature. Default is the first slot.
     *                              Use either one of:
     *                              @ref eDEVICE_PRIKEY_1 (Default)
     *                              @ref eDEVICE_PRIKEY_2 
     *                              @ref eDEVICE_PRIKEY_3 
     *                              @ref eDEVICE_PRIKEY_4 
     *                              slots define below or @ref eSessionCtxId_d session contexts
     * @param[out] result           Pointer to the data array where the final result should be stored.
     * @param[out] rlen             Length of the output data. Will be modified in case of success.
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t calculateSignatureECDSA(uint8_t dataToSign[], uint16_t dlen, uint16_t privateKey_oid, uint8_t result[], uint16_t& rlen);
    int32_t calculateSignatureECDSA(uint8_t dataToSign[], uint16_t dlen, uint8_t result[], uint16_t& rlen) { 
        return calculateSignatureECDSA(dataToSign, dlen, OPTIGA_KEY_ID_E0F0, result, rlen);
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
     * This function verifies an ECDSA signature.
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
    int32_t verifySignature(uint8_t hash[], uint16_t hashLength, uint8_t signature[], uint16_t signatureLength, uint16_t publicKey_oid)
    {   return verifySignatureECDSA(hash, hashLength, signature, signatureLength, publicKey_oid); }
    int32_t verifySignature(uint8_t hash[], uint16_t hashLength, uint8_t signature[], uint16_t signatureLength ) 
    {   return verifySignatureECDSA(hash, hashLength, signature, signatureLength, eDEVICE_PUBKEY_CERT_IFX); }
    int32_t verifySignature(uint8_t hash[], uint16_t hashLength, uint8_t signature[], uint16_t signatureLength, uint8_t pubKey[], uint16_t plen)
    {   return verifySignatureECDSA(hash, hashLength, signature, signatureLength, pubKey, plen, OPTIGA_ECC_CURVE_NIST_P_256);}

    /**
     * This function verifies an RSA signature.
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
     * @param[in] rsa_key_type      Public key RSA type
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t verifySignatureRSA(uint8_t hash[], uint16_t hashLength, uint8_t signature[], uint16_t signatureLength, uint16_t publicKey_oid);
    int32_t verifySignatureRSA(uint8_t hash[], uint16_t hashLength, uint8_t signature[], uint16_t signatureLength, uint8_t pubKey[], uint16_t plen, optiga_rsa_key_type rsa_key_type);

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
     * @param[in] ecc_key_type      Public key ECC curve type   
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t verifySignatureECDSA(uint8_t hash[], uint16_t hashLength, uint8_t signature[], uint16_t signatureLength, uint16_t publicKey_oid);
    int32_t verifySignatureECDSA(uint8_t hash[], uint16_t hashLength, uint8_t signature[], uint16_t signatureLength, uint8_t pubKey[], uint16_t plen, optiga_ecc_curve_t ecc_key_type);
    
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
    int32_t sharedSecret(uint8_t publicKey[], uint16_t plen) { 
		return calculateSharedSecretGeneric(OPTIGA_ECC_CURVE_NIST_P_256, eSESSION_ID_2, publicKey, plen, eSESSION_ID_2);
	}
    int32_t sharedSecret(uint16_t oid, uint8_t publicKey[], uint16_t plen) {
		return calculateSharedSecretGeneric(OPTIGA_ECC_CURVE_NIST_P_256, oid, publicKey, plen, oid);
	}
    int32_t sharedSecret(String curveName, uint8_t publicKey[], uint16_t plen) {
		return calculateSharedSecretGeneric(str2cur(curveName),eSESSION_ID_2, publicKey, plen, eSESSION_ID_2);
	}
    int32_t sharedSecret(String curveName, uint16_t oid, uint8_t publicKey[], uint16_t plen) { 
		return calculateSharedSecretGeneric(str2cur(curveName),oid, publicKey, plen, oid);
	}
    int32_t sharedSecretWithExport(uint8_t publicKey[], uint16_t plen, uint8_t sharedSecret[], uint16_t& shlen) {
		return calculateSharedSecretGeneric(OPTIGA_ECC_CURVE_NIST_P_256, eSESSION_ID_2, publicKey, plen, 0x0000, sharedSecret, shlen);
	}
    int32_t sharedSecretWithExport(String curveName, uint8_t publicKey[], uint16_t plen, uint8_t sharedSecret[], uint16_t& shlen) {
		return calculateSharedSecretGeneric(str2cur(curveName), eSESSION_ID_2, publicKey, plen, 0x0000, sharedSecret, shlen);
	}  
    
    /**
     * This function generates a RSA 1024 bits exponential public private keypair. You can store the private key internally or export it for your usage
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
	int32_t generateKeypair(uint8_t publicKey[], uint16_t& plen ) { return generateKeypairRSA(publicKey, plen, eRSA_DEVICE_PRIKEY_1, OPTIGA_RSA_KEY_1024_BIT_EXPONENTIAL); }
    int32_t generateKeypair(uint8_t publicKey[], uint16_t& plen, uint16_t privateKey_oid) { return generateKeypairRSA(publicKey, plen, privateKey_oid, OPTIGA_RSA_KEY_1024_BIT_EXPONENTIAL); };
    int32_t generateKeypair(uint8_t publicKey[], uint16_t& plen, uint8_t privateKey[], uint16_t& prlen) {return generateKeypairRSA(publicKey, plen, privateKey, prlen, OPTIGA_RSA_KEY_1024_BIT_EXPONENTIAL);};
    
    /**
     * This function generates a RSA type public private keypair. You can store the private key internally or export it for your usage
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
     * @param[in] rsa_key_type      [Optional] Public key RSA type
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t generateKeypairRSA(uint8_t publicKey[], uint16_t& plen ) {return generateKeypairRSA(publicKey, plen, eRSA_DEVICE_PRIKEY_1, OPTIGA_RSA_KEY_1024_BIT_EXPONENTIAL); };
    int32_t generateKeypairRSA(uint8_t publicKey[], uint16_t& plen, uint16_t privateKey_oid, optiga_rsa_key_type_t rsa_key_type);
    int32_t generateKeypairRSA(uint8_t publicKey[], uint16_t& plen, uint8_t  privateKey[], uint16_t& prlen, optiga_rsa_key_type_t rsa_key_type);

    /**
     * This function generates RSA 1024 bits exponential type public private keypair. You can store the private key internally or export it for your usage
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
    int32_t generateKeypairRSA1024(uint8_t publicKey[], uint16_t& plen, uint8_t privateKey[], uint16_t& prlen) { return generateKeypairRSA(publicKey, plen, privateKey, prlen, OPTIGA_RSA_KEY_1024_BIT_EXPONENTIAL); };
    int32_t generateKeypairRSA1024(uint8_t publicKey[], uint16_t& plen, uint16_t privateKey_oid) { return generateKeypairRSA(publicKey, plen, privateKey_oid, OPTIGA_RSA_KEY_1024_BIT_EXPONENTIAL); };

        /**
     * This function generates RSA 2048 bits exponential type public private keypair. You can store the private key internally or export it for your usage
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
    int32_t generateKeypairRSA2048(uint8_t publicKey[], uint16_t& plen, uint8_t privateKey[], uint16_t& prlen)  {return generateKeypairRSA(publicKey, plen, privateKey, prlen, OPTIGA_RSA_KEY_2048_BIT_EXPONENTIAL); };
    int32_t generateKeypairRSA2048(uint8_t publicKey[], uint16_t& plen, uint16_t privateKey_oid) { return generateKeypairRSA(publicKey, plen, privateKey_oid, OPTIGA_RSA_KEY_2048_BIT_EXPONENTIAL); };
    
    /**
     * This function generates an ECC public private keypair. You can store the private key internally or export it for your usage
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
     * @param[in] ecc_key_type      [Optional] Public key ECC curve type  
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t generateKeypairECC(uint8_t publicKey[], uint16_t& plen ) { return generateKeypairECC(publicKey, plen, eSESSION_ID_2, OPTIGA_ECC_CURVE_NIST_P_256); };
    int32_t generateKeypairECC(uint8_t publicKey[], uint16_t& plen, uint16_t privateKey_oid, optiga_ecc_curve_t ecc_key_type);
    int32_t generateKeypairECC(uint8_t publicKey[], uint16_t& plen, uint8_t privateKey[], uint16_t& prlen, optiga_ecc_curve_t ecc_key_type);

    
    /**
     * This function generates an ECC NIST P 256 public private keypair. You can store the private key internally or export it for your usage
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
    int32_t generateKeypairECCP256(uint8_t publicKey[], uint16_t& plen, uint8_t privateKey[], uint16_t& prlen) { return generateKeypairECC(publicKey, plen, privateKey, prlen, OPTIGA_ECC_CURVE_NIST_P_256); };
    int32_t generateKeypairECCP256(uint8_t publicKey[], uint16_t& plen, uint16_t privateKey_oid) { return generateKeypairECC(publicKey, plen, privateKey_oid, OPTIGA_ECC_CURVE_NIST_P_256); };
    
        /**
     * This function generates an ECC NIST P 384 public private keypair. You can store the private key internally or export it for your usage
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
    int32_t generateKeypairECCP384(uint8_t publicKey[], uint16_t& plen, uint8_t privateKey[], uint16_t& prlen) { return generateKeypairECC(publicKey, plen, privateKey, prlen, OPTIGA_ECC_CURVE_NIST_P_384); };
    int32_t generateKeypairECCP384(uint8_t publicKey[], uint16_t& plen, uint16_t privateKey_oid) { return generateKeypairECC(publicKey, plen, privateKey_oid, OPTIGA_ECC_CURVE_NIST_P_384); };

	/**
     * This function encrypt user given data with RSAES PKCS1 v1.5 Scheme.
     *
     * @param[in] dataToEncrypt     Pointer to the data
     * @param[in] dlen              Length of the input data
	 * @param[in] pubkeyFromUser    A user provided RSA public key.
     *                              @note The folloiwng format is expected
	 * static uint8_t public_key [] = {
	 * //BIT String
	 * 0x03,
	 * 	//BIT String Length
	 * 	0x81, 0x8E,
	 * 		//UnusedBits
	 * 		0x00,
	 * 		//SEQUENCE
	 * 		0x30,
	 * 			//Length
	 * 			0x81, 0x8A,
	 * 			//INTEGER (pub key)
	 * 			0x02,
	 * 			//Pub key modulus length
	 * 			0x81, 0x81,
	 * 				//Public key modulus
	 * 				0x00,
	 * 				0xA1, 0xD4, 0x6F, 0xBA, 0x23, 0x18, 0xF8, 0xDC,
	 * 				0xEF, 0x16, 0xC2, 0x80, 0x94, 0x8B, 0x1C, 0xF2,
	 * 				0x79, 0x66, 0xB9, 0xB4, 0x72, 0x25, 0xED, 0x29,
	 * 				0x89, 0xF8, 0xD7, 0x4B, 0x45, 0xBD, 0x36, 0x04,
	 * 				0x9C, 0x0A, 0xAB, 0x5A, 0xD0, 0xFF, 0x00, 0x35,
	 * 				0x53, 0xBA, 0x84, 0x3C, 0x8E, 0x12, 0x78, 0x2F,
	 * 				0xC5, 0x87, 0x3B, 0xB8, 0x9A, 0x3D, 0xC8, 0x4B,
	 * 				0x88, 0x3D, 0x25, 0x66, 0x6C, 0xD2, 0x2B, 0xF3,
	 * 				0xAC, 0xD5, 0xB6, 0x75, 0x96, 0x9F, 0x8B, 0xEB,
	 * 				0xFB, 0xCA, 0xC9, 0x3F, 0xDD, 0x92, 0x7C, 0x74,
	 * 				0x42, 0xB1, 0x78, 0xB1, 0x0D, 0x1D, 0xFF, 0x93,
	 * 				0x98, 0xE5, 0x23, 0x16, 0xAA, 0xE0, 0xAF, 0x74,
	 * 				0xE5, 0x94, 0x65, 0x0B, 0xDC, 0x3C, 0x67, 0x02,
	 * 				0x41, 0xD4, 0x18, 0x68, 0x45, 0x93, 0xCD, 0xA1,
	 * 				0xA7, 0xB9, 0xDC, 0x4F, 0x20, 0xD2, 0xFD, 0xC6,
	 * 				0xF6, 0x63, 0x44, 0x07, 0x40, 0x03, 0xE2, 0x11,
	 * 			//INTEGER (publicExponent)
	 *			0x02,
	 *				//Pub key exponent length
	 *				0x04,
	 *				//Public Exponent
	 *				0x00, 0x01, 0x00, 0x01
	 * };
	 * @param[in] pklen             Public key length
     * @param[out] result           Pointer to the data array where the final result should be stored.
     * @param[out] rlen             Length of the output data. Will be modified in case of success.
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
    int32_t encrypt(uint8_t dataToEncrypt[], uint16_t dlen,  uint8_t pubkeyFromUser[], uint16_t pklen, uint8_t result[], uint16_t& rlen);
	
	/**
     * This function encrypt user given data with RSAES PKCS1 v1.5 Scheme.
     *
     * @param[in] dataToEncrypt     Pointer to the data
     * @param[in] dlen              Length of the input data
     * @param[in] certOID           Object ID defines which public key will be used to encrypt the message (the public key will be extracted from the certificate). Default is the first slot.
     *                              Use either one of:
     *                              @ref eDEVICE_PUBKEY_CERT_IFX (Default)
     *                              @ref eDEVICE_PUBKEY_CERT_PRJSPC_1
     *                              @ref eDEVICE_PUBKEY_CERT_PRJSPC_2
     *                              @ref eDEVICE_PUBKEY_CERT_PRJSPC_3
     * @param[out] result           Pointer to the data array where the final result should be stored.
     * @param[out] rlen             Length of the output data. Will be modified in case of success.
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
	int32_t encrypt(uint8_t dataToEncrypt[], uint16_t dlen,  uint16_t certOID, uint8_t result[], uint16_t& rlen);

	/**
     * This function decrypt user given data with RSAES PKCS1 v1.5 Scheme.
     *
     * @param[in] dataToDecrypt     Pointer to the data
     * @param[in] dlen              Length of the input data
     * @param[in] keyOID            Object ID defines which key will be used to decrypt the message. Default is the first slot.
     *                              Use either one of:
     *                              @ref eRSA_DEVICE_PRIKEY_1 (Default)
     *                              @ref eRSA_DEVICE_PRIKEY_2 
     * @param[out] result           Pointer to the data array where the final result should be stored.
     * @param[out] rlen             Length of the output data. Will be modified in case of success.
     *
     * @retval  0 If function was successful.
     * @retval  1 If the operation failed.
     */
	int32_t decrypt(uint8_t dataToDecrypt[], uint16_t dlen,  uint16_t keyOID, uint8_t result[], uint16_t& rlen);
	

protected:

    optiga_util_t  * me_util  = NULL;
    optiga_crypt_t * me_crypt = NULL;
	bool active;

    int32_t getGenericData(uint16_t oid, uint8_t* p_data, uint16_t& hashLength);
    int32_t getGenericMetadata(uint16_t oid, uint8_t* p_data, uint16_t& length);
    int32_t setGenericData(uint16_t oid, uint8_t* p_data, uint16_t hashLength);
    int32_t getState(uint16_t oid, uint8_t& p_data);
    int32_t str2cur(String curve_name);
	int32_t calculateSharedSecretGeneric( int32_t curveID, uint16_t priv_oid, uint8_t* p_pubkey, uint16_t plen, uint16_t out_oid) {
	uint16_t dummy_len; 
	return calculateSharedSecretGeneric(curveID, priv_oid, p_pubkey, plen, out_oid, NULL, dummy_len);
	}
    int32_t calculateSharedSecretGeneric( int32_t curveID, uint16_t priv_oid, uint8_t* p_pubkey, uint16_t plen, uint16_t out_oid, uint8_t* p_out, uint16_t& olen);

    // int32_t getGlobalSecurityStatus(uint8_t& status); //TODO?: Not implemented (as in arduino-trust-x)
    // int32_t setGlobalSecurityStatus(uint8_t status);  //TODO?: Not implemented (as in arduino-trust-x)
    // int32_t getAppSecurityStatus(uint8_t* p_data, uint16_t& hashLength);  //TODO?: Not implemented (as in arduino-trust-x)
    // int32_t setAppSecurityStatus(uint8_t status);  //TODO?: Not implemented (as in arduino-trust-x)
    // int32_t deriveKey(uint8_t hash[], uint16_t hashLength, uint8_t publicKey[], uint16_t plen); //TODO: ?? Private function unused by any other function.
};
/**
 * @}
 */

//Preinstantiated object
extern IFX_OPTIGA_TrustM trustM;

/*************************************************************************

 *  Inline functions 

 *************************************************************************/
                         

#endif /* IFXOPTIGATRUSTM_H_ */
