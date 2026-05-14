set(BINARY_A "test")
set(BINARY_B "ParseUtilsTest")
#set(BINARY ${CMAKE_PROJECT_NAME})

file(GLOB cpp_lib_files
    "../utils/utils.cpp"
    "../abstract_syntax_tree/*.cpp"
    "../abstract_syntax_tree/nodes/*.cpp"
    "../script_runtime/*.cpp"
)


add_executable(${BINARY_A}_run ${cpp_lib_files} "./test.cpp")
add_executable(${BINARY_B}_run ${cpp_lib_files} "./parse_utils_test.cpp")

install(TARGETS ${BINARY_A}_run
 RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
 BUNDLE  DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
 LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}

 
 )
install(TARGETS ${BINARY_B}_run RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)


# add_library(${BINARY}_lib STATIC ${cpp_files})

#find_package(Threads REQUIRED)
#target_link_libraries(${BINARY}_run Threads::Threads)