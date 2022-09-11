
# 前言

&emsp;&emsp;到本文为止，该项目的前置知识已经写的差不多了。本文规划了数据库表的设计 和 后端与前端接口的设计，接下来就可以编写fastcgi程序进行运行了。项目以及项目相关源码地址：[gopherWxf git ](https://github.com/gopherWxf/c-c-linux-LearningCode/tree/master/%E6%96%87%E4%BB%B6%E6%9C%8D%E5%8A%A1%E5%99%A8%E9%A1%B9%E7%9B%AE)


# 1. 数据库建表
![在这里插入图片描述](https://img-blog.csdnimg.cn/27affc1553c14a6db17ca3ee2b3ad404.png)

## 1.1 用户信息表 user_info



| 字段       | 解释                     |
   | ---------- | ------------------------ |
| id         | 用户序号，自动递增，主键 |
| user_name       | 用户名字                 |
| nick_name| 用户昵称                 |
| password   | 密码                     |
| phone      | 手机号码                 |
| email      | 邮箱                     |
| create_time| 注册时间                     |


```sql
CREATE TABLE `user_info`
(
    `id`          bigint(20)                        NOT NULL AUTO_INCREMENT COMMENT '用户序号，自动递增，主键',
    `user_name`   varchar(32)                       NOT NULL DEFAULT '' COMMENT '用户名称',
    `nick_name`   varchar(32) CHARACTER SET utf8mb4 NOT NULL DEFAULT '' COMMENT '用户昵称',
    `password`    varchar(32)                       NOT NULL DEFAULT '' COMMENT '密码',
    `phone`       varchar(16)                       NOT NULL DEFAULT '' COMMENT '手机号码',
    `email`       varchar(64)                                DEFAULT '' COMMENT '邮箱',
    `create_time` timestamp                         NULL     DEFAULT CURRENT_TIMESTAMP COMMENT '注册时间',
    PRIMARY KEY (`id`),
    UNIQUE KEY `uq_nick_name` (`nick_name`),
    UNIQUE KEY `uq_user_name` (`user_name`)
) ENGINE = InnoDB
  DEFAULT CHARSET = utf8 COMMENT ='用户信息表';
```



## 1.2 文件信息表 file_info

| 字段     | 解释                                                         |
   | -------- | :----------------------------------------------------------- |
| id         | 文件序号，自动递增，主键 |
| md5      | 文件md5, 识别文件的唯一表示(身份证号)                        |
| file_id  | 文件id-/group1/M00/00/00/xxx.png                             |
| url      | 文件url 192.168.1.1:80/group1/M00/00/00/xxx.png - 下载的时候使用 |
| size     | 文件大小, 以字节为单位                                       |
| type     | 文件类型： png, zip, mp4……                                   |
| count    | 文件引用计数， 默认为1    每增加一个用户拥有此文件，此计数器+1 |

```sql
CREATE TABLE `file_info`
(
    `id`      bigint(20)   NOT NULL AUTO_INCREMENT COMMENT '文件序号，自动递增，主键',
    `md5`     varchar(256) NOT NULL COMMENT '文件md5',
    `file_id` varchar(256) NOT NULL COMMENT '文件id:/group1/M00/00/00/xxx.png',
    `url`     varchar(512) NOT NULL COMMENT '文件url 192.168.52.139:80/group1/M00/00/00/xxx.png',
    `size`    bigint(20)  DEFAULT '0' COMMENT '文件大小, 以字节为单位',
    `type`    varchar(32) DEFAULT '' COMMENT '文件类型： png, zip, mp4……',
    `count`   int(11)     DEFAULT '0' COMMENT '文件引用计数,默认为1。每增加一个用户拥有此文件，此计数器+1',
    PRIMARY KEY (`id`)
) ENGINE = InnoDB
  DEFAULT CHARSET = utf8 COMMENT ='文件信息表';
```



## 1.3 用户文件列表表 user_file_list


| 字段          | 解释                               |
   | ------------- | ---------------------------------- |
| id         | 序号，自动递增，主键 |
| user          | 文件所属用户                       |
| md5           | 文件md5                            |
| create_time| 文件创建时间                       |
| file_name| 文件名字                           |
| shared_status | 共享状态, 0为没有共享， 1为共享    |
| pv            | 文件下载量，默认值为0，下载一次加1 |

```sql
CREATE TABLE `user_file_list`
(
    `id`            int(11)      NOT NULL AUTO_INCREMENT COMMENT '编号',
    `user`          varchar(32)  NOT NULL COMMENT '文件所属用户',
    `md5`           varchar(256) NOT NULL COMMENT '文件md5',
    `create_time`   timestamp    NULL DEFAULT CURRENT_TIMESTAMP COMMENT '文件创建时间',
    `file_name`     varchar(128)      DEFAULT NULL COMMENT '文件名字',
    `shared_status` int(11)           DEFAULT NULL COMMENT '共享状态, 0为没有共享， 1为共享',
    `pv`            int(11)           DEFAULT NULL COMMENT '文件下载量，默认值为0，下载一次加1',
    PRIMARY KEY (`id`)
) ENGINE = InnoDB
  DEFAULT CHARSET = utf8 COMMENT ='用户文件列表';
```

## 1.4 用户文件数量表 user_file_count


| 字段  | 解释           |
   | ----- | -------------- |
| id         | 序号，自动递增，主键 |
| user  | 文件所属用户   |
| count | 拥有文件的数量 |

```sql
CREATE TABLE `user_file_count`
(
    `id`    int(11)      NOT NULL AUTO_INCREMENT,
    `user`  varchar(128) NOT NULL COMMENT '文件所属用户',
    `count` int(11) DEFAULT NULL COMMENT '拥有文件的数量',
    PRIMARY KEY (`id`),
    UNIQUE KEY `user_UNIQUE` (`user`)
) ENGINE = InnoDB
  DEFAULT CHARSET = utf8 COMMENT ='用户文件数量表';
```

## 1.5 共享文件列表 share_file_list

| 字段       | 解释                               |
   | ---------- | ---------------------------------- |
| id         | 序号，自动递增，主键 |
| user       | 文件所属用户                       |
| md5        | 文件md5                            |
| file_name| 文件名字                           |
| pv         | 文件下载量，默认值为1，下载一次加1 |
| create_time| 文件共享时间                       |
```sql
CREATE TABLE `share_file_list`
(
    `id`          int(11)      NOT NULL AUTO_INCREMENT COMMENT '编号',
    `user`        varchar(32)  NOT NULL COMMENT '文件所属用户',
    `md5`         varchar(256) NOT NULL COMMENT '文件md5',
    `file_name`   varchar(128)      DEFAULT NULL COMMENT '文件名字',
    `pv`          int(11)           DEFAULT '1' COMMENT '文件下载量，默认值为1，下载一次加1',
    `create_time` timestamp    NULL DEFAULT CURRENT_TIMESTAMP COMMENT '文件共享时间',
    PRIMARY KEY (`id`)
) ENGINE = InnoDB
  DEFAULT CHARSET = utf8 COMMENT ='共享文件列表';
```

## 1.6 共享图片列表 share_picture_list
| 字段       | 解释                               |
   | ---------- | ---------------------------------- |
| id         | 序号，自动递增，主键 |
| user       | 图片所属用户                       |
| filemd5| 图片md5                            |
| file_name| 图片名字                           |
| urlmd5| 图片urlmd5                           |
| pv         | 图片下载量，默认值为1，下载一次加1 |
| create_time| 图片共享时间                       |




```sql
CREATE TABLE `share_picture_list`
(
    `id`          int(11)      NOT NULL AUTO_INCREMENT COMMENT '编号',
    `user`        varchar(32)  NOT NULL COMMENT '文件所属用户',
    `filemd5`     varchar(256) NOT NULL COMMENT '文件md5',
    `file_name`   varchar(128)      DEFAULT NULL COMMENT '文件名字',
    `urlmd5`      varchar(256) NOT NULL COMMENT '图床urlmd5',
    `pv`          int(11)           DEFAULT '1' COMMENT '文件下载量，默认值为1，下载一次加1',
    `create_time` timestamp    NULL DEFAULT CURRENT_TIMESTAMP COMMENT '文件创建时间',
    PRIMARY KEY (`id`)
) ENGINE = InnoDB
  DEFAULT CHARSET = utf8 COMMENT ='共享图片列表';
```

## 1.7 导入数据库
`gopher.sql`在前言源码中

```bash
root@wxf:/temp# mysql -uroot -p123456
mysql> source /temp/gopher.sql
```


# 2. 后端接口设计


## 2.1 注册 - reg
注册是一个简单的 HTTP 接口，根据用户输入的注册信息，创建一个新的用户。

- 请求 URL

|URL| http://ip:port/api/reg|
|--|--|
|请求方式| POST|
|HTTP| 版本 1.1|
|Content-Type| application/json|

- 请求参数

|参数名 |含义 |规则说明| 是否必须 |缺省值|
|--|--|-- |--| --|
|userName |用户名称 |不能超过 32 个字符 |必填| 无|
|nickName |用户昵称 |不能超过 32 个字符 |必填 |无|
|firstPwd| 密码| md5 |加密后的值| 必填 |无|
|phone| 手机号码 |不能超过 16 个字符 |可选| 无|
|email| 邮箱 |必须符合 email 规范 |可选 |无|


- 返回结果参数说明


|名称| 含义 |规则说明|
|-- |--| --|
|code |结果值|0：成功 1：失败 2：用户存在|



1. 客户端

```bash
# URL
http://192.168.1.1/api/reg
# post数据格式
{
	userName:xxxx,
    nickName:xxx,
    firstPwd:xxx,
    phone:xxx,
    email:xxx
}
```

2. 服务器端-Nginx

```bash
location /api/reg
{
    # 转发数据
    fastcgi_pass localhost:10000;
    include fastcgi.conf;
}
```
3. 编写fastcgi程序

```bash
int main()
{
    while(FCGI_Accept() >= 0)
    {
        // 1. 根据content-length得到post数据块的长度
        // 2. 根据长度将post数据块读到内存
        // 3. 解析json对象, 得到用户名, 密码, 昵称, 邮箱, 手机号
        // 4. 连接数据库 - mysql, oracle
        // 5. 查询, 看有没有用户名, 昵称冲突 -> {"code":"2"}
        // 6. 有冲突 - 注册失败, 通知客户端
        // 7. 没有冲突 - 用户数据插入到数据库中
        // 8. 成功-> 通知客户端 -> {"code":"0"}
        // 9. 通知客户端回传的字符串的格式
        printf("content-type: application/json\r\n");
        printf("{\"code\":\"0\"}");
    }
}
```

## 2.2 登陆 - login
登录，根据用户输入的登录信息，登录进入到后台系统。




- 请求 URL

|URL| http://ip:port/api/login|
|--|--|
|请求方式| POST|
|HTTP| 版本 1.1|
|Content-Type| application/json|


- 请求参数


|参数名| 含义| 规则说明| 是否必须 |缺省值|
|--|--|--|--|--|
|user| 用户名称 |不能超过 32 个字符| 必填 |无|
|pwd |密码 |md5 加密后的值 |必填 |无|




- 返回结果参数说明



|名称| 含义 |规则说明|
|--|--|--|
|code| 结果值|0： 成功   1： 失败|
|token| 令牌 |每次登录后，生成的 token 不一样，后续其他接口请求时，需要带上 token。|


1. 客户端

```bash
#URL
http://192.168.1.1:80/api/login
# post数据格式
{
    user:xxxx,
    pwd:xxx
}
```

2. 服务器端

```bash
location /aip/login
{
    # 转发数据
    fastcgi_pass localhost:10001;
    include fastcgi.conf;
}
```



3. 编写fastcgi程序


```bash
int main()
{
    while(FCGI_Accept() >= 0)
    {
        // 1. 根据content-length得到post数据块的长度
        // 2. 根据长度将post数据块读到内存
        // 3. 解析json对象, 得到用户名, 密码
        // 4. 连接数据库 - mysql, oracle
        // 5. 查询, 根据用户名密码查询有没有这条记录
        // 6. 有记录- 登陆成功-生成token-> {"code":"0","token":"xxxx"}
        // 7. 没有记录- 登陆失败-> {"code":"1","token":"faild"}
        printf("content-type: application/json\r\n");
        printf("{\"code\":\"0\",\"token\":\"xxx\"}");
    }
}
```

```bash
token -> 客户端成功连接了服务器, 服务器针对于客户端的个人信息生成了一个唯一的身份标识
	- 可以按照每个人的身份证号理解
    - 服务器将这个token发送给客户端
	- 客户端token的使用:
		- 使用: 登录成功之后, 向服务器在发送任意请求都需要携带该token值
	- 服务器端的使用和保存:
		- 使用: 接收客户端发送的token, 和服务器端保存的token进行认证
			- 认证成功: 合法客户端, 失败: 客户端非法
		- 保存: 服务器需要保存所有客户端的token
				- 数据库中
				- 配置文件 -> 效率低
				- redis中 -> 效率最高

token = (客户端信息+随机数)*des*md5*base64
```


## 2.3 上传文件 - 秒传 - md5
上传文件的时候：
- 先调用 md5 接口判断服务器是否有该文件，如果 md 调用成功，则说明服务器已经存在该文件，客户端不需要再去调用 upload 接口上传文件。
- 如果不成功则客户端继续调用 upload 接口上传文件。

> - 尝试秒传 -> 文件并没上传
    >   - 给服务器发送的不是文件内容, 是文件的哈希值
>   - 在服务器端收到哈希值, 查询数据库
      >     - 查到了 -> 秒传成功
>     - 没查到 -> 秒传失败, 需要进行一个真正的上传操作
> - 进行真正的上传
    >   - 需要的时间长
>   - 上传有文件内容, 文件的哈希值
      >     - 文件内容 -> 分布式文件系统
>     - 哈希值 -> 数据库


- 请求 URL



|URL |http://ip:port/api/md5|
|--|--|
|请求方式 |POST|
|HTTP 版本 |1.1|
|Content-Type |application/json|



- 请求参数



|参数名| 含义| 规则说明 |是否必须| 缺省值|
|--|--|--|--|--|
|token |令牌 |同上| 必填 |无|
|md5 |md5 值 |不能超过 32 个字符 |必填 |无|
|filename| 文件名称| 不能超过 128 个字符| 必填 |无|
|user| 用户名称| 不能超过 32 个字符| 必填| 无|



- 返回结果参数说明


|名称| 含义| 规则说明|
|--|--|--|
|code |结果值| 0: 秒传成功1: 秒传失败 2: 已有该文件 3: token 校验失败|



1. 客户端

```bash
# url
http://127.0.0.1:80/api/md5
# post数据格式
{
    user:xxxx,
    token:xxxx,
    md5:xxx,
    fileName: xxx
}
```
- 服务器

```bash
location /api/md5
{
    # 转发数据
    fastcgi_pass localhost:10003;
    include fastcgi.conf;
}
```


- fastCGI程序编写

```bash
int main()
{
    while(FCGI_Accept() >= 0)
    {
        // 1. 得到post数据的长度
        char* length = getenv("content-length");
        int len = atoi(length);
        // 2. 根据len将数据读到内存中, json对象字符串
        // 3. 解析json对象, user,md5, token, fileName
        // 4. token认证 , 查询redis/数据库
        //     -- 成功: 继续后续操作, 失败, 返回, 给客户端一个结果
        // 5. 打开数据库, 并查询md5是否存在
        //     -- 存在   {"code":"0"}
        			-- 上传成功之后，业务逻辑--->存数据库一些相关信息
        //     -- 不存在  {"code":"1"}
    }
}
```

## 2.4 上传文件 - 真上传 - upload

上传文件

- 请求 URL




|URL |http://ip:port/api/upload|
|--|--|
|请求方式 |POST|
|HTTP 版本 |1.1|
|Content-Type |application/octet-stream|


- 请求参数


|参数名| 含义| 规则说明 |是否必须 |缺省值|
|--|--|--|--|--|
|token |密码| md5 |加密后的值| 必填| 无|



- 返回结果参数说明

|名称| 含义 |规则说明|
|-- |--| --|
|code |结果值|0: 上传成功  1: 上传失败|



- 客户端


```bash
# url
http://127.0.0.1:80/api/upload
# post数据格式
------WebKitFormBoundary88asdgewtgewx\r\n
# 文件信息
Content-Disposition: form-data; user="wxf"; filename="xxx.jpg"; md5="xxxx"; size=10240
Content-Type: image/jpg
# 真正的文件内容xxxxxxxxxxxxxxx
------WebKitFormBoundary88asdgewtgewx--
```


- 服务器

```bash
location /api/upload
{
    # 转发数据
    fastcgi_pass localhost:10002;
    include fastcgi.conf;
}
```
- 服务器端fastCGI 部分 伪代码

```bash
// fastcgi程序
int main()
{
    while(FCGI_Accept() >= 0)
    {
        // 1. 读一次数据 - buf, 保证能够将分界线和两个头都装进去
        char buf[4096];
        // len == 实际读出的字节数
        int len = fread(buf, 1, 4096, stdin);
        // 2. 跳过第一行的分界线
        len = len - 第一行的长度
        // 3. 将第二行中的user, filename, md5, size的值读出 -> 内存
        len = len - 第二行的长度;
        // 4. 跳过第3行
        len = len-第三行的长度
        // 5. 跳过空行
        len = len-空行的长度;
        // 6. 现在得到的位置的就是传输的真正数据的正文开始
        // 7. 循环的将剩余的内容读出, 有必要就写磁盘
        // 8. 读完了, 将最后的分界线剔除
        // 9. 以上8步处理完毕, 文件内容就被扣出来了
    }
}
```


```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "fcgi_stdio.h"

int recv_save_file(char *user, char *filename, char *md5, long *p_size) {
    int ret = 0;
    char *file_buf = NULL;
    char *begin = NULL;
    char *p, *q, *k;

    char content_text[512] = {0}; //文件头部信息
    char boundary[512] = {0};     //分界线信息

    //==========> 开辟存放文件的 内存 <===========
    file_buf = (char *) malloc(4096);
    if (file_buf == NULL) {
        return -1;
    }

    //从标准输入(web服务器)读取内容
    int len = fread(file_buf, 1, 4096, stdin);
    if (len == 0) {
        ret = -1;
        free(file_buf);
        return ret;
    }

    //===========> 开始处理前端发送过来的post数据格式 <============
    begin = file_buf;    //内存起点
    p = begin;

    /*
       ------WebKitFormBoundary88asdgewtgewx\r\n
       Content-Disposition: form-data; user="mike"; filename="xxx.jpg"; md5="xxxx"; size=10240\r\n
       Content-Type: application/octet-stream\r\n
       \r\n
       真正的文件内容\r\n
       ------WebKitFormBoundary88asdgewtgewx
       */

    //get boundary 得到分界线, ------WebKitFormBoundary88asdgewtgewx
    p = strstr(begin, "\r\n");
    if (p == NULL) {
        ret = -1;
        free(file_buf);
        return ret;
    }

    //拷贝分界线
    strncpy(boundary, begin, p - begin);
    boundary[p - begin] = '\0';   //字符串结束符
    //LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC,"boundary: [%s]\n", boundary);

    p += 2; //\r\n
    //已经处理了p-begin的长度
    len -= (p - begin);
    //get content text head
    begin = p;
    //Content-Disposition: form-data; user="mike"; filename="xxx.jpg"; md5="xxxx"; size=10240\r\n
    p = strstr(begin, "\r\n");
    if (p == NULL) {
        ret = -1;
        free(file_buf);
        return ret;
    }
    strncpy(content_text, begin, p - begin);
    content_text[p - begin] = '\0';

    p += 2;//\r\n
    len -= (p - begin);

    //========================================获取文件上传者
    //Content-Disposition: form-data; user="mike"; filename="xxx.jpg"; md5="xxxx"; size=10240\r\n
    q = begin;
    q = strstr(begin, "user=");
    q += strlen("user=");
    q++;    //跳过第一个"
    k = strchr(q, '"');
    strncpy(user, q, k - q);  //拷贝用户名
    user[k - q] = '\0';

    //========================================获取文件名字
    //"; filename="xxx.jpg"; md5="xxxx"; size=10240\r\n
    begin = k;
    q = begin;
    q = strstr(begin, "filename=");
    q += strlen("filename=");
    q++;    //跳过第一个"
    k = strchr(q, '"');
    strncpy(filename, q, k - q);  //拷贝文件名
    filename[k - q] = '\0';

    //========================================获取文件MD5码
    //"; md5="xxxx"; size=10240\r\n
    begin = k;
    q = begin;
    q = strstr(begin, "md5=");
    q += strlen("md5=");
    q++;    //跳过第一个"
    k = strchr(q, '"');
    strncpy(md5, q, k - q);   //拷贝文件名
    md5[k - q] = '\0';

    //========================================获取文件大小
    //"; size=10240\r\n
    begin = k;
    q = begin;
    q = strstr(begin, "size=");
    q += strlen("size=");
    k = strstr(q, "\r\n");
    char tmp[256] = {0};
    strncpy(tmp, q, k - q);   //内容
    tmp[k - q] = '\0';
    *p_size = strtol(tmp, NULL, 10); //字符串转long

    begin = p;
    p = strstr(begin, "\r\n");
    // 跳过最后一个请求头行和空行
    p += 4; //\r\n\r\n
    // len就是正文的起始位置到数组结束的长度
    len -= (p - begin);

    //下面才是文件的真正内容
    /*
       ------WebKitFormBoundary88asdgewtgewx\r\n
       Content-Disposition: form-data; user="mike"; filename="xxx.jpg"; md5="xxxx"; size=10240\r\n
       Content-Type: application/octet-stream\r\n
       \r\n
       真正的文件内容\r\n
       ------WebKitFormBoundary88asdgewtgewx
    */
    begin = p;
    // 将数组中剩余的数据(文件的内容), 写入到磁盘中
    int fd = open(filename, O_CREAT | O_WRONLY, 0664);
    write(fd, begin, len);
    // 1. 文件已经读完了
    // 2. 文件还没读完
    if (*p_size > len) {
        // 读文件 -> 接收post数据
        // fread , read  , 返回值>0== 实际读出的字节数; ==0, 读完了; -1, error
        while ((len = fread(file_buf, 1, 4096, stdin)) > 0) {
            // 读出的数据写文件
            write(fd, file_buf, len);
        }
    }
    // 3. 解写入到文件中的分界线删除
    ftruncate(fd, *p_size);
    close(fd);

    free(file_buf);
    return ret;
}

int main() {
    while (FCGI_Accept() >= 0) {
        // int recv_save_file(char *user, char *filename, char *md5, long *p_size)
        char user[24];
        char fileName[32];
        char md5[64];
        long size;
        recv_save_file(user, fileName, md5, &size);
        // filename对应的文件上传到fastdfs
        // 上传得到的文件ID需要写数据库
        // 给客户端发送响应数据
        printf("Content-type: text/plain\r\n\r\n");
        printf("用户名: %s\n", user);
        printf("文件名: %s, md5: %s, size: %ld\n", fileName, md5, size);
        printf("客户端处理数据完毕, 恭喜...");
    }
}
```


## 2.5 获取用户文件数量 - myfiles
获取用户文件数量

- 请求 URL

|URL| http://ip:port/api/myfiles?cmd=count|
|--|--|
|请求方式| POST|
|HTTP 版本| 1.1|
|Content-Type| application/json|


- 请求参数


|参数名| 含义 |规则说明 |是否必须 |缺省值|
|--|--|--|--|--|
|token |token |同上 |必填| 无|
|user |用户名称 |不能超过 32 个字符| 必填| 无|




- 返回结果参数说明

|名称| 含义 |规则说明|
|--|--|--|
|code| 结果值|0：验证成功 1：验证失败|
|total| 文件数量|无|



1. 客户端

```bash
url : http://127.0.0.1:80/api/myfiles?cmd=count
post数据 ：{   "user": "wxf",   "token": "xxxx"   }
```
2. 服务器

```bash
服务器添加的nginx指令:
location /api/myfiles
{
    fastcgi_pass localhost:10004;
    include fastcgi.conf;
}
```
3. 编写fastcgi程序


```bash
// fastcgi程序
int main()
{
	while(FCGI_Accept() >= 0)
    {
        // 接收数据
        // 将url?后边的内容取出
        // cmd=normal; cmd=count
        char* type = getenv("QUERY_STRING");
        // 将 = 后的内容取出, 保存在数组cmd中
        if(strcmp(cmd, "count") == 0)
        {
            // 解析数据, user, token
            // token认证
            // 认证成功, 查询数据库   
        }
		else if()
        {
        }
    }
}
```

```bash
{
	"total":"8",
	"code":"0"     // token验证成功
}
```



## 2.6 获取用户文件信息 - myfiles

获取用户文件信息


- 请求 URL


|URL| http://ip:port/api/myfiles?cmd=见下文|
|--|--|
|请求方式| POST|
|HTTP 版本| 1.1|
|Content-Type| application/json|

```bash
cmd = normal 或 pvasc 或 pvdesc
```


- 请求参数



|参数名 |含义| 规则说明 |是否必须 |缺省值|
|--|--|--|--|--|
|token |令牌 |同上| 必填| 无|
|user |用户名称| 不能超过 32 个字符| 必填| 无|
|count |文件个数 |文件个数需大于 0| 必填 |无|
|start |开始位置 ||必填 |无|



- 返回结果参数说明

|名称 |含义 |规则说明|
|--|--|--|
| files       |              文件结果集      | 见下 |

```bash
"code": 0 正常，1 失败
"count": 返回的当前文件数量，比如 2
"total": 个人文件总共的数量
"user": 用户名称,
"md5": md5 值,
"create_time": 创建时间,
"file_name": 文件名,
"share_status": 共享状态, 0 为没有共享， 1 为共享
"pv": 文件下载量，下载一次加 1
"url": URL,
"size": 文件大小,
"type": 文件类型
```



1. 客户端

   | 获取用户文件信息 | http://127.0.0.1:80/myfiles&cmd=normal                    |
       | ---------------- | ------------------------------------------------------------ |
   | 按下载量升序     | http://127.0.0.1:80/myfiles?cmd=pvasc                      |
   | 按下载量降序     | http://127.0.0.1:80/myfiles?cmd=pvdesc                     |
   | post数据         | {  "user": "wxf" , "token": "xxxx", "start": 0, "count": 10 } |






```bash
http://127.0.0.1:80/api/myfiles&cmd=normal
# post body数据
{
    "count": 2, 
    "start": 0, 
    "token": "3a58ca22317e637797f8bcad5c047446", 
    "user": "wxf"
}
```


```bash
# 服务器返回的json
{
    "code": 0, 
    "count": 2, # 如果为 0 则不需要解析 files
    "total": 2, # 个人文件总共的数量
    "files": [
        {
            "user": "wxf", 
            "md5": "6c5fa2864bb264c91167258b3e478fa0", 
            "create_time": "2022-09-09 23:32:01", 
            "file_name": "Qt5Svg.dll", 
            "share_status": 0, 
            "pv": 1, 
            "url": "http://192.168.109.101:80/group1/M00/00/00/eBuDxWCfQHSATopyAAV8AJV_1mw866.dll", 
            "size": 359424, 
            "type": "dll"
        }, 
        {
            "user": "wxf", 
            "md5": "c32b5d82be3d2fcec96de06e81f87814", 
            "create_time": "2022-09-09 23:32:01", 
            "file_name": "1.png", 
            "share_status": 0, 
            "pv": 0, 
            "url": "http://192.168.109.101:80/group1/M00/00/00/eBuDxWCfQPCAapRFAACtu7eiL0000.png", 
            "size": 44475, 
            "type": "png"
        }
    ]
}
```

```bash
# 文件数量为 0 的场景
{
    "code": 0, 
    "count": 0
}
```


## 2.7 获取共享列表数量 - sharefiles

获取共享列表数量



- 请求 URL


|URL| http://ip:port/api/sharefiles?cmd=count|
|--|--|
|请求方式| GET|
|HTTP 版本 |1.1|
|Content-Type| application/x-www-form-urlencoded|

&emsp;
- 请求参数： 无

&emsp;&emsp;

- 返回结果参数说明


|名称| 含义 |规则说明|
|--|--|--|
|code |结果码| 0：正常；1：失败|
|total |总数量|无|


1. 客户端

```bash
get请求对应的url
http://127.0.0.1:80/sharefiles&cmd=count
```
2. 服务器

```bash
location /api/sharefiles
{
    # 转发数据
    fastcgi_pass localhost:10005;
    include fastcgi.conf;
}
```


## 2.8 获取共享列表文件 - sharefiles

获取共享列表文件

- 请求 URL


|URL| http://ip:port/api/sharefiles?cmd=normal|
|--|--|
|请求方式| POST|
|HTTP 版本 |1.1|
|Content-Type| application/json|




- 请求参数



|参数名| 含义|  是否必须 |缺省值|
|--|--|--|--|
|count |数量 |必填| 无|
|start |开始位置 |必填 |无|





- 返回结果参数说明


|名称| 含义| 规则说明|
|--|--|--|
|   files    |       文件结果集    |     见下   |


```bash
"code"：0：正常；1：失败
"count": 2, // 分页返回数量，如果为 0 则不需要解析 files
"total": 2, 总的文件数量
"user": 用户名称,
"md5": md5 值,
"create_time": 创建时间,
"file_name": 文件名,
"share_status": 共享状态, 0 为没有共享， 1 为共享
"pv": 文件下载量，下载一次加 1
"url": URL,
"size": 文件大小,
"type": 文件类型
```



1. 客户端

```http
请求的url, 获取共享文件信息
http://127.0.0.1:80/api/sharefiles&cmd=normal
```
```json
/* post数据 */
{
   "start": 0,
   "count": 1
}
```


```bash
# 返回结果
{
    "code": 0, 
    "count": 1, 
    "total": 2, 
    "files": [
        {
            "user": "wxf", 
            "md5": "602fdf30db2aacf517badf4565124f51", 
            "file_name": "Makefile", 
            "share_status": 1, 
            "pv": 0, 
            "create_time": "2022-09-09 23:49:15", 
            "url": "http://192.168.109.101:80/group1/M00/00/00/rBAAD17_Jn6AMiuAACBBC5AIsc2532509", 
            "size": 33028, 
            "type": "null"
        }
    ]
}
```



## 2.9 下载榜 - sharefiles


- 请求 URL



|URL| http://ip:port/api/sharefiles?cmd=pvdesc|
|--|--|
|请求方式 |POST|
|HTTP 版本 |1.1|
|Content-Type |application/json|


- 请求参数

|参数名| 含义|  是否必须| 缺省值|
|--|--|--|--|
|count| 数量 |必填| 无|
|start |开始位置 |必填| 无|


- 返回结果参数说明


|名称| 含义 |规则说明|
|--|--|--|
|files| 文件结果集| "filename": 文件名,"pv": 文件下载量，下载一次加 1|
|code |返回码| 0：正常；1：失败|
|cout |分页文件数量 |比如 1，如果为 0 则不需要解析 files|
|total |总文件数量 |比如 2|













## 2.10  文件分享 - dealfile
分享文件

- 请求 URL

|URL| http://ip:port/api/dealfile?cmd=share|
|--|--|
|请求方式| POST|
|HTTP 版本 |1.1|
|Content-Type| application/json|



- 请求参数



|参数名| 含义| 规则说明 |是否必须 |缺省值|
|--|--|--|--|--|
|token |令牌 |同上 |必填 |无|
|user |用户名称 |不能超过 32 个字符| 必填 |无|
|md5| md5 值 |md5 加密后的值| 必填| 无|
|filename |文件名称| 不能超过 128 个字符| 必填 |无|


- 返回结果参数说明


|名称| 含义 |规则说明|
|--|--|--|
|code |结果值| 见下|

```bash
0： 成功
1： 失败
3： 别人已经分享此文件
4： token 验证失败
```

1. 客户端

```bash
请求的url
http://127.0.0.1:80/api/dealfile?cmd=share
```

```bash
/* post数据格式 */
{
   "user": "wxf",
   "token": "xxxx",
   "md5": "xxx",
   "filename": "xxx"
 }
```

```bash
/* 返回结果 */
{
	"code": 0
}
```


- 服务端


```bash
location /api/dealfile
{
    # 转发数据
    fastcgi_pass localhost:10006;
    include fastcgi.conf;
}
```

## 2.11 文件删除 - dealfile

文件删除


- 请求 URL

|URL| http://ip:port/api/dealfile?cmd=del|
|--|--|
|请求方式| POST|
|HTTP 版本 |1.1|
|Content-Type| application/json|


- 请求参数



|参数名| 含义| 规则说明 |是否必须 |缺省值|
|--|--|--|--|--|
|token |令牌 |同上 |必填 |无|
|user |用户名称 |不能超过 32 个字符| 必填 |无|
|md5| md5 值 |md5 加密后的值| 必填| 无|
|filename |文件名称| 不能超过 128 个字符| 必填 |无|


- 返回结果参数说明


|名称| 含义 |规则说明|
|--|--|--|
|code |结果值|0: 成功;1: 失败|



1. 客户端


```bash
请求的url
http://127.0.0.1:80/api/dealfile?cmd=del
```

```bash
/* post数据格式 */
{
    "user": "wxf",
    "token": "xxxx",
    "md5": "xxx",
    "filename": "xxx"
}
```

```bash
/* 返回结果 */
{
	"code": 0
}
```

## 2.12 文件下载之后, 下载量pv字段的处理 - dealfile

是用来更新指定文件的下载量，每次成功下载一个文件成功后，调用该接口更新对应文件的 pv 值。


- 请求 URL

|URL| http://ip:port/api/dealfile?cmd=pv|
|--|--|
|请求方式| POST|
|HTTP 版本 |1.1|
|Content-Type| application/json|


- 请求参数


|参数名| 含义| 规则说明 |是否必须 |缺省值|
|--|--|--|--|--|
|token |令牌 |同上 |必填 |无|
|user |用户名称 |不能超过 32 个字符| 必填 |无|
|md5| md5 值 |md5 加密后的值| 必填| 无|
|filename |文件名称| 不能超过 128 个字符| 必填 |无|


- 返回结果参数说明


|名称| 含义 |规则说明|
|--|--|--|
|code |结果值|0: 成功;1: 失败|



1. 客户端

```bash
请求的url: http://127.0.0.1:80/api/dealfile?cmd=pv
通过值请求更新数据库中该文件对应的下载量字段值
```

```bash
/* post数据块 */
{
   "user": "wxf",
   "token": "xxx",
   "md5": "xxx",
   "filename": "xxx"
 }
```
```bash
/* 返回结果 */
{
	"code": 0
}
```

## 2.13 取消分享文件 - dealsharefile
- 请求 URL

|URL| http://ip:port/api/dealsharefile?cmd=cancel|
|--|--|
|请求方式 |POST|
|HTTP 版本 |1.1|
|Content-Type |application/json|

- 请求参数

|参数名| 含义| 规则说明 |是否必须 |缺省值|
|--|--|--|--|--|
|user |用户名称 |不能超过 32 个字符| 必填 |无|
|md5| md5 值 |md5 加密后的值| 必填| 无|
|filename |文件名称| 不能超过 128 个字符| 必填 |无|

- 返回结果参数说明


|名称| 含义 |规则说明|
|--|--|--|
|code |结果值|0: 成功;1: 失败|




1. 客户端

```bash
http://127.0.0.1/api/dealsharefile?cmd=cancel
```
```json
/* post数据块格式 */
{
	"user": "wxf",
	"md5": "xxx",
	"filename": "xxx"
}
```

```bash
/* 返回结果 */
{
	"code": 0
}
```


- 服务端


```bash
location /api/dealsharefile
{
    # 转发数据
    fastcgi_pass localhost:10007;
    include fastcgi.conf;
}
```

## 2.14 转存文件 - dealsharefile


- 请求 URL

|URL| http://ip:port/api/dealsharefile?cmd=save|
|--|--|
|请求方式 |POST|
|HTTP 版本 |1.1|
|Content-Type |application/json|

- 请求参数

|参数名| 含义| 规则说明 |是否必须 |缺省值|
|--|--|--|--|--|
|user |用户名称 |不能超过 32 个字符| 必填 |无|
|md5| md5 值 |md5 加密后的值| 必填| 无|
|filename |文件名称| 不能超过 128 个字符| 必填 |无|

- 返回结果参数说明


|名称| 含义 |规则说明|
|--|--|--|
|code |结果值|0: 成功;1: 失败 ; 5： 文件已存在|

1. 客户端
```bash
# 调用接口
http://127.0.0.1/api/dealsharefile?cmd=save
```
```json
/* post数据块格式 */
{
	"user": "wxf",
	"md5": "xxx",
	"filename": "xxx"
}
```

```bash
/* 返回结果 */
{
	"code": 0
}
```


## 2.15 共享文件下载pv标志处理 - dealsharefile
即是共享文件下载成功后，发该 url 的请求，该文件下次数量+1

- 请求 URL

|URL| http://ip:port/api/dealsharefile?cmd=pv|
|--|--|
|请求方式 |POST|
|HTTP 版本 |1.1|
|Content-Type |application/json|

- 请求参数

|参数名| 含义| 规则说明 |是否必须 |缺省值|
|--|--|--|--|--|
|user |用户名称 |不能超过 32 个字符| 必填 |无|
|md5| md5 值 |md5 加密后的值| 必填| 无|
|filename |文件名称| 不能超过 128 个字符| 必填 |无|

- 返回结果参数说明


|名称| 含义 |规则说明|
|--|--|--|
|code |结果值|0: 成功;1: 失败 |




## 2.16 图片分享 - sharepic

- 请求 URL

|URL| http://ip:port/api/sharepic?cmd=share|
|--|--|
|请求方式 |POST|
|HTTP 版本 |1.1|
|Content-Type |application/json|


- 请求参数

|参数名| 含义| 规则说明 |是否必须 |缺省值|
|--|--|--|--|--|
|token |令牌 |同上 |必填 |无|
|user |用户名称 |不能超过 32 个字符| 必填 |无|
|md5| md5 值 |md5 加密后的值| 必填| 无|
|filename |文件名称| 不能超过 128 个字符| 必填 |无|


- 返回结果参数说明


|名称| 含义| 规则说明|
|--|--|--|
|code| 结果值|0： 成功；1： 失败；4： token 验证失败
|urlmd5 |分享图片的标识 |取消分享时也要该标识|



## 2.17 请求浏览图片 - sharepic

请求接口 http://ip:port/api/sharepic?cmd=browse，主要用来返回具体的图片下载地址，对图片浏览量pv+1


- 请求 URL

|URL| http://ip:port/api/sharepic?cmd=browse|
|--|--|
|请求方式 |POST|
|HTTP 版本 |1.1|
|Content-Type |application/json|


- 请求参数

|参数名  |含义  | 是否必须  |缺省值 |
|--|--|--|--|
|urlmd5 | 分享图片的标识 | 必填 | 无 |


- 返回结果参数说明

|名称| 含义|规则说明|
|--|--|--|
|code | 结果值  |0：请求到 url 下载的 url；1：提取码错误；2：文件已经被删除； |
|url  | 图片下载地址  |
| user | 分享者 | 分享者 |
| time | 分享时间 |  |
|  pv|  浏览次数| pv=pv+1 |

## 2.18 我的图片分享 - sharepic
和我的文件类似显示自己的分享图片信息

- 请求 URL

|URL| http://ip:port/api/sharepic?cmd=normal|
|--|--|
|请求方式 |POST|
|HTTP 版本 |1.1|
|Content-Type |application/json|


- 请求参数

|参数名| 含义| 规则说明 |是否必须 |缺省值|
|--|--|--|--|--|
|token |令牌 |同上 |必填 |无|
|user |用户名称 |不能超过 32 个字符| 必填 |无|
|count| 数量 || 必填| 无|
|start |开始位置| | 必填 |无|



|名称 |含义| 规则说明|
|--|--|--|
|files |文件结果集 |见下|

```bash
"code"：0：正常；1：失败
"count": 2, # 分页返回数量，如果为 0 则不需要解析 files
"total": 2, 总的文件数量
"user": 用户名称,
"filemd5": 文件 md5 值,
"urlmd5": 图传共享时的 URL,
"create_time": 创建时间,
"file_name": 文件名,
"pv": 文件浏览量，浏览一次加 1
"size": 文件大小
```


## 2.19 取消图片分享 - sharepic


- 请求 URL

|URL| http://ip:port/api/sharepic?cmd=cancel|
|--|--|
|请求方式 |POST|
|HTTP 版本 |1.1|
|Content-Type |application/json|


- 请求参数

|参数名| 含义| 规则说明 |是否必须 |缺省值|
|--|--|--|--|--|
|token |令牌 |同上 |必填 |无|
|user |用户名称 |不能超过 32 个字符| 必填 |无|
|md5| md5 值 |md5 加密后的值| 必填| 无|



- 返回结果参数说明

|名称| 含义| 规则说明|
|--|--|--|
|code| 结果值|0： 成功；1： 失败；4： token 验证失败




# 3. 总结

上文的服务器配置的location的端口有些是写错了，只是个示例，在nginx.conf配置的端口要与spawn-fcgi相对应，别搞混了。

```bash
location /api/login {
	fastcgi_pass 127.0.0.1:10000;
    include fastcgi.conf;
}

location /api/reg {
    fastcgi_pass 127.0.0.1:10001;
    include fastcgi.conf;
}

location /api/upload {
    fastcgi_pass 127.0.0.1:10002;
    include fastcgi.conf;
}

location /api/md5 {
    fastcgi_pass 127.0.0.1:10003;
    include fastcgi.conf;
}

location /api/myfiles {
    fastcgi_pass 127.0.0.1:10004;
    include fastcgi.conf;
}

location /api/dealfile {
    fastcgi_pass 127.0.0.1:10005;
    include fastcgi.conf;
}

location /api/sharefiles {
    fastcgi_pass 127.0.0.1:10006;
    include fastcgi.conf;
}
location /api/dealsharefile {
    fastcgi_pass 127.0.0.1:10007;
    include fastcgi.conf;
}

location /api/sharepic {
    fastcgi_pass 127.0.0.1:10008;
    include fastcgi.conf;
}
```



```bash
# ******************************* 重新启动CGI进程 ******************************* 
if [ "$START" -eq 1 ];then
    # 登录
    echo -n "登录："
    spawn-fcgi -a 127.0.0.1 -p 10000 -f ./bin_cgi/login
    # 注册
    echo -n "注册："
    spawn-fcgi -a 127.0.0.1 -p 10001 -f ./bin_cgi/register
    # 上传文件
    echo -n "上传："
    spawn-fcgi -a 127.0.0.1 -p 10002 -f ./bin_cgi/upload
    # MD5秒传
    echo -n "MD5："
    spawn-fcgi -a 127.0.0.1 -p 10003 -f ./bin_cgi/md5
    # 我的文件
    echo -n "MyFile："
    spawn-fcgi -a 127.0.0.1 -p 10004 -f ./bin_cgi/myfiles
    # 分享删除文件
    echo -n "DealFile："
    spawn-fcgi -a 127.0.0.1 -p 10005 -f ./bin_cgi/dealfile
    # 共享文件列表
    echo -n "ShareList："
    spawn-fcgi -a 127.0.0.1 -p 10006 -f ./bin_cgi/sharefiles
    # 共享文件pv字段处理、取消分享、转存文件
    echo -n "DealShare："
    spawn-fcgi -a 127.0.0.1 -p 10007 -f ./bin_cgi/dealsharefile
	  # 图床共享图片
    echo -n "SharePicture："
    spawn-fcgi -a 127.0.0.1 -p 10008 -f ./bin_cgi/sharepicture
    echo "CGI 程序已经成功启动 ^_^..."
fi
```
