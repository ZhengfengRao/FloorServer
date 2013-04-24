/*
 * File: test.h
 * Author: chu
 *
 * Created on 2013年3月22日, 下午9:49
 */

#ifndef TEST_H
#define TEST_H

#include "common.h"

#ifdef ENABLE_AES
    #include "AesWarpper.h"
#endif

void HexDump(const char* str, int len)
{
    printf("Len:%d\n", len);
    for (int i = 0; i < len; i++)
    {
        printf("%x%x ", (str[i] >> 4)& 0xf, str[i]&0xf);
    }
    printf("\n");
}

int test_aes()
{
#ifdef ENABLE_AES
    AesWarpper _warpper;
    std::string _plain = "{\
\"msg\":0,\
\"uid\":\"13286790089_MX13435455\",\
\"data\":\
[\
{\"num\":\"18709671222\", \"name\":\"oooo\",\"status\":2},\
{\"num\":\"18709671233\", \"name\":\"ooXX\",\"status\":2},\
{\"num\":\"18709671244\", \"name\":\"oXXX\",\"status\":2}\
]\
}";

    int _len = _plain.length();
    LOG(LOG_LEVEL_DEBUG, "Original:");
    HexDump(_plain.c_str(), _len);

    unsigned char _en[SOCK_RECV_BUF] = {0};
    _warpper.Encrypt(_plain.c_str(), _en, _len);
    LOG(LOG_LEVEL_DEBUG, "Encrypted:");
    HexDump((const char*) _en, _len);

    unsigned char _de[SOCK_RECV_BUF] = {0};
    if (_warpper.Decrypt((const unsigned char*) _en, (unsigned char*) _de, _len) == true)
    {
        LOG(LOG_LEVEL_DEBUG, "Decrypted:");
        HexDump((const char*) _de, _len);
    }
    else
    {
        LOG(LOG_LEVEL_ERROR, "Decrypt Failed!");
    }
#endif    

    return 0;
}

#endif /* TEST_H */

