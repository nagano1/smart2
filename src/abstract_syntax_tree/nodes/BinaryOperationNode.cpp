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

    //    +--------------------------+
    //    |  Binary Operation        |
    //    +--------------------------+

    static CodeLine *binaryop_appendToLine(BinaryOperationNodeStruct *self, CodeLine *currentCodeLine)
    {
        int formerParentDepth = self->context->parentDepth;

        if (self->leftExprNode) {
            // leftExpr
            currentCodeLine = VTableCall::callAppendToLine(self->leftExprNode, currentCodeLine);
        }

        int formerArithmeticDepth = self->context->arithmeticBaseDepth;

        int diff = currentCodeLine->depth == self->context->parentDepth ? 0 : 1;

        int newDepth = self->context->arithmeticBaseDepth > -1 ?
                       self->context->arithmeticBaseDepth : formerParentDepth + diff;

        self->context->arithmeticBaseDepth = newDepth;
        self->context->parentDepth = newDepth;

        // operator +
        currentCodeLine = VTableCall::callAppendToLine(&self->opNode, currentCodeLine);


        if (self->rightExprNode) {
            // rightExpr
            currentCodeLine = VTableCall::callAppendToLine(self->rightExprNode, currentCodeLine);
        }

        self->context->parentDepth = formerParentDepth;
        self->context->arithmeticBaseDepth = formerArithmeticDepth;

        return currentCodeLine;
    }

    static void copySelfText_binaryOp(BinaryOperationNodeStruct *self, utf8byte *buf)
    {
    }

    static int binaryop_selfTextLength(BinaryOperationNodeStruct *self)
    {
        return 0;
    }

    static int BinaryOperationNodeStruct_applyFuncToDescendants(BinaryOperationNodeStruct *node, ApplyFunc_params3)
    {
        if (parentIsFirst) {
            if (targetVTable == nullptr || node->vtable == targetVTable) {
                func(Cast::upcast(node), ApplyFunc_pass);
            }
        }

        if (node->leftExprNode) {
            node->leftExprNode->vtable->applyFuncToDescendants(node->leftExprNode,
                                                           ApplyFunc_pass2);
        }

        if (node->rightExprNode) {
            node->rightExprNode->vtable->applyFuncToDescendants(node->rightExprNode,
                                                            ApplyFunc_pass2);
        }

        if (!parentIsFirst) {
            if (targetVTable == nullptr || node->vtable == targetVTable) {
                func(Cast::upcast(node), ApplyFunc_pass);
            }
        }
        return 0;
    }


    static constexpr const char binaryop_NodeTypeText[] = "<binary op>";

    static node_vtable binaryop_VTable = CREATE_VTABLE(BinaryOperationNodeStruct ,
                                                             binaryop_selfTextLength,
                                                             copySelfText_binaryOp,
                                                             binaryop_appendToLine,
                                                       BinaryOperationNodeStruct_applyFuncToDescendants,
                                                             binaryop_NodeTypeText,
                                                                NodeTypeId::BinaryOperation);

    const node_vtable *VTables::BinaryOperationVTable = &binaryop_VTable;



    static int inner_op_binaryOpTokenizer(TokenizerParams_parent_ch_start_context) {

        if (ch == '+' || ch == '*' || ch == '-' || ch == '/' || ch == '%'
            || ch == '&' || ch == '|') {

            auto *binaryOpNode = Alloc::newBinaryOperationNode(context, parent, ch);

            context->leftNode = Cast::upcast(&binaryOpNode->opNode);
            context->generatedMainNode = Cast::upcast(binaryOpNode);
            return start + 1;
        }

        return Search::NOTFOUND;
    }


    int Tokenizers::binaryOperationTokenizer(TokenizerParams_parent_ch_start_context)
    {
        assert(context->generatedMainNode != nullptr);

        auto *virtualNode = context->generatedMainNode;
        auto *leftNode = context->leftNode;

        int resultPos = Scanner::scanOnce(parent, inner_op_binaryOpTokenizer, start, context);
        context->leftNode = leftNode;

        if (Search::IsTokenized(resultPos)) {
            auto* binaryOpNode = Cast::downcast<BinaryOperationNodeStruct*>(context->generatedMainNode);
            binaryOpNode->leftExprNode = virtualNode;
            binaryOpNode->leftExprNode->parentNode = Cast::upcast(binaryOpNode);

            if (Search::IsTokenized(resultPos = Scanner::scanOnce(binaryOpNode,
                                                    Tokenizers::tokenizeExpression,
                                                    resultPos, context))) {
                binaryOpNode->rightExprNode = context->generatedMainNode;
                context->generatedMainNode = Cast::upcast(binaryOpNode);
                context->leftNode = leftNode;
                return resultPos;
            }
        }
        return Search::NOTFOUND;
    }


    BinaryOperationNodeStruct *Alloc::newBinaryOperationNode(ParseContext *context, NodeBase *parentNode, char op)
    {
        auto *node = context->newMem<BinaryOperationNodeStruct>();
        INIT_NODE(node, context, parentNode, VTables::BinaryOperationVTable);

        node->leftExprNode = nullptr;
        node->rightExprNode = nullptr;

        Init::initSymbolNode(&node->opNode, context, node, op);

        return node;
    }








} // namespace
