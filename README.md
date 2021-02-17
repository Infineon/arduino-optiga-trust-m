# Infineon OPTIGA&trade; Trust M Arduino library

[![Build Status](https://travis-ci.org/Infineon/arduino-optiga-trust-m.svg?branch=master)](https://travis-ci.org/Infineon/arduino-optiga-trust-m)


<img src="https://www.infineon.com/export/sites/default/_images/product/security-smart-card-solutions/OPTIGA_Trust_M.png_295599509.png" alt="drawing" width="300"/>


Infineon's [OPTIGA&trade; Trust M](https://www.infineon.com/cms/en/product/security-smart-card-solutions/optiga-embedded-security-solutions/optiga-trust/optiga-trust-m-sls32aia/) security solution library for Arduino

## Summary
[OPTIGA&trade; Trust M](https://www.infineon.com/dgdl/Infineon-OPTIGA%20TRUST%20X%20SLS%2032AIA-DS-v02_18-EN.pdf) is a security solution based on a secure micrcontroller.
Each device is shipped with a unique elliptic-curve keypair and a corresponding X.509 certificate. OPTIGA&trade; Trust M enables easy integration into existing PKI infrastructure.


## Key Features and Benefits
* High-end security controller
* Turnkey solution
* Common Criteria Certified EAL6+ (high) hardware
* I2C interface with Shielded Connection (encrypted communication)
* Up to 10 KB user memory
* Cryptographic support: ECC NIST P256/P384/P521, ECC Brainpool P256/384/512 r1, SHA-256, TRNG, DRNG, RSA® 1024/2048, AES 128/192/256 (ECB, CBC, CBC-MAC, CMAC)
* PG-USON-10-2 package (3 x 3 mm)
* Temperature range (−40°C to +105°C)
* Crypto ToolBox commands with ECC NIST P256/P384/P521, ECC Brainpool P256/384/512 r1, SHA-256 (sign, verify, key generation, ECDH(E), key derivation), RSA® 1024/2048 (sign, verify, key generation, encrypt and decrypt)  
* Device Security Monitor
* Hibernate for zero power consumption
* Lifetime for Industrial Automation and Infrastructure is 20 years and 15 years for other Application Profiles


## Hardware

<img src="https://github.com/Infineon/Assets/raw/master/Pictures/OPTIGA_Trust_M_Shield2Go%2002.jpg" alt="drawing" width="360"/>

The wiring to your arduino board depends on the [evaluation board](https://www.infineon.com/cms/en/product/evaluation-boards/optiga-trust-m-eval-kit/) or the 
[Shield2GO](https://www.infineon.com/cms/en/product/evaluation-boards/s2go-security-optiga-m/) you are using.

Notes to the S2Go Security OPTIGA M:
* Supply voltage VCC is max. 5.5 V, please refer to the OPTIGA™ Trust M datasheet for more details about maximum ratings
* Ensure that no voltage applied to any of the pins exceeds the absolute maximum rating of VCC + 0.3 V
* Pin out on top (head) is directly connected to the pins of the OPTIGA™ Trust M
* If head is broken off, only one capacitor is connected to the OPTIGA™ Trust M

<details>
<summary> S2Go Security OPTIGA M Pinout</summary>
<img src="https://github.com/Infineon/Assets/blob/master/Pictures/optiga_trust_m_shield2go_pinout.png" alt="drawing"/>
</details>

<details>
<summary> S2Go Security OPTIGA M Schematic</summary>
<img src="https://github.com/Infineon/Assets/blob/master/Pictures/optiga_trust_m_shield2go_schematics.png" alt="drawing"/>
</details>

## Installation

### Integration of Library
Please download this repository from GitHub either from the latest [release](https://github.com/Infineon/arduino-optiga-trust-m/releases) or directly [here](https://github.com/Infineon/arduino-optiga-trust-m/archive/v1.0.0.zip).

To install the OPTIGA&trade; Trust M library in the Arduino IDE several options can be taken:
1. via [Library Manager](https://www.arduino.cc/en/Guide/Libraries#toc3) you can find and control this library. Simply type in the Search Field **arduino-optiga-trust-m**
1. **Sketch** > **Include Library** > **Add .ZIP Library...** in the Arduino IDE and navigate to the downloaded .ZIP file of this repository. The library will be installed in your Arduino sketch folder in libraries and you can select as well as include this one to your project under **Sketch** > **Include Library** > **arduino-optiga-trust-m**.

![Install Library](https://raw.githubusercontent.com/infineon/assets/master/Pictures/Library_Install_ZIP.png)

## Supported Devices
In general, the library should be compatible with any Arduino board, however it has been tested for the following platforms:
* XMC1100 2Go (Infineon Technologies, XMC1100, Cortex M0)
* XMC1100 Arduino Boot Kit (Infineon Technologies, XMC1100, Cortex M0)
* XMC4700 Relax Kit (Infineon Technologies, XMC4700, Cortex M4)
* Arduino Zero (Atmel SAM, SAMD21G18A, Cortex M0+)
* MH ET LIVE ESP32MiniKit (Espressif, ESP32, Tensilica Xtensa LX6)

## Usage
The library is equiped with eight groups of examples which can be found on the following path: File->Examples>arduino-optiga-trust-m 
The following sections describe all the examples in more detail.

### selfTest
selfTest example demonstrates a trustM.checkChip() method usage, which authenticates the OPTIGA™ Trust M on the host MCU. 
This method implements a simple challenge-response authentication scheme, in which the host side authenticates the OPTIGA™ Trust M security chip.  

### calculateHash 
calculateHash demonstrates example usage of the SHA256 hash, as well as a simple benchamarking for your microcontroller. 
The performance of this benchmark test greatly depends on I2C  bus frame size (it affects mainly big blocks of data transmitted to the OPTIGA™ Trust M chip for hashing), 
which was limited by default down to 32 bytes (in case of 32 bytes the library will perfrom fragmentation). 

Please check settings for your specific platform, if you want to improve the performance of the hashing function. 

### calculateSharedSecret
calculateSharedSecret demonstrates example usage of the calculation of a shared secret given the public key of the peer. The sharedSecret allows the shared secret to be stored in one of the secure storage data objects (which also can be specified from the Object ID as an argument. If not the default is used) on chip.

Additionally, the sharedSecretWithExport function is used, and the shared secret is in this case exported to the host.

### calculateSignVerifySign  
calculateSignVerifySign demonstrates signature generation and signature verification methods of the library. 
This example shows two modes of operation: 
1) Calculate a signature using manufacturer private key, the result value is then verified  against the public key
2) Generate a public-private keypair and store the latter inside one of Object IDs of the OPTIGA™ Trust M, 
then sign the digest giving only the latter  Object ID, the result value is then verified  against the public key.  

For the verification three methods are available: 
1) with a given raw public key
2) with Object ID pointing to the memory slot where the public key is located, 
3) if neither Object ID nor raw public key were specified, the function will use a default Object ID with manufacturer public key certificate. 

Sample output for a XMC1100 BootKit
```
My Public Key Length: 68
My Public Key:
0x000000: 03 42 00 04 e2 32 bd 3e 4b 05 a6 f9 1d dc 84 5d .B...2.>K......]
0x000010: 29 aa a1 5d 02 e9 ca e3 04 31 c1 c8 bc c5 4e 4a )..].....1....NJ
0x000020: fd cb 1d 78 78 a5 c5 df b9 d0 77 c0 37 d8 8a 2b ...xx.....w.7..+
0x000030: 7f 6f 33 66 b4 4c 4f 35 7b 90 3d 7f 72 9f 2f 53 .o3f.LO5{.=.r./S
0x000040: 14 ff 05 59                                     ...Y            

Calculate Hash ... 
[OK] | Command executed in 68 ms
Hash Length: 32
Hash:
0x000000: 53 bc c3 a1 94 52 bc cd 71 8a 48 58 fc 3e da 82 S....R..q.HX.>..
0x000010: 30 5b 96 e2 1a 2b 3f 43 32 50 c4 32 fd 92 ac bf 0[...+?C2P.2....

Generate Signature ... 
[OK] | Command executed in 152 ms
Signature #1 Length: 68
Signature #1:
0x000000: 02 20 38 0f 56 c8 90 53 18 9d 8f 58 b4 46 35 a0 . 8.V..S...X.F5.
0x000010: d7 07 63 ef 9f a2 30 64 93 e4 3d bf 7b db 57 a1 ..c...0d..=.{.W.
0x000020: b6 d7 02 20 4f 5e 3a db 6b 1a eb ac 66 9a 15 69 ... O^:.k...f..i
0x000030: 0d 7d 46 5b 44 72 40 06 a5 7b 06 84 0f d7 6e 0f .}F[Dr@..{....n.
0x000040: 4b 45 7f 50                                     KE.P            

Verify Signature ... 
[OK] | Command executed in 231 ms

Calculate Hash ... 
[OK] | Command executed in 64 ms
Hash Length: 32
Hash:
0x000000: 53 bc c3 a1 94 52 bc cd 71 8a 48 58 fc 3e da 82 S....R..q.HX.>..
0x000010: 30 5b 96 e2 1a 2b 3f 43 32 50 c4 32 fd 92 ac bf 0[...+?C2P.2....
Generate Key Pair ... [OK] | Command executed in 114 ms
Public key Length: 68
Public key:
0x000000: 03 42 00 04 f1 c0 8f 47 98 7f 67 2e 27 eb 2d 63 .B.....G..g.'.-c
0x000010: 8d c3 50 94 0a b6 39 1c 1a d6 c6 b1 01 f1 e4 07 ..P...9.........
0x000020: bd 6d 1d 7e d2 6b a7 4e 63 b7 5b ef cd 84 20 28 .m.~.k.Nc.[... (
0x000030: 2d ef 8b f0 0b bd 42 5d 6e 08 14 ba 01 fc d6 58 -.....B]n......X
0x000040: 75 13 fa 48                                     u..H            

Generate Signature ... 
[OK] | Command executed in 152 ms
Signature #2 Length: 70
Signature #2:
0x000000: 02 21 00 c5 6d 74 35 bb 8f 9a 82 ff e0 07 ab 20 .!..mt5........ 
0x000010: f2 d7 91 5c 3b 09 93 97 39 d5 53 60 a9 16 18 89 ...\;...9.S`....
0x000020: 15 32 30 02 21 00 f3 11 f8 a2 fd d7 6b f0 59 0f .20.!.......k.Y.
0x000030: fb ed 56 76 c7 6f d0 76 53 7b 72 72 2c 18 98 1b ..Vv.o.vS{rr,...
0x000040: 41 38 6e b7 fb 15                               A8n...          

Verify Signature ... 
[OK] | Command executed in 230 ms
```

### encryptDecryptData

This example demonstrates RSA Data Encryption/Decryption using PKCS#1 v1.5 Scheme.

The example does the following:
* Initializes the Secure Element and pairs the Host and the OPTIGA(TM) Trust M (Shielded Connection)
* Generate a new RSA1024 keypair and exports the public key to the host
* Encrypts sample message using OPTIGA(TM) Trust M rev.1. The message sent is protected.
* Gets the resulting cipher from the Secure Element
* Sends to the chip the chiphertext over the protected communication channle and gets the responce.

The sample output is following:
```
Initializing ...
Begin to trust ... OK
Limit the Current ... OK

Generate Key Pair RSA 1024. Store Private Key on Board ...
[OK] | Command executed in 540 ms
Public Key  Length: 144
Public Key :
0x000000: 03 81 8d 00 30 81 89 02 81 81 00 a4 06 98 4b d2 ....0.........K.
0x000010: 04 ba 61 66 5e 69 ae c3 18 40 75 75 a3 be 31 01 ..af^i...@uu..1.
0x000020: a5 10 a7 f4 2a d3 24 9c f5 af 2e d5 99 4a da a1 ....*.$......J..
0x000030: 13 bc 89 f3 9a 53 b9 34 88 ef f8 a9 ab 1f 7e 28 .....S.4......~(
0x000040: 3a 03 7a cf 41 09 2e 57 b6 36 2d 3b 76 e0 71 91 :.z.A..W.6-;v.q.
0x000050: 8c 6b 67 22 aa 0a 61 42 45 e6 ce 7d d4 8b b8 c2 .kg"..aBE..}....
0x000060: 2a 90 81 82 df 3b 88 92 2e 3a 31 ef 58 5c 95 f4 *....;...:1.X\..
0x000070: a8 f3 9c 2b 9c 51 e5 b0 54 7a 0c 25 cd 07 f7 6b ...+.Q..Tz.%...k
0x000080: 65 e6 c3 97 dd 4d 30 e3 20 b8 51 02 03 01 00 01 e....M0. .Q.....

Message to Encrypt: Encrypt and Decrypt Data using OPTIGA(TM) Trust M

[OK] | Command executed in 274 ms

Encrypted message  Length: 128

Encrypted message :
0x000000: 0f 85 53 f2 1b 55 69 f9 15 10 40 56 da 12 09 09 ..S..Ui...@V....
0x000010: 7e d7 6c 5b 87 55 8b f9 48 2b d1 88 13 b0 ac 9b ~.l[.U..H+......
0x000020: 01 4b 10 14 e1 3d 16 85 14 c5 53 ca 3d dd c2 04 .K...=....S.=...
0x000030: d4 85 87 22 66 e8 6b e2 b2 d0 1a f2 15 d9 26 6e ..."f.k.......&n
0x000040: 29 dc dd f9 e4 40 d5 c7 b2 66 ff d5 7f ee 85 3e )....@...f.....>
0x000050: e8 2f b2 7e cf 96 54 62 4a 07 16 64 59 e6 ec fd ./.~..TbJ..dY...
0x000060: 13 9d e0 41 35 6c 47 08 bb 46 41 c1 7b 0b 9a 9f ...A5lG..FA.{...
0x000070: 85 d8 b6 7c 2d 53 8a 10 fd f3 d3 ca 57 4f b8 57 ...|-S......WO.W

[OK] | Command executed in 149 ms

Decrypted message  Length: 50

Decrypted message :
0x000000: 45 6e 63 72 79 70 74 20 61 6e 64 20 44 65 63 72 Encrypt and Decr
0x000010: 79 70 74 20 44 61 74 61 20 75 73 69 6e 67 20 4f ypt Data using O
0x000020: 50 54 49 47 41 28 54 4d 29 20 54 72 75 73 74 20 PTIGA(TM) Trust
0x000030: 4d 00
```

### generateKeypair
calculateSignVerifySign demonstrates methods for keypair generation, either with a private key export, or without. 
In the latter case the developer should specify the Object ID of the private key. 

### getCertificate, getUniqueID 
getCertificate and getUniqueID demonstrate examples of retrievieng various properties of the OPTIGA™ Trust M Chip. 
As well as these examples the developer can also call getCurrentLimit/setCurrentLimit in order to get or modify the
current limitation of the chip (varies from 5mA by default to maximum 15mA) 

### getRandom
getRandom demonstrates random number generator capabilities. This example outputs random numbers of various sizes (16, 32, 64, 128, 256). The chip can generate minimum 8 bytes of random. If you need less you can discard the result output.

### testFullAPI
testFullAPI is used to briefly test major API calls to the library. The expected output of this function can be found in Figure below. 
*Certificate output might be different

![testFullAPI expected result](https://github.com/Infineon/Assets/raw/master/Pictures/OPTIGA%20Trust%20X%20testFullAPI.png)
	
## Available Functions

The available Arduino API is described in the [Doxygen documentation](https://infineon.github.io/arduino-optiga-trust-m/).

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct and the process of submitting pull requests to us.

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details
