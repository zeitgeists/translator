#pragma once

#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/ExecutorProcessControl.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LLVMContext.h"
#include <memory>

namespace llvm {
namespace orc {

class MyCustomJIT {
public:
    MyCustomJIT(std::unique_ptr<ExecutionSession> es,
                  JITTargetMachineBuilder jtmb, DataLayout dl);
    ~MyCustomJIT();
    static Expected<std::unique_ptr<MyCustomJIT>> Create();
    const llvm::DataLayout &getDataLayout() const;
    JITDylib &getMainJITDylib();
    Error addModule(ThreadSafeModule TSM, ResourceTrackerSP RT = nullptr);
    Expected<JITEvaluatedSymbol> lookup(StringRef Name);
private:
    std::unique_ptr<ExecutionSession> ES;

    DataLayout DL;
    MangleAndInterner Mangle;

    RTDyldObjectLinkingLayer ObjectLayer;
    IRCompileLayer CompileLayer;

    JITDylib &MainJD;
};

} // end namespace orc
} // end namespace llvm
