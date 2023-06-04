#include "CodeGenerator.hpp"

CodeGenerator::CodeGenerator() {
    TheJIT = ExitOnErr(llvm::orc::MyCustomJIT::Create());
    InitializeModuleAndFPM();
}

void CodeGenerator::InitializeModuleAndFPM() {
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
    paramsStack.push(std::move(t));
}

void CodeGenerator::PushArg(Token t) {
    if (valuesStack.empty()) {
        fmt::print("PushArg called but valuesStack is empty!\n");
        return;
    }
    llvm::Value *V = valuesStack.top();
    valuesStack.pop();
    argsV.push_back(std::move(V));
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
    valuesStack.pop();
    llvm::Value *L = valuesStack.top();
    valuesStack.pop();
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

    std::vector<llvm::Value*> args(argsV.size());
    for (auto it = argsV.begin(); it != argsV.end(); it++) {
        args.push_back(*it);
        argsV.pop_back();
    }

    llvm::Value* V = Builder->CreateCall(CalleeF, args, "calltmp");
    if (!V) return false;

    valuesStack.push(V);
    return true;
}

llvm::Function* CodeGenerator::GenPrototype() {
    if (termsStack.empty()) {
        fmt::print("GenPrototype called but termsStack empty!\n");
        return nullptr;
    }
    std::string name = termsStack.top().str;
    termsStack.pop();

    // TODO: maybe this is obsolete
    auto Pr = FunctionProtos.find(name);
    if (Pr != FunctionProtos.end()) {
        return nullptr;
    }

    // Make the function type:  double(double,double) etc.
    std::vector<llvm::Type*> Doubles(paramsStack.size(), llvm::Type::getDoubleTy(*TheContext));
    llvm::FunctionType *FT =
        llvm::FunctionType::get(llvm::Type::getDoubleTy(*TheContext), Doubles, false);

    llvm::Function *P =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, name, TheModule.get());

    if (!P) return nullptr;

    // Set names for all arguments.
    for (auto &Arg : P->args()) {
        Arg.setName(paramsStack.top().str);
        paramsStack.pop();
    }

    FunctionProtos[P->getName()] = std::move(P);
    return P;
}

llvm::Function* CodeGenerator::GenFunction() {
    if (termsStack.empty()) {
        fmt::print("GenFunction called but termsStack empty!\n");
        return nullptr;
    }
    std::string name = termsStack.top().str;

    GenPrototype();

    llvm::Function *TheFunction = getFunction(name);
    if (!TheFunction) return nullptr;

    // Create a new basic block to start insertion into.
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", TheFunction);
    Builder->SetInsertPoint(BB);

    // Record the function arguments in the NamedValues map.
    NamedValues.clear();
    for (auto &Arg : TheFunction->args())
        NamedValues[std::string(Arg.getName())] = &Arg;

    if (valuesStack.empty()) {
        fmt::print("GenFunction called but valuesStack empty!\n");
        return nullptr;
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

        return TheFunction;
    }

    // Error reading body, remove function.
    TheFunction->eraseFromParent();
    return nullptr;
}

bool CodeGenerator::GenAnonFunction() {
    // Evaluate a top-level expression into an anonymous function.
    Token functionIdentifier;
    TokenSet::Identifier(functionIdentifier);
    functionIdentifier.str = "__anonymous_expr";
    PushStr(functionIdentifier);
    if (!GenFunction()) return false;

    // Create a ResourceTracker to track JIT'd memory allocated to our
    // anonymous expression -- that way we can free it after executing.
    auto RT = TheJIT->getMainJITDylib().createResourceTracker();

    auto TSM = llvm::orc::ThreadSafeModule(std::move(TheModule),
                                           std::move(TheContext));
    ExitOnErr(TheJIT->addModule(std::move(TSM), RT));
    InitializeModuleAndFPM();

    // Search the JIT for the __anonymous_expr symbol.
    auto ExprSymbol = ExitOnErr(TheJIT->lookup("__anonymous_expr"));

    // Get the symbol's address and cast it to the right type (takes no
    // arguments, returns a double) so we can call it as a native function.
    double (*FP)() = (double (*)())(intptr_t)ExprSymbol.getAddress();
    fprintf(stderr, "Evaluated to %f\n", FP());

    // Delete the anonymous expression module from the JIT.
    ExitOnErr(RT->remove());
    return true;
}

bool CodeGenerator::GenExtern() {
    auto P = GenPrototype();
    if (!P) return false;
    fprintf(stderr, "Read extern:\n");
    P->print(llvm::errs());
    fprintf(stderr, "\n");
    FunctionProtos[P->getName()] = std::move(P);
    return true;
}
bool CodeGenerator::GenDef() {
    if (!GenFunction()) return false;
    fprintf(stderr, "Read function definition\n");
    ExitOnErr(TheJIT->addModule(llvm::orc::ThreadSafeModule(
        std::move(TheModule), std::move(TheContext))));
    InitializeModuleAndFPM();
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
        // Token functionIdentifier;
        // TokenSet::Identifier(functionIdentifier);
        // functionIdentifier.str = name;
        // PushStr(functionIdentifier);
        return FI->second;
    }

    // If no existing prototype exists, return null.
    return nullptr;
}

void CodeGenerator::PrintGeneratedCode() {
    TheModule->print(llvm::errs(), nullptr);
}
