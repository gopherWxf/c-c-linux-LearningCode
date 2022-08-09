#include <iostream>
#include "DBPool.h"
#include "ZeroThreadpool.h"
#include "IMUser.h"

using namespace std;


void func0() {
    //    std::this_thread::sleep_for(std::chrono::seconds(5));
    cout << "func0()" << endl;
}

void func1(int a) {
    cout << "func1() a=" << a << endl;
}

void func2(int a, string b) {
    cout << "func2() a=" << a << ", b=" << b << endl;
}

void test1() // 简单测试线程池
{
    ZERO_ThreadPool threadpool;
    threadpool.init(1); // 设置线程数量
    threadpool.start(); // 启动线程池
    // 假如要执行的任务
    threadpool.exec(1000, func0);         // 放入要执行的任务
    threadpool.exec(func1, 10);           // 放入要执行的任务
    threadpool.exec(func2, 20, "darren"); // 可变参数
    threadpool.waitForAllDone();          // 等待所有执行万再退出
    threadpool.stop();
}


// 真正测试连接池的代码起始
#define TASK_NUMBER 100

#define DB_HOST_IP "127.0.0.1" // 数据库服务器ip
#define DB_HOST_PORT 3306
#define DB_DATABASE_NAME "mysql_pool_test" // 数据库对应的库名字, 这里需要自己提前用命令创建完毕
#define DB_USERNAME "root"                 // 数据库用户名
#define DB_PASSWORD "123456"               // 数据库密码
#define DB_POOL_NAME "mysql_pool"          // 连接池的名字，便于将多个连接池集中管理
#define DB_POOL_MAX_CON 4                  // 连接池支持的最大连接数量



// 使用连接池的方式
void *workUsePool(void *arg, int id)    // 任务
{
    // printf("workUsePool id:%d\n", id);
    CDBPool *pDBPool = (CDBPool *) arg;
    CDBConn *pDBConn = pDBPool->GetDBConn(2000); // 获取连接，超时2000ms
    if (pDBConn) {
         bool ret = insertUser(pDBConn, id); // 插入用户信息
         if (!ret)
         {
             printf("insertUser failed\n");
         }
    }
    else {
        printf("GetDBConn failed\n");
    }
    pDBPool->RelDBConn(pDBConn);     // 然后释放连接
    // printf("exit id:%d\n", id);
    return NULL;
}

// 没有用连接池，每次任务的执行都重新初始化连接
void *workNoPool(void *arg, int id) {
    // printf("workNoPool\n");
    arg = arg; // 避免警告
    const char *db_pool_name = DB_POOL_NAME;
    const char *db_host = DB_HOST_IP;
    int db_port = DB_HOST_PORT;
    const char *db_dbname = DB_DATABASE_NAME;
    const char *db_username = DB_USERNAME;
    const char *db_password = DB_PASSWORD;

    int db_maxconncnt = 1;

    CDBPool *pDBPool = new CDBPool(db_pool_name, db_host, db_port,
                                   db_username, db_password, db_dbname, db_maxconncnt);
    if (!pDBPool) {
        printf("workNoPool new CDBPool failed\n");
        return NULL;
    }
    if (pDBPool->Init()) {
        printf("init db instance failed: %s\n", db_pool_name);
        return NULL;
    }

    CDBConn *pDBConn = pDBPool->GetDBConn();
    if (pDBConn) {
        bool ret = insertUser(pDBConn, id);
        if (!ret) {
            printf("insertUser failed\n");
        }
    }
    else {
        printf("GetDBConn failed\n");
    }
    pDBPool->RelDBConn(pDBConn);
    delete pDBPool;         // 销毁连接池，实际是销毁连接
    return NULL;
}

// 使用连接池的测试
int testWorkUsePool(int thread_num, int db_maxconncnt, int task_num) {
    const char *db_pool_name = DB_POOL_NAME;
    const char *db_host = DB_HOST_IP;
    int db_port = DB_HOST_PORT;
    const char *db_dbname = DB_DATABASE_NAME;
    const char *db_username = DB_USERNAME;
    const char *db_password = DB_PASSWORD;

    // 每个连接池都对应一个对象
    CDBPool *pDBPool = new CDBPool(db_pool_name, db_host, db_port,db_username, db_password, db_dbname, db_maxconncnt);
    if (pDBPool->Init()) {
        printf("init db instance failed: %s", db_pool_name);
        return -1;
    }

    CDBConn *pDBConn = pDBPool->GetDBConn(); // 获取连接
    if (pDBConn) {
        bool ret = pDBConn->ExecuteDrop(DROP_IMUSER_TABLE); // 删除表
        if (ret) {
            printf("DROP_IMUSER_TABLE ok\n");
        }
        // 1. 创建表
        ret = pDBConn->ExecuteCreate(CREATE_IMUSER_TABLE); // 创建表
        if (ret) {
            printf("CREATE_IMUSER_TABLE ok\n");
        }
        else {
            printf("ExecuteCreate failed\n");
            return -1;
        }
    }
    pDBPool->RelDBConn(pDBConn); // 一定要归还

    printf("thread_num = %d, connection_num:%d, use_pool:1\n",
           thread_num, db_maxconncnt);

    ZERO_ThreadPool threadpool;
    threadpool.init(thread_num); // 设置线程数量
    threadpool.start();          // 启动线程池
    for (int i = 0; i < task_num; i++) {
        threadpool.exec(workUsePool, (void *) pDBPool, i);
    }

    threadpool.waitForAllDone(); // 等待所有执行万再退出
    threadpool.stop();
    return 0;
}

// 初始化和使用连接池是一样的
int testWorkNoPool(int thread_num, int db_maxconncnt, int task_num) {
    const char *db_pool_name = DB_POOL_NAME;
    const char *db_host = DB_HOST_IP;
    int db_port = DB_HOST_PORT;
    const char *db_dbname = DB_DATABASE_NAME;
    const char *db_username = DB_USERNAME;
    const char *db_password = DB_PASSWORD;

    // 每个连接池都对应一个对象
    CDBPool *pDBPool = new CDBPool(db_pool_name, db_host, db_port,
                                   db_username, db_password, db_dbname, db_maxconncnt);

    if (pDBPool->Init()) {
        printf("init db instance failed: %s", db_pool_name);
        return -1;
    }

    CDBConn *pDBConn = pDBPool->GetDBConn(); // 获取连接
    if (pDBConn) {
        bool ret = pDBConn->ExecuteDrop(DROP_IMUSER_TABLE); // 删除表
        if (ret) {
            printf("DROP_IMUSER_TABLE ok\n");
        }
        // 1. 创建表
        ret = pDBConn->ExecuteCreate(CREATE_IMUSER_TABLE); // 创建表
        if (ret) {
            printf("CREATE_IMUSER_TABLE ok\n");
        }
        else {
            printf("ExecuteCreate failed\n");
            return -1;
        }
    }
    pDBPool->RelDBConn(pDBConn);

    printf("thread_num = %d, connection_num:%d, use_pool:0\n",
           thread_num, db_maxconncnt);
    ZERO_ThreadPool threadpool;
    threadpool.init(thread_num); // 设置线程数量
    threadpool.start();          // 启动线程池
    for (int i = 0; i < task_num; i++) {
        threadpool.exec(workNoPool, (void *) pDBPool, i);  // 主要在于执行函数的区别。
    }

    threadpool.waitForAllDone(); // 等待所有执行万再退出
    threadpool.stop();
    return 0;
}

// select name,phone  from IMUser;
int main(int argc, char *argv[]) {
    int thread_num = 1;                  // 线程池线程数量
    int db_maxconncnt = DB_POOL_MAX_CON; // 连接池最大连接数量(核数*2 + 磁盘数量)
    int use_pool = 1;                    // 是否使用连接池
    int task_num = TASK_NUMBER;
    if (argc != 4 && argc != 5) {
        printf("usage:  ./test_ThreadPool thread_num db_maxconncnt use_pool\n \
                example: ./test_ThreadPool 4  4 1\n");

        return 1;
    }
    thread_num = atoi(argv[1]);
    db_maxconncnt = atoi(argv[2]);
    use_pool = atoi(argv[3]);
    if (argc == 5) {
        task_num = atoi(argv[4]);
    }

    if (use_pool) {
        printf("testWorkUsePool\n");
        testWorkUsePool(thread_num, db_maxconncnt, task_num);
    }
    else {
        printf("testWorkNoPool\n");
        testWorkNoPool(thread_num, db_maxconncnt, task_num);
    }

    cout << "main finish!" << endl;
    return 0;
}
