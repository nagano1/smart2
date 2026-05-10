#include <cstdio>

#include "code_nodes.hpp"
#include "parse_util.hpp"
#include "script_runtime.hpp"
#include "main.h"

bool stopped = false;
int wakeup_count = 0;

using namespace smart;

int main()
{
    printf("cshort");
    fflush(stdout);

    assert(false);

    testAssignment();
    testAssignment2();
    testAssignment3();

    return 0;
}

void testAssignment2()
{
    {
        auto *text = const_cast<char *>(u8R"(
class FooClass
{
    fn funcB()
    {
        let a = 893214
        let *str = "0jfoiwjoie"
        int ab = 123412

        #float f = 4503
        ?let *f = null
        $let g = true
        
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
        auto *document = Alloc::newDocument(DocumentType::CodeDocument, nullptr);
        DocumentUtils::parseText(document, text, strlen(text));
        char *treeText = DocumentUtils::getTextFromTree(document);

        free(treeText);
        Alloc::deleteDocument(document);
    }
}

void testAssignment()
{
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
}

void testAssignment3()
{
    {
        auto *text = const_cast<char *>(u8R"(
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
        auto *document = Alloc::newDocument(DocumentType::CodeDocument, nullptr);
        DocumentUtils::parseText(document, text, strlen(text));
        char *treeText = DocumentUtils::getTextFromTree(document);

        free(treeText);
        Alloc::deleteDocument(document);
    }
}