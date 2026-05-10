#include <cstdio>

#include "parse_util.hpp"
#include "script_runtime.hpp"

using namespace smart;

void testSimpleCalculation();
void testParsing();

int main()
{
    printf("cshort");
    fflush(stdout);

    testSimpleCalculation();
    testParsing();

    return 0;
}

constexpr char source[] = R"(
fn Main()
{
    int a = 8
    int b = 1
    int c = -9
    
    return a - (b + c)
}
)";

void testSimpleCalculation()
{
    printf("%s", source);
    int ret = ScriptEnv::startScript(source);
    printf("returned value: %d", ret);
}


constexpr auto *text = const_cast<char *>(u8R"(
class FooClass
{
    fn funcB()
    {
        let a = 893214
        let *str = "0jfoiwjoie"
        int ab = 123412

        float f = 4503
        ?let *f = null
        let g = true
        
        "abcdefg"
        3142
        null
        false
        true
        unknownIdentifier
        true
        "string sample"

        /*
          block comment test
          コメントテスト
        */
        // line comment test
        // コメントテスト

        "fjoiiw" // comment test

        let *abc = "joifwjoe01234"
        let f = 343214213
        int a = 3124
    }
}
    )");


constexpr auto *testCode3 = const_cast<char *>(u8R"(
class OuterClass
{
    class InnerClass/**/
    {/**/
        // awef
        fn func1()
        {
            // jfoiaweoifaw
            
            func(true, "jfoiw", 1203)
            
            return/*true*/3241//面白すぎ
            return/**/21241
            false//tugi ga saigono kyokudesu
            return 1

        }


        fn a()
        {
            // afjiowe

        } // afweo
    } // joiwafjoefwa 
    //
    /* fwaei */
}
)");

void testParsing(char *code)
{
    auto *document = Alloc::newDocument(DocumentType::CodeDocument, nullptr);
    DocumentUtils::parseText(document, code, strlen(code));
    char *treeText = DocumentUtils::getTextFromTree(document);

    assert(strcmp(code, treeText) == 0);

    free(treeText);
    Alloc::deleteDocument(document);
}


void testParsing()
{
    testParsing(text);
    testParsing(testCode3);
}