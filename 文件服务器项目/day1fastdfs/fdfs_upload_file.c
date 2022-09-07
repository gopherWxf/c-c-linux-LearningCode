#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fdfs_client.h"
#include <sys/wait.h>

//使用多进程方式
int upload_file1(const char *confFile, const char *uploadFile, char *fileID, int size) {
    //1. 创建管道
    int fd[2];
    int ret = pipe(fd);
    if (ret == -1) {
        perror("pipe error");
        exit(0);
    }
    //2. 创建子进程
    pid_t pid = fork();
    //子进程
    if (pid == 0) {
        //3. 标准输出重定向 -> pipe写端
        dup2(fd[1], STDOUT_FILENO);
        //4. 关闭读端
        close(fd[0]);
        execlp("fdfs_upload_file", "fdfs_upload_file", confFile, uploadFile, NULL);
        perror("execlp error");
    }
    else {
        //父进程读管道
        close(fd[1]);
        read(fd[0], fileID, size);
        //回收子进程的PCB
        wait(NULL);
    }

}

//使用fastDFS API实现
int upload_file2(const char *confFile, const char *myFile, char *fileID) {
    char group_name[FDFS_GROUP_NAME_MAX_LEN + 1];
    ConnectionInfo *pTrackerServer;
    int result;
    int store_path_index;
    ConnectionInfo storageServer;

    //TODO fdfs_client_init解析传入的配置文件
    if ((result = fdfs_client_init(confFile)) != 0) {
        return result;
    }
    //TODO tracker_get_connection在上面client配置文件里面
    //TODO 有追踪器的ip:port，所以这里就是连接追踪器，初始化pTrackerServer指针
    pTrackerServer = tracker_get_connection();
    if (pTrackerServer == NULL) {
        fdfs_client_destroy();
        return errno != 0 ? errno : ECONNREFUSED;
    }

    *group_name = '\0';

    //TODO tracker_query_storage_store追踪器 查询 存储节点 的 存储
    //TODO storageServer是传入参数，对storageServer赋值
    //TODO group_name是一个char数组，对group_name赋值
    //TODO store_path_index是传入参数，对store_path_index赋值
    if ((result = tracker_query_storage_store(pTrackerServer, \
                    &storageServer, group_name, &store_path_index)) != 0) {
        fdfs_client_destroy();
        fprintf(stderr, "tracker_query_storage fail, " \
            "error no: %d, error info: %s\n", \
            result, STRERROR(result));
        return result;
    }
    //TODO storage_upload_by_filename1查到了存储节点，就进行上传的动作
    //TODO file_id传出参数，如果上传成功了，就打印file_id
    result = storage_upload_by_filename1(pTrackerServer, \
            &storageServer, store_path_index, \
            myFile, NULL, \
            NULL, 0, group_name, fileID);
    if (result == 0) {
        printf("%s\n", fileID);
    }
    else {
        fprintf(stderr, "upload file fail, " \
            "error no: %d, error info: %s\n", \
            result, STRERROR(result));
    }
    //TODO tracker_close_connection_ex与追踪器断开连接
    tracker_close_connection_ex(pTrackerServer, true);
    fdfs_client_destroy();

    return result;
}

