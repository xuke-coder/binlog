# project root path
PROJECT ("")
SET (SRC_PATH "${PROJECT_SOURCE_DIR}/../")
SET (ROOT_PATH "${SRC_PATH}/../")
SET (COMMON_CMAKE "common.cmake")
# include common define
INCLUDE ("${SRC_PATH}/${COMMON_CMAKE}")
# the whole CMakeLists.txt for project
PROJECT ("${PROJ_NAME}")

# cmake version required
CMAKE_MINIMUM_REQUIRED(VERSION "${CMAKE_MIN_VERSION}")
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

# sub module
SET (INDEX_TEST_BIN "test_write")

# src files (must use relative path)
FILE(GLOB_RECURSE INDEX_TEST_BIN_SRC RELATIVE ${PROJECT_SOURCE_DIR}  
 	*.cpp	 
 )
MESSAGE(STATUS "INDEX_TEST_BIN_SRC: ${INDEX_TEST_BIN_SRC}")
ADD_EXECUTABLE("${INDEX_TEST_BIN}"  ${INDEX_TEST_BIN_SRC})
TARGET_LINK_LIBRARIES("${INDEX_TEST_BIN}" libxf_index.a)
TARGET_LINK_LIBRARIES("${INDEX_TEST_BIN}" libxf_index_proto.a)
TARGET_LINK_LIBRARIES("${INDEX_TEST_BIN}" libssf_common.a)
TARGET_LINK_LIBRARIES("${INDEX_TEST_BIN}" libssf_dbuf_ht.a)
TARGET_LINK_LIBRARIES("${INDEX_TEST_BIN}" libredis_wrapper.a)
TARGET_LINK_LIBRARIES("${INDEX_TEST_BIN}" libhiredis.a)
TARGET_LINK_LIBRARIES("${INDEX_TEST_BIN}" libbase.a)
TARGET_LINK_LIBRARIES("${INDEX_TEST_BIN}" libgflags.a;;)
TARGET_LINK_LIBRARIES("${INDEX_TEST_BIN}" nameapi;)
TARGET_LINK_LIBRARIES("${INDEX_TEST_BIN}" libprotobuf.a;)
TARGET_LINK_LIBRARIES("${INDEX_TEST_BIN}" libtcmalloc.a;)
TARGET_LINK_LIBRARIES("${INDEX_TEST_BIN}" pthread;rt;dl;)
TARGET_LINK_LIBRARIES("${INDEX_TEST_BIN}" libbossapi.a;)

