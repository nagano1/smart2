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

    // -----------------------------------------------------------------------------------
    //
    //                              FuncArgumentItemStruct
    //
    // -----------------------------------------------------------------------------------
    static CodeLine *FuncArgument_appendToLine2(FuncArgumentItemStruct *self, CodeLine *currentCodeLine) {
        currentCodeLine = currentCodeLine->AddAttachedFormatNodes(self);

        currentCodeLine->appendNode(self);

        if (self->exprNode) {
            currentCodeLine = VTableCall::callAppendToLine(self->exprNode, currentCodeLine);
        }

        if (self->hasComma) {
            currentCodeLine = VTableCall::callAppendToLine(&self->follwingComma, currentCodeLine);
        }

        return currentCodeLine;
    };


    // virtual node does not have self text. underlying nodes will be appended to code line.
    static void copySelfText_FuncArgument(FuncArgumentItemStruct *self, utf8byte *buf) {
        return;
    }

    static int FuncArgument_selfTextLength2(FuncArgumentItemStruct *) {
        return 0;
    }


    static int FuncArgumentItemStruct_applyFuncToDescendants(
            FuncArgumentItemStruct *node, ApplyFunc_params3)
    {

        if (parentIsFirst) {
            if (targetVTable == nullptr || node->vtable == targetVTable) {
                func(Cast::upcast(node), ApplyFunc_pass);
            }
        }
        if (node->exprNode) {
            node->exprNode->vtable->applyFuncToDescendants(Cast::upcast(node->exprNode), ApplyFunc_pass2);
        }
        if (!parentIsFirst) {
            if (targetVTable == nullptr || node->vtable == targetVTable) {
                func(Cast::upcast(node), ApplyFunc_pass);
            }
        }
        return 0;
    }


    static node_vtable _funcArgumentItemVTable = CREATE_VTABLE(FuncArgumentItemStruct,
                                                                     FuncArgument_selfTextLength2,
                                                                     copySelfText_FuncArgument,
                                                                     FuncArgument_appendToLine2,
                                                               FuncArgumentItemStruct_applyFuncToDescendants,
                                                                     "<FuncArgument>",
                                                                     NodeTypeId::FuncArgument);

    const struct node_vtable *VTables::FuncArgumentVTable  = &_funcArgumentItemVTable;

    FuncArgumentItemStruct *Alloc::newFuncArgumentItem(ParseContext *context, NodeBase *parentNode) {
        auto *keyValueItem = context->newMem<FuncArgumentItemStruct>();

        INIT_NODE(keyValueItem, context, parentNode, &_funcArgumentItemVTable);

        Init::initSymbolNode(&keyValueItem->follwingComma, context, keyValueItem, ',');

        keyValueItem->hasComma = false;
        keyValueItem->exprNode = nullptr;

        return keyValueItem;
    }



    // -----------------------------------------------------------------------------------
    //
    //                              CallFunc Node
    //
    // -----------------------------------------------------------------------------------

    static CodeLine *callfunc_appendToLine(CallFuncNodeStruct *self, CodeLine *currentCodeLine)
    {
        // currentCodeLine = currentCodeLine->AddAttachedFormatNodes(self->exprNode);

        if (self->exprNode) {
            currentCodeLine = VTableCall::callAppendToLine(self->exprNode, currentCodeLine);
        }

        //int formerParentDepth = self->context->parentDepth;
        //self->context->parentDepth += 1;
        //self->context->parentDepth = formerParentDepth;

        currentCodeLine = VTableCall::callAppendToLine(&self->openNode, currentCodeLine);

        self->context->parentDepth += 1;

        auto *item = self->firstArgumentItem;
        while (item != nullptr) {
            currentCodeLine = VTableCall::callAppendToLine(item, currentCodeLine);
            item = Cast::downcast<FuncArgumentItemStruct *>(item->nextNode);
        }

        self->context->parentDepth -= 1;


        currentCodeLine = VTableCall::callAppendToLine(&self->closeNode2, currentCodeLine);

        return currentCodeLine;
    }


    // virtual node does not have self text. underlying nodes will be appended to code line.
    static void copySelfText_CallFunc(CallFuncNodeStruct *self, utf8byte *buf)
    {
        return;
    }

    static int callfun_selfTextLength(CallFuncNodeStruct *self)
    {
        return 0;
    }


    static constexpr const char callfuncNodeTypeText[] = "<FuncCall>";


    static inline void appendRootNode(CallFuncNodeStruct *arr, FuncArgumentItemStruct *arrayItem) {
        assert(arr != nullptr && arrayItem != nullptr);

        if (arr->firstArgumentItem == nullptr) {
            arr->firstArgumentItem = arrayItem;
        }
        if (arr->lastArgumentItem != nullptr) {
            arr->lastArgumentItem->nextNode = Cast::upcast(arrayItem);
        }
        arr->lastArgumentItem = arrayItem;
    }


    enum phase {
        EXPECT_VALUE = 0,
        EXPECT_COMMA = 3
    };



    static inline int parseNextValue(TokenizerParams_parent_ch_start_context, CallFuncNodeStruct* funcCallNode)
    {
        int result;
        if (Search::IsTokenized(result = Tokenizers::tokenizeExpression(TokenizerParams_pass))) {
            auto *nextItem = Alloc::newFuncArgumentItem(context, parent);

            nextItem->exprNode = context->generatedMainNode;
            appendRootNode(funcCallNode, nextItem);
            funcCallNode->parsePhase = phase::EXPECT_COMMA;
            return result;
        }
        return Search::NOTFOUND;
    }


    static int tokenizeFuncCallInternal(TokenizerParams_parent_ch_start_context) {
        auto *funcCallNode = Cast::downcast<CallFuncNodeStruct*>(parent);

        if (ch == ')') {
            context->setCodeNode(&funcCallNode->closeNode2);
            context->scanEnd = true;
            return start + 1;
        }

        if (funcCallNode->parsePhase == phase::EXPECT_VALUE) {
            return parseNextValue(TokenizerParams_pass, funcCallNode);
        }

        auto *currentKeyValueItem = funcCallNode->lastArgumentItem;

        if (funcCallNode->parsePhase == phase::EXPECT_COMMA) {
            if (ch == ',') { // try to find ',' which leads to next key-value
                currentKeyValueItem->hasComma = true;
                context->setCodeNode(&currentKeyValueItem->follwingComma);
                funcCallNode->parsePhase = phase::EXPECT_VALUE;
                return start + 1;
            }
            else if (context->isAfterLineBreak) {
                // comma is not required after a line break
                return parseNextValue(TokenizerParams_pass, funcCallNode);
            }
            return Search::NOTFOUND;
        }
        return Search::NOTFOUND;
    }


    int Tokenizers::tokenizeFuncCall(TokenizerParams_parent_ch_start_context)
    {
        if ('(' != ch) {
            return Search::NOTFOUND;
        }

        assert(context->generatedMainNode != nullptr);

        auto *funcCallNode = Alloc::newFuncCallNode(context, parent);

        funcCallNode->exprNode = context->generatedMainNode;
        funcCallNode->exprNode->parentNode = Cast::upcast(funcCallNode);

        auto *leftNode = context->leftNode;

        int currentPos = start + 1;
        int resultPos;
        if (Search::IsTokenized(resultPos = Scanner::scanMulti(funcCallNode,
                                                               tokenizeFuncCallInternal, context, currentPos))) {
            context->generatedMainNode = Cast::upcast(funcCallNode);
            context->leftNode = leftNode;
            return resultPos;
        }
        return Search::NOTFOUND;
    }


    static int callfunc_applyFuncToDescendants(
            CallFuncNodeStruct *node, ApplyFunc_params3)
    {
        if (parentIsFirst) {
            if (targetVTable == nullptr || node->vtable == targetVTable) {
                func(Cast::upcast(node), ApplyFunc_pass);
            }
        }

        if (node->exprNode) {
            node->exprNode->vtable->applyFuncToDescendants(
                    Cast::upcast(node->exprNode), ApplyFunc_pass2);
        }

        auto *item = node->firstArgumentItem;
        while (item != nullptr) {
            item->vtable->applyFuncToDescendants(
                    Cast::upcast(item), ApplyFunc_pass2);
            item = Cast::downcast<FuncArgumentItemStruct *>(item->nextNode);
        }

        if (!parentIsFirst) {
            if (targetVTable == nullptr || node->vtable == targetVTable) {
                func(Cast::upcast(node), ApplyFunc_pass);
            }
        }
        return 0;
    }

    static node_vtable _callfuncVTable = CREATE_VTABLE(CallFuncNodeStruct,
                                                             callfun_selfTextLength,
                                                             copySelfText_CallFunc,
                                                             callfunc_appendToLine,
                                                       callfunc_applyFuncToDescendants,
                                                             callfuncNodeTypeText,
                                                             NodeTypeId::CallFunc);

    const node_vtable *VTables::CallFuncVTable = &_callfuncVTable;


    CallFuncNodeStruct *Alloc::newFuncCallNode(ParseContext *context, NodeBase *parentNode)
    {
        auto *node = context->newMem<CallFuncNodeStruct>();
        INIT_NODE(node, context, parentNode, VTables::CallFuncVTable);
        node->exprNode = nullptr;
        node->parsePhase = phase::EXPECT_VALUE;

        Init::initSymbolNode(&node->openNode, context, node, '(');
        Init::initSymbolNode(&node->closeNode2, context, node, ')');

        node->firstArgumentItem = nullptr;
        node->lastArgumentItem = nullptr;

        return node;
    }
}