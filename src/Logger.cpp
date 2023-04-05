#include "Logger.hpp"

std::unique_ptr<ExprAST> Logger::LogError(std::string str) {
    fprintf(stderr, "Error: %s\n", str.c_str());
    return nullptr;
}

std::unique_ptr<PrototypeAST> Logger::LogErrorP(std::string str) {
    Logger::LogError(str);
    return nullptr;
}

llvm::Value* Logger::LogErrorV(std::string str) {
    Logger::LogError(str);
    return nullptr;
}
