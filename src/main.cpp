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
    Parser::getNextToken();
    exit(1);
  }
}

static void MainLoop() {
  while (true) {
    fprintf(stderr, "$ ");
    switch (Parser::CurTok) {
    case tok_eof:
      return;
    case ';': // ignore top-level semicolons.
      Parser::getNextToken();
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
    // Prime the first token.
    fprintf(stderr, "ready> ");
    Parser::getNextToken();

    MainLoop();
    return 0;
}
