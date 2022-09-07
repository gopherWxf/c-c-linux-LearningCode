#include <stdio.h>
#include <hiredis.h>

int main()
{
    // 1. 连接redis服务器
    redisContext* c = redisConnect("127.0.0.1", 6379);
    if (c->err != 0)
    {
        return -1;
    }
    // 2. 执行redis命令
    void *prt = redisCommand(c, "hmset user userName zhang3 passwd 123456 age 23 sex man");
    redisReply* ply = (redisReply*)prt;
    if(ply->type == 5)
    {
        // 状态输出
        printf("状态: %s\n", ply->str);
    }
    freeReplyObject(ply);

    // 3. 从数据库中读数据
    prt = redisCommand(c, "hgetall user");
    ply = (redisReply*)prt;
    if(ply->type == 2)
    {
        // 遍历
        for(int i=0; i<ply->elements; i+=2)
        {
            printf("key: %s, value: %s\n", ply->element[i]->str, ply->element[i+1]->str);
        }
    }
    freeReplyObject(ply);

    redisFree(c);
    return 0;
}
