#include <cstdio>

#include "code_nodes.hpp"
#include "parse_util.hpp"
#include "script_runtime.hpp"


bool stopped = false;
int wakeup_count = 0;

using namespace smart;

int main()
{
    printf("cshort");
    fflush(stdout);

    //exit(1);
    //assert(false);

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
