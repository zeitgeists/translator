#include <cstdio>
#include "Lexer.hpp"
#include "Parser.hpp"
#include "AST.hpp"
#include "JIT.hpp"
#include "STD.hpp"

static void HandleDefinition() {
    if (auto FnAST = Parser::ParseDefinition()) {
        if (auto *FnIR = FnAST->codegen()) {
            fprintf(stderr, "Read function definition:\n");
            // FnAST->ToStdOut("", false);
            FnIR->print(llvm::errs());
            fprintf(stderr, "\n");
            ExitOnErr(TheJIT->addModule(llvm::orc::ThreadSafeModule(
                std::move(TheModule), std::move(TheContext))));
            AST::InitializeModuleAndFPM();
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
            // ProtoAST->ToStdOut("", false);
            FnIR->print(llvm::errs());
            fprintf(stderr, "\n");
            FunctionProtos[ProtoAST->getName()] = std::move(ProtoAST);
        }
    } else {
        // Skip token for error recovery.
        Parser::GetNextToken();
    }
}

static void HandleTopLevelExpression() {
    // Evaluate a top-level expression into an anonymous function.
    if (auto FnAST = Parser::ParseTopLevelExpr()) {
        if (FnAST->codegen()) {
            // Create a ResourceTracker to track JIT'd memory allocated to our
            // anonymous expression -- that way we can free it after executing.
            auto RT = TheJIT->getMainJITDylib().createResourceTracker();

            auto TSM = llvm::orc::ThreadSafeModule(std::move(TheModule),
                                                   std::move(TheContext));
            ExitOnErr(TheJIT->addModule(std::move(TSM), RT));
            AST::InitializeModuleAndFPM();

            // Search the JIT for the __anonymous_expr symbol.
            auto ExprSymbol = ExitOnErr(TheJIT->lookup("__anonymous_expr"));

            // Get the symbol's address and cast it to the right type (takes no
            // arguments, returns a double) so we can call it as a native function.
            double (*FP)() = (double (*)())(intptr_t)ExprSymbol.getAddress();
            fprintf(stderr, "Evaluated to %f\n", FP());

            // Delete the anonymous expression module from the JIT.
            ExitOnErr(RT->remove());
        }
    } else {
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
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    fprintf(stderr, "$ \n");
    Parser::GetNextToken();

    TheJIT = ExitOnErr(llvm::orc::MyCustomJIT::Create());

    AST::InitializeModuleAndFPM();

    MainLoop();
    // AST::PrintGeneratedCode();
    return 0;
}
