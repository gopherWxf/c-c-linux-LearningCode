/*
 * @Descripttion:
 * @version: 1.0
 * @Author: Darren
 * @Date: 2019-10-28 16:44:13
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-06-09 22:08:38
 */
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>

#include "Codec.pb.h"

#define CHECK_PB_PARSE_MSG(ret)                    \
    {                                              \
        if (ret == false)                          \
        {                                          \
            std::cout << "parse pb msg failed.\n"; \
            return;                                \
        }                                          \
    }

static uint64_t getNowTime()
{
    struct timeval tval;
    uint64_t nowTime;

    gettimeofday(&tval, NULL);

    nowTime = tval.tv_sec * 1000L + tval.tv_usec / 1000L;
    return nowTime;
}

void printHex(uint8_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        printf("%02x ", data[i]);
    }
    printf("\n");
}

void Tint32()
{
    std::cout << __FUNCTION__ << " test into " << std::endl;
    std::string strPb;
    uint8_t *szData;
    Codec::Tint32 int1;

    uint32_t int1Size = int1.ByteSize(); // 序列化后的大小

    std::cout << "null int1Size = " << int1Size << std::endl;

    int1.set_n1(666);
    int1Size = int1.ByteSize(); // 序列化后的大小
    std::cout << "666 int1Size = " << int1Size << std::endl;
    strPb.clear();
    strPb.resize(int1Size);
    szData = (uint8_t *)strPb.c_str();
    int1.SerializeToArray(szData, int1Size); // 拷贝序列化后的数据
    printHex(szData, int1Size);              // 08 01

    int1.set_n1(0x1);
    int1Size = int1.ByteSize(); // 序列化后的大小
    std::cout << "0x1 int1Size = " << int1Size << std::endl;
    strPb.clear();
    strPb.resize(int1Size);
    szData = (uint8_t *)strPb.c_str();
    int1.SerializeToArray(szData, int1Size); // 拷贝序列化后的数据
    printHex(szData, int1Size);              // 08 01

    int1.set_n1(0x12);
    int1Size = int1.ByteSize(); // 序列化后的大小
    std::cout << "0x12 int1Size = " << int1Size << std::endl;
    strPb.clear();
    strPb.resize(int1Size);
    szData = (uint8_t *)strPb.c_str();
    int1.SerializeToArray(szData, int1Size); // 拷贝序列化后的数据
    printHex(szData, int1Size);              // 08 12

    int1.set_n1(-5);
    int1Size = int1.ByteSize(); // 序列化后的大小, 占用11字节
    std::cout << "-5 int1Size = " << int1Size << std::endl;
    strPb.clear();
    strPb.resize(int1Size);
    szData = (uint8_t *)strPb.c_str();
    int1.SerializeToArray(szData, int1Size); // 拷贝序列化后的数据
    printHex(szData, int1Size);              // 08 fb ff ff ff ff ff ff ff ff 01  低位在前

    int1.set_n1(0x7f);
    int1Size = int1.ByteSize(); // 序列化后的大小
    std::cout << "0x7f int1Size = " << int1Size << std::endl;
    strPb.clear();
    strPb.resize(int1Size);
    szData = (uint8_t *)strPb.c_str();
    int1.SerializeToArray(szData, int1Size); // 拷贝序列化后的数据
    printHex(szData, int1Size);              // 08 7f

    int1.set_n1(0xff);
    int1Size = int1.ByteSize(); // 序列化后的大小
    std::cout << "0xff int1Size = " << int1Size << std::endl;
    strPb.clear();
    strPb.resize(int1Size);
    szData = (uint8_t *)strPb.c_str();
    int1.SerializeToArray(szData, int1Size); // 拷贝序列化后的数据
    printHex(szData, int1Size);              // 08 ff 01  ->  0x01 << 7 | 0x7f = 0xff

    int1.set_n1(0x1234);
    int1Size = int1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234 int1Size = " << int1Size << std::endl;
    strPb.clear();
    strPb.resize(int1Size);
    szData = (uint8_t *)strPb.c_str();
    int1.SerializeToArray(szData, int1Size); // 拷贝序列化后的数据
    printHex(szData, int1Size);              // 08 b4 24 -> 0x24 << 7 | 0xb4 = 0b 00100100 0110100 = 0x1234

    int1.set_n1(0x123456);
    int1Size = int1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456 int1Size = " << int1Size << std::endl;
    strPb.clear();
    strPb.resize(int1Size);
    szData = (uint8_t *)strPb.c_str();
    int1.SerializeToArray(szData, int1Size); // 拷贝序列化后的数据
    printHex(szData, int1Size);

    int1.set_n1(0x1234567);
    int1Size = int1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234567 int1Size = " << int1Size << std::endl;
    strPb.clear();
    strPb.resize(int1Size);
    szData = (uint8_t *)strPb.c_str();
    int1.SerializeToArray(szData, int1Size); // 拷贝序列化后的数据
    printHex(szData, int1Size);

    int1.set_n1(0x12345678);
    int1Size = int1.ByteSize(); // 序列化后的大小
    std::cout << "0x12345678 int1Size = " << int1Size << std::endl;
    strPb.clear();
    strPb.resize(int1Size);
    szData = (uint8_t *)strPb.c_str();
    int1.SerializeToArray(szData, int1Size); // 拷贝序列化后的数据
    printHex(szData, int1Size);

    int1.set_n1(0x123456789);
    int1Size = int1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456789 int1Size = " << int1Size << std::endl;
    strPb.clear();
    strPb.resize(int1Size);
    szData = (uint8_t *)strPb.c_str();
    int1.SerializeToArray(szData, int1Size); // 拷贝序列化后的数据
    printHex(szData, int1Size);

    int1.set_n1(0x98765432);
    int1Size = int1.ByteSize(); // 序列化后的大小
    std::cout << "0x98765432 int1Size = " << int1Size << std::endl;
    strPb.clear();
    strPb.resize(int1Size);
    szData = (uint8_t *)strPb.c_str();
    int1.SerializeToArray(szData, int1Size); // 拷贝序列化后的数据
    printHex(szData, int1Size);

    Codec::Tint32 int2;
    CHECK_PB_PARSE_MSG(int2.ParseFromArray((uint8_t *)strPb.c_str(), strPb.size()));
    std::cout << "反序列化后: n1 = " << std::hex << int2.n1() << std::endl;

    std::cout << __FUNCTION__ << " test leave \n"
              << std::endl;
}

void Tint64()
{
    std::cout << __FUNCTION__ << " test into " << std::endl;
    std::string strPb;
    uint8_t *szData;
    Codec::Tint64 n1;

    uint32_t n1Size = n1.ByteSize(); // 序列化后的大小

    std::cout << "null n1Size = " << n1Size << std::endl;

    n1.set_n1(0x1);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 01

    n1.set_n1(0x12);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x12 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 12

    n1.set_n1(-5);
    n1Size = n1.ByteSize(); // 序列化后的大小, 占用11字节
    std::cout << "-5 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x7f);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x7f n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 7f

    n1.set_n1(0xff);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0xff n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x1234);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x123456);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x1234567);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234567 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x12345678);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x12345678 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x123456789);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456789 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x23456789);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x23456789 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    Codec::Tint32 int2;
    CHECK_PB_PARSE_MSG(int2.ParseFromArray((uint8_t *)strPb.c_str(), strPb.size()));
    std::cout << "反序列化后: n1 = " << std::hex << int2.n1() << std::endl;

    std::cout << __FUNCTION__ << " test leave \n"
              << std::endl;
}

void Tuint32()
{
    std::cout << __FUNCTION__ << " test into " << std::endl;
    std::string strPb;
    uint8_t *szData;
    Codec::Tuint32 n1;

    uint32_t n1Size = n1.ByteSize(); // 序列化后的大小

    std::cout << "null n1Size = " << n1Size << std::endl;

    n1.set_n1(0x1);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 01

    n1.set_n1(0x12);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x12 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 12

    n1.set_n1(-5);
    n1Size = n1.ByteSize(); // 序列化后的大小, 占用11字节
    std::cout << "-5 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x7f);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x7f n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 7f

    n1.set_n1(0xff);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0xff n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x1234);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x123456);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x1234567);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234567 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x12345678);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x12345678 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x123456789);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456789 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x98765432);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x98765432 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    Codec::Tuint32 n2;
    CHECK_PB_PARSE_MSG(n2.ParseFromArray((uint8_t *)strPb.c_str(), strPb.size()));
    std::cout << "反序列化后: n1 = " << std::hex << n2.n1() << std::endl;

    std::cout << __FUNCTION__ << " test leave \n"
              << std::endl;
}

void Tuint64()
{
    std::cout << __FUNCTION__ << " test into " << std::endl;
    std::string strPb;
    uint8_t *szData;
    Codec::Tuint64 n1;

    uint32_t n1Size = n1.ByteSize(); // 序列化后的大小

    std::cout << "null n1Size = " << n1Size << std::endl;

    n1.set_n1(0x1);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 01

    n1.set_n1(0x12);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x12 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 12

    n1.set_n1(-5);
    n1Size = n1.ByteSize(); // 序列化后的大小, 占用11字节
    std::cout << "-5 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x7f);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x7f n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 7f

    n1.set_n1(0xff);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0xff n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x1234);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x123456);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x1234567);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234567 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x12345678);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x12345678 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x123456789);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456789 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x98765432);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x98765432 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    Codec::Tuint64 n2;
    CHECK_PB_PARSE_MSG(n2.ParseFromArray((uint8_t *)strPb.c_str(), strPb.size()));
    std::cout << "反序列化后: n1 = " << std::hex << n2.n1() << std::endl;

    std::cout << __FUNCTION__ << " test leave \n"
              << std::endl;
}

// 使用变长编码，这些编码在负值时比int32高效的多
void Tsint32()
{
    std::cout << __FUNCTION__ << " test into " << std::endl;
    std::string strPb;
    uint8_t *szData;
    Codec::Tsint32 n1;

    uint32_t n1Size = n1.ByteSize(); // 序列化后的大小

    std::cout << "null n1Size = " << n1Size << std::endl;

    n1.set_n1(0x1);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 01

    n1.set_n1(0x12);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x12 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 12

    n1.set_n1(-5);
    n1Size = n1.ByteSize(); // 序列化后的大小, 占用11字节
    std::cout << "-5 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x7f);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x7f n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 7f

    n1.set_n1(0xff);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0xff n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x1234);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x123456);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x1234567);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234567 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x12345678);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x12345678 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x123456789);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456789 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x98765432);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x98765432 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    Codec::Tsint32 n2;
    CHECK_PB_PARSE_MSG(n2.ParseFromArray((uint8_t *)strPb.c_str(), strPb.size()));
    std::cout << "反序列化后: n1 = " << std::hex << n2.n1() << std::endl;

    std::cout << __FUNCTION__ << " test leave \n"
              << std::endl;
}

void Tsint64()
{
    std::cout << __FUNCTION__ << " test into " << std::endl;
    std::string strPb;
    uint8_t *szData;
    Codec::Tsint64 n1;

    uint32_t n1Size = n1.ByteSize(); // 序列化后的大小

    std::cout << "null n1Size = " << n1Size << std::endl;

    n1.set_n1(0x1);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 01

    n1.set_n1(0x12);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x12 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 12

    n1.set_n1(-5);
    n1Size = n1.ByteSize(); // 序列化后的大小, 占用11字节
    std::cout << "-5 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x7f);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x7f n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 7f

    n1.set_n1(0xff);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0xff n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x1234);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x123456);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x1234567);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234567 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x12345678);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x12345678 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x123456789);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456789 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x98765432);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x98765432 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    Codec::Tsint64 n2;
    CHECK_PB_PARSE_MSG(n2.ParseFromArray((uint8_t *)strPb.c_str(), strPb.size()));
    std::cout << "反序列化后: n1 = " << std::hex << n2.n1() << std::endl;

    std::cout << __FUNCTION__ << " test leave \n"
              << std::endl;
}

void Tbool()
{
    std::cout << __FUNCTION__ << " test into " << std::endl;
    std::string strPb;
    uint8_t *szData;
    Codec::Tbool n1;

    uint32_t n1Size = n1.ByteSize(); // 序列化后的大小

    std::cout << "null n1Size = " << n1Size << std::endl;

    n1.set_n1(true);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "true n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 01

    n1.set_n1(false);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "false n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    Codec::Tbool n2;
    CHECK_PB_PARSE_MSG(n2.ParseFromArray((uint8_t *)strPb.c_str(), strPb.size()));
    std::cout << "反序列化后: n1 = " << std::hex << n2.n1() << std::endl;

    std::cout << __FUNCTION__ << " test leave \n"
              << std::endl;
}

void Tenum()
{
    std::cout << __FUNCTION__ << " test into " << std::endl;
    std::string strPb;
    uint8_t *szData;
    Codec::Tenum n1;

    uint32_t n1Size = n1.ByteSize(); // 序列化后的大小

    std::cout << "null n1Size = " << n1Size << std::endl;

    n1.set_n1(Codec::PhoneType::PHONE_HOME);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "PHONE_HOME n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    Codec::Tbool n2;
    CHECK_PB_PARSE_MSG(n2.ParseFromArray((uint8_t *)strPb.c_str(), strPb.size()));
    std::cout << "反序列化后: n1 = " << std::hex << n2.n1() << std::endl;

    std::cout << __FUNCTION__ << " test leave \n"
              << std::endl;
}

void Tfixed64()
{
    std::cout << __FUNCTION__ << " test into " << std::endl;
    std::string strPb;
    uint8_t *szData;
    Codec::Tfixed64 n1;

    uint32_t n1Size = n1.ByteSize(); // 序列化后的大小

    std::cout << "null n1Size = " << n1Size << std::endl;

    n1.set_n1(0x1);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 01

    n1.set_n1(0x12);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x12 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 12

    n1.set_n1(-5);
    n1Size = n1.ByteSize(); // 序列化后的大小, 占用11字节
    std::cout << "-5 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x7f);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x7f n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 7f

    n1.set_n1(0xff);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0xff n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x1234);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x123456);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x1234567);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234567 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x12345678);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x12345678 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x123456789);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456789 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x98765432);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x98765432 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    Codec::Tfixed64 n2;
    CHECK_PB_PARSE_MSG(n2.ParseFromArray((uint8_t *)strPb.c_str(), strPb.size()));
    std::cout << "反序列化后: n1 = " << std::hex << n2.n1() << std::endl;

    std::cout << __FUNCTION__ << " test leave \n"
              << std::endl;
}

void Tsfixed64()
{
    std::cout << __FUNCTION__ << " test into " << std::endl;
    std::string strPb;
    uint8_t *szData;
    Codec::Tsfixed64 n1;

    uint32_t n1Size = n1.ByteSize(); // 序列化后的大小

    std::cout << "null n1Size = " << n1Size << std::endl;

    n1.set_n1(0x1);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 01

    n1.set_n1(0x12);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x12 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 12

    n1.set_n1(-5);
    n1Size = n1.ByteSize(); // 序列化后的大小, 占用11字节
    std::cout << "-5 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x7f);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x7f n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 7f

    n1.set_n1(0xff);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0xff n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x1234);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x123456);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x1234567);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234567 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x12345678);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x12345678 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x123456789);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456789 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x98765432);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x98765432 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    Codec::Tsfixed64 n2;
    CHECK_PB_PARSE_MSG(n2.ParseFromArray((uint8_t *)strPb.c_str(), strPb.size()));
    std::cout << "反序列化后: n1 = " << std::hex << n2.n1() << std::endl;

    std::cout << __FUNCTION__ << " test leave \n"
              << std::endl;
}

void Tdouble()
{
    std::cout << __FUNCTION__ << " test into " << std::endl;
    std::string strPb;
    uint8_t *szData;
    Codec::Tdouble n1;

    uint32_t n1Size = n1.ByteSize(); // 序列化后的大小

    std::cout << "null n1Size = " << n1Size << std::endl;

    n1.set_n1(0x1);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 01

    n1.set_n1(0x12);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x12 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 12

    n1.set_n1(-5);
    n1Size = n1.ByteSize(); // 序列化后的大小, 占用11字节
    std::cout << "-5 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x7f);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x7f n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 7f

    n1.set_n1(0xff);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0xff n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x1234);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x123456);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x1234567);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234567 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x12345678);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x12345678 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x123456789);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456789 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x98765432);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x98765432 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    Codec::Tdouble n2;
    CHECK_PB_PARSE_MSG(n2.ParseFromArray((uint8_t *)strPb.c_str(), strPb.size()));
    printf("n2 = %lf\n", n2.n1());
    std::cout << "反序列化后: 0x98765432 = " << n2.n1() << std::endl;

    std::cout << __FUNCTION__ << " test leave \n"
              << std::endl;
}

void Tstring(void)
{
    std::cout << __FUNCTION__ << " test into -------------------->" << std::endl;
    std::string strPb;
    uint8_t *szData;
    Codec::Tstring n1;

    uint32_t str1Size = n1.ByteSize(); // 序列化后的大小

    std::cout << "null str1Size = " << str1Size << std::endl;

    n1.set_n1("1");
    str1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "1 str1Size = " << str1Size << std::endl;
    strPb.clear();
    strPb.resize(str1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, str1Size); // 拷贝序列化后的数据
    printHex(szData, str1Size);

    n1.set_n1("1234");
    str1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "1234 str1Size = " << str1Size << std::endl;
    strPb.clear();
    strPb.resize(str1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, str1Size); // 拷贝序列化后的数据
    printHex(szData, str1Size);

    n1.set_n1("老师");
    str1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "老师 str1Size = " << str1Size << std::endl;
    strPb.clear();
    strPb.resize(str1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, str1Size); // 拷贝序列化后的数据
    printHex(szData, str1Size);

    std::cout << __FUNCTION__ << " test leave \n"
              << std::endl;
}

void Tbytes(void)
{
    std::cout << __FUNCTION__ << " test into -------------------->" << std::endl;
    std::string strPb;
    uint8_t *szData;
    Codec::Tbytes n1;

    uint32_t str1Size = n1.ByteSize(); // 序列化后的大小

    std::cout << "null str1Size = " << str1Size << std::endl;

    n1.set_n1("1");
    str1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "1 str1Size = " << str1Size << std::endl;
    strPb.clear();
    strPb.resize(str1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, str1Size); // 拷贝序列化后的数据
    printHex(szData, str1Size);

    n1.set_n1("1234");
    str1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "1234 str1Size = " << str1Size << std::endl;
    strPb.clear();
    strPb.resize(str1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, str1Size); // 拷贝序列化后的数据
    printHex(szData, str1Size);

    n1.set_n1("012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789");
    str1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "老师 str1Size = " << str1Size << std::endl;
    strPb.clear();
    strPb.resize(str1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, str1Size); // 拷贝序列化后的数据
    printHex(szData, str1Size);

    std::cout << __FUNCTION__ << " test leave \n"
              << std::endl;
}

void Tembeddedmessages(void)
{
    std::cout << __FUNCTION__ << " test into -------------------->" << std::endl;
    std::string strPb;
    uint8_t *szData;
    Codec::Tembeddedmessages n1;

    uint32_t n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "null str1Size = " << n1Size << std::endl;

    // protobuf mutable_* 函数
    // 从该函数的实现上来看，该函数返回指向该字段的一个指针。同时将该字段置为被设置状态。
    // 若该对象存在，则直接返回该对象，若不存在则新new 一个。
    Codec::Tembeddedmessages::TMsg *tmsg = n1.mutable_n1();
    tmsg->set_n1(100);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456789 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    Codec::Tembeddedmessages n2;
    CHECK_PB_PARSE_MSG(n2.ParseFromArray((uint8_t *)strPb.c_str(), strPb.size()));

    std::cout << "反序列化后  " << n2.n1().n1() << std::endl;

    std::cout << __FUNCTION__ << " test leave \n"
              << std::endl;
}

void TRepeatedfields(void)
{
    std::cout << __FUNCTION__ << " test into -------------------->" << std::endl;
    std::string strPb;
    uint8_t *szData;
    Codec::TRepeatedfields n1;

    uint32_t n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "null str1Size = " << n1Size << std::endl;

    // 添加Repeated int
    for (int i = 0; i < 5; i++)
    {
        n1.add_n1(i);
    }
    printf("TRepeatedfields int32 size:%d\n", n1.n1_size());

    // 添加Repeated Tbytes
    Codec::Tbytes *bytes = n1.add_n2();
    bytes->set_n1("darren-1");
    bytes = n1.add_n2();
    bytes->set_n1("darren-2");
    bytes = n1.add_n2();
    bytes->set_n1("darren-3");

    printf("TRepeatedfields Tbytes size:%d\n", n1.n2_size());
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "TRepeatedfields n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    std::cout << "反序列化后  " << std::endl;
    Codec::TRepeatedfields n2;
    CHECK_PB_PARSE_MSG(n2.ParseFromArray((uint8_t *)strPb.c_str(), strPb.size()));

    // repeated int32   n1  ->  n2.n1_size()
    for (int i = 0; i < n2.n1_size(); i++)
    {
        printf("int32 i: %d\n", n2.n1(i));
    }
    // repeated Tbytes  n2  -> n2.n2_size() 虽然是对象，但是他是repeated的，没有has
    for (int i = 0; i < n2.n2_size(); i++)
    {
        const Codec::Tbytes &bytes = n2.n2(i);
        printf("bytes %d: %s\n", i, bytes.n1().c_str()); // 返回的是string类型
    }

    std::cout << __FUNCTION__ << " test leave \n"
              << std::endl;
}

void Tfixed32()
{
    std::cout << __FUNCTION__ << " test into " << std::endl;
    std::string strPb;
    uint8_t *szData;
    Codec::Tfixed32 n1;

    uint32_t n1Size = n1.ByteSize(); // 序列化后的大小

    std::cout << "null n1Size = " << n1Size << std::endl;

    n1.set_n1(0x1);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 01

    n1.set_n1(0x12);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x12 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 12

    n1.set_n1(-5);
    n1Size = n1.ByteSize(); // 序列化后的大小, 占用11字节
    std::cout << "-5 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x7f);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x7f n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 7f

    n1.set_n1(0xff);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0xff n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x1234);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x123456);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x1234567);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234567 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x12345678);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x12345678 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x123456789);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456789 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x98765432);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x98765432 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    Codec::Tfixed32 n2;
    CHECK_PB_PARSE_MSG(n2.ParseFromArray((uint8_t *)strPb.c_str(), strPb.size()));
    std::cout << "反序列化后: n1 = " << std::hex << n2.n1() << std::endl;

    std::cout << __FUNCTION__ << " test leave \n"
              << std::endl;
}

void Tsfixed32()
{
    std::cout << __FUNCTION__ << " test into " << std::endl;
    std::string strPb;
    uint8_t *szData;
    Codec::Tsfixed32 n1;

    uint32_t n1Size = n1.ByteSize(); // 序列化后的大小

    std::cout << "null n1Size = " << n1Size << std::endl;

    n1.set_n1(0x1);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 01

    n1.set_n1(0x12);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x12 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 12

    n1.set_n1(-5);
    n1Size = n1.ByteSize(); // 序列化后的大小, 占用11字节
    std::cout << "-5 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x7f);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x7f n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 7f

    n1.set_n1(0xff);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0xff n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x1234);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x123456);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x1234567);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234567 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x12345678);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x12345678 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x123456789);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456789 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x98765432);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x98765432 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    Codec::Tsfixed32 n2;
    CHECK_PB_PARSE_MSG(n2.ParseFromArray((uint8_t *)strPb.c_str(), strPb.size()));
    std::cout << "反序列化后: n1 = " << std::hex << n2.n1() << std::endl;

    std::cout << __FUNCTION__ << " test leave \n"
              << std::endl;
}

void Tfloat()
{
    std::cout << __FUNCTION__ << " test into " << std::endl;
    std::string strPb;
    uint8_t *szData;
    Codec::Tfloat n1;

    uint32_t n1Size = n1.ByteSize(); // 序列化后的大小

    std::cout << "null n1Size = " << n1Size << std::endl;

    n1.set_n1(0x1);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 01

    n1.set_n1(0x12);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x12 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 12

    n1.set_n1(-5);
    n1Size = n1.ByteSize(); // 序列化后的大小, 占用11字节
    std::cout << "-5 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x7f);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x7f n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            // 08 7f

    n1.set_n1(0xff);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0xff n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x1234);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);            //

    n1.set_n1(0x123456);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x1234567);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x1234567 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x12345678);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x12345678 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x123456789);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x123456789 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    n1.set_n1(0x98765432);
    n1Size = n1.ByteSize(); // 序列化后的大小
    std::cout << "0x98765432 n1Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    n1.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    Codec::Tfloat n2;
    CHECK_PB_PARSE_MSG(n2.ParseFromArray((uint8_t *)strPb.c_str(), strPb.size()));
    printf("n2 = %f\n", n2.n1());
    std::cout << "反序列化后: 0x98765432 = " << n2.n1() << std::endl;

    std::cout << __FUNCTION__ << " test leave \n"
              << std::endl;
}
// null THasitem Size = 0
// THasitem  Size = 3
// 1  -> 08 01
// 16 -> 80 01 01   10000000  0000 001  00000001
// 64 -> 80 04 01         01000                                 1000 0000   0000 0100
// 65 -> 88 04 01          number:01000001 ，低位在前，高位在后  1000 1000   0000 0100
// 反序列化后
// n1_int32  1
// n2_int32  0
// n3_uint32  0
// n4_sint32  0
// n5_fixed32  0
// n7_bytes
// has_n8_tbytes  0
// n9_int32  0
// THasitem test leave

void THasitem(void)
{
    std::cout << __FUNCTION__ << " test into -------------------->" << std::endl;
    std::string strPb;
    uint8_t *szData;
    Codec::THasitem hasitem;

    uint32_t n1Size = hasitem.ByteSize(); // 序列化后的大小
    std::cout << "null THasitem Size = " << n1Size << std::endl;

    hasitem.set_n1_int32(1);

    n1Size = hasitem.ByteSize(); // 序列化后的大小
    std::cout << "THasitem  Size = " << n1Size << std::endl;
    strPb.clear();
    strPb.resize(n1Size);
    szData = (uint8_t *)strPb.c_str();
    hasitem.SerializeToArray(szData, n1Size); // 拷贝序列化后的数据
    printHex(szData, n1Size);

    Codec::THasitem hasitem2;
    CHECK_PB_PARSE_MSG(hasitem2.ParseFromArray((uint8_t *)strPb.c_str(), strPb.size()));

    std::cout << "反序列化后  " << std::endl;
    std::cout << "n1_int32  " << hasitem2.n1_int32() << std::endl;
    std::cout << "n2_int32  " << hasitem2.n2_int32() << std::endl;
    std::cout << "n3_uint32  " << hasitem2.n3_uint32() << std::endl;
    std::cout << "n4_sint32  " << hasitem2.n4_sint32() << std::endl;
    std::cout << "n5_fixed32  " << hasitem2.n5_fixed32() << std::endl;
    std::cout << "n7_bytes  " << hasitem2.n7_bytes() << std::endl;
    std::cout << "has_n8_tbytes  " << hasitem2.has_n8_tbytes() << std::endl;
    if (hasitem2.has_n8_tbytes())
    { //只有对应字段是对象，而是单个对象的时候才有has_xxx的判断
        std::cout << "n8_Tbytes  " << hasitem2.n8_tbytes().n1() << std::endl;
    }
    std::cout << "n9_int32  " << hasitem2.n9_int32() << std::endl;

    std::cout << __FUNCTION__ << " test leave \n"
              << std::endl;
}

int main(int argc, char *argv[])
{
    int type = -1;
    if (argc == 2)
    {
        type = atoi(argv[1]);
    }
    std::cout << __FUNCTION__ << "type = " << type << std::endl;

    if (type == 0 || type == -1)
    { 
        // type 0
        std::cout << __FUNCTION__ << " type = 0 ---------------------------------------------------" << std::endl;
        Tint32();
        Tint64();
        Tuint32();
        Tuint64();
        Tsint32();
        Tsint64();
        Tbool();
        Tenum();
    }
    if (type == 1 || type == -1)
    {
        // Type = 1  没有用base128
         std::cout << __FUNCTION__ << " type = 1 ---------------------------------------------------" << std::endl;
        Tfixed64();  // uint64
        Tsfixed64(); // int64
        Tdouble();
    }
    if (type == 2 || type == -1)
    {
        // Type = 2
         std::cout << __FUNCTION__ << " type = 2 ---------------------------------------------------" << std::endl;
        Tstring(); // 检测是否为utf8 VerifyUTF8StringNamedField, 在确定字段编码格式后直接使用bytes，减少utf8编码的判断
        Tbytes();
        Tembeddedmessages();
        TRepeatedfields();
    }

    if (type == 5 || type == -1)
    {
        // Type = 5 没有用base128
         std::cout << __FUNCTION__ << " type = 5 ---------------------------------------------------" << std::endl;
        Tfixed32();  // uint32  没有用base128
        Tsfixed32(); // int32
        Tfloat();
        THasitem();
    }
    return 0;
}

/*
null int1Size = 0
0x12 int1Size = 2
08 12

0xff int1Size = 2
08 7f

0xff int1Size = 3
08 ff 01

0x1234 int1Size = 3
08 b4 24

0x123456 int1Size = 4
08 d6 e8 48

null str1Size = 0
1 str1Size = 3
0a 01 31

1234 str1Size = 6
0a 04 31 32 33 34

老师 str1Size = 8
0a 06 e8 80 81 e5 b8 88
*/
