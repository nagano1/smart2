set(BINARY ${CMAKE_PROJECT_NAME}_tst)

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false
        ../src/thread_pool.cpp
        ../src/utils.cpp
        ../src/codenodes/*.cpp
        ../src/script_runtime/*.cpp
        google_tests/*.cpp
        google_tests/*.h
        ./test.cpp
        ./test.h
        )



set(SOURCES ${TEST_SOURCES})

add_executable(${BINARY} ${TEST_SOURCES})
if(DEFINED DOOR32)
    set_target_properties(${BINARY} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif()


add_definitions("-DCMAKE_TEST=1")

if(NO_SPEED_TEST EQUAL 1)
    add_definitions("-DNO_SPEED_TEST=1")
else()
    #add_definitions("-DNO_SPEED_TEST=0")
endif()

#add_test(NAME ${BINARY} COMMAND ${BINARY})

find_package(Threads REQUIRED)
target_link_libraries(${BINARY} PUBLIC ${CMAKE_PROJECT_NAME}_lib gtest Threads::Threads)

