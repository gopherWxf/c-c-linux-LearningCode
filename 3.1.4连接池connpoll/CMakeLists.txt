cmake_minimum_required(VERSION 2.8)
PROJECT(test_curd)

SET(SRC_LIST
        DBPool.cpp IMUser.cpp
        )

SET(EXECUTABLE_OUTPUT_PATH ./)


ADD_DEFINITIONS(-g -W -Wall -D_REENTRANT -D_FILE_OFFSET_BITS=64 -DAC_HAS_INFO
        -DAC_HAS_WARNING -DAC_HAS_ERROR -DAC_HAS_CRITICAL -DTIXML_USE_STL
        -DAC_HAS_DEBUG -DLINUX_DAEMON -std=c++11 -DENCRYPT)

INCLUDE_DIRECTORIES(./)
# 注意自己的库路径
INCLUDE_DIRECTORIES(/usr/include/mysql)
LINK_DIRECTORIES(/usr/lib/x86_64-linux-gnu/ /usr/lib64/mysql)

ADD_EXECUTABLE(test_curd test_curd.cpp ${SRC_LIST})
ADD_EXECUTABLE(test_dbpool test_dbpool.cpp ZeroThreadpool.cpp ${SRC_LIST})


TARGET_LINK_LIBRARIES(test_curd mysqlclient pthread)
TARGET_LINK_LIBRARIES(test_dbpool mysqlclient pthread) 