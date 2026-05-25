#include <cstdio>
#include <iostream>
#include <array>
#include <algorithm>


#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <vector>

#include <cstdint>
#include <ctime>
#include <cstring>

#include "code_nodes.hpp"


namespace smart {

    // --------------------- Defines ReturnStatement VTable ---------------------- /

    // virtual node does not have self text. underlying nodes will be appended to code line.
    static int selfTextLength(ReturnStatementNodeStruct *)
    {
        return 0;
    }

    static void copySelfText(ReturnStatementNodeStruct *self, utf8byte *buf)
    {
    }

    static CodeLine *appendToLine(ReturnStatementNodeStruct*self, CodeLine *currentCodeLine) {
        currentCodeLine = VTableCall::callAppendToLine(&self->returnText, currentCodeLine);

        auto *prevCodeLine = currentCodeLine;
        auto formerParentDepth = self->context->parentDepth;

        if (self->valueNode) {
            currentCodeLine = VTableCall::callAppendToLine(self->valueNode, currentCodeLine);

            if (prevCodeLine != currentCodeLine) {
                currentCodeLine->depth = formerParentDepth + 1;
            }
        }

        self->context->parentDepth = formerParentDepth;

        return currentCodeLine;
    }


    static int ReturnStatementNodeStruct_applyFuncToDescendants(
            ReturnStatementNodeStruct *node, ApplyFunc_params3)
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

    static constexpr const char assignTypeText[] = "<ReturnStatement>";

    /*
     * return statement
     */
    static node_vtable _returnVTable = CREATE_VTABLE(ReturnStatementNodeStruct,
                                                          selfTextLength,
                                                          copySelfText,
                                                          appendToLine,
                                                          ReturnStatementNodeStruct_applyFuncToDescendants,
                                                          assignTypeText
                                                          , NodeTypeId::ReturnStatement);

    const struct node_vtable *VTables::ReturnStatementVTable = &_returnVTable;


    // -------------------- Implements AssignStatement Allocator --------------------- //
    ReturnStatementNodeStruct *Alloc::newReturnStatement(ParseContext *context, NodeBase *parentNode) {
        auto *returnStatement = context->newMem<ReturnStatementNodeStruct>();

        returnStatement->valueNode = nullptr;

        Init::initReturnStatement(context, parentNode, returnStatement);
        return returnStatement;
    }

    constexpr char returnText[] = "return";
    constexpr int returnTextSize = sizeof(returnText) - 1;

    void Init::initReturnStatement(ParseContext *context, NodeBase *parentNode, ReturnStatementNodeStruct *returnStatement) {
        INIT_NODE(returnStatement, context, parentNode, &_returnVTable);

        returnStatement->valueNode = nullptr;

        Init::initSimpleTextNode(&returnStatement->returnText, context, returnStatement, returnTextSize);
    }

    // --------------------- Implements Return Statement Parser ----------------------
    static int parenthesesTokenizerInternal(TokenizerParams_argNode_ch_start_context) {

        if (context->isAfterLineBreak) {
            return Search::NOTFOUND;
        }

        auto *returnNode = Cast::downcast<ReturnStatementNodeStruct *>(argNode);
        int result;
        if (Search::IsTokenized(result = Tokenizers::tokenizeExpression(Cast::upcast(returnNode), ch,
                                                           start, context))) {
            returnNode->valueNode = context->generatedMainNode;
            context->scanEnd = true;

            return result;
        }
        else {
            //context->scanEnd = true;
            context->setError(ErrorCode::no_value_for_return, start);
        }

        return Search::NOTFOUND;
    }

    // return 1234
    int Tokenizers::returnStatementTokenizer(TokenizerParams_argNode_ch_start_context) {
        // return
        if ('r' != ch) {
            return Search::NOTFOUND;
        }

        auto idx = ParseUtil::matchAt(context->chars, context->length, start, returnText);
        if (idx > -1) {
            auto *returnNode = Alloc::newReturnStatement(context, argNode);
            Init::assignText_SimpleTextNode(&returnNode->returnText, context, start, returnTextSize);

            int currentPos = idx + returnTextSize;
            int resultPos;
            if (Search::IsTokenized(resultPos = Scanner::scanLoop(returnNode,
                                                        parenthesesTokenizerInternal,
                                                        context, currentPos))) {

                context->mostLeftNode = Cast::upcast(&returnNode->returnText);
                context->generatedMainNode = Cast::upcast(returnNode);
                return resultPos;
            }
            else {
                context->mostLeftNode = Cast::upcast(&returnNode->returnText);
                context->generatedMainNode = Cast::upcast(returnNode);
                return currentPos;
            }
        }

        return Search::NOTFOUND;
    }
}



