#pragma once
#include <iostream>
#include <queue>
#include <string>
#include <vector>
#include <memory>
#include <map>

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

#include "JIT.hpp"
#include "Token.hpp"

class CodeGenerator {
public:
    CodeGenerator();
    void PushStr(Token);
    void PushOperator(Token);
    void PushParam(Token);
    void PushArg(Token);

    bool GenVariable();
    bool GenNumber();
    bool GenNegation();
    bool GenOperator();
    bool GenCall();
    bool GenPrototype();
    bool GenFunction();
    bool GenAnonFunction();
    bool GenExtern();
private:
    llvm::Function *getFunction(std::string name);

    std::stack<Token> termsStack;
    std::stack<Token> operatorsStack;
    std::stack<Token> paramsStack;
    std::vector<llvm::Value*> argsV;
    std::stack<llvm::Value*> valuesStack;
    std::stack<llvm::Function*> prototypesStack;
    std::stack<llvm::Function*> functionsStack;

    std::unique_ptr<llvm::LLVMContext> TheContext;
    std::unique_ptr<llvm::IRBuilder<>> Builder;
    std::unique_ptr<llvm::Module> TheModule;
    std::map<std::string, llvm::Value*> NamedValues;
    std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;
    std::unique_ptr<llvm::orc::MyCustomJIT> TheJIT;
    llvm::ExitOnError ExitOnErr;
    std::map<llvm::StringRef, llvm::Function*> FunctionProtos;
};
