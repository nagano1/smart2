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

using namespace smart;

int main(int argc, char **argv) {

    constexpr char source[] = R"(
fn Main()
{
    int b = 1
    int a = 1
    int c = -9
    
    return c - (b - a)
}
)";
    printf("%s", source);
    int ret = ScriptEnv::startScript(source);
    printf("ret = %d", ret);

	return 0;
}