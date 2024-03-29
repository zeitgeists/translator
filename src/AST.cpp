#ifdef DO_NOT_BILD
#include "AST.hpp"

void ExprAST::ToStdOut(const std::string& prefix, bool isLeft) {
    std::cout << "expr\n";
}

NumberExprAST::NumberExprAST(double Val) : Val(Val) {
}

llvm::Value* NumberExprAST::codegen() {
    return llvm::ConstantFP::get(*TheContext, llvm::APFloat(Val));
}

VariableExprAST::VariableExprAST(const std::string &Name) : Name(Name) {
}

llvm::Value* VariableExprAST::codegen() {
    llvm::Value *V = NamedValues[Name];
    if (!V) return Logger::LogErrorV("Unknown variable name " + Name);
    return V;
}

OperatorAST::OperatorAST(std::string Operator, std::unique_ptr<ExprAST> LHS,
                        std::unique_ptr<ExprAST> RHS)
    : Operator(Operator), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

llvm::Value* OperatorAST::codegen() {
    llvm::Value *L = LHS->codegen();
    llvm::Value *R = RHS->codegen();
    if (!L || !R) return nullptr;

    if (Operator == "+") {
        return Builder->CreateFAdd(L, R, "addtmp");
    } else if (Operator == "-") {
        return Builder->CreateFSub(L, R, "subtmp");
    } else if (Operator == "*") {
        return Builder->CreateFMul(L, R, "multmp");
    } else if (Operator == "/") {
        return Builder->CreateFDiv(L, R, "divtmp");
    } else {
        return Logger::LogErrorV("Invalid binary operator " + Operator);
    }
}

CallExprAST::CallExprAST(const std::string &Callee,
                        std::unique_ptr<std::vector<std::unique_ptr<ExprAST>>> Args)
    : Callee(Callee), Args(std::move(Args)) {
}

llvm::Value* CallExprAST::codegen() {
    // Look up the name in the global module table.
    llvm::Function *CalleeF = getFunction(Callee);
    if (!CalleeF) return Logger::LogErrorV("Unknown function referenced " + Callee);

    // If argument mismatch error.
    if (CalleeF->arg_size() != Args->size())
        return Logger::LogErrorV("Incorrect amount of arguments passed");

    std::vector<llvm::Value*> ArgsV;
    for (unsigned i = 0, e = Args->size(); i != e; ++i) {
        ArgsV.push_back((*Args)[i]->codegen());
        if (!ArgsV.back()) return nullptr;
    }
    return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

PrototypeAST::PrototypeAST(const std::string &Name,
        std::unique_ptr<std::vector<std::string>> Args)
    : Name(Name), Args(std::move(Args)) {
    }

llvm::Function* PrototypeAST::codegen() {
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

FunctionAST::FunctionAST(std::unique_ptr<PrototypeAST> Prototype,
                        std::unique_ptr<ExprAST> Body)
    : Prototype(std::move(Prototype)), Body(std::move(Body)) {}

llvm::Function* FunctionAST::codegen() {
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

void AST::InitializeModuleAndFPM() {
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

void AST::PrintGeneratedCode() {
    TheModule->print(llvm::errs(), nullptr);
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
#endif
