#include <cstdio>

#include "ParseUtil.hpp"
#include "script_runtime.hpp"

using namespace smart;
void callAllTests();
int main()
{
    callAllTests();
    return 0;
}

int testA() {
    assert(true == ParseUtil::isIdentifierLetter('a'));

    // ParseUtil::letterCheck(&func);

    // static inline int indexOfBreakOrEnd(const char *chars, int charsLength, int startIndex)
    {
        static constexpr char chars[] = "class\n A{}";
        assert(5 == ParseUtil::indexOfBreakOrEnd(chars, sizeof(chars)-1, 0));

        static constexpr char chars2[] = "class\0 A{}";
        assert(5 == ParseUtil::indexOfBreakOrEnd(chars2, sizeof(chars2) - 1, 3));

        static constexpr char chars3[] = "class\0 A{}";
        assert(10 == ParseUtil::indexOfBreakOrEnd(chars3, sizeof(chars3) - 1, 7));
    }

    {
        static constexpr char chars[] = "class\n A{}";
        assert(7 == ParseUtil::indexOf(chars, sizeof(chars)-1, 0, 'A'));

        static constexpr char chars2[] = "class\n A{}";
        assert(-1 == ParseUtil::indexOf(chars2, sizeof(chars2)-1, 0, 'G'));

    }


    {
        static constexpr char chars[] = "return    \n";
        assert(false == ParseUtil::hasCharBeforeLineBreak(chars, sizeof(chars)-1, 6));

        static constexpr char chars2[] = "return    a\r\n";
        assert(true == ParseUtil::hasCharBeforeLineBreak(chars2, sizeof(chars2)-1, 6));

    }

    assert(true == ParseUtil::isIdentifierLetter('a'));
    assert(true == ParseUtil::isIdentifierLetter(std::string{ u8"😂" }.c_str()[0]));
    assert(false == ParseUtil::isIdentifierLetter('\n'));


    static constexpr char chars[] = "class A{}";
    assert(0 == ParseUtil::matchAt(chars, sizeof(chars) - 1, 0, "class"));

    assert(-1 == ParseUtil::matchAt("", 0, 0, "class"));
    assert(-1 == ParseUtil::matchAt("", 0, 0, ""));

    constexpr char txt[] = "aefvariable aowef \n";
    assert(-1 == ParseUtil::matchAt(txt, sizeof(txt)-1, 2, "false"));


    {
        std::string class_text(u8"     \tclassauto * 😂日本語=10234;");
        int index = ParseUtil::matchAt(class_text.c_str(), class_text.length(), 0, "class");
        assert(6 == index);
    }


    {
        std::string class_text(u8"😂classauto;");
        int index = ParseUtil::matchAt(class_text.c_str(), class_text.length(), 0, "class");
        assert(-1 == index);
    }

    return 0;

    // matchWord
    {
        std::string class_text(u8"class");
        assert(class_text.length() == 5);
        auto result = ParseUtil::matchWord(class_text.c_str(), class_text.length(), "class", 5, 0);
        assert(result == true);
    }

    {
        std::string class_text(u8" class"); //space
        auto result = ParseUtil::matchWord(class_text.c_str(), class_text.length(), "class", 5, 0);
        assert(result == false);
    }

    {
        std::string class_text(u8"abcclass");
        auto result = ParseUtil::matchWord(class_text.c_str(), class_text.length(), "class", 5, 3);
        assert(result == true);
    }

    {
        std::string class_text(u8"classauto;");
        auto result = ParseUtil::matchWord(class_text.c_str(), class_text.length(), "class", 5, 0);
        assert(result == true);
    }

    {
        std::string text(u8"ab");
        auto result = ParseUtil::matchWord(text.c_str(), text.length(), "abcdefg", 5, 0);
        assert(result == false);
    }



    {
        // endsWith
        {
            std::string text(u8"ab");
            auto result = ParseUtil::endsWith2(text.c_str(), text.length(), "ab");
            assert(result == true);
        }

        {
            std::string text(u8"abcdefg");
            auto result = ParseUtil::endsWith2(text.c_str(), text.length(), "efg");
            assert(result == true);
        }
        {
            std::string text(u8"abcd");
            auto result = ParseUtil::endsWith2(text.c_str(), text.length(), "aabcd");
            assert(result == false);
        }
    }
}


void callAllTests() {
    testA();
}