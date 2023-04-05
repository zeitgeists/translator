#pragma once

#include "AST.hpp"

#include <iostream>
#include <string>
#include <memory>

namespace Logger {
    std::unique_ptr<ExprAST> LogError(std::string str);
    std::unique_ptr<PrototypeAST> LogErrorP(std::string str);
    llvm::Value* LogErrorV(std::string str);
}
