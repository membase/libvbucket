CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
   # We are being built as a single module
   SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../tlm/cmake/Modules/")
   INCLUDE(CouchbaseSingleModuleBuild)
ENDIF ()

ADD_LIBRARY(vbucket SHARED src/vbucket.c src/ketama.c src/crc32.c)
SET_TARGET_PROPERTIES(vbucket PROPERTIES SOVERSION 1.1.1)
SET_TARGET_PROPERTIES(vbucket PROPERTIES
                      COMPILE_FLAGS -DBUILDING_LIBVBUCKET=1)

ADD_LIBRARY(cJSON SHARED src/cJSON.c)
SET_TARGET_PROPERTIES(cJSON PROPERTIES SOVERSION 1.0.0)
SET_TARGET_PROPERTIES(cJSON PROPERTIES COMPILE_FLAGS -DBUILDING_CJSON=1)

INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/include
                           ${CMAKE_CURRENT_SOURCE_DIR})

IF (WIN32)
    INCLUDE_DIRECTORIES(AFTER ${CMAKE_CURRENT_SOURCE_DIR}/win32)
    TARGET_LINK_LIBRARIES(vbucket cJSON)
ELSE (WIN32)
    TARGET_LINK_LIBRARIES(vbucket cJSON m)
ENDIF (WIN32)

INSTALL(FILES
        include/libvbucket/vbucket.h
        include/libvbucket/visibility.h
        DESTINATION include/libvbucket)
INSTALL(FILES include/cJSON.h DESTINATION include)

INSTALL(TARGETS vbucket cJSON
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

ADD_EXECUTABLE(vbuckettool src/vbuckettool.c)
TARGET_LINK_LIBRARIES(vbuckettool vbucket)

ADD_EXECUTABLE(vbucketkeygen src/vbucketkeygen.c)
TARGET_LINK_LIBRARIES(vbucketkeygen vbucket)

#
# The tests. These are automatically executed as part of the build!
#
ADD_EXECUTABLE(libvbucket_testapp tests/testapp.c)
TARGET_LINK_LIBRARIES(libvbucket_testapp vbucket)

ADD_EXECUTABLE(libvbucket_regression tests/regression.c)
TARGET_LINK_LIBRARIES(libvbucket_regression vbucket)

ADD_EXECUTABLE(libvbucket_testketama tests/testketama.c src/ketama.c)
SET_TARGET_PROPERTIES(libvbucket_testketama PROPERTIES
                      COMPILE_FLAGS -I${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(libvbucket_testketama vbucket)

ADD_TEST(libvbucket-basic-tests libvbucket_testapp ${CMAKE_CURRENT_SOURCE_DIR})
ADD_TEST(libvbucket-regression-tests libvbucket_regression ${CMAKE_CURRENT_SOURCE_DIR})
ADD_TEST(libvbucket-ketama-tests libvbucket_testketama)
