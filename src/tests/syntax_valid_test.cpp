#include <cstdio>

#include "ParseUtil.hpp"
#include "script_runtime.hpp"

using namespace smart;

void testSimpleCalculation();
void testNodeTypeEquality();
void testParsing();

int main()
{
    printf("cshort");
    fflush(stdout);

    testSimpleCalculation();
    testParsing();
    testNodeTypeEquality();

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
    int result = ScriptEnv::startScript(source);
    printf("result: %d", result);
    assert(result == 16);
}


constexpr auto *text = const_cast<char *>(u8R"(
class FooClass
{
    fn funcB()
    {
        int a = 3124
    }
}
    )");


constexpr auto *testCode3 = const_cast<char *>(u8R"(
class OuterClass
{
    class InnerClass/*[A]this is so nice![A]*/
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
constexpr auto *testCode4 = const_cast<char*>( u8R"(
class fjawioejap
{
    fn funcB()
    {
        // comment
        let a = 893214 // fawoiefjiawo
        let *str = "0jfoiwjoie" // comment
        int a = 123412
        float test = 3214

        float f = 4503
        ?let *f = null
        let g = true
        a = 5
        varibleAjoawiefjaipo
        
        "jfoiwjeioaf"
            

            /*
            comment here
            */

        let f = 3

        
        (true) + false + (
            241
        )
        
        funcA(true, "jfiwo")

        return
        return 3421
    }
}

class/**/A/**/{

}
)");

const char commentTestText[] = " /* \r\n  bck\r\n\n*/";
const char parenthesisText[] = "fn a() { (12) }";

void checkTextEquality(const char *name, const char* code)
{
    printf("checking: %s\n", name);
    fprintf(stderr, "checking: %s\n", name);

    auto *document = Alloc::newDocument(DocumentType::CodeDocument);
    DocumentUtils::parseText(document, code, strlen(code));
    char *treeText = DocumentUtils::getTextFromTree(document);

    assert(document->context->syntaxErrorInfo.hasError == false);

    if (strcmp(code, treeText) == 0) {

    }
    else {
        printf("expected:\n%s\n", code);
        printf("actual:\n%s\n", treeText);
        assert(false && "text not equal");
    }

    free(treeText);
    Alloc::deleteDocument(document);
}

#define FUNC(x) checkTextEquality(#x, x)
void testParsing()
{
    FUNC(text);
    FUNC(testCode3);
    FUNC(testCode4);
    FUNC(commentTestText);
    FUNC(parenthesisText);
    FUNC(""); // empty text
    FUNC(" \r\n \n\n  ");

}

void testNodeTypeEquality() {
    std::string text = u8R"(

class A
{
    class B
    {
        class TestCl😂日本語10234ass
        {

            fn aFunc ()
            {
                
            }

        }

        class C { }
    }
}
)";

    const char *chars = text.c_str();
    auto *document = Alloc::newDocument(DocumentType::CodeDocument);

    DocumentUtils::parseText(document, chars, text.size());

    char *treeText = DocumentUtils::getTextFromTree(document);
    assert(std::string(treeText) ==  std::string(chars));
    assert(strlen(treeText) == strlen(chars));

    assert(document->context->syntaxErrorInfo.hasError == false);


    assert(document->firstCodeLine->firstNode->vtable == VTables::LineBreakVTable);
    assert(document->firstCodeLine->nextLine->firstNode->vtable == VTables::LineBreakVTable);

    Alloc::deleteDocument(document);
}