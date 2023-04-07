#include <cstdio>
#include "Lexer.hpp"
#include "Parser.hpp"
#include "AST.hpp"

static void HandleDefinition() {
    if (auto FnAST = Parser::ParseDefinition()) {
        if (auto *FnIR = FnAST->codegen()) {
            fprintf(stderr, "Read function definition:\n");
            FnAST->ToStdOut("", false);
            FnIR->print(llvm::errs());
            fprintf(stderr, "\n");
        }
    } else {
        // Skip token for error recovery.
        Parser::GetNextToken();
    }
}

static void HandleExtern() {
    if (auto ProtoAST = Parser::ParseExtern()) {
        if (auto *FnIR = ProtoAST->codegen()) {
            fprintf(stderr, "Read extern:\n");
            ProtoAST->ToStdOut("", false);
            FnIR->print(llvm::errs());
            fprintf(stderr, "\n");
        }
    } else {
        // Skip token for error recovery.
        Parser::GetNextToken();
    }
}

static void HandleTopLevelExpression() {
    // Evaluate a top-level expression into an anonymous function.
    if (auto FnAST = Parser::ParseTopLevelExpr()) {
        if (auto *FnIR = FnAST->codegen()) {
            fprintf(stderr, "Read top-level expression:\n");
            FnAST->ToStdOut("", false);
            FnIR->print(llvm::errs());
            fprintf(stderr, "\n");

            // Remove the anonymous expression.
            FnIR->eraseFromParent();
        }
    } else {
        fprintf(stderr, "error in parsing top-level\n");
        // Skip token for error recovery.
        Parser::GetNextToken();
    }
}

static void MainLoop() {
    while (true) {
        fprintf(stderr, "$ \n");
        switch (Parser::GetCurrentToken().tokenId) {
        case tok_eof:
            return;
        case ';': // ignore top-level semicolons.
            Parser::GetNextToken();
            break;
            case tok_def:
              HandleDefinition();
              break;
            case tok_extern:
              HandleExtern();
              break;
        default:
            HandleTopLevelExpression();
            break;
        }
    }
}

int main (int argc, char *argv[]) {
    fprintf(stderr, "$ \n");
    Parser::GetNextToken();
    AST::InitializeModule();
    MainLoop();
    AST::PrintGeneratedCode();
    return 0;
}
