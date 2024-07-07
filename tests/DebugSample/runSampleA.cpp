//#include "test.h"

#include <stdio.h>
#include <iostream>
#include <string>
#include <mutex>
#include <thread>
#include <array>
#include <algorithm>


#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <atomic>
#include <unordered_map>
#include <vector>

#include <cstdint>
#include <ctime>

#include "code_nodes.hpp"
#include "parse_util.hpp"
#include "script_runtime.hpp"

//#include "gtest/gtest.h"

using namespace smart;

int main(int argc, char **argv) {
     constexpr char source[] = R"(
fn main()
{
    int b = 9
    int a = 500
    int c = 500
    
    return c - (b + a)
}
)";
    printf("%s", source);
    int ret = ScriptEnv::startScript((char*)source, sizeof(source) - 1);
    printf("ret = %d", ret);
	return 0;
}
