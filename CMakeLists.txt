CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/include
                           ${CMAKE_CURRENT_SOURCE_DIR})

ADD_LIBRARY(vbucket SHARED
            include/cJSON.h
            include/libvbucket/vbucket.h
            include/libvbucket/visibility.h
            src/crc32.c
            src/hash.h
            src/hash.h
            src/ketama.c
            src/rfc1321/global.h
            src/rfc1321/md5.h
            src/vbucket.c)

SET_TARGET_PROPERTIES(vbucket PROPERTIES SOVERSION 1.1.1)
SET_TARGET_PROPERTIES(vbucket PROPERTIES
                      COMPILE_FLAGS -DBUILDING_LIBVBUCKET=1)
SET_TARGET_PROPERTIES(vbucket PROPERTIES INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/lib)

ADD_LIBRARY(cJSON SHARED src/cJSON.c include/cJSON.h)
SET_TARGET_PROPERTIES(cJSON PROPERTIES SOVERSION 1.0.0)
SET_TARGET_PROPERTIES(cJSON PROPERTIES COMPILE_FLAGS -DBUILDING_CJSON=1)
SET_TARGET_PROPERTIES(cJSON PROPERTIES INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/lib)


IF (WIN32)
    TARGET_LINK_LIBRARIES(vbucket cJSON)
ELSE (WIN32)
    TARGET_LINK_LIBRARIES(cJSON m)
    TARGET_LINK_LIBRARIES(vbucket cJSON m)
ENDIF (WIN32)

IF (INSTALL_HEADER_FILES)
   INSTALL(FILES
           include/libvbucket/vbucket.h
           include/libvbucket/visibility.h
           DESTINATION include/libvbucket)
   INSTALL(FILES include/cJSON.h DESTINATION include)
ENDIF(INSTALL_HEADER_FILES)

INSTALL(TARGETS vbucket cJSON
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

ADD_EXECUTABLE(vbuckettool
               include/libvbucket/vbucket.h
               include/libvbucket/visibility.h
               src/vbuckettool.c)
TARGET_LINK_LIBRARIES(vbuckettool vbucket)

ADD_EXECUTABLE(vbucketkeygen
               include/libvbucket/vbucket.h
               include/libvbucket/visibility.h
               src/vbucketkeygen.c)
TARGET_LINK_LIBRARIES(vbucketkeygen vbucket)

#
# The tests. These are automatically executed as part of the build!
#
ADD_EXECUTABLE(libvbucket_cjson_test include/cJSON.h tests/cjson_test.c)
TARGET_LINK_LIBRARIES(libvbucket_cjson_test cJSON)

ADD_EXECUTABLE(libvbucket_testapp
               include/libvbucket/vbucket.h
               include/libvbucket/visibility.h
               tests/macros.h
               tests/testapp.c)
TARGET_LINK_LIBRARIES(libvbucket_testapp vbucket)

ADD_EXECUTABLE(libvbucket_regression
               include/libvbucket/vbucket.h
               include/libvbucket/visibility.h
               tests/macros.h
               tests/regression.c)
TARGET_LINK_LIBRARIES(libvbucket_regression vbucket)

ADD_EXECUTABLE(libvbucket_testketama
               src/ketama.c
               src/rfc1321/global.h
               src/rfc1321/md5.h
               tests/testketama.c)
TARGET_LINK_LIBRARIES(libvbucket_testketama vbucket)

ADD_TEST(libvbucket-cjson-test libvbucket_cjson_test)
ADD_TEST(libvbucket-basic-tests libvbucket_testapp ${CMAKE_CURRENT_SOURCE_DIR})
ADD_TEST(libvbucket-regression-tests libvbucket_regression ${CMAKE_CURRENT_SOURCE_DIR})
ADD_TEST(libvbucket-ketama-tests libvbucket_testketama)
