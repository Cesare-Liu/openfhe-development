//==================================================================================
// BSD 2-Clause License
//
// Copyright (c) 2014-2022, NJIT, Duality Technologies Inc. and other contributors
//
// All rights reserved.
//
// Author TPOC: contact@openfhe.org
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//==================================================================================

#ifndef LBCRYPTO_CRYPTO_BASE_PKE_H
#define LBCRYPTO_CRYPTO_BASE_PKE_H

#include "ciphertext-fwd.h"
#include "key/privatekey.h"
#include "key/publickey.h"
#include "key/keypair.h"

#include <vector>
#include <memory>

/**
 * @namespace lbcrypto
 * The namespace of lbcrypto
 */
namespace lbcrypto {

struct EncryptResult {
    EncryptResult() : isValid(false), numBytesEncrypted(0) {}

    explicit EncryptResult(size_t len) : isValid(true), numBytesEncrypted(len) {}

    bool isValid;  // whether the encryption was successful
    // count of the number of plaintext bytes that were encrypted
    usint numBytesEncrypted;
};

/**
 * @brief Decryption result.  This represents whether the decryption of a
 * cipheretext was performed correctly.
 *
 * This is intended to eventually incorporate information about the amount of
 * padding in a decoded ciphertext, to ensure that the correct amount of
 * padding is stripped away. It is intended to provided a very simple kind of
 * checksum eventually. This notion of a decoding output is inherited from the
 * crypto++ library. It is also intended to be used in a recover and restart
 * robust functionality if not all ciphertext is recieved over a lossy
 * channel, so that if all information is eventually received,
 * decoding/decryption can be performed eventually. This is intended to be
 * returned with the output of a decryption operation.
 */
struct DecryptResult {
    /**
   * Constructor that initializes all message lengths to 0.
   */
    DecryptResult() : isValid(false), messageLength(0), scalingFactorInt(1) {}

    /**
   * Constructor that initializes all message lengths.
   * @param len the new length.
   */
    explicit DecryptResult(size_t len) : isValid(true), messageLength(len), scalingFactorInt(1) {}

    /**
   * Constructor that initializes all message lengths.
   * @param len the new length.
   * @param scf the new scaling factor.
   */
    explicit DecryptResult(size_t len, NativeInteger scf) : isValid(true), messageLength(len), scalingFactorInt(scf) {}

    bool isValid;                   /**< whether the decryption was successful */
    usint messageLength;            /**< the length of the decrypted plaintext message */
    NativeInteger scalingFactorInt; /**< Scaling factor for BGV FlexibleAuto mode. */
};

/**
 * @brief Abstract interface for encryption algorithm
 * @tparam Element a ring element.
 */
template <class Element>
class PKEBase {
    using ParmType = typename Element::Params;
    using IntType  = typename Element::Integer;
    using DugType  = typename Element::DugType;
    using DggType  = typename Element::DggType;
    using TugType  = typename Element::TugType;

public:
    virtual ~PKEBase() {}

    /**
   * Function to generate public and private keys
   *
   * @param &publicKey private key used for decryption.
   * @param &privateKey private key used for decryption.
   * @return function ran correctly.
   */
    virtual KeyPair<Element> KeyGen(CryptoContext<Element> cc, bool makeSparse);

    //  virtual KeyPair<Element> KeyGen(CryptoContext<Element> cc,
    //                                    bool makeSparse,
    //                                    PublicKey<Element>
    // publicKey);

    /**
   * Method for encrypting plaintex using LBC
   *
   * @param privateKey private key used for encryption.
   * @param plaintext copy of the plaintext input. NOTE a copy is passed! That
   * is NOT an error!
   * @param doEncryption encrypts if true, embeds (encodes) the plaintext into
   * cryptocontext if false
   * @param *ciphertext ciphertext which results from encryption.
   */
    virtual Ciphertext<Element> Encrypt(Element plaintext, const PrivateKey<Element> privateKey) const;

    /**
   * Method for encrypting plaintext using LBC
   *
   * @param&publicKey public key used for encryption.
   * @param plaintext copy of the plaintext element. NOTE a copy is passed!
   * That is NOT an error!
   * @param doEncryption encrypts if true, embeds (encodes) the plaintext into
   * cryptocontext if false
   * @param *ciphertext ciphertext which results from encryption.
   */
    virtual Ciphertext<Element> Encrypt(Element plaintext, const PublicKey<Element> publicKey) const;

    /**
   * Method for decrypting plaintext using LBC
   *
   * @param &privateKey private key used for decryption.
   * @param &ciphertext ciphertext id decrypted.
   * @param *plaintext the plaintext output.
   * @return the decoding result.
   */
    virtual DecryptResult Decrypt(ConstCiphertext<Element> ciphertext, const PrivateKey<Element> privateKey,
                                  NativePoly* plaintext) const {
        OPENFHE_THROW(config_error, "Decryption to NativePoly is not supported");
    }

    /**
   * Method for decrypting plaintext using LBC
   *
   * @param &privateKey private key used for decryption.
   * @param &ciphertext ciphertext id decrypted.
   * @param *plaintext the plaintext output.
   * @return the decoding result.
   */
    virtual DecryptResult Decrypt(ConstCiphertext<Element> ciphertext, const PrivateKey<Element> privateKey,
                                  Poly* plaintext) const {
        OPENFHE_THROW(config_error, "Decryption to Poly is not supported");
    }

    /////////////////////////////////////////
    // CORE OPERATIONS
    /////////////////////////////////////////

    virtual std::shared_ptr<std::vector<Element> > EncryptZeroCore(const PrivateKey<Element> privateKey,
                                                                   const std::shared_ptr<ParmType> params) const;

    virtual std::shared_ptr<std::vector<Element> > EncryptZeroCore(const PublicKey<Element> publicKey,
                                                                   const std::shared_ptr<ParmType> params) const;

    virtual Element DecryptCore(const std::vector<Element>& cv, const PrivateKey<Element> privateKey) const;
};

}  // namespace lbcrypto

#endif
