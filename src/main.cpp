#include <cstdio>
#include <memory>
#include "Lexer.hpp"
#include "Parser.hpp"
#include "STD.hpp"

int main (int argc, const char *argv[]) {
    std::string fileName = "";
    if (argc <= 1) {
        fileName = "";
    } else {
        fileName = argv[1];
    }

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    std::unique_ptr<Lexer> lexer(new Lexer(fileName));

    Parser parser(std::move(lexer));

    parser.ParseLoop();

    return 0;
}
