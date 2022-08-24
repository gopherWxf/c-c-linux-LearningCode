cmake_minimum_required(VERSION 2.6)

project (pb)

ADD_DEFINITIONS(-W -Wall -D_REENTRANT -D_FILE_OFFSET_BITS=64 -DAC_HAS_INFO
-DAC_HAS_WARNING -DAC_HAS_ERROR -DAC_HAS_CRITICAL -DTIXML_USE_STL
-DAC_HAS_DEBUG -DLINUX_DAEMON -std=c++11)

set(pb_srcs
    Codec.pb.cc
)

add_executable(code_test code_test.cpp ${pb_srcs})
TARGET_LINK_LIBRARIES(code_test protobuf pthread)
# TARGET_LINK_LIBRARIES(code_test protobuf-lite pthread)




