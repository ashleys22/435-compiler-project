#include "Node.h"
#include "parser.hpp"
#include <iostream>

void NBlock::CodeGen(CodeContext& context)
{
    for (NStatement* mStatement: mStatements) {
        mStatement->CodeGen(context);
    }
}

void NData::CodeGen(CodeContext& context)
{
    for (NDecl* mDecl: mDecls) {
        mDecl->CodeGen(context);
    }
}

void NProgram::CodeGen(CodeContext& context)
{
    mData->CodeGen(context);
    mMain->CodeGen(context);
    context.addOp("exit", std::vector<std::string>());
}

void NNumeric::CodeGen(CodeContext& context)
{
    ++context.lastReg;
    std::string virtualReg = "%" + std::to_string(context.lastReg);
    context.regIntervals[virtualReg].first = context.ops.size(); // current instruction #
    context.regIntervals[virtualReg].second = context.ops.size();
    context.addOp("movi", std::vector<std::string>({ virtualReg, std::to_string(mValue) }));
}

void NVarDecl::CodeGen(CodeContext& context)
{
    context.addOp("push", std::vector<std::string>({"r0"}));
    context.varIndices[mName] = context.varIndices.size();
}

void NArrayDecl::CodeGen(CodeContext& context)
{
    for (int i = 0; i < mSize->GetValue(); ++i) {
        context.addOp("push", std::vector<std::string>({"r0"}));
    }
    context.varIndices[mName] = context.varIndices.size();
}

void NNumericExpr::CodeGen(CodeContext& context)
{
    mNumeric->CodeGen(context);
    mResultRegister = "%" + std::to_string(context.lastReg);
}

void NVarExpr::CodeGen(CodeContext& context)
{
    ++context.lastReg;
    mResultRegister = "%" + std::to_string(context.lastReg);
    context.regIntervals[mResultRegister].first = context.ops.size(); 
    context.regIntervals[mResultRegister].second = context.ops.size();
    context.addOp("loadi", std::vector<std::string>({ mResultRegister, std::to_string(context.varIndices[mName]) }));
}

void NBinaryExpr::CodeGen(CodeContext& context)
{
    mLhs->CodeGen(context);
    mRhs->CodeGen(context);
    ++context.lastReg;
    std::string op;
    switch (mType)
    {
        case TADD:
            op = "add";
            break;
        case TSUB:
            op = "sub";
            break;
        case TMUL:
            op = "mul";
            break;
        case TDIV:
            op = "div";
            break;
        default:
            break;
    }
    mResultRegister = "%" + std::to_string(context.lastReg);
    context.regIntervals[mResultRegister].first = context.ops.size(); // current instruction #
    context.regIntervals[mResultRegister].second = context.ops.size();
    context.regIntervals[mLhs->GetResultRegister()].second = context.ops.size();
    context.regIntervals[mRhs->GetResultRegister()].second = context.ops.size();
    context.addOp(op, std::vector<std::string>({ mResultRegister, mLhs->GetResultRegister(), mRhs->GetResultRegister() }));
}

void NArrayExpr::CodeGen(CodeContext& context)
{
    mSubscript->CodeGen(context);
    ++context.lastReg;
    std::string virtualReg1 = "%" + std::to_string(context.lastReg);
    context.regIntervals[virtualReg1].first = context.ops.size();
    context.addOp("movi", std::vector<std::string>({virtualReg1, std::to_string(context.varIndices[mName])}));
    ++context.lastReg;
    std::string virtualReg2 = "%" + std::to_string(context.lastReg);
    context.regIntervals[virtualReg2].first = context.ops.size();
    context.regIntervals[virtualReg1].second = context.ops.size();
    context.regIntervals[mSubscript->GetResultRegister()].second = context.ops.size();
    context.addOp("add", std::vector<std::string>({virtualReg2, virtualReg1, mSubscript->GetResultRegister()}));
    ++context.lastReg;
    mResultRegister = "%" + std::to_string(context.lastReg);
    context.regIntervals[mResultRegister].first = context.ops.size();
    context.regIntervals[mResultRegister].second = context.ops.size();
    context.regIntervals[virtualReg2].second = context.ops.size();
    context.addOp("load", std::vector<std::string>({mResultRegister, virtualReg2}));
}

void NAssignVarStmt::CodeGen(CodeContext& context)
{
    mRhs->CodeGen(context);
    std::string stackIndex = std::to_string(context.varIndices[mName]); // get stack index of variable
    context.regIntervals[mRhs->GetResultRegister()].second = context.ops.size();
    context.addOp("storei", std::vector<std::string>({stackIndex, mRhs->GetResultRegister()}));
}

void NAssignArrayStmt::CodeGen(CodeContext& context)
{
    mRhs->CodeGen(context);
    mSubscript->CodeGen(context);
    ++context.lastReg;
    std::string virtualReg1 = "%" + std::to_string(context.lastReg);
    context.regIntervals[virtualReg1].first = context.ops.size();
    context.addOp("movi", std::vector<std::string>({virtualReg1, std::to_string(context.varIndices[mName])}));
    ++context.lastReg;
    std::string virtualReg2 = "%" + std::to_string(context.lastReg);
    context.regIntervals[virtualReg2].first = context.ops.size();
    context.regIntervals[virtualReg1].second = context.ops.size();
    context.regIntervals[mSubscript->GetResultRegister()].second = context.ops.size();
    context.addOp("add", std::vector<std::string>({virtualReg2, virtualReg1, mSubscript->GetResultRegister()}));
    context.regIntervals[virtualReg2].second = context.ops.size();
    context.regIntervals[mRhs->GetResultRegister()].second = context.ops.size();
    context.addOp("store", std::vector<std::string>({virtualReg2, mRhs->GetResultRegister()}));
}

void NIncStmt::CodeGen(CodeContext& context)
{
    ++context.lastReg;
    std::string virtualReg = "%" + std::to_string(context.lastReg);
    context.regIntervals[virtualReg].first = context.ops.size();
    context.addOp("loadi", std::vector<std::string>({virtualReg, std::to_string(context.varIndices[mName])}));
    context.addOp("inc", std::vector<std::string>({virtualReg}));
    context.regIntervals[virtualReg].second = context.ops.size();
    context.addOp("storei", std::vector<std::string>({std::to_string(context.varIndices[mName]), virtualReg}));
}

void NDecStmt::CodeGen(CodeContext& context)
{
    ++context.lastReg;
    std::string virtualReg = "%" + std::to_string(context.lastReg);
    context.regIntervals[virtualReg].first = context.ops.size();
    context.addOp("loadi", std::vector<std::string>({virtualReg, std::to_string(context.varIndices[mName])}));
    context.addOp("dec", std::vector<std::string>({virtualReg}));
    context.regIntervals[virtualReg].second = context.ops.size();
    context.addOp("storei", std::vector<std::string>({std::to_string(context.varIndices[mName]), virtualReg}));
}

void NComparison::CodeGen(CodeContext& context)
{
    mLhs->CodeGen(context);
    mRhs->CodeGen(context);
    std::string op;
    switch (mType)
    {
        case TLESS:
            op = "cmplt";
            break;
        case TISEQUAL:
            op = "cmpeq";
            break;
        default:
            break;
    }
    context.regIntervals[mLhs->GetResultRegister()].second = context.ops.size();
    context.regIntervals[mRhs->GetResultRegister()].second = context.ops.size();
    context.addOp(op, std::vector<std::string>({mLhs->GetResultRegister(), mRhs->GetResultRegister()}));
    
}

void NIfStmt::CodeGen(CodeContext& context)
{
    mComp->CodeGen(context);
    ++context.lastReg;
    std::string virtualReg1 = "%" + std::to_string(context.lastReg);
    context.regIntervals[virtualReg1].first = context.ops.size();
    int placeholderIndex = context.ops.size(); // get the index of the op we want to change
    context.addOp("movi", std::vector<std::string>({virtualReg1, ""})); // "" for placeholder address
    context.regIntervals[virtualReg1].second = context.ops.size();
    context.addOp("jnt", std::vector<std::string>({virtualReg1}));
    mIfBlock->CodeGen(context);
    if (mElseBlock) {
        ++context.lastReg;
        std::string virtualReg2 = "%" + std::to_string(context.lastReg);
        context.regIntervals[virtualReg2].first = context.ops.size();
        int placeholderIndex2 = context.ops.size(); // get the index of the op we want to change
        context.addOp("movi", std::vector<std::string>({virtualReg2, ""})); // "" for placeholder address
        context.regIntervals[virtualReg2].second = context.ops.size();
        context.addOp("jmp", std::vector<std::string>({virtualReg2}));
        int instruction = context.ops.size(); // get the instruction number to jump to for placeholder address
        context.ops[placeholderIndex].second[1] = std::to_string(instruction); // replace the placeholder address with the real instruction #
        mElseBlock->CodeGen(context);
        int instruction2 = context.ops.size(); // get the instruction number to jump to for placeholder address
        context.ops[placeholderIndex2].second[1] = std::to_string(instruction2); // replace the placeholder address with the real instruction #
    }
    else {
        int instruction = context.ops.size(); // get the instruction number to jump to for placeholder address
        context.ops[placeholderIndex].second[1] = std::to_string(instruction); // replace the placeholder address with the real instruction #
    }
}

void NWhileStmt::CodeGen(CodeContext& context)
{
    int beforeWhile = context.ops.size(); // get the instruction number for beginning of while loop
    mComp->CodeGen(context);
    ++context.lastReg;
    std::string virtualReg1 = "%" + std::to_string(context.lastReg);
    context.regIntervals[virtualReg1].first = context.ops.size();
    int jntPlaceholder = context.ops.size(); // get the index of the op we want to change
    context.addOp("movi", std::vector<std::string>({virtualReg1, ""})); // "" for placeholder address
    context.regIntervals[virtualReg1].second = context.ops.size();
    context.addOp("jnt", std::vector<std::string>({virtualReg1}));
    mBlock->CodeGen(context);
    ++context.lastReg;
    std::string virtualReg2 = "%" + std::to_string(context.lastReg);
    context.regIntervals[virtualReg2].first = context.ops.size();
    int jmpPlaceholder = context.ops.size(); // get the index of the op we want to change
    context.addOp("movi", std::vector<std::string>({virtualReg2, ""})); // "" for placeholder address
    context.regIntervals[virtualReg2].second = context.ops.size();
    context.addOp("jmp", std::vector<std::string>({virtualReg2}));
    int afterWhile = context.ops.size(); // get the instruction number for after while loop
    context.ops[jntPlaceholder].second[1] = std::to_string(afterWhile); // replace the placeholder address with the real instruction #
    context.ops[jmpPlaceholder].second[1] = std::to_string(beforeWhile); // replace the placeholder address with the real instruction #
}

void NPenUpStmt::CodeGen(CodeContext& context)
{
    context.addOp("penup", std::vector<std::string>());
}

void NPenDownStmt::CodeGen(CodeContext& context)
{
    context.addOp("pendown", std::vector<std::string>());
}

void NSetPosStmt::CodeGen(CodeContext& context)
{
    mXExpr->CodeGen(context);
    mYExpr->CodeGen(context);
    context.regIntervals[mXExpr->GetResultRegister()].second = context.ops.size();
    context.addOp("mov", std::vector<std::string>({"tx", mXExpr->GetResultRegister()}));
    context.regIntervals[mYExpr->GetResultRegister()].second = context.ops.size();
    context.addOp("mov", std::vector<std::string>({"ty", mYExpr->GetResultRegister()}));
}

void NSetColorStmt::CodeGen(CodeContext& context)
{
    mColor->CodeGen(context);
    context.regIntervals[mColor->GetResultRegister()].second = context.ops.size();
    context.addOp("mov", std::vector<std::string>({"tc", mColor->GetResultRegister()}));
}

void NFwdStmt::CodeGen(CodeContext& context)
{
    mParam->CodeGen(context);
    context.regIntervals[mParam->GetResultRegister()].second = context.ops.size();
    context.addOp("fwd", std::vector<std::string>({mParam->GetResultRegister()}));
}

void NBackStmt::CodeGen(CodeContext& context)
{
    mParam->CodeGen(context);
    context.regIntervals[mParam->GetResultRegister()].second = context.ops.size();
    context.addOp("back", std::vector<std::string>({mParam->GetResultRegister()}));
}

void NRotStmt::CodeGen(CodeContext& context)
{
    mParam->CodeGen(context);
    context.regIntervals[mParam->GetResultRegister()].second = context.ops.size();
    context.addOp("add", std::vector<std::string>({"tr", "tr", mParam->GetResultRegister()}));
}
