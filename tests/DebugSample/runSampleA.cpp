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

#include "parse_util.hpp"
#include "script_runtime.hpp"
#include "thread_pool.hpp"
#include "common.hpp"

//#include "gtest/gtest.h"

int main(int argc, char **argv) {
	//::testing::InitGoogleTest(&argc, argv);
	//return RUN_ALL_TESTS();
            constexpr char source[] = R"(
fn main()
{
    int b = 9
    int a = 500
    int c = 500
    
    return c - (b + a)
}
)";
        int ret = ScriptEnv::startScript((char*)source, sizeof(source) - 1);
	return 0;
}
