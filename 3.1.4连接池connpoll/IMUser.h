#ifndef IMUSER_H_
#define IMUSER_H_

#include <iostream>
#include <sstream>
#include "DBpool.h"

#define DROP_IMUSER_TABLE "DROP TABLE IF EXISTS IMUser" /* if EXISTS 好处 是如果表不存在,执行不会报错 */


#define CREATE_IMUSER_TABLE "CREATE TABLE IMUser (     \
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT COMMENT '用户id',   \
  `sex` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT '1男2女0未知', \
  `name` varchar(32) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '用户名',  \
  `domain` varchar(32) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '拼音',  \
  `nick` varchar(32) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '花名,绰号等', \
  `password` varchar(32) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '密码',    \
  `salt` varchar(4) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '混淆码',   \
  `phone` varchar(11) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '手机号码',   \
  `email` varchar(64) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT 'email',  \
  `company` varchar(64) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '公司名称', \
  `address` varchar(64) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '所在地区', \
  `avatar` varchar(255) COLLATE utf8mb4_bin DEFAULT '' COMMENT '自定义用户头像',    \
  `validateMethod` tinyint(2) unsigned DEFAULT '1' COMMENT '好友验证方式',  \
  `departId` int(11) unsigned NOT NULL DEFAULT '1' COMMENT '所属部门Id',    \
  `status` tinyint(2) unsigned DEFAULT '0' COMMENT '1. 试用期 2. 正式 3. 离职 4.实习',  \
  `created` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '创建时间',   \
  `updated` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '更新时间',   \
  `push_shield_status` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT '0关闭勿扰 1开启勿扰',  \
  `sign_info` varchar(128) COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '个性签名',  \
  PRIMARY KEY (`id`),   \
  KEY `idx_domain` (`domain`),  \
  KEY `idx_name` (`name`),  \
  KEY `idx_phone` (`phone`) \
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;"

#define INSERT_SAMPLE "INSERT INTO user(name,email,phone) VALUES(?,?,?)"
#define SELECT_SAMPLE "SELECT name,email,phone FROM user"

//1.创建数据库mysql_pool_test:  create database mysql_pool_test;
// show databases;    查看数据库
// show tables;       查看有哪些表
// desc table_name;   查看表结构
//

std::string int2string(uint32_t user_id);  // 只是一个工具函数
bool insertUser(CDBConn *pDBConn, int id);

#endif