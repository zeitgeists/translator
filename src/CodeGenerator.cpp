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

void CodeGenerator::PushStr(Token t) {
    termsStack.push(std::move(t));
}

void CodeGenerator::PushOperator(Token t) {
    operatorsStack.push(std::move(t));
}

void CodeGenerator::PushParam(Token t) {
    // paramsStack.push(std::move(t));
}

void CodeGenerator::PushArg(Token t) {
}

bool CodeGenerator::GenVariable() {
    if (termsStack.empty()) {
        fmt::print("GenVariable called but stack is empty!\n");
        return false;
    }
    std::string name  = termsStack.top().str;
    termsStack.pop();
    llvm::Value *V = NamedValues[name];
    if (!V) {
        fmt::print("Unknown variable name {}\n", name);
        return false;
    }
    valuesStack.push(V);
    return true;
}

bool CodeGenerator::GenNumber() {
    if (termsStack.empty()) {
        fmt::print("GenNumber called but stack is empty!\n");
        return false;
    }
    double number = std::stod(termsStack.top().str);
    termsStack.pop();
    llvm::Value* V =
        llvm::ConstantFP::get(*TheContext, llvm::APFloat(number));
    if (!V) return false;
    valuesStack.push(V);
    return true;
}

bool CodeGenerator::GenNegation() {
    fmt::print("GenNegation not implemented!\n");
    return false;
}

bool CodeGenerator::GenOperator() {
    if (valuesStack.size() < 2) {
        fmt::print("GenOperator called but valuesStack size < 2!\n");
        return false;
    }
    llvm::Value *R = valuesStack.top();
    termsStack.pop();
    llvm::Value *L = valuesStack.top();
    termsStack.pop();
    if (!L || !R) return false;

    if (operatorsStack.empty()) {
        fmt::print("GenOperator called but operatorsStack empty!\n");
        return false;
    }
    Token op = operatorsStack.top();
    operatorsStack.pop();

    if (op.type != Token::Operator) {
        fmt::print("GenOperator called but operator token is [{}, {}, '{}']\n",
                op.typeToStr(), op.subtypeToStr(), op.str);
        return false;
    }

    llvm::Value* V;
    switch (op.subtype) {
        case Token::Plus:
            V = Builder->CreateFAdd(L, R, "addtmp");
            break;
        case Token::Minus:
            V = Builder->CreateFSub(L, R, "subtmp");
            break;
        case Token::Multiply:
            V = Builder->CreateFMul(L, R, "multmp");
            break;
        case Token::Divide:
            V = Builder->CreateFDiv(L, R, "divtmp");
            break;
        default:
            fmt::print("GenOperator called but operator token sybtype unexpected [{}, {}, '{}']\n",
                    op.typeToStr(), op.subtypeToStr(), op.str);
            return false;
    }
    if (!V) return false;

    valuesStack.push(V);
    return true;
}

bool CodeGenerator::GenCall() {
    if (termsStack.empty()) {
        fmt::print("GenCall called but termsStack empty!\n");
        return false;
    }
    std::string Callee = termsStack.top().str;
    termsStack.pop();

    // Look up the name in the global module table.
    llvm::Function *CalleeF = getFunction(Callee);
    if (!CalleeF) {
        fmt::print("Unknown function referenced {}\n", Callee);
        return false;
    }

    // If argument mismatch error.
    if (CalleeF->arg_size() != argsV.size()) {
        fmt::print("Incorrect amount of arguments passed");
        return false;
    }

    llvm::Value* V = Builder->CreateCall(CalleeF, argsV, "calltmp");
    if (!V) return false;

    valuesStack.push(V);
    return true;
}

bool CodeGenerator::GenPrototype() {
    if (termsStack.empty()) {
        fmt::print("GenPrototype called but termsStack empty!\n");
        return false;
    }
    std::string name = termsStack.top().str;
    termsStack.pop();
    // Make the function type:  double(double,double) etc.
    std::vector<llvm::Type*> Doubles(paramsStack.size(), llvm::Type::getDoubleTy(*TheContext));
    llvm::FunctionType *FT =
        llvm::FunctionType::get(llvm::Type::getDoubleTy(*TheContext), Doubles, false);

    llvm::Function *F =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, name, TheModule.get());

    if (!F) return false;

    // Set names for all arguments.
    for (auto &Arg : F->args()) {
        Arg.setName(paramsStack.top().str);
        paramsStack.pop();
    }

    prototypesStack.push(F);
    return true;
}

bool CodeGenerator::GenFunction() {
    if (prototypesStack.empty()) {
        fmt::print("GenFunction called but prototypesStack empty!\n");
        return false;
    }
    llvm::Function* Prototype = prototypesStack.top();
    prototypesStack.pop();

    // Transfer ownership of the prototype to the FunctionProtos map, but keep a
    // reference to it for use below.
    auto &P = *Prototype;
    FunctionProtos[Prototype->getName()] = std::move(Prototype);
    std::string name(P.getName());
    llvm::Function *TheFunction = getFunction(name);
    if (!TheFunction) return false;

    // Create a new basic block to start insertion into.
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", TheFunction);
    Builder->SetInsertPoint(BB);

    // Record the function arguments in the NamedValues map.
    NamedValues.clear();
    for (auto &Arg : TheFunction->args())
        NamedValues[std::string(Arg.getName())] = &Arg;

    if (valuesStack.empty()) {
        fmt::print("GenFunction called but valuesStack empty!\n");
        return false;
    }
    llvm::Value* retVal = valuesStack.top();
    valuesStack.pop();

    if (retVal) {
        // Finish off the function.
        Builder->CreateRet(retVal);

        // Validate the generated code, checking for consistency.
        verifyFunction(*TheFunction);

        // Optimize the function.
        TheFPM->run(*TheFunction);

        functionsStack.push(TheFunction);
        return true;
    }

    // Error reading body, remove function.
    TheFunction->eraseFromParent();
    return false;
}

bool CodeGenerator::GenAnonFunction() {
    return true;
}

bool CodeGenerator::GenExtern() {
    return true;
}

llvm::Function *CodeGenerator::getFunction(std::string name) {
    // First, see if the function has already been added to the current module.
    if (auto *F = TheModule->getFunction(name))
        return F;

    // If not, check whether we can codegen the declaration from some existing
    // prototype.
    auto FI = FunctionProtos.find(name);
    if (FI != FunctionProtos.end()) {
        return FI->second->codegen();
    }

    // If no existing prototype exists, return null.
    return nullptr;
}
