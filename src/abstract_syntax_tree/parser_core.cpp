
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

    
    struct InnerParsingResult {
        int newPosition;
        NodeBase *createdNode = nullptr;
        int32_t whitespace_startpos = -1;
    };
    


    int Scanner::scanOnce(void *parentNode,
                      TokenizerFunction tokenizer,
                      int start,
                      ParseContext *context
    ) {

        return Scanner::scan_for_root(parentNode, tokenizer, start, context, false, false);
    }

    // scan until scanEnd==true
    int Scanner::scanMulti(void *parentNode,
        TokenizerFunction tokenizer,
        int start,
        ParseContext *context
    ) {
        return Scanner::scan_for_root(parentNode, tokenizer, start, context, false, true);
    }

    CodeLine *VTableCall::callAppendToLine(void *node, CodeLine *currentCodeLine) {
        if (node == nullptr) {
            return currentCodeLine;
        }
        auto *nodeBase = Cast::upcast(node);
        //if (nodeBase->prevLineBreakNode)
        return nodeBase->vtable->appendToLine(nodeBase, currentCodeLine);
    }

    int tryDetectComments(ParseContext* context, int32_t i, void**commentNode
        , int32_t& whitespace_startpos, void* parentNode, LineBreakNodeStruct**prevLineBreak, InnerParsingResult* parsingResult)
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
            int textStartPos = i + 2;

            if (context->chars[i + 2] == '[') { // /*[hoge] ... [hoge]*/
                int nameStartPos = i + 3;
                // finds out the tag name: hoge
                int endOfStartTagPos = ParseUtil::indexOf(context->chars, context->length, nameStartPos, ']');
                int lineEndPos = ParseUtil::indexOfBreakOrEnd(context->chars, context->length, nameStartPos);
                // the name of the named block comment must be in the same line with the start tag
                if (endOfStartTagPos > -1 && endOfStartTagPos < lineEndPos) {
                    tagLength = endOfStartTagPos - nameStartPos;
                    textStartPos = endOfStartTagPos + 1;
                    tagText = context->memBuffer.newMem<char>(tagLength + 1);
                    TEXT_MEMCPY(tagText, context->chars + nameStartPos, tagLength);
                    tagText[tagLength] = '\0';
                }
            }

            int searchEndPos = textStartPos;
            while (true) {
                int endCommentPos = ParseUtil::indexOf2(context->chars, context->length, searchEndPos, '*', '/');

                if (endCommentPos == -1) {
                    // the end of block comment not found, treat the rest of chars as comment
                    commentEndIndex = context->length;
                    break;
                }

                if (tagLength > 0) {
                    // [hoge]*/
                    if (context->chars[endCommentPos - 1] == ']'
                        && context->chars[endCommentPos - tagLength - 2] == '['
                        && ParseUtil::matchWord(context->chars, context->length, tagText, tagLength, endCommentPos - tagLength - 1)) {
                        // the end tag of the named block comment found
                        commentEndIndex = endCommentPos + 2;
                        break;
                    }
                    else {
                        // not the end of the named block comment, continue to search
                        searchEndPos = endCommentPos + 2;
                        continue;;
                    }
                }
                else {
                    // not a named block comment, the first */ is the end of the block comment
                    commentEndIndex = endCommentPos + 2;
                    break;
                }
                break;
            }
        }

        if (commentEndIndex > -1) {
            auto* prevCommentNode = *commentNode;

            if (isLineComment) {
                auto* comment = Alloc::newLineCommentNode(context, Cast::upcast(parentNode));
                Init::assignText_SimpleTextNode(comment, context, i, commentEndIndex - i);

                *commentNode = comment;
            }
            else {
                *commentNode = Scanner::generateBlockCommentFragments(parentNode, context, i, commentEndIndex, tagText, tagLength);
            }

            NodeBase* comment2 = Cast::upcast(*commentNode);
            if (whitespace_startpos != -1 && whitespace_startpos < i) {
                comment2->prev_chars = i - whitespace_startpos;
                whitespace_startpos = -1;
            }

            if (prevCommentNode != nullptr) {
                comment2->prevCommentNode = prevCommentNode;
            }

            if (*prevLineBreak != nullptr) {
                comment2->prevLineBreakNode = *prevLineBreak;
                *prevLineBreak = nullptr;
            }
        }

        parsingResult->newPosition = commentEndIndex;
        return commentEndIndex;
    }

    
    inline void *Scanner::generateBlockCommentFragments(void *parentNode, ParseContext *context,
                                           const int32_t &i, int commentEndIndex, char* tagText, int tagLength) {
        void *commentNode;
        auto *blockComment = Alloc::newBlockCommentNode(context, Cast::upcast(parentNode));
        blockComment->tagText = tagText;
        blockComment->tagTextLength = tagLength;

        int currentIndex = i;
        BlockCommentFragmentStruct *lastNode = nullptr;
        LineBreakNodeStruct *lastBreakLine = nullptr;

        // split block comment into fragments by line break, and create LineBreakNodeStruct for each line break
        while (currentIndex < commentEndIndex) {
            int lineEndIndex = ParseUtil::indexOfBreakOrEnd(context->chars, context->length, currentIndex);
            int endIndex = commentEndIndex < lineEndIndex ? commentEndIndex : lineEndIndex;

            if (endIndex > -1 && currentIndex < endIndex) {
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
        commentNode = blockComment;
        return commentNode;
    }



    int createLineBreakNode(smart::ParseContext* context, void* parentNode,
        smart::LineBreakNodeStruct** prevLineBreak, smart::LineBreakNodeStruct** lastLineBreak,
        int32_t& whitespace_startpos, int32_t& position, void** commentNode, utf8byte ch)
    {
        auto* newLineBreak = Alloc::newLineBreakNode(context, Cast::upcast(parentNode));

        if (*prevLineBreak == nullptr) {
            (*lastLineBreak) = (*prevLineBreak) = newLineBreak;
        }
        else {
            (*lastLineBreak)->nextLineBreakNode = newLineBreak;
            *lastLineBreak = newLineBreak;
        }

        if (whitespace_startpos != -1) {
            if (whitespace_startpos < position) {
                (*lastLineBreak)->prev_chars = position - whitespace_startpos;
            }
            whitespace_startpos = -1;
        }

        if (*commentNode != nullptr) {
            newLineBreak->prevCommentNode = *commentNode;
            *commentNode = nullptr;
        }


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
        context->afterLineBreak = true;
        return result;
    }


    int Scanner::scan_for_root(void *parentNode,
        TokenizerFunction tokenizer,
        int start,
        ParseContext *context,
        bool root, bool scanMulti
    ) {
        LineBreakNodeStruct *prevLineBreak = nullptr;
        LineBreakNodeStruct *lastLineBreak = nullptr;

        utf8byte ch;
        int returnResult = -1;
        int32_t whitespace_startpos = -1;
        void *commentNode = nullptr;
        context->afterLineBreak = false;
        InnerParsingResult  parsingResult;

        for (int32_t i = start; i <= context->length;) {
            ch = context->chars[i];
            
            parsingResult.newPosition = -1;
            parsingResult.createdNode = nullptr;
            parsingResult.whitespace_startpos = -1;

            if (ch == '/') { // comment
                int commentEndIndex = tryDetectComments(context, i, &commentNode, whitespace_startpos, parentNode, &prevLineBreak, &parsingResult);
                if (commentEndIndex > -1) {
                    i = commentEndIndex;
                    returnResult = i;
                    continue;
                }
            }
            else if (ParseUtil::isBreakLine(ch)) {
                i = createLineBreakNode(context, parentNode, &prevLineBreak, &lastLineBreak, whitespace_startpos, i, &commentNode, ch);
                continue;
            }
            else if (ParseUtil::isSpace(ch)) {
                int spaceEndIndex = i + 1;
                for (; spaceEndIndex < context->length; spaceEndIndex++) {
                    if (!ParseUtil::isSpace(context->chars[spaceEndIndex])) {
                        break;
                    }
                }

                whitespace_startpos = i;
                i = spaceEndIndex;
                continue;
            }

            int result = tokenizer(Cast::upcast(parentNode), ch, i, context);

            if (context->syntaxErrorInfo.hasError) {
                return -1;
            }

            returnResult = result;
            if (result > -1) {
                context->afterLineBreak = false;
                context->prevFoundPos = result;

                if (context->leftNode != nullptr) {
                    if (whitespace_startpos != -1) {
                        context->leftNode->prev_chars = i - whitespace_startpos;
                        whitespace_startpos = -1;
                    }

                    if (commentNode != nullptr) {
                        context->leftNode->prevCommentNode = commentNode;
                        commentNode = nullptr;
                    }

                    context->leftNode->prevLineBreakNode = prevLineBreak;
                }

                i = result;

                prevLineBreak = nullptr;
                lastLineBreak = nullptr;

                if (context->scanEnd) {
                    context->scanEnd = false;
                    break;
                }

                if (scanMulti) {
                    continue;
                }
            }
            break;
            //if (ch == '\0') {
            //    break;
            //}

            //if ((ch & 0x80) != 0x80)
            //{

            //}
            ////if (!root) {
            //    break;
            ////}
            ////i++;
        }

        if (root) {
            context->remainedLineBreakNode = prevLineBreak;
            context->remainedCommentNode = commentNode;
            if (whitespace_startpos > -1 && whitespace_startpos < context->length) {
                context->remaindPrevChars = context->length - whitespace_startpos;
            }
        }
        //context->scanEnd = false;
        return returnResult;
    }


    int Tokenizers::expressionTokenizer(TokenizerParams_parent_ch_start_context) {
        int result = numberTokenizer(TokenizerParams_pass);

        if (result == -1) { result = boolTokenizer(TokenizerParams_pass); }
        if (result == -1) { result = nullTokenizer(TokenizerParams_pass); }
        if (result == -1) { result = parenthesesTokenizer(TokenizerParams_pass); }
        if (result == -1) { result = variableTokenizer(TokenizerParams_pass); }
        if (result == -1) { result = stringLiteralTokenizer(TokenizerParams_pass); }

        if (result == -1) { return -1; }

        // access operator
        // pointer->val
        // "jfiowj".length


        // call func expression: func()
        int extraPos;
        if (-1 < (extraPos = Tokenizers::funcCallTokenizer(parent, context->chars[result],
                                                           result, context))) {
            result = extraPos;
        }

        //  binary operator expression: calc() + 421431
        if (-1 < (extraPos = Tokenizers::binaryOperationTokenizer(parent, context->chars[result],
                                                                  result, context))) {
            result = extraPos;
        }


        return result;
    }
}
