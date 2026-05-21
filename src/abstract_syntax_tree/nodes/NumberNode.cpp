#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
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

#include "code_nodes.hpp"

namespace smart {

    /*
        +--------------------------+
        |                          |
        |                          |
        |      nullTokenizer       |
        |                          |
        |                          |
        +--------------------------+
    */

    int Tokenizers::nullTokenizer(TokenizerParams_parent_ch_start_context) {
        static constexpr const char null_chars[] = "null";
        return Tokenizers::tokenizeWord(TokenizerParams_pass, Alloc::newNullNode, 'n', null_chars);
    }




    /*
        +----------------------------------------------------+
        |                          
        |      BoolNode            
        |                          
        +----------------------------------------------------+
    */

    static CodeLine *appendToLine2(BoolNodeStruct *self, CodeLine *currentCodeLine) {
        return currentCodeLine->addPrevLineBreakNode(self)->appendNode(self);
    }

    static void copySelfText2(BoolNodeStruct *self, utf8byte *buf) {
        TEXT_MEMCPY(buf, self->text, self->textLength);
    }

    static int selfTextLength2(BoolNodeStruct*self) {
        return self->textLength;
    }


    int Tokenizers::boolTokenizer(TokenizerParams_parent_ch_start_context)
    {
        int result = Tokenizers::tokenizeWord(TokenizerParams_pass,
                                                Alloc::newBoolNode
                                                ,'t', "true");
        bool trueFound = Search::IsTokenized(result);
        if (!trueFound) {
            result = Tokenizers::tokenizeWord(TokenizerParams_pass,
                                                Alloc::newBoolNode,
                                                'f', "false");
            if (!Search::IsTokenized(result)) {
                return Search::NOTFOUND;
            }
        }

        auto *boolNode = Cast::downcast<BoolNodeStruct*>(context->generatedMainNode);
        boolNode->found = start;
        boolNode->boolValue = trueFound;
        return result;
    }

    static int BoolNodeStruct_applyFuncToDescendants(
            BoolNodeStruct *node, ApplyFunc_params3)
    {
        if (targetVTable == nullptr || node->vtable == targetVTable) {
            func(Cast::upcast(node), ApplyFunc_pass);
        }

        return 0;
    }



    static constexpr const char boolNodeTypeText[] = "<bool>";
    static node_vtable _boolVTable = CREATE_VTABLE(BoolNodeStruct, selfTextLength2,
                                                         copySelfText2, appendToLine2,
                                                   BoolNodeStruct_applyFuncToDescendants,
                                                         boolNodeTypeText, NodeTypeId::Bool);

    const node_vtable *VTables::BoolVTable = &_boolVTable;

    BoolNodeStruct* Alloc::newBoolNode(ParseContext *context, NodeBase *parentNode) {
        auto *node = context->newMem<BoolNodeStruct>();
        INIT_NODE(node, context, parentNode, VTables::BoolVTable);
        node->text = nullptr;
        node->textLength = 0;
        node->boolValue = false;
        return node;
    }













    //    +--------------------------+
    //    | Number                   |
    //    +--------------------------+

    static CodeLine *appendToLine(NumberNodeStruct *self, CodeLine *currentCodeLine)
    {
        assert(self->text != nullptr);

        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self);
        currentCodeLine->appendNode(self);

        return currentCodeLine;
    }

    static void copySelfText(NumberNodeStruct *self, utf8byte *buf)
    {
        TEXT_MEMCPY(buf, self->text, self->textLength);
    }

    static int selfTextLength(NumberNodeStruct *self)
    {
        return self->textLength;
    }

    //*/
    inline int64_t S64(const char *s, int length) {
        // have to check over flow
        return atoll(s); // can't use strtoll beacause of wasm conversion
        //return atoi(s);
        /*
        int64_t i;
        char c ;
        int scanned = sscanf(s, "%lld%c", &i, &c); // NOLINT(cert-err34-c)
        if (scanned == 1) return i;
        if (scanned > 1) {
            // TBD about extra data found
            return i;
        }
        // TBD failed to scan;
        return 0;
         */
    }
    //*/
    static constexpr const char numberNodeTypeText[] = "<number>";

    // jfiowaef
    int Tokenizers::numberTokenizer(TokenizerParams_parent_ch_start_context)
    {
        bool hasNegative = false;
        
        int numberStart;
        int charCount;

        if (context->chars[start] == '-') {
            hasNegative = true;
            numberStart = start + 1;
            charCount = 1;
        } else {
            numberStart = start;
            charCount = 0;
        }

        for (int_fast32_t i = numberStart; i < context->length; i++) {
            if (!ParseUtil::isNumberLetter(context->chars[i])) {
                break;
            }

            charCount++;
        }

        if (hasNegative ? charCount > 1 : charCount > 0) {

            auto *numberNode = Alloc::newNumberNode(context, parent);

            context->setCodeNode(numberNode);
            numberNode->text = context->memBuffer.newMem<char>(charCount + 1/* \0 */ + 1/*L*/);
            numberNode->textLength = charCount;

            TEXT_MEMCPY(numberNode->text, context->chars + start, charCount);
            numberNode->text[charCount] = '\0';

            numberNode->num = S64(numberNode->text, charCount);

            if ('L' == context->chars[start + charCount]) {
                numberNode->textLength++;
                numberNode->unit = 64;
                numberNode->text[charCount] = 'L';
                charCount++;
                numberNode->text[charCount] = '\0';
            } else if (!ParseUtil::isTerminatableChar(context->chars[start + charCount])) {
                // invalid suffix character for numbers
            } else {
                numberNode->num = (int32_t)numberNode->num;
            }

            return start + charCount;
        }

        return Search::NOTFOUND;
    }


    static int NumberNodeStruct_applyFuncToDescendants(
            NumberNodeStruct *node, ApplyFunc_params3)
    {
        if (targetVTable == nullptr || node->vtable == targetVTable) {
            func(Cast::upcast(node), ApplyFunc_pass);
        }

        return 0;
    }


    static node_vtable _numberVTable_ = CREATE_VTABLE(NumberNodeStruct, selfTextLength,
                                                            copySelfText,
                                                            appendToLine,
                                                      NumberNodeStruct_applyFuncToDescendants,
                                                            numberNodeTypeText,
                                                            NodeTypeId::Number);

    const node_vtable *VTables::NumberVTable = &_numberVTable_;



    NumberNodeStruct *Alloc::newNumberNode(ParseContext *context, NodeBase *parentNode)
    {
        auto *node = context->newMem<NumberNodeStruct>();
        INIT_NODE(node, context, parentNode, VTables::NumberVTable);
        node->text = nullptr;
        node->textLength = 0;

        return node;
    }








    //    +--------------------------+
    //    | Parentheses value        |
    //    +--------------------------+
    static CodeLine *parentheses_appendToLine(ParenthesesNodeStruct *self, CodeLine *currentCodeLine)
    {
        // (
        currentCodeLine = currentCodeLine->addPrevLineBreakNode(self)
                                         ->appendNode(self);

        auto *openCodeLine = currentCodeLine;
        int formerDepth = currentCodeLine->depth;


        if (self->valueNode) {
            int formerParentDepth = self->context->parentDepth;
            int formerArithmeticDepth = self->context->arithmeticBaseDepth;

            self->context->arithmeticBaseDepth = -1;

            int diff = currentCodeLine->depth == self->context->parentDepth ? 0 : 1;
            self->context->parentDepth += diff;
            currentCodeLine = VTableCall::callAppendToLine(self->valueNode, currentCodeLine);

            self->context->arithmeticBaseDepth = formerArithmeticDepth;
            self->context->parentDepth = formerParentDepth;
        }


        // )
        currentCodeLine = VTableCall::callAppendToLine(&self->closeNode, currentCodeLine);

        if (currentCodeLine != openCodeLine) {
            bool hasNonBracketEntity = false;
            auto *node = currentCodeLine->firstNode;
            while (node) {
                if (node->vtable == VTables::SymbolVTable) {
                    auto *symbol = Cast::downcast<SymbolStruct *>(node);
                    bool end = symbol->symbol[0] == ')' || symbol->symbol[0] == '}';
                    if (!end) {
                        hasNonBracketEntity = true;
                        break;
                    }
                } else if (node->vtable != VTables::LineBreakVTable
                           && node->vtable != VTables::LineCommentVTable
                           && node->vtable != VTables::BlockCommentFragmentVTable
                           && node->vtable != VTables::BlockCommentVTable
                           && node->vtable != VTables::EndOfFileVTable
                        ) {
                    hasNonBracketEntity = true;
                    break;
                }
                node = node->nextNode;
            }
            if (!hasNonBracketEntity) {
                currentCodeLine->depth = formerDepth;
            }
        }

        return currentCodeLine;
    }

    static void copySelfText3(ParenthesesNodeStruct *self, utf8byte *buf)
    {
        buf[0] = '(';
    }

    static int parentheses_selfTextLength(ParenthesesNodeStruct *self)
    {
        return 1;
    }


    static constexpr const char parenthesesNodeTypeText[] = "<parentheses>";

    static int parenthesesTokenizerInternal(TokenizerParams_parent_ch_start_context) {
        auto *parenthesesNode = Cast::downcast<ParenthesesNodeStruct *>(parent);

        if (ch == ')') {
            context->setCodeNode(&parenthesesNode->closeNode);
            context->scanEnd = true;
            return start + 1;
        }
        else {
            if (parenthesesNode->valueNode != nullptr && parenthesesNode->valueNode->found > -1) {
                context->setError(ErrorCode::expect_end_parenthesis, context->prevFoundPos);
            }
            else {
                int result = Tokenizers::tokenizeExpression(Cast::upcast(parenthesesNode), TokenizerParams_pass_3);
                if (Search::IsTokenized(result)) {
                    parenthesesNode->valueNode = context->generatedMainNode;
                    parenthesesNode->valueNode->found = start;

                    return result;
                } 
                else {
                    context->setError(ErrorCode::expect_end_parenthesis_for_fn_params,
                                      context->prevFoundPos);
                }
            }
        }
        return Search::NOTFOUND;
    }


    int Tokenizers::parenthesesTokenizer(TokenizerParams_parent_ch_start_context)
    {
        if ('(' == ch) {
            auto *parenthesesNode = Alloc::newParenthesesNode(context, parent);
            int currentPos = start + 1;
            int resultPos =  Scanner::scanMulti(parenthesesNode, parenthesesTokenizerInternal, currentPos, context);
            if (Search::IsTokenized(resultPos)) {
                context->setCodeNode(parenthesesNode);
                return resultPos;
            }
        }

        return Search::NOTFOUND;
    }

    static int parentheses_applyFuncToDescendants(ParenthesesNodeStruct *node, ApplyFunc_params3)
    {
        if (parentIsFirst) {
            if (targetVTable == nullptr || node->vtable == targetVTable) {
                func(Cast::upcast(node), ApplyFunc_pass);
            }
        }
        if (node->valueNode) {
            node->valueNode->vtable->applyFuncToDescendants(node->valueNode, ApplyFunc_pass2);
        }

        if (!parentIsFirst) {
            if (targetVTable == nullptr || node->vtable == targetVTable) {
                func(Cast::upcast(node), ApplyFunc_pass);
            }
        }

        return 0;
    }

    static node_vtable _parenthesesVTable = CREATE_VTABLE(ParenthesesNodeStruct,
                                                                parentheses_selfTextLength,
                                                                copySelfText3,
                                                                parentheses_appendToLine,
                                                          parentheses_applyFuncToDescendants,
                                                                parenthesesNodeTypeText,
                                                            NodeTypeId::Parentheses);

    const node_vtable *VTables::ParenthesesVTable = &_parenthesesVTable;

    ParenthesesNodeStruct *Alloc::newParenthesesNode(ParseContext *context, NodeBase *parentNode)
    {
        auto *node = context->newMem<ParenthesesNodeStruct>();
        INIT_NODE(node, context, parentNode, VTables::ParenthesesVTable);
        node->valueNode = nullptr;

        //Init::initSymbolNode(&node->openNode, context, node, '(');
        Init::initSymbolNode(&node->closeNode, context, node, ')');
        return node;
    }
} // namespace
