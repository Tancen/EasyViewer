#ifndef CRYPTOLOGY_H
#define CRYPTOLOGY_H

#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <memory>
#include <string>
#include <functional>

/*
*   This class needs to use with EasyIO::ByteBuffer
*
*/
namespace Cryptology
{
#define CRYPTOLOGY_NEW_INSTANCE_CHECK_ERROR(__err) \
    if((long)__err <= 0) \
    { \
        errorString0 = ERR_error_string(ERR_get_error(), NULL);  \
        break; \
    }

#define CRYPTOLOGY_NEW_INSTANCE_CHECK_ERROR2(__p) \
    if(!__p) \
    { \
        errorString0 = ERR_error_string(ERR_get_error(), NULL);  \
        break; \
    }

    template <typename Data>
    struct Result
    {
        bool isOk;
        std::string errorString;
        Data data;
    };

    template <typename Data>
    class RSA
    {
    public:
        ~RSA()
        {
            BIO_free(m_bio);
            EVP_PKEY_CTX_free(m_ctxEncrypt);
            EVP_PKEY_CTX_free(m_ctxDecrypt);
        }

        static std::unique_ptr<RSA> newInstanceForPublicKey(const std::string& key, std::string* errorString = nullptr)
        {
            return newInstance(key, errorString, PEM_read_bio_PUBKEY, EVP_PKEY_encrypt_init, EVP_PKEY_verify_recover_init,
                               EVP_PKEY_encrypt, EVP_PKEY_verify_recover);
        }

        static std::unique_ptr<RSA> newInstanceForPrivateKey(const std::string& key, std::string* errorString = nullptr)
        {
            return newInstance(key, errorString, PEM_read_bio_PrivateKey, EVP_PKEY_sign_init, EVP_PKEY_decrypt_init,
                               EVP_PKEY_sign, EVP_PKEY_decrypt);
        }

        const std::string& key()
        {
            return m_key;
        }

        Result<Data> encrypt(const char* src, size_t srcLen, Data dst)
        {
            Result<Data> ret;
            ret.data = dst;

            do
            {
                size_t dstLen;
                int err = m_encrypt(m_ctxEncrypt, nullptr, &dstLen, (const unsigned char*)src, srcLen);
                if (err <= 0)
                    break;

                ret.data.ensureWritable(dstLen);
                err = m_encrypt(m_ctxEncrypt, (unsigned char*)ret.data.readableBytes() + ret.data.numReadableBytes(), &dstLen, (const unsigned char*)src, srcLen);
                if (err <= 0)
                    break;

                ret.data.moveWriterIndex(dstLen);
                ret.isOk = true;
                return ret;
            } while (0);


            ret.isOk = false;
            ret.errorString = ERR_error_string(ERR_get_error(), NULL);
            return ret;
        }

        Result<Data> encrypt(const char* src, size_t srcLen)
        {
            return encrypt(src, srcLen, Data());
        }

        Result<Data> decrypt(const char* src, size_t srcLen, Data dst)
        {
            Result<Data> ret;
            ret.data = dst;

            do
            {
                size_t dstLen;
                int err = m_decrypt(m_ctxDecrypt, nullptr, &dstLen, (const unsigned char*)src, srcLen);
                if (err <= 0)
                    break;

                ret.data.ensureWritable(dstLen);
                err = m_decrypt(m_ctxDecrypt, (unsigned char*)ret.data.readableBytes() + ret.data.numReadableBytes(), &dstLen, (const unsigned char*)src, srcLen);
                if (err <= 0)
                    break;

                ret.data.moveWriterIndex(dstLen);
                ret.isOk = true;
                return ret;
            } while (0);


            ret.isOk = false;
            ret.errorString = ERR_error_string(ERR_get_error(), NULL);
            return ret;
        }

        Result<Data> decrypt(const char* src, size_t srcLen)
        {
            return decrypt(src, srcLen, Data());
        }


    private:
        RSA(){}

        static std::unique_ptr<RSA> newInstance(const std::string& key, std::string* errorString,
                        std::function<EVP_PKEY*(BIO *, EVP_PKEY **, pem_password_cb *, void *)> keyReader,
                        std::function<int(EVP_PKEY_CTX *)> funcEncryptInit, std::function<int(EVP_PKEY_CTX *)> funcDecryptInit,
                        std::function<int(EVP_PKEY_CTX *, unsigned char *, size_t *, const unsigned char *, size_t )> funcEncrypt,
                        std::function<int(EVP_PKEY_CTX *, unsigned char *, size_t *, const unsigned char *, size_t )> funcDecrypt)
        {
            std::string errorString0;
            BIO *bio = nullptr;
            EVP_PKEY *pkey = nullptr;
            EVP_PKEY_CTX *ctxEncrypt = nullptr, *ctxDecrypt = nullptr;

            do
            {
                bio = BIO_new_mem_buf(key.c_str(), -1);
                CRYPTOLOGY_NEW_INSTANCE_CHECK_ERROR2(bio);

                pkey = keyReader(bio, &pkey, nullptr, nullptr);
                CRYPTOLOGY_NEW_INSTANCE_CHECK_ERROR2(pkey);

                //encrypt
                ctxEncrypt = EVP_PKEY_CTX_new(pkey, nullptr);
                CRYPTOLOGY_NEW_INSTANCE_CHECK_ERROR2(ctxEncrypt);

                int err = funcEncryptInit(ctxEncrypt);
                CRYPTOLOGY_NEW_INSTANCE_CHECK_ERROR(err);

                //decrypt
                ctxDecrypt = EVP_PKEY_CTX_new(pkey, nullptr);
                CRYPTOLOGY_NEW_INSTANCE_CHECK_ERROR2(ctxDecrypt);

                err = funcDecryptInit(ctxDecrypt);
                CRYPTOLOGY_NEW_INSTANCE_CHECK_ERROR(err);


                std::unique_ptr<RSA> ret(new RSA());

                ret->m_key = key;
                ret->m_bio = bio;
                ret->m_pkey = pkey;
                ret->m_ctxEncrypt = ctxEncrypt;
                ret->m_ctxDecrypt = ctxDecrypt;
                ret->m_encrypt = funcEncrypt;
                ret->m_decrypt = funcDecrypt;

                return ret;
            } while (0);

            if (bio)
                BIO_free(bio);

            if (ctxEncrypt)
                EVP_PKEY_CTX_free(ctxEncrypt);

            if (ctxDecrypt)
                EVP_PKEY_CTX_free(ctxDecrypt);

            if (errorString)
                *errorString = errorString0;

            return std::unique_ptr<RSA>();
        }

    private:
        std::string m_key;
        BIO* m_bio = nullptr;
        EVP_PKEY *m_pkey = nullptr;
        EVP_PKEY_CTX* m_ctxEncrypt = nullptr;
        EVP_PKEY_CTX* m_ctxDecrypt = nullptr;

        std::function<int(EVP_PKEY_CTX *, unsigned char *, size_t *, const unsigned char *, size_t)> m_encrypt;
        std::function<int(EVP_PKEY_CTX *, unsigned char *, size_t *, const unsigned char *, size_t)> m_decrypt;
    };

    template <typename Data>
    class AES
    {
    public:
        ~AES()
        {
            EVP_CIPHER_CTX_free(m_ctxEncrypt);
            EVP_CIPHER_CTX_free(m_ctxDecrypt);
        }

        static std::unique_ptr<AES> newInstance(std::string* errorString = nullptr)
        {
            std::string key(16, 0);
            srand(time(NULL));
            for (int i = 0; i < 16; i++)
            {
                unsigned v = rand();
                switch (v & 3)
                {
                case 0:
                case 1:
                    key.data()[i] = '0' + (v % 10);
                break;
                case 2:
                    key.data()[i] = 'a' + (v % 26);
                    break;
                case 3:
                    key.data()[i] = 'A' + (v % 26);
                    break;
                }
            }

            return newInstance(key, errorString);
        }

        static std::unique_ptr<AES> newInstance(const std::string& key, std::string* errorString = nullptr)
        {
            std::string errorString0;
            EVP_CIPHER_CTX *ctxEncrypt = nullptr, *ctxDecrypt = nullptr;

            do
            {
                //encrypt
                ctxEncrypt = EVP_CIPHER_CTX_new();
                int err = EVP_CIPHER_CTX_init(ctxEncrypt);
                CRYPTOLOGY_NEW_INSTANCE_CHECK_ERROR(err);

                err = EVP_EncryptInit_ex(ctxEncrypt, EVP_aes_128_ecb(), NULL, (const unsigned char*)key.c_str(), NULL);
                CRYPTOLOGY_NEW_INSTANCE_CHECK_ERROR(err);

                err = EVP_CIPHER_CTX_set_padding(ctxEncrypt, EVP_PADDING_PKCS7);
                CRYPTOLOGY_NEW_INSTANCE_CHECK_ERROR(err);

                //decrypt
                ctxDecrypt = EVP_CIPHER_CTX_new();
                EVP_CIPHER_CTX_init(ctxDecrypt);

                err = EVP_DecryptInit_ex(ctxDecrypt, EVP_aes_128_ecb(), NULL, (const unsigned char*)key.c_str(), NULL);
                CRYPTOLOGY_NEW_INSTANCE_CHECK_ERROR(err);

                err = EVP_CIPHER_CTX_set_padding(ctxDecrypt, EVP_PADDING_PKCS7);
                CRYPTOLOGY_NEW_INSTANCE_CHECK_ERROR(err);


                std::unique_ptr<AES> ret(new AES());
                ret->m_key = key;
                ret->m_ctxEncrypt = ctxEncrypt;
                ret->m_ctxDecrypt = ctxDecrypt;

                return ret;
            } while (0);

            if (ctxEncrypt)
                EVP_CIPHER_CTX_free(ctxEncrypt);

            if (ctxDecrypt)
                EVP_CIPHER_CTX_free(ctxDecrypt);

            if (errorString)
                *errorString = errorString0;

            return std::unique_ptr<AES>();
        }

        const std::string& key()
        {
            return m_key;
        }

        Result<Data> encrypt(const char* src, int len, Data dst)
        {
            Result<Data> ret;
            ret.data = dst;
            ret.data.ensureWritable(len + 16);

            do
            {
                int out;
                int err = EVP_EncryptUpdate(m_ctxEncrypt, (unsigned char*)ret.data.readableBytes() + ret.data.numReadableBytes(), &out, (const unsigned char*)src, len);
                if (err <= 0)
                    break;
                ret.data.moveWriterIndex(out);

                err = EVP_EncryptFinal_ex(m_ctxEncrypt, (unsigned char*)ret.data.readableBytes() + ret.data.numReadableBytes(), &out);
                if (err <= 0)
                    break;
                ret.data.moveWriterIndex(out);

                ret.isOk = true;
                return ret;
            } while (0);

            ret.isOk = false;
            ret.errorString = ERR_error_string(ERR_get_error(), NULL);
            return ret;
        }

        Result<Data> encrypt(const char* src, int len)
        {
            return encrypt(src, len, Data());
        }

        Result<Data> decrypt(const char* src, int len, Data dst)
        {
            Result<Data> ret;
            ret.data = dst;
            ret.data.ensureWritable(len + 16);
            do
            {
                int out;
                int err = EVP_DecryptUpdate(m_ctxDecrypt, (unsigned char*)ret.data.readableBytes() + ret.data.numReadableBytes(), &out, (unsigned char*)src, len);
                if (err <= 0)
                    break;
                ret.data.moveWriterIndex(out);

                err = EVP_DecryptFinal_ex(m_ctxDecrypt, (unsigned char*)ret.data.readableBytes() + ret.data.numReadableBytes(), &out);
                if (err <= 0)
                    break;
                ret.data.moveWriterIndex(out);

                ret.isOk = true;
                return ret;
            } while (0);

            ret.isOk = false;
            ret.errorString = ERR_error_string(ERR_get_error(), NULL);
            return ret;
        }

        Result<Data> decrypt(const char* src, int len)
        {
            return decrypt(src, len, Data());
        }

    private:
        AES(){}

    private:
        std::string m_key;
        EVP_CIPHER_CTX* m_ctxEncrypt = nullptr;
        EVP_CIPHER_CTX* m_ctxDecrypt = nullptr;
    };
}


#endif // CRYPTOLOGY_H
