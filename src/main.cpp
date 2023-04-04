#include <cstdio>
#include "Lexer.hpp"
#include "Parser.hpp"
#include "AST.hpp"

static void HandleTopLevelExpression() {
    // Evaluate a top-level expression into an anonymous function.
    if (Parser::ParseTopLevelExpr()) {
        fprintf(stderr, "Parsed a top-level expr\n");
    } else {
        fprintf(stderr, "error in parsing top-level\n");
        // Skip token for error recovery.
        Parser::GetNextToken();
        exit(1);
    }
    Lexer::PrintLoggedTokens();
}

static void MainLoop() {
    fprintf(stderr, "$ ");
    Parser::GetNextToken();
    while (true) {
        fprintf(stderr, "$ ");
        switch (Parser::GetCurrentToken().tokenId) {
        case tok_eof:
            return;
        case ';': // ignore top-level semicolons.
            Parser::GetNextToken();
            break;
            // case tok_def:
            //   HandleDefinition();
            //   break;
            // case tok_extern:
            //   HandleExtern();
            //   break;
        default:
            HandleTopLevelExpression();
            break;
        }
    }
}

int main (int argc, char *argv[]) {
    MainLoop();
    return 0;
}
