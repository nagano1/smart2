
#include <stdio.h>
#include <iostream>
#include <string>
#include <array>
#include <algorithm>


#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <unordered_map>
#include <vector>

#include <cstdint>
#include <ctime>
#include <stdint.h>

#include "code_nodes.hpp"

namespace smart
{
    ErrorInfo ErrorInfo::ErrorInfoList[errorListSize];
    bool ErrorInfo::errorInfoInitialized{false};
    //static int _ab = initErrorInfoList();

    
    struct InnerParsingData;

    int Scanner::scanOnce(void *parentNode,
                      TokenizerFunction tokenizer,
                      int start,
                      ParseContext *context
    ) {

        return Scanner::scanWithTokenizer(parentNode, tokenizer, start, context, false, false);
    }

    // scan until scanEnd==true, tokenizer should set scanEnd to true when it wants to stop scanning
    int Scanner::scanMulti(void *parentNode,
        TokenizerFunction tokenizer,
        int start,
        ParseContext *context
    ) {
        return Scanner::scanWithTokenizer(parentNode, tokenizer, start, context, false, true);
    }

    CodeLine *VTableCall::callAppendToLine(void *node, CodeLine *currentCodeLine) {
        if (node == nullptr) {
            return currentCodeLine;
        }
        auto *nodeBase = Cast::upcast(node);
        //if (nodeBase->prevLineBreakNode)
        return nodeBase->vtable->appendToLine(nodeBase, currentCodeLine);
    }

    
    static inline int detectBlockCommentEnd(int32_t i, smart::ParseContext *context, int &tagLength, char *&tagText)
    {
        int textStartPos = i + 2;

        if (context->chars[i + 2] == '[')
        { // /*[hoge] ... [hoge]*/
            int nameStartPos = i + 3;
            // finds out the tag name: hoge
            int endOfStartTagPos = ParseUtil::indexOf(context->chars, context->length, nameStartPos, ']');
            int lineEndPos = ParseUtil::indexOfBreakOrEnd(context->chars, context->length, nameStartPos);
            // the name of the named block comment must be in the same line with the start tag
            if (endOfStartTagPos > -1 && endOfStartTagPos < lineEndPos)
            {
                tagLength = endOfStartTagPos - nameStartPos;
                textStartPos = endOfStartTagPos + 1;
                tagText = context->memBuffer.newMem<char>(tagLength + 1);
                TEXT_MEMCPY(tagText, context->chars + nameStartPos, tagLength);
                tagText[tagLength] = '\0';
            }
        }

        int searchEndPos = textStartPos;
        while (true)
        {
            int endCommentPos = ParseUtil::indexOf2(context->chars, context->length, searchEndPos, '*', '/');

            if (endCommentPos == -1)
            {
                // the end of block comment not found, treat the rest of chars as comment
                return context->length;
            }

            if (tagLength > 0)
            {
                // [hoge]*/
                if (context->chars[endCommentPos - 1] == ']'
                     && context->chars[endCommentPos - tagLength - 2] == '['
                     && ParseUtil::matchWord(context->chars, context->length, tagText, tagLength, endCommentPos - tagLength - 1))
                {
                    // the end tag of the named block comment found
                    return endCommentPos + 2;
                }
                else
                {
                    // not the end of the named block comment, continue to search
                    searchEndPos = endCommentPos + 2;
                    continue;
                }
            }
            else
            {
                // not a named block comment, the first */ is the end of the block comment
                return endCommentPos + 2;
            }
            break;
        }

        return -1;
    }

    inline NodeBase* generateBlockCommentFragments(void *parentNode, ParseContext *context,
                                           const int32_t &i, int commentEndIndex, char* tagText, int tagLength) {
        auto *blockComment = Alloc::newBlockCommentNode(context, Cast::upcast(parentNode));
        blockComment->tagText = tagText;
        blockComment->tagTextLength = tagLength;

        int currentIndex = i;
        BlockCommentFragmentStruct *lastNode = nullptr;
        LineBreakNodeStruct *lastBreakLine = nullptr;

        // split block comment into fragments by line break, and create LineBreakNodeStruct for each line break
        while (currentIndex <= commentEndIndex) {
            int endIndex  = ParseUtil::indexOfBreakOrEnd(context->chars, context->length, currentIndex);
            if (commentEndIndex < endIndex) {
                endIndex = commentEndIndex;
            }

            if (endIndex > -1 && currentIndex <= endIndex) {
                auto *commentFragment = Alloc::newBlockCommentFragmentNode(context, Cast::upcast(parentNode));

                // link with previous line break node
                commentFragment->prevLineBreakNode = lastBreakLine;

                int commentLength = endIndex - currentIndex;
                Init::assignText_SimpleTextNode(commentFragment, context, currentIndex, commentLength);

                // create a line break node for the line break after the comment fragment
                auto *newLineBreak = Alloc::newLineBreakNode(context, Cast::upcast(parentNode));
                bool rn = context->chars[endIndex] == '\r' && context->chars[endIndex+1] == '\n';
                if (rn) { // \r\n
                    newLineBreak->text[0] = '\r';
                    newLineBreak->text[1] = '\n';
                    newLineBreak->text[2] = '\0';
                    currentIndex = endIndex + 2;
                }
                else {
                    currentIndex = endIndex + 1;
                }

                lastBreakLine = newLineBreak;

                if (lastNode != nullptr) {
                    lastNode->nextNode = Cast::upcast(commentFragment);
                }
                lastNode = commentFragment;
                if (blockComment->firstCommentFragment == nullptr) {
                    blockComment->firstCommentFragment = commentFragment;
                }
            }
            else {
                break;
            }
        }
        return Cast::upcast(blockComment);
    }


    struct InnerParsingData
     {
        int32_t whitespace_startpos = -1;

        LineBreakNodeStruct *prevLineBreak = nullptr;
        LineBreakNodeStruct *lastLineBreak = nullptr;

        NodeBase *commentNode = nullptr;

        void assignCommentNode(NodeBase* leftNode) {
            assert(leftNode != nullptr);

            if (commentNode != nullptr) {
                leftNode->prevCommentNode = Cast::upcast(commentNode);
                commentNode = nullptr;
            }
        }

        void assignWhiteSpaces(NodeBase* comment2, int i) {
            if (whitespace_startpos != -1 && whitespace_startpos < i) {
                comment2->prev_chars = i - whitespace_startpos;
                whitespace_startpos = -1;
            }
            /*
            if (parsingResult->whitespace_startpos != -1) {
                if (parsingResult->whitespace_startpos < position) {
                    (*lastLineBreak)->prev_chars = position - parsingResult->whitespace_startpos;
                }
                parsingResult->whitespace_startpos = -1;
            }

            */
        }
    };
    


    int tryDetectComments(ParseContext* context, int32_t i, void* parentNode, InnerParsingData* parsingResult)
    {
        int commentEndIndex = -1;
        bool isLineComment = false;

        char *tagText = nullptr; // name of named block comment
        int tagLength = 0;

        // line comment with "//"
        if ('/' == context->chars[i + 1]) {
            commentEndIndex = ParseUtil::indexOfBreakOrEnd(context->chars, context->length, i);
            isLineComment = true;

        } // block comment /* */
        else if ('*' == context->chars[i + 1]) {
            commentEndIndex = detectBlockCommentEnd(i, context, tagLength, tagText);
        }

        if (commentEndIndex > -1) {
            auto* prevCommentNode = parsingResult->commentNode;

            NodeBase *newCommentNode;
            if (isLineComment) {
                auto* comment = Alloc::newLineCommentNode(context, Cast::upcast(parentNode));
                Init::assignText_SimpleTextNode(comment, context, i, commentEndIndex - i);

                newCommentNode = Cast::upcast(comment);
            }
            else {
                newCommentNode = generateBlockCommentFragments(parentNode, context, i, commentEndIndex, tagText, tagLength);
            }

            parsingResult->commentNode = newCommentNode;
            parsingResult->assignWhiteSpaces(newCommentNode, i);

            if (prevCommentNode != nullptr) {
                newCommentNode->prevCommentNode = prevCommentNode;
            }

            if (parsingResult->prevLineBreak != nullptr) {
                newCommentNode->prevLineBreakNode = parsingResult->prevLineBreak;
                parsingResult->prevLineBreak = nullptr;
            }
        }

        return commentEndIndex;
    }

    

    int createLineBreakNode(smart::ParseContext* context, void* parentNode,
        int32_t& position, utf8byte ch, InnerParsingData* parsingResult)
    {
        auto* newLineBreak = Alloc::newLineBreakNode(context, Cast::upcast(parentNode));

        if (parsingResult->prevLineBreak == nullptr) {
            parsingResult->lastLineBreak = parsingResult->prevLineBreak = newLineBreak;
        }
        else {
            parsingResult->lastLineBreak->nextLineBreakNode = newLineBreak;
            parsingResult->lastLineBreak = newLineBreak;
        }

        parsingResult->assignWhiteSpaces(Cast::upcast(newLineBreak), position);
        parsingResult->assignCommentNode(Cast::upcast(newLineBreak));

        bool rn = ch == '\r' && context->chars[position + 1] == '\n';
        int result;
        if (rn) { // \r\n
            newLineBreak->text[0] = '\r';
            newLineBreak->text[1] = '\n';
            newLineBreak->text[2] = '\0';
            result = position + 2;
        }
        else {
            result = position + 1;
        }
        return result;
    }


    // scan with the given tokenizer, if root is true, it will save the last line break node and comment node to context for later use,
    // if scanMulti is true, it will continue to scan after a token is found until scanEnd is set to true by tokenizer
    // this scanner handles spaces, line breaks and comments, so tokenizer can focus on scanning code tokens without worrying about spaces, line breaks and comments
    int Scanner::scanWithTokenizer(void *parentNode,
                                   TokenizerFunction tokenizer,
                                   int start,
                                   ParseContext *context,
                                   bool root, bool scanMulti
    ) {

        utf8byte ch;
        int returnResult = -1;
        context->afterLineBreak = false;
        InnerParsingData  parsingResult;
            
        parsingResult.whitespace_startpos = -1;
        parsingResult.commentNode = nullptr;

        for (int32_t i = start; i <= context->length;) {
            ch = context->chars[i];

            if (ch == '/') { // comment
                int pos = tryDetectComments(context, i, parentNode, &parsingResult);
                if (pos > -1) {
                    returnResult = i = pos;
                    continue;
                }
            }
            else if (ParseUtil::isBreakLine(ch)) {
                i = createLineBreakNode(context, parentNode, i, ch, &parsingResult);
                context->afterLineBreak = true;
                continue;
            }
            else if (ParseUtil::isSpace(ch)) {
                int spaceEndIndex = i + 1;
                for (; spaceEndIndex < context->length; spaceEndIndex++) {
                    if (!ParseUtil::isSpace(context->chars[spaceEndIndex])) {
                        break;
                    }
                }

                parsingResult.whitespace_startpos = i;
                i = spaceEndIndex;
                continue;
            }

            int result = tokenizer(Cast::upcast(parentNode), ch, i, context);
            returnResult = result;

            if (context->syntaxErrorInfo.hasError) {
                return -1;
            }

            if (result == Search::DONE_WITH_PREVIUS_POSITION) {
                break;
            }

            if (Search::IsTokenized(result)) {
                context->afterLineBreak = false;
                context->prevFoundPos = result;

                assert(context->leftNode != nullptr);

                parsingResult.assignWhiteSpaces(Cast::upcast(context->leftNode), i);
                parsingResult.assignCommentNode(context->leftNode);

                context->leftNode->prevLineBreakNode = parsingResult.prevLineBreak;

                parsingResult.prevLineBreak = nullptr;
                parsingResult.lastLineBreak = nullptr;


                if (scanMulti && !context->scanEnd) {
                    i = result;
                    continue;
                }
            }
            break;
        }

        if (root) {
            context->remainedLineBreakNode = parsingResult.prevLineBreak;
            context->remainedCommentNode = parsingResult.commentNode;
            if (parsingResult.whitespace_startpos > -1 && parsingResult.whitespace_startpos < context->length) {
                context->remaindPrevChars = context->length - parsingResult.whitespace_startpos;
            }
        }
        if (context->scanEnd) {
            context->scanEnd = false;
        }
        return returnResult;
    }


    int Tokenizers::tokenizeExpression(TokenizerParams_parent_ch_start_context) {
        int result = numberTokenizer(TokenizerParams_pass);

        if (!Search::IsTokenized(result)) { result = boolTokenizer(TokenizerParams_pass); }
        if (!Search::IsTokenized(result)) { result = nullTokenizer(TokenizerParams_pass); }
        if (!Search::IsTokenized(result)) { result = parenthesesTokenizer(TokenizerParams_pass); }
        if (!Search::IsTokenized(result)) { result = variableTokenizer(TokenizerParams_pass); }
        if (!Search::IsTokenized(result)) { result = stringLiteralTokenizer(TokenizerParams_pass); }

        if (!Search::IsTokenized(result)) { return Search::NOTFOUND; }

        // access operator
        // pointer->val
        // "jfiowj".length

        // call func expression: func()
        int extraPos;
        if (Search::IsTokenized(extraPos = Tokenizers::tokenizeFuncCall(parent, context->chars[result],
                                                           result, context))) {
            result = extraPos;
        }

        //  binary operator expression: calc() + 421431
        if (Search::IsTokenized(extraPos = Tokenizers::binaryOperationTokenizer(parent, context->chars[result],
                                                                  result, context))) {
            result = extraPos;
        }


        return result;
    }
}
