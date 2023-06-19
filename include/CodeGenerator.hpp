#pragma once
#include <iostream>
#include <queue>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <array>

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
    bool GenAnonFunction();
    bool GenExtern();
    bool GenDef();

    void PrintGeneratedCode();

private:
    // ExprAST - Base class for all expression nodes.
    class ExprAST {
    public:
        virtual void ToStdOut(const std::string& prefix, bool isLeft);
        virtual llvm::Value *codegen() = 0;
        virtual ~ExprAST() = default;
    };

    // NumberExprAST - Expression class for numeric literals like "1.0".
    class NumberExprAST : public ExprAST {
        double Val;

    public:
        explicit NumberExprAST(double Val);
        void ToStdOut(const std::string& prefix, bool isLeft) override;
        llvm::Value* codegen() override;
    };

    // VariableExprAST - Expression class for referencing a variable, like "a".
    class VariableExprAST : public ExprAST {
        std::string Name;

    public:
        explicit VariableExprAST(const std::string &Name);
        void ToStdOut(const std::string& prefix, bool isLeft) override;
        llvm::Value* codegen() override;
    };

    class OperatorAST : public ExprAST {
        Token Operator;
        std::unique_ptr<ExprAST> LHS, RHS;

    public:
        OperatorAST(Token Operator, std::unique_ptr<ExprAST> LHS,
                    std::unique_ptr<ExprAST> RHS);
        void ToStdOut(const std::string& prefix, bool isLeft) override;
        llvm::Value* codegen() override;
    };

    // CallExprAST - Expression class for function calls.
    class CallExprAST : public ExprAST {
        std::string Callee;
        std::unique_ptr<std::vector<std::unique_ptr<ExprAST>>> Args;

    public:
        CallExprAST(const std::string &Callee,
                    std::unique_ptr<std::vector<std::unique_ptr<ExprAST>>> Args);
        llvm::Value* codegen() override;
        void ToStdOut(const std::string& prefix, bool isLeft) override;
    };

    // PrototypeAST - This class represents the "prototype" for a function,
    // which captures its name, and its argument names
    class PrototypeAST {
        std::string Name;
        std::unique_ptr<std::vector<std::string>> Args;

    public:
        PrototypeAST(const std::string &Name,
                std::unique_ptr<std::vector<std::string>> Args);
        llvm::Function* codegen();
        void ToStdOut(const std::string& prefix, bool isLeft);

        const std::string &getName() const;
    };

    // FunctionAST - This class represents a function definition itself.
    class FunctionAST {
        std::unique_ptr<PrototypeAST> Prototype;
        std::unique_ptr<ExprAST> Body;

    public:
        FunctionAST(std::unique_ptr<PrototypeAST> Prototype,
            std::unique_ptr<ExprAST> Body);
        llvm::Function* codegen();
        void ToStdOut(const std::string& prefix, bool isLeft);
    };

    static llvm::Function *getFunction(std::string name);
    void InitializeModuleAndFPM();

    std::unique_ptr<PrototypeAST> GenPrototype();
    std::unique_ptr<FunctionAST> GenFunction(std::unique_ptr<PrototypeAST>);

    std::stack<Token> termsStack;
    std::stack<Token> operatorsStack;
    // std::stack<Token> paramsStack;
    // std::stack<Token> argsStack;
    std::unique_ptr<std::vector<std::string>> paramsV;
    std::unique_ptr<std::vector<std::unique_ptr<ExprAST>>> argsV;
    // std::vector<llvm::Value*> argsV;
    // std::stack<llvm::Value*> valuesStack;
    std::stack<std::unique_ptr<ExprAST>> exprASTStack;

    static std::unique_ptr<llvm::LLVMContext> TheContext;
    static std::unique_ptr<llvm::IRBuilder<>> Builder;
    static std::unique_ptr<llvm::Module> TheModule;
    static std::map<std::string, llvm::Value*> NamedValues;
    static std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;
    static std::unique_ptr<llvm::orc::MyCustomJIT> TheJIT;
    static llvm::ExitOnError ExitOnErr;

    static std::map<llvm::StringRef, std::unique_ptr<PrototypeAST>> FunctionProtos;
};
