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

const char commentTestText[] = u8" /*[A] \r\n/**/  bck\r\n\n[A]*/";
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
        fprintf(stderr, "expected:\n%s\n", code);
        fprintf(stderr, "actual:\n%s\n", treeText);
        assert(false && "text not equal");
    }

    free(treeText);
    Alloc::deleteDocument(document);
}


void testNodeTypeEquality() {
    std::string text = u8R"(

class A
{
    class B
    {
        class TestCl😂日本語10234ass
        {

            fnaFunc ()
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



void aFunc() {
    
    std::string text = "class A \r\n // comment \r\n {}";

    const char* chars = text.c_str();
    auto* document = Alloc::newDocument(DocumentType::CodeDocument);
    DocumentUtils::parseText(document, chars, text.size());

    char* treeText = DocumentUtils::getTextFromTree(document);

    assert(treeText != nullptr);
    assert(std::string{ treeText } == std::string{ chars });
    assert(strlen(treeText) == strlen(chars));

    Alloc::deleteDocument(document);
}




void testTypeTreeTest() {
    std::string text = u8R"(
        class TestCl😂日本語10234ass
        {
            fn func()
            {
                let aw = 242
                true
                null
                printf(214)
            }
        }
)";

    const char* chars = text.c_str();
    auto* document = Alloc::newDocument(DocumentType::CodeDocument);
    DocumentUtils::parseText(document, chars, text.size());

    char* treeText = DocumentUtils::getTextFromTree(document);
    char* typeTreeText = DocumentUtils::getTypeTextFromTree(document);

    assert(treeText != nullptr);
    assert(typeTreeText != nullptr);
    assert(std::string{ treeText } == std::string{ chars });
    assert(strlen(treeText) == strlen(chars));


    auto&& typeTree = u8R"(<lineBreak>
<Class>        class<Name> TestCl😂日本語10234ass<lineBreak>
<Symbol>        {<lineBreak>
<fn>            fn<Name> func<Symbol>(<Symbol>)<lineBreak>
<body>            {<lineBreak>
<Type>                let<Name> aw<Symbol> =<number> 242<lineBreak>
<bool>                true<lineBreak>
<NULL>                null<lineBreak>
<Variable>                printf<Symbol>(<FuncArgument><number>214<Symbol>)<lineBreak>
<Symbol>            }<lineBreak>
<Symbol>        }<lineBreak>
<EndOfFile>)";

    assert(std::string{ typeTreeText } == std::string{ typeTree });

    Alloc::deleteDocument(document);
}



void DepthTest() {
    std::string text = u8R"(
class TestCl
{
    fn func()
    {
        let aw = 242
    }
}
)";

    const char* chars = text.c_str();
    auto* document = Alloc::newDocument(DocumentType::CodeDocument);
    DocumentUtils::parseText(document, chars, text.size());

    auto* line = document->firstCodeLine;
    int i = 0;
    int depthList[] = {0,0,0,1,1,2,1,0,0};
    while (line) {
        assert(line->depth == depthList[i]);
        line = line->nextLine;
        i++;
    }

    Alloc::deleteDocument(document);
}

/*
* 
fn method() : int {


}

prop 
{

}

public static bool
fn afunc(
    int intA
    double b
    float c
) {
    @Attr(awf="jofwie")
    mut a = 342


    @Attr(awf="jofwie")
    $let b = 224

    $let b = 3142
    let c = 314
    
    if true {
        a = 314
    }

    for i = 0
        i < 10
        i++ {

    }

    let res2 = method()

    method()
    =let myVarTranslation
    =set existingVar

    let res = try {
        method()
    } {
        ok(n) -> 
        Err(e) -> 0
        else -> false
    }

    let res3 = try method() else 0


    throws ErrorA()

    switch one {
        1 -> {
        
        }
        2 -> {

        }
    }


    when abc {
        3 => 45
        else => 54
    }
    
    0
    =let aboiajw

    32 / 123 - 4321 + 5
    =set ex
    =var wow


}
*/


void NodeTypeEqualityTest() {
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
    assert(std::string(treeText) == std::string(chars));
    assert(strlen(treeText) == strlen(chars));

    assert(document->context->syntaxErrorInfo.hasError == false);


    assert(document->firstCodeLine->firstNode->vtable == VTables::LineBreakVTable);
    assert(document->firstCodeLine->nextLine->firstNode->vtable == VTables::LineBreakVTable);


    Alloc::deleteDocument(document);
}


void ErrorNodeTest() {
    return;
    //std::string text = "   class           A   {    }   ";
    std::string text = u8R"(
class A {

    class B {
        @hoge(akaw=3242, ajwe=2342)
        class TestCl😂日本語10234ass {
            
        }

        cmpl fn fawe() {

        }
    }
}
class A {}
class A {}
class A {}
class BDD{}




class AABC  {  }
)";

    const char *chars = text.c_str();
    auto *document = Alloc::newDocument(DocumentType::CodeDocument);

    DocumentUtils::parseText(document, chars, text.size());

    char *treeText = DocumentUtils::getTextFromTree(document);
    assert(std::string(treeText) == std::string(chars));
    assert(strlen(treeText) == strlen(chars));

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

    NodeTypeEqualityTest();
    DepthTest();
    testTypeTreeTest();
}
