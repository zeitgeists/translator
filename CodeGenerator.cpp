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
    if (valuesV.empty()) {
        fmt::print("PushArg called but valuesV is empty!\n");
        return;
    }
    llvm::Value *V = valuesV.back();
    valuesV.pop_back();
    argsV.push_back(V);
}

// bool CodeGenerator::GenVariable() {
//     if (termsStack.empty()) {
//         fmt::print("GenVariable called but stack is empty!\n");
//         return false;
//     }
//     std::string name  = termsStack.top().str;
//     termsStack.pop();
//     llvm::Value *V = NamedValues[name];
//     if (!V) {
//         fmt::print("Unknown variable name {}\n", name);
//         return false;
//     }
//     valuesV.push_back(V);
//     return true;
// }
//
// bool CodeGenerator::GenNumber() {
//     if (termsStack.empty()) {
//         fmt::print("GenNumber called but stack is empty!\n");
//         return false;
//     }
//     double number = std::stod(termsStack.top().str);
//     termsStack.pop();
//     llvm::Value* V =
//         llvm::ConstantFP::get(*TheContext, llvm::APFloat(number));
//     if (!V) return false;
//     valuesV.push_back(V);
//     return true;
// }
//
// bool CodeGenerator::GenNegation() {
//     fmt::print("GenNegation not implemented!\n");
//     return false;
// }
//
// bool CodeGenerator::GenOperator() {
//     if (valuesV.size() < 2) {
//         fmt::print("GenOperator called but valuesV size < 2!\n");
//         return false;
//     }
//     llvm::Value *R = valuesV.back();
//     llvm::Value *L = valuesV[valuesV.size() - 2];
//     if (!L || !R) return false;
//
//     if (operatorsStack.empty()) {
//         fmt::print("GenOperator called but operatorsStack empty!\n");
//         return false;
//     }
//     Token op = operatorsStack.top();
//     operatorsStack.pop();
//
//     if (op.type != Token::Operator) {
//         fmt::print("GenOperator called but operator token is [{}, {}, '{}']\n",
//                 op.typeToStr(), op.subtypeToStr(), op.str);
//         return false;
//     }
//
//     llvm::Value* V;
//     switch (op.subtype) {
//         case Token::Plus:
//             V = Builder->CreateFAdd(L, R, "addtmp");
//             break;
//         case Token::Minus:
//             V = Builder->CreateFSub(L, R, "subtmp");
//             break;
//         case Token::Multiply:
//             V = Builder->CreateFMul(L, R, "multmp");
//             break;
//         case Token::Divide:
//             V = Builder->CreateFDiv(L, R, "divtmp");
//             break;
//         default:
//             fmt::print("GenOperator called but operator token sybtype unexpected [{}, {}, '{}']\n",
//                     op.typeToStr(), op.subtypeToStr(), op.str);
//             return false;
//     }
//     if (!V) return false;
//
//     valuesV.push_back(V);
//     return true;
// }
//
// bool CodeGenerator::GenCall() {
//     fmt::print("Generating call\n");
//     if (termsStack.empty()) {
//         fmt::print("GenCall called but termsStack empty!\n");
//         return false;
//     }
//     std::string Callee = termsStack.top().str;
//     termsStack.pop();
//
//     // Look up the name in the global module table.
//     llvm::Function *CalleeF = getFunction(Callee);
//     if (!CalleeF) {
//         fmt::print("Unknown function referenced {}\n", Callee);
//         return false;
//     }
//     fmt::print("Generating call phase 2\n");
//
//     // If argument mismatch error.
//     if (CalleeF->arg_size() != argsV.size()) {
//         fmt::print("Incorrect amount of arguments passed");
//         return false;
//     }
//     fmt::print("Generating call phase 3\n");
//
//     std::vector<llvm::Value*> args(argsV.size());
//     for (auto it = argsV.begin(); it != argsV.end(); it++) {
//         args.push_back(*it);
//     }
//     for (size_t i = 0; i < argsV.size(); i++) {
//         argsV.pop_back();
//     }
//     fmt::print("Generating call phase 4\n");
//
//     llvm::Value* V = Builder->CreateCall(CalleeF, args, "calltmp");
//     if (!V) return false;
//
//     fmt::print("Generating call phase 5\n");
//
//     valuesV.push_back(V);
//     return true;
// }
//
// bool CodeGenerator::GenPrototype() {
//     if (termsStack.empty()) {
//         fmt::print("GenPrototype called but termsStack empty!\n");
//         return false;
//     }
//     std::string name = termsStack.top().str;
//     termsStack.pop();
//
//     // Make the function type:  double(double,double) etc.
//     std::vector<llvm::Type*> Doubles(paramsStack.size(), llvm::Type::getDoubleTy(*TheContext));
//     llvm::FunctionType *FT =
//         llvm::FunctionType::get(llvm::Type::getDoubleTy(*TheContext), Doubles, false);
//
//     llvm::Function *P =
//         llvm::Function::Create(FT, llvm::Function::ExternalLinkage, name, TheModule.get());
//
//     if (!P) return false;
//
//     fmt::print("GenPrototype: paramsStack.size() = {}\n", paramsStack.size()); // DEBUG
//
//     // Set names for all arguments.
//     for (auto &Arg : P->args()) {
//         fmt::print("GenPrototype: paramsStack.top() = {}\n", paramsStack.top().str); // DEBUG
//         Arg.setName(paramsStack.top().str);
//         paramsStack.pop();
//         fmt::print("GenPrototype: Arg.getName() = {}\n", Arg.getName()); // DEBUG
//     }
//
//     // Record protos params in the NamedValues map.
//     NamedValues.clear();
//     for (auto &Arg : P->args())
//         NamedValues[std::string(Arg.getName())] = &Arg;
//
//     lastProtoName = name;
//
//     FunctionProtos[P->getName()] = std::move(P);
//     return true;
// }
//
// llvm::Function* CodeGenerator::GenFunction() {
//     // if (termsStack.empty()) {
//     //     fmt::print("GenFunction called but termsStack empty!\n");
//     //     return nullptr;
//     // }
//     // std::string name = termsStack.top().str;
//
//     // GenPrototype();
//
//     std::string name = lastProtoName;
//
//     llvm::Function *TheFunction = getFunction(name);
//     if (!TheFunction) return nullptr;
//
//     // Create a new basic block to start insertion into.
//     llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", TheFunction);
//     Builder->SetInsertPoint(BB);
//
//     if (valuesV.empty()) {
//         fmt::print("GenFunction called but valuesV empty!\n");
//         return nullptr;
//     }
//     llvm::Value* retVal = valuesV.back();
//
//     if (retVal) {
//         // Finish off the function.
//         Builder->CreateRet(retVal);
//
//         // Validate the generated code, checking for consistency.
//         verifyFunction(*TheFunction);
//
//         // Optimize the function.
//         TheFPM->run(*TheFunction);
//
//         return TheFunction;
//     }
//
//     // Error reading body, remove function.
//     TheFunction->eraseFromParent();
//     return nullptr;
// }
//
// bool CodeGenerator::GenAnonFunction() {
//     // Evaluate a top-level expression into an anonymous function.
//     Token functionIdentifier;
//     TokenSet::Identifier(functionIdentifier);
//     functionIdentifier.str = "__anonymous_expr";
//     PushStr(functionIdentifier);
//     fmt::print("GenAnonFunction: generating prototype\n"); // DEBUG
//     if (!GenPrototype()) return false;
//     fmt::print("GenAnonFunction: generated prototype\n"); // DEBUG
//     fmt::print("GenAnonFunction: generating function\n"); // DEBUG
//     // if (!GenFunction()) return false;
//     auto F = GenFunction();
//     if (!F) return false;
//     fmt::print("GenAnonFunction: generated function\n"); // DEBUG
//     fmt::print("function name: {}\n", F->getName());
//     F->print(llvm::errs());
//
//     // Create a ResourceTracker to track JIT'd memory allocated to our
//     // anonymous expression -- that way we can free it after executing.
//     auto RT = TheJIT->getMainJITDylib().createResourceTracker();
//     fmt::print("GenAnonFunction: RT created\n"); // DEBUG
//
//     auto TSM = llvm::orc::ThreadSafeModule(std::move(TheModule),
//                                            std::move(TheContext));
//     fmt::print("GenAnonFunction: TSM created\n"); // DEBUG
//     ExitOnErr(TheJIT->addModule(std::move(TSM), RT));
//     fmt::print("GenAnonFunction: module added\n"); // DEBUG
//     InitializeModuleAndFPM();
//     fmt::print("GenAnonFunction: new module initialized \n"); // DEBUG
//
//     // Search the JIT for the __anonymous_expr symbol.
//     auto ExprSymbol = ExitOnErr(TheJIT->lookup("__anonymous_expr"));
//     fmt::print("GenAnonFunction: anon found\n"); // DEBUG
//
//     // Get the symbol's address and cast it to the right type (takes no
//     // arguments, returns a double) so we can call it as a native function.
//     double (*FP)() = (double (*)())(intptr_t)ExprSymbol.getAddress();
//     fprintf(stderr, "Evaluated to %f\n", FP());
//
//     // Delete the anonymous expression module from the JIT.
//     ExitOnErr(RT->remove());
//     fmt::print("GenAnonFunction: generated and called AnonFunction\n"); // DEBUG
//     return true;
// }
//
// bool CodeGenerator::GenExtern() {
//     // auto P = GenPrototype();
//     // if (!P) return false;
//     // fprintf(stderr, "Read extern:\n");
//     // P->print(llvm::errs());
//     // fprintf(stderr, "\n");
//     // FunctionProtos[P->getName()] = std::move(P);
//     return true;
// }
//
// bool CodeGenerator::GenDef() {
//     if (!GenFunction()) return false;
//     fprintf(stderr, "Read function definition\n");
//     ExitOnErr(TheJIT->addModule(llvm::orc::ThreadSafeModule(
//         std::move(TheModule), std::move(TheContext))));
//     InitializeModuleAndFPM();
//     return true;
// }
//
// llvm::Function *CodeGenerator::getFunction(std::string name) {
//     // First, see if the function has already been added to the current module.
//     if (auto *F = TheModule->getFunction(name))
//         return F;
//
//     // If not, check whether we can codegen the declaration from some existing
//     // prototype.
//     auto FI = FunctionProtos.find(name);
//     if (FI != FunctionProtos.end()) {
//         // Token functionIdentifier;
//         // TokenSet::Identifier(functionIdentifier);
//         // functionIdentifier.str = name;
//         // PushStr(functionIdentifier);
//         return FI->second;
//     }
//
//     // If no existing prototype exists, return null.
//     return nullptr;
// }

void CodeGenerator::PrintGeneratedCode() {
    TheModule->print(llvm::errs(), nullptr);
}






void CodeGenerator::ExprAST::ToStdOut(const std::string& prefix, bool isLeft) {
    fmt::print("expr\n");
}

CodeGenerator::NumberExprAST::NumberExprAST(double Val) : Val(Val) {
}

llvm::Value* CodeGenerator::NumberExprAST::codegen() {
    return llvm::ConstantFP::get(*TheContext, llvm::APFloat(Val));
}

CodeGenerator::VariableExprAST::VariableExprAST(const std::string &Name) : Name(Name) {
}

llvm::Value* CodeGenerator::VariableExprAST::codegen() {
    llvm::Value *V = NamedValues[Name];
    if (!V) {
        fmt::print("Unknown variable name {}", Name);
        return nullptr;
    }
    return V;
}

CodeGenerator::OperatorAST::OperatorAST(Token Operator, std::unique_ptr<ExprAST> LHS,
                        std::unique_ptr<ExprAST> RHS)
    : Operator(Operator), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

llvm::Value* CodeGenerator::OperatorAST::codegen() {
    llvm::Value *L = LHS->codegen();
    llvm::Value *R = RHS->codegen();
    if (!L || !R) return nullptr;

    switch (Operator.subtype) {
        case Token::Plus:
            return Builder->CreateFAdd(L, R, "addtmp");
        case Token::Minus:
            return Builder->CreateFSub(L, R, "subtmp");
        case Token::Multiply:
            return Builder->CreateFMul(L, R, "multmp");
        case Token::Divide:
            return Builder->CreateFDiv(L, R, "divtmp");
        default:
            fmt::print("GenOperator called but operator token sybtype unexpected [{}, {}, '{}']\n",
                    Operator.typeToStr(), Operator.subtypeToStr(), Operator.str);
            return nullptr;
    }
}

CodeGenerator::CallExprAST::CallExprAST(const std::string &Callee,
                        std::unique_ptr<std::vector<std::unique_ptr<ExprAST>>> Args)
    : Callee(Callee), Args(std::move(Args)) {
}

llvm::Value* CodeGenerator::CallExprAST::codegen() {
    // Look up the name in the global module table.
    llvm::Function *CalleeF = getFunction(Callee);
    if (!CalleeF) {
        fmt::print("Unknown function referenced {}", Callee);
        return nullptr;
    }

    // If argument mismatch error.
    if (CalleeF->arg_size() != Args->size()) {
        fmt::print("Incorrect amount of arguments passed");
        return nullptr;
    }

    std::vector<llvm::Value*> ArgsV;
    for (unsigned i = 0, e = Args->size(); i != e; ++i) {
        ArgsV.push_back((*Args)[i]->codegen());
        if (!ArgsV.back()) return nullptr;
    }
    return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

CodeGenerator::PrototypeAST::PrototypeAST(const std::string &Name,
        std::unique_ptr<std::vector<std::string>> Args)
    : Name(Name), Args(std::move(Args)) {
    }

llvm::Function* CodeGenerator::PrototypeAST::codegen() {
    // Make the function type:  double(double,double) etc.
    std::vector<llvm::Type*> Doubles(Args->size(), llvm::Type::getDoubleTy(*TheContext));
    llvm::FunctionType *FT =
        llvm::FunctionType::get(llvm::Type::getDoubleTy(*TheContext), Doubles, false);

    llvm::Function *F =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Name, TheModule.get());

    // Set names for all arguments.
    unsigned Idx = 0;
    for (auto &Arg : F->args())
        Arg.setName((*Args)[Idx++]);

    return F;
}

CodeGenerator::FunctionAST::FunctionAST(std::unique_ptr<PrototypeAST> Prototype,
                        std::unique_ptr<ExprAST> Body)
    : Prototype(std::move(Prototype)), Body(std::move(Body)) {}

llvm::Function* CodeGenerator::FunctionAST::codegen() {
    // Transfer ownership of the prototype to the FunctionProtos map, but keep a
    // reference to it for use below.
    auto &P = *Prototype;
    FunctionProtos[Prototype->getName()] = std::move(Prototype);
    llvm::Function *TheFunction = getFunction(P.getName());
    if (!TheFunction)
        return nullptr;

    // Create a new basic block to start insertion into.
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", TheFunction);
    Builder->SetInsertPoint(BB);

    // Record the function arguments in the NamedValues map.
    NamedValues.clear();
    for (auto &Arg : TheFunction->args())
        NamedValues[std::string(Arg.getName())] = &Arg;

    if (llvm::Value *RetVal = Body->codegen()) {
        // Finish off the function.
        Builder->CreateRet(RetVal);

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

llvm::Function *getFunction(std::string name) {
    // First, see if the function has already been added to the current module.
    if (auto *F = TheModule->getFunction(name))
        return F;

    // If not, check whether we can codegen the declaration from some existing
    // prototype.
    auto FI = FunctionProtos.find(name);
    if (FI != FunctionProtos.end())
        return FI->second->codegen();

    // If no existing prototype exists, return null.
    return nullptr;
}
