#include "JIT.hpp"

llvm::orc::MyCustomJIT::MyCustomJIT(std::unique_ptr<ExecutionSession> es,
                  JITTargetMachineBuilder jtmb, DataLayout dl)
      : ES(std::move(es)), DL(std::move(dl)), Mangle(*this->ES, this->DL),
        ObjectLayer(*this->ES,
                    []() { return std::make_unique<SectionMemoryManager>(); }),
        CompileLayer(*this->ES, ObjectLayer,
                     std::make_unique<ConcurrentIRCompiler>(std::move(jtmb))),
        MainJD(this->ES->createBareJITDylib("<main>")) {
    MainJD.addGenerator(
        cantFail(DynamicLibrarySearchGenerator::GetForCurrentProcess(
            DL.getGlobalPrefix())));
    if (jtmb.getTargetTriple().isOSBinFormatCOFF()) {
      ObjectLayer.setOverrideObjectFlagsWithResponsibilityFlags(true);
      ObjectLayer.setAutoClaimResponsibilityForObjectSymbols(true);
    }
}

llvm::orc::MyCustomJIT::~MyCustomJIT() {
    if (auto Err = ES->endSession())
        ES->reportError(std::move(Err));
}

llvm::Expected<std::unique_ptr<llvm::orc::MyCustomJIT>>
llvm::orc::MyCustomJIT::Create() {
    auto EPC = SelfExecutorProcessControl::Create();
    if (!EPC) return EPC.takeError();

    auto ES = std::make_unique<ExecutionSession>(std::move(*EPC));

    JITTargetMachineBuilder JTMB(
        ES->getExecutorProcessControl().getTargetTriple());

    auto DL = JTMB.getDefaultDataLayoutForTarget();
    if (!DL) return DL.takeError();

    return std::make_unique<MyCustomJIT>(
        std::move(ES), std::move(JTMB), std::move(*DL));
}

const llvm::DataLayout &llvm::orc::MyCustomJIT::getDataLayout() const {
    return DL;
}

llvm::orc::JITDylib & llvm::orc::MyCustomJIT::getMainJITDylib() {
    return MainJD;
}

llvm::Error llvm::orc::MyCustomJIT::addModule(ThreadSafeModule TSM,
        ResourceTrackerSP RT) {
    if (!RT)
        RT = MainJD.getDefaultResourceTracker();
    return CompileLayer.add(RT, std::move(TSM));
}

llvm::Expected<llvm::JITEvaluatedSymbol> llvm::orc::MyCustomJIT::lookup(
        StringRef Name) {
    return ES->lookup({&MainJD}, Mangle(Name.str()));
}
