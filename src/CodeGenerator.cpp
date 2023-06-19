#include "CodeGenerator.hpp"
#include <memory>

std::unique_ptr<llvm::LLVMContext> CodeGenerator::TheContext;
std::unique_ptr<llvm::IRBuilder<>> CodeGenerator::Builder;
std::unique_ptr<llvm::Module> CodeGenerator::TheModule;
std::map<std::string, llvm::Value*> CodeGenerator::NamedValues;
std::unique_ptr<llvm::legacy::FunctionPassManager> CodeGenerator::TheFPM;
std::unique_ptr<llvm::orc::MyCustomJIT> CodeGenerator::TheJIT;
llvm::ExitOnError CodeGenerator::ExitOnErr;

std::map<llvm::StringRef, std::unique_ptr<CodeGenerator::PrototypeAST>> CodeGenerator::FunctionProtos;

CodeGenerator::CodeGenerator() {
    TheJIT = ExitOnErr(llvm::orc::MyCustomJIT::Create());
    InitializeModuleAndFPM();
    paramsV = std::make_unique<std::vector<std::string>>();
    argsV = std::make_unique<std::vector<std::unique_ptr<ExprAST>>>();
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
    termsStack.push(t);
}

void CodeGenerator::PushOperator(Token t) {
    operatorsStack.push(t);
}

void CodeGenerator::PushParam(Token t) {
    paramsV->push_back(t.str);
}

void CodeGenerator::PushArg(Token t) {
    if (exprASTStack.empty()) {
        fmt::print("PushArg called but exprASTStack is empty!\n");
        return;
    }
    std::unique_ptr<ExprAST> E = std::move(exprASTStack.top());
    exprASTStack.pop();
    argsV->push_back(std::move(E));
}

bool CodeGenerator::GenVariable() {
    if (termsStack.empty()) {
        fmt::print("GenVariable called but stack is empty!\n");
        return false;
    }
    std::string name  = termsStack.top().str;
    termsStack.pop();
    auto result = std::make_unique<VariableExprAST>(name);
    exprASTStack.push(std::move(result));
    return true;
}

bool CodeGenerator::GenNumber() {
    if (termsStack.empty()) {
        fmt::print("GenNumber called but stack is empty!\n");
        return false;
    }
    double number = std::stod(termsStack.top().str);
    termsStack.pop();
    auto result = std::make_unique<NumberExprAST>(number);
    exprASTStack.push(std::move(result));
    return true;
}

bool CodeGenerator::GenNegation() {
    fmt::print("GenNegation not implemented!\n");
    return false;
}

bool CodeGenerator::GenOperator() {
    if (exprASTStack.size() < 2) {
        fmt::print("GenOperator called but exprASTStack size < 2!\n");
        return false;
    }
    fmt::print("GenOperator trying std::move(exprASTStack.top())\n"); // DEBUG
    std::unique_ptr<ExprAST> R = std::move(exprASTStack.top());
    exprASTStack.pop();
    std::unique_ptr<ExprAST> L = std::move(exprASTStack.top());
    exprASTStack.pop();
    if (!L || !R) return false;
    fmt::print("GenOperator successfull std::move(exprASTStack.top())\n"); // DEBUG

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

    auto result = std::make_unique<OperatorAST>(op, std::move(L), std::move(R));
    exprASTStack.push(std::move(result));
    return true;
}

bool CodeGenerator::GenCall() {
    fmt::print("Generating call\n");
    if (termsStack.empty()) {
        fmt::print("GenCall called but termsStack empty!\n");
        return false;
    }

    std::string idName = termsStack.top().str;
    termsStack.pop();

    auto result = std::make_unique<CallExprAST>(idName, std::move(argsV));

    argsV = std::make_unique<std::vector<std::unique_ptr<ExprAST>>>();

    exprASTStack.push(std::move(result));
    return true;
}

std::unique_ptr<CodeGenerator::PrototypeAST> CodeGenerator::GenPrototype() {
    if (termsStack.empty()) {
        fmt::print("GenPrototype called but termsStack empty!\n");
        return nullptr;
    }
    std::string name = termsStack.top().str;
    termsStack.pop();

    auto result = std::make_unique<PrototypeAST>(name, std::move(paramsV));

    paramsV = std::make_unique<std::vector<std::string>>();

    return std::move(result);
}

std::unique_ptr<CodeGenerator::FunctionAST>  CodeGenerator::GenFunction(
        std::unique_ptr<PrototypeAST> ProtoAST) {
    if (exprASTStack.empty()) {
        fmt::print("GenFunction called but exprASTStack is empty!\n");
        return nullptr;
    }
    std::unique_ptr<ExprAST> E = std::move(exprASTStack.top());
    exprASTStack.pop();
    if (!E) return nullptr;
    E->ToStdOut("", false);

    return std::make_unique<FunctionAST>(std::move(ProtoAST), std::move(E));
}

bool CodeGenerator::GenAnonFunction() {
    // Evaluate a top-level expression into an anonymous function.
    Token functionIdentifier;
    TokenSet::Identifier(functionIdentifier);
    functionIdentifier.str = "__anonymous_expr";
    PushStr(functionIdentifier);
    fmt::print("GenAnonFunction: generating prototype\n"); // DEBUG
    if (auto ProtoAST = GenPrototype()) {
        fmt::print("GenAnonFunction: generated prototype\n"); // DEBUG
        // ProtoAST->ToStdOut("", false); // DEBUG
        fmt::print("GenAnonFunction: generating function\n"); // DEBUG
        if (auto FnAST = GenFunction(std::move(ProtoAST))) {
            if (auto *FnIR = FnAST->codegen()) {
                fmt::print("GenAnonFunction: generated function\n"); // DEBUG
                fmt::print("function name: {}\n", FnIR->getName());

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
        }
    }
    return false;
}

bool CodeGenerator::GenExtern() {
    if (auto ProtoAST = GenPrototype()) {
        if (auto *FnIR = ProtoAST->codegen()) {
            fprintf(stderr, "Read extern:\n");
            // ProtoAST->ToStdOut("", false);
            FnIR->print(llvm::errs());
            fprintf(stderr, "\n");
            FunctionProtos[ProtoAST->getName()] = std::move(ProtoAST);
            return true;
        }
    }
    return false;
}

bool CodeGenerator::GenDef() {
    if (auto ProtoAST = GenPrototype()) {
        if (auto FnAST = GenFunction(std::move(ProtoAST))) {
            if (auto *FnIR = FnAST->codegen()) {
                fprintf(stderr, "Read function definition:\n");
                // FnAST->ToStdOut("", false);
                FnIR->print(llvm::errs());
                fprintf(stderr, "\n");
                ExitOnErr(TheJIT->addModule(llvm::orc::ThreadSafeModule(
                    std::move(TheModule), std::move(TheContext))));
                InitializeModuleAndFPM();
                return true;
            }
        }
    }
    return false;
}

void CodeGenerator::PrintGeneratedCode() {
    TheModule->print(llvm::errs(), nullptr);
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
    if (!Args) {
        fmt::print("ARGS NULL!!!!\n"); //DEBUG
        return nullptr;
    }
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

const std::string& CodeGenerator::PrototypeAST::getName() const {
    return Name;
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

llvm::Function *CodeGenerator::getFunction(std::string name) {
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




void CodeGenerator::ExprAST::ToStdOut(const std::string& prefix, bool isLeft) {
    fmt::print("expr\n");
}

void CodeGenerator::NumberExprAST::ToStdOut(const std::string& prefix, bool isLeft) {
    fmt::print("{}", prefix);
    fmt::print("{}", isLeft ? "├──" : "└──");
    fmt::print("{}\n", this->Val);
}

void CodeGenerator::VariableExprAST::ToStdOut(const std::string& prefix, bool isLeft) {
    fmt::print("{}", prefix);
    fmt::print("{}", isLeft ? "├──" : "└──");
    fmt::print("{}\n", this->Name);
}

void CodeGenerator::OperatorAST::ToStdOut(const std::string& prefix, bool isLeft) {
    fmt::print("{}", prefix);
    fmt::print("{}", isLeft ? "├──" : "└──");

    fmt::print("{}\n", this->Operator.str);

    LHS->ToStdOut(prefix + (isLeft ? "│   " : "    "), true);
    RHS->ToStdOut(prefix + (isLeft ? "│   " : "    "), false);
}

void CodeGenerator::CallExprAST::ToStdOut(const std::string& prefix, bool isLeft) {
    std::string tmpPrefix = prefix;
    fmt::print("{}", tmpPrefix);
    fmt::print("{}", isLeft ? "├──" : "└──");

    fmt::print("{} (function call)\n", this->Callee);

    int currentAmount = 0;
    for(auto it = Args->begin(); it != Args->end(); it++, currentAmount++) {
        (*it)->ToStdOut(tmpPrefix + (isLeft ? "│   " : "    "),
                (currentAmount != Args->size() - 1) ? true : false);
    }
}

void CodeGenerator::PrototypeAST::ToStdOut(const std::string& prefix, bool isLeft) {
    fmt::print("{}", prefix);
    fmt::print("{}", isLeft ? "├──" : "└──");

    fmt::print("{}\n", this->Name);

    int currentAmount = 0;
    for(auto it = Args->begin(); it != Args->end(); it++, currentAmount++) {
        fmt::print("{}{}{}{}\n", prefix, isLeft ? "│   " : "    ",
            isLeft ? "├──" : "└──", *it);
    }
}

void CodeGenerator::FunctionAST::ToStdOut(const std::string& prefix, bool isLeft) {
    fmt::print("{}", prefix);
    fmt::print("{}", isLeft ? "├──" : "└──");

    fmt::print("{} (function declaration)\n", this->Prototype->getName());

    fmt::print("{}{}├──(arguments)\n", prefix, isLeft ? "│   " : "    ");
    Prototype->ToStdOut(prefix + "    │   " , false);


    fmt::print("{}{}└──(body)\n", prefix, isLeft ? "│   " : "    ");
    Body->ToStdOut(prefix + "        ", false);
}
