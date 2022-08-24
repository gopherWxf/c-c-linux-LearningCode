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

#include "IM.BaseDefine.pb.h"
#include "IM.Login.pb.h"

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
    for(uint32_t i = 0; i < len; i++)
    {
        printf("%02x ", data[i]);
    }
    printf("\n\n");
}

void TInt()
{
    std::string strPb;
    uint8_t *szData;
    IM::Login::TInt32 int1;

    uint32_t int1Size = int1.ByteSize();        // 序列化后的大小

    std::cout << "null int1Size = " << int1Size << std::endl;

    int1.set_int1(0x12);
    int1Size = int1.ByteSize();        // 序列化后的大小
    std::cout << "0x12 int1Size = " << int1Size << std::endl;
    strPb.clear();
    strPb.resize(int1Size);
    szData = (uint8_t *)strPb.c_str();
    int1.SerializeToArray(szData, int1Size);   // 拷贝序列化后的数据
    printHex(szData, int1Size);
    
	int1.set_int1(-11);
    int1Size = int1.ByteSize();        // 序列化后的大小
    std::cout << "-11 int1Size = " << int1Size << std::endl;
    strPb.clear();
    strPb.resize(int1Size);
    szData = (uint8_t *)strPb.c_str();
    int1.SerializeToArray(szData, int1Size);   // 拷贝序列化后的数据
    printHex(szData, int1Size);
	

    int1.set_int1(0x7f);
    int1Size = int1.ByteSize();        // 序列化后的大小
    std::cout << "0xff int1Size = " << int1Size << std::endl;
    strPb.clear();
    strPb.resize(int1Size);
    szData = (uint8_t *)strPb.c_str();
    int1.SerializeToArray(szData, int1Size);   // 拷贝序列化后的数据
    printHex(szData, int1Size);
    

    int1.set_int1(0xff);
    int1Size = int1.ByteSize();        // 序列化后的大小
    std::cout << "0xff int1Size = " << int1Size << std::endl;
    strPb.clear();
    strPb.resize(int1Size);
    szData = (uint8_t *)strPb.c_str();
    int1.SerializeToArray(szData, int1Size);   // 拷贝序列化后的数据
    printHex(szData, int1Size);

    int1.set_int1(0x1234);
    int1Size = int1.ByteSize();        // 序列化后的大小
    std::cout << "0x1234 int1Size = " << int1Size << std::endl;
    strPb.clear();
    strPb.resize(int1Size);
    szData = (uint8_t *)strPb.c_str();
    int1.SerializeToArray(szData, int1Size);   // 拷贝序列化后的数据
    printHex(szData, int1Size);

    int1.set_int1(0x123456);
    int1Size = int1.ByteSize();        // 序列化后的大小
    std::cout << "0x123456 int1Size = " << int1Size << std::endl;
    strPb.clear();
    strPb.resize(int1Size);
    szData = (uint8_t *)strPb.c_str();
    int1.SerializeToArray(szData, int1Size);   // 拷贝序列化后的数据
    printHex(szData, int1Size);
}

void TString(void)
{
    std::string strPb;
    uint8_t *szData;
    IM::Login::TString str1;

    uint32_t str1Size = str1.ByteSize();        // 序列化后的大小

    std::cout << "null str1Size = " << str1Size << std::endl;

    str1.set_str1("1");
    str1Size = str1.ByteSize();                 // 序列化后的大小
    std::cout << "1 str1Size = " << str1Size << std::endl;
    strPb.clear();
    strPb.resize(str1Size);
    szData = (uint8_t *)strPb.c_str();
    str1.SerializeToArray(szData, str1Size);   // 拷贝序列化后的数据
    printHex(szData, str1Size);
    
    str1.set_str1("1234");
    str1Size = str1.ByteSize();                 // 序列化后的大小
    std::cout << "1234 str1Size = " << str1Size << std::endl;
    strPb.clear();
    strPb.resize(str1Size);
    szData = (uint8_t *)strPb.c_str();
    str1.SerializeToArray(szData, str1Size);   // 拷贝序列化后的数据
    printHex(szData, str1Size);

    str1.set_str1("老师");
    str1Size = str1.ByteSize();                 // 序列化后的大小
    std::cout << "老师 str1Size = " << str1Size << std::endl;
    strPb.clear();
    strPb.resize(str1Size);
    szData = (uint8_t *)strPb.c_str();
    str1.SerializeToArray(szData, str1Size);   // 拷贝序列化后的数据
    printHex(szData, str1Size);
}

int main(void)
{
    TInt();
    TString();
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

