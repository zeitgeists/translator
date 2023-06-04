#include "CodeGenerator.hpp"

CodeGenerator::CodeGenerator() {
    TheJIT = ExitOnErr(llvm::orc::MyCustomJIT::Create());

    // Open a new context and module.
    TheContext = std::make_unique<llvm::LLVMContext>();
    TheModule = std::make_unique<llvm::Module>("JIT module", *TheContext);
    TheModule->setDataLayout(TheJIT->getDataLayout());

    // Create a new builder for the module.
    Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);

    // Create a new pass manager attached to it.
    TheFPM = std::make_unique<llvm::legacy::FunctionPassManager>(TheModule.get());

    // Do simple "peephole" optimizations and bit-twiddling optzns.
    TheFPM->add(llvm::createInstructionCombiningPass());
    // Reassociate expressions.
    TheFPM->add(llvm::createReassociatePass());
    // Eliminate Common SubExpressions.
    TheFPM->add(llvm::createGVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    TheFPM->add(llvm::createCFGSimplificationPass());

    TheFPM->doInitialization();
}

void CodeGenerator::PushStr(Token) {
}

void CodeGenerator::PushOperator(Token) {
}

void CodeGenerator::PushParam(Token) {
}

void CodeGenerator::PushArg(Token) {
}

bool CodeGenerator::GenVariable() {
    return true;
}

bool CodeGenerator::GenNumber() {
    fmt::print("GenNumber called\n");
    return true;
}

bool CodeGenerator::GenNegation() {
    return true;
}

bool CodeGenerator::GenOperator() {
    return true;
}

bool CodeGenerator::GenCall() {
    return true;
}

bool CodeGenerator::GenPrototype() {
    return true;
}

bool CodeGenerator::GenFunction() {
    return true;
}

bool CodeGenerator::GenAnonFunction() {
    return true;
}
