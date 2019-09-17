/**
 * MIT License
 *
 * Copyright (c) 2019 Infineon Technologies AG
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

#include <OPTIGATrustM.h>

void setup() 
{
    /*
     * Initialise a serial port for debug output
     */
    Serial.begin(100000);
    delay(100);
    Serial.println("Initializing ... ");


    example_optiga_crypt_hash();
    // example_optiga_crypt_ecc_generate_keypair();
    // example_optiga_crypt_ecdsa_sign();
    // example_optiga_crypt_ecdsa_verify();
    // example_optiga_crypt_ecdh();
    // example_optiga_crypt_random();  
    // example_optiga_crypt_tls_prf_sha256();
    // example_optiga_util_read_data();
    // example_optiga_util_write_data();
    // example_optiga_crypt_rsa_generate_keypair();
    // example_optiga_crypt_rsa_sign();
    // example_optiga_crypt_rsa_verify();
    // example_optiga_crypt_rsa_decrypt_and_export();
    // example_optiga_crypt_rsa_decrypt_and_store();
    // example_optiga_crypt_rsa_encrypt_message();
    // example_optiga_crypt_rsa_encrypt_session();
    // example_optiga_util_update_count();
    // example_optiga_util_protected_update();
    // example_optiga_util_read_uuid();
    // example_pair_host_and_optiga_using_pre_shared_secret();
    // example_optiga_util_hibernate_restore();

}

void loop()
{

}



