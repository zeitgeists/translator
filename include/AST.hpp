#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>

// ExprAST - Base class for all expression nodes.
class ExprAST {
public:
    virtual void ToStdOut(const std::string& prefix, bool isLeft);
    virtual ~ExprAST() = default;
};

// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
    double Val;

public:
    explicit NumberExprAST(double Val);
    void ToStdOut(const std::string& prefix, bool isLeft) override;
};

// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
    std::string Name;

public:
    explicit VariableExprAST(const std::string &Name);
    void ToStdOut(const std::string& prefix, bool isLeft) override;
};

class OperatorAST : public ExprAST {
    std::string Operator;
    std::unique_ptr<ExprAST> LHS, RHS;

public:
    OperatorAST(std::string Operator, std::unique_ptr<ExprAST> LHS,
                std::unique_ptr<ExprAST> RHS);
    void ToStdOut(const std::string& prefix, bool isLeft) override;
};

// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;

public:
    CallExprAST(const std::string &Callee,
                std::vector<std::unique_ptr<ExprAST>> Args);
};

// PrototypeAST - This class represents the "prototype" for a function,
// which captures its name, and its argument names
class PrototypeAST {
    std::string Name;
    std::vector<std::string> Args;

public:
    PrototypeAST(const std::string &Name, std::vector<std::string> Args);

    const std::string &getName() const;
};

// FunctionAST - This class represents a function definition itself.
class FunctionAST {
    std::unique_ptr<PrototypeAST> Prototype;
    std::unique_ptr<ExprAST> Body;

public:
    FunctionAST(std::unique_ptr<PrototypeAST> Prototype,
        std::unique_ptr<ExprAST> Body);
};

namespace AST {
    void PrintAST(ExprAST* expr);
}
