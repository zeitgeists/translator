#include "CodeGenerator.hpp"

CodeGenerator::CodeGenerator() {
    termsQueue = {};
    operatorsQueue = {};
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
