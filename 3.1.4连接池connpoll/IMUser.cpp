#include "IMUser.h"

std::string int2string(uint32_t user_id) {
    std::stringstream ss;
    ss << user_id;
    return ss.str();
}

bool insertUser(CDBConn *pDBConn, int id) {
    bool bRet = false;
    string strSql;
    strSql = "insert into IMUser(`salt`,`sex`,`nick`,`password`,`domain`,`name`,`phone`,`email`,`company`,`address`,`avatar`,`sign_info`,`departId`,`status`,`created`,`updated`) values(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
    int id_index = id;      // 用来区别姓名

    CPrepareStatement *stmt = new CPrepareStatement();
    if (stmt->Init(pDBConn->GetMysql(), strSql)) {
        uint32_t nNow = (uint32_t) time(NULL);
        uint32_t index = 0;
        string strOutPass = "987654321";
        string strSalt = "abcd";

        int nSex = 1;                             // 用户性别 1.男;2.女
        int nStatus = 0;                          // 用户状态0 正常， 1 离职
        uint32_t nDeptId = 0;                     // 所属部门
        string strNick = "明华";                  // 花名
        string strDomain = "minghua";             // 花名拼音
        string strName = "廖庆富";                // 真名
        string strTel = "1857036";            // 手机号码
        string strEmail = "326873713@qq.com";     // Email
        string strAvatar = "";                    // 头像
        string sign_info = "一切只为你";          //个性签名
        string strPass = "123456";                //密码
        string strCompany = "零声学院";           //公司
        string strAddress = "长沙岳麓区雅阁国际"; //地址

        stmt->SetParam(index++, strSalt);
        stmt->SetParam(index++, nSex);
        stmt->SetParam(index++, strNick);
        stmt->SetParam(index++, strOutPass);
        stmt->SetParam(index++, (strDomain + int2string(id_index)));
        stmt->SetParam(index++, (strName + int2string(id_index)));
        stmt->SetParam(index++, (strTel + int2string(id_index)));
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
            uint32_t nId = (uint32_t) stmt->GetInsertId();
            // printf("register then get user_id:%d\n", nId);
        }
    }
    delete stmt;

    return true;
}