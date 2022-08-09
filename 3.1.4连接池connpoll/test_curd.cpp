/*
 * @Author: your name
 * @Date: 2020-01-09 11:54:16
 * @LastEditTime : 2020-01-09 21:09:18
 * @LastEditors  : Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \mysql-pool\test_DBPool.cpp
 */
#include "DBPool.h"
#include "IMUser.h"

using namespace std;
// 登录数据库: mysql -u root -p 
// 创建数据库:  CREATE DATABASE 数据库名;
// 删除数据库: DROP DATABASE 数据库名;
// 显示所有的数据库: show DATABASE;
// 进入某个数据库: USE 数据库名;
// 查看所有表: SHOW tables 
// 查看某个表结构: 

#define DB_HOST_IP          "127.0.0.1"             // 数据库服务器ip
#define DB_HOST_PORT        3306
#define DB_DATABASE_NAME    "mysql_pool_test"       // 数据库对应的库名字, 这里需要自己提前用命令创建完毕
#define DB_USERNAME         "root"                  // 数据库用户名
#define DB_PASSWORD         "123456"                // 数据库密码
#define DB_POOL_NAME        "mysql_pool"            // 连接池的名字，便于将多个连接池集中管理
#define DB_POOL_MAX_CON     4                       // 连接池支持的最大连接数量


// #define DB_HOST_IP          "114.215.169.66"             // 数据库服务器ip
// #define DB_HOST_PORT        3306
// #define DB_DATABASE_NAME    "mysql_pool_test"       // 数据库对应的库名字, 这里需要自己提前用命令创建完毕
// #define DB_USERNAME         "root"                  // 数据库用户名
// #define DB_PASSWORD         "23232"                // 数据库密码
// #define DB_POOL_NAME        "mysql_pool"            // 连接池的名字，便于将多个连接池集中管理
// #define DB_POOL_MAX_CON     4                       // 连接池支持的最大连接数量



static uint32_t IMUser_nId = 0;

// 把连接传递进去
bool insertUser(CDBConn *pDBConn) {
    bool bRet = false;
    string strSql;
    strSql = "insert into IMUser(`salt`,`sex`,`nick`,`password`,`domain`,`name`,`phone`,`email`,`company`,`address`,`avatar`,`sign_info`,`departId`,`status`,`created`,`updated`) values(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";

    CPrepareStatement *stmt = new CPrepareStatement();
    if (stmt->Init(pDBConn->GetMysql(), strSql)) {
        uint32_t nNow = (uint32_t) time(NULL);
        uint32_t index = 0;
        string strOutPass = "987654321";
        string strSalt = "abcd";

        int nSex = 1;// 用户性别 1.男;2.女
        int nStatus = 0; // 用户状态0 正常， 1 离职
        uint32_t nDeptId = 0;// 所属部门
        string strNick = "明华";// 花名
        string strDomain = "minghua";// 花名拼音
        string strName = "廖庆富";// 真名
        string strTel = "18570368134";// 手机号码
        string strEmail = "326873713@qq.com";// Email
        string strAvatar = "";// 头像
        string sign_info = "一切只为你";//个性签名
        string strPass = "123456"; //密码
        string strCompany = "零声学院"; //公司
        string strAddress = "长沙岳麓区麓谷企业广场"; //地址

        stmt->SetParam(index++, strSalt);
        stmt->SetParam(index++, nSex);
        stmt->SetParam(index++, strNick);
        stmt->SetParam(index++, strOutPass);
        stmt->SetParam(index++, strDomain);
        stmt->SetParam(index++, strName);
        stmt->SetParam(index++, strTel);
        stmt->SetParam(index++, strEmail);
        stmt->SetParam(index++, strCompany);
        stmt->SetParam(index++, strAddress);
        stmt->SetParam(index++, strAvatar);
        stmt->SetParam(index++, sign_info);
        stmt->SetParam(index++, nDeptId);
        stmt->SetParam(index++, nStatus);
        stmt->SetParam(index++, nNow);
        stmt->SetParam(index++, nNow);
        bRet = stmt->ExecuteUpdate();

        if (!bRet) {
            printf("insert user failed: %s\n", strSql.c_str());
        }
        else {
            IMUser_nId = stmt->GetInsertId();
            printf("register then get user_id:%d\n", IMUser_nId);
        }
    }
    delete stmt;

    return bRet;
}

bool queryUser(CDBConn *pDBConn) {
    string strSql = "select * from IMUser where id=" + int2string(IMUser_nId);
    CResultSet *pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
    bool bRet = false;
    if (pResultSet) {
        if (pResultSet->Next()) {
            uint32_t nId;//用户ID
            uint8_t nSex;// 用户性别 1.男;2.女
            uint8_t nStatus; // 用户状态0 正常， 1 离职
            uint32_t nDeptId;// 所属部门
            string strNick;// 花名
            string strDomain;// 花名拼音
            string strName;// 真名
            string strTel;// 手机号码
            string strEmail;// Email
            string strAvatar;// 头像
            string sign_info;//个性签名
            string strPass; //密码
            string strCompany; //公司
            string strAddress; //地址
            nId = pResultSet->GetInt("id");
            nSex = pResultSet->GetInt("sex");
            strNick = pResultSet->GetString("nick");
            strDomain = pResultSet->GetString("domain");
            strName = pResultSet->GetString("name");
            strTel = pResultSet->GetString("phone");
            strEmail = pResultSet->GetString("email");
            strAvatar = pResultSet->GetString("avatar");
            sign_info = pResultSet->GetString("sign_info");
            nDeptId = (uint32_t) pResultSet->GetInt("departId");
            nStatus = (uint32_t) pResultSet->GetInt("status");

            printf("nId:%u\n", nId);
            printf("nSex:%u\n", nSex);
            printf("strNick:%s\n", strNick.c_str());
            printf("strDomain:%s\n", strDomain.c_str());
            printf("strName:%s\n", strName.c_str());
            printf("nDeptId:%u\n", nDeptId);
            printf("nStatus:%u\n", nStatus);
            printf("strTel:%s\n", strTel.c_str());
            printf("strEmail:%s\n", strEmail.c_str());
            printf("sign_info:%s\n", sign_info.c_str());

            bRet = true;
        }
        delete pResultSet;
    }
    else {
        printf("no result set for sql:%s\n", strSql.c_str());
    }

    return bRet;
}

bool updateUser(CDBConn *pDBConn) {
    bool bRet = false;
    if (pDBConn) {
        int nSex = 1;// 用户性别 1.男;2.女
        int nStatus = 0; // 用户状态0 正常， 1 离职
        uint32_t nDeptId = 0;// 所属部门
        string strNick = "mark";// 花名
        string strDomain = "mark";// 花名拼音
        string strName = "谢帆";// 真名
        string strTel = "18570368134";// 手机号码
        string strEmail = "517609429@qq.com";// Email
        string strAvatar = "";// 头像
        string sign_info = "一切只为你";//个性签名
        string strPass = "123456"; //密码
        string strCompany = "零声学院"; //公司
        string strAddress = "长沙岳麓区麓谷企业广场"; //地址

        uint32_t nNow = (uint32_t) time(NULL);
        string strSql = "update IMUser set `sex`=" + int2string(nSex) + ", `nick`='" + strNick
                        + "', `domain`='" + strDomain + "', `name`='" + strName + "', `phone`='" + strTel
                        + "', `email`='" + strEmail + "', `avatar`='" + strAvatar + "', `sign_info`='" + sign_info
                        + "', `departId`='" + int2string(nDeptId) + "', `status`=" + int2string(nStatus) +
                        ", `updated`=" + int2string(nNow)
                        + ", `company`='" + strCompany + "', `address`='" + strAddress + "' where id=" +
                        int2string(IMUser_nId);
        bRet = pDBConn->ExecuteUpdate(strSql.c_str());
        if (!bRet) {
            printf("updateUser: update failed:%s\n", strSql.c_str());
        }
    }
    else {
        printf("no db connection!\n");
    }
    return bRet;
}

// 测试本地的数据库连接
void testConnect() {
    const char *db_pool_name = DB_POOL_NAME;
    const char *db_host = DB_HOST_IP;
    int db_port = DB_HOST_PORT;
    const char *db_dbname = DB_DATABASE_NAME;
    const char *db_username = DB_USERNAME;
    const char *db_password = DB_PASSWORD;
    int db_maxconncnt = DB_POOL_MAX_CON;
    CDBPool *pDBPool = new CDBPool(db_pool_name, db_host, db_port,
                                   db_username, db_password, db_dbname, db_maxconncnt);
    if (pDBPool->Init()) {
        printf("init db instance failed: %s", db_pool_name);
        return;
    }

    CDBConn *pDBConn = pDBPool->GetDBConn();
    if (pDBConn) {
        bool ret = pDBConn->ExecuteDrop(DROP_IMUSER_TABLE);
        if (ret) {
            printf("DROP_IMUSER_TABLE ok\n");
        }
        // 1. 创建表
        ret = pDBConn->ExecuteCreate(CREATE_IMUSER_TABLE);
        if (ret) {
            printf("CREATE_IMUSER_TABLE ok\n");
        }

        pDBPool->RelDBConn(pDBConn);
    }
    else {
        printf("pDBConn is null\n");
    }
    delete pDBPool;
}

// 测试本地数据库的创建（Create）、更新（Update）、读取（Retrieve）和删除（Delete）操作。
// 测试增删改查
void testCurd() {
    const char *db_pool_name = DB_POOL_NAME;
    const char *db_host = DB_HOST_IP;
    int db_port = DB_HOST_PORT;
    const char *db_dbname = DB_DATABASE_NAME;
    const char *db_username = DB_USERNAME;
    const char *db_password = DB_PASSWORD;
    int db_maxconncnt = DB_POOL_MAX_CON;
    CDBPool *pDBPool = new CDBPool(db_pool_name, db_host, db_port,
                                   db_username, db_password, db_dbname, db_maxconncnt);
    if (pDBPool->Init()) {
        printf("init db instance failed: %s", db_pool_name);
        return;
    }

    CDBConn *pDBConn = pDBPool->GetDBConn();
    if (pDBConn) {
        bool ret = pDBConn->ExecuteDrop(DROP_IMUSER_TABLE);
        if (ret) {
            printf("DROP_IMUSER_TABLE ok\n");
        }
        // 1. 创建表
        ret = pDBConn->ExecuteCreate(CREATE_IMUSER_TABLE);
        if (ret) {
            printf("CREATE_IMUSER_TABLE ok\n");
        }

        // 2. 插入内容
        ret = insertUser(pDBConn);
        if (ret) {
            printf("insertUser ok -------\n\n");
        }
        // 3. 查询内容
        ret = queryUser(pDBConn);
        if (ret) {
            printf("queryUser ok -------\n\n");
        }
        // 4. 修改内容
        ret = updateUser(pDBConn);
        if (ret) {
            printf("updateUser ok -------\n\n");
        }
        ret = queryUser(pDBConn);
        if (ret) {
            printf("queryUser ok -------\n\n");
        }
        //归还连接
        pDBPool->RelDBConn(pDBConn);
    }
    else {
        printf("pDBConn is null\n");
    }
    delete pDBPool;
}

// 默认端口 3306
// 测试一次连接和端口的情况： tcpdump -i any port 3306 
// 参考宏定义设置自己数据库的相关信息
/*
#define DB_HOST_IP          "127.0.0.1"             // 数据库服务器ip
#define DB_HOST_PORT        3306
#define DB_DATABASE_NAME    "mysql_pool_test"       // 数据库对应的库名字, 这里需要自己提前用命令创建完毕
#define DB_USERNAME         "root"                  // 数据库用户名
#define DB_PASSWORD         "123456"                // 数据库密码
#define DB_POOL_NAME        "mysql_pool"            // 连接池的名字，便于将多个连接池集中管理
#define DB_POOL_MAX_CON     4                       // 连接池支持的最大连接数量
*/
int main() {
    printf("test TestConnect begin\n");
    testConnect();
    printf("test TestConnect finish\n\n");

    printf("test TestCurd begin\n");
    testCurd();
    printf("test TestCurd finish\n");
    return 0;
}