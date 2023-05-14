#include "STD.hpp"
#include "AST.hpp"

extern "C" DLLEXPORT double graph(double min, double max, double n) {
    std::string functionName =  std::format("{}{}", "func", n);

    auto Result = TheJIT->lookup(functionName);
    if (auto E = Result.takeError()) {
        Logger::LogError(
            std::format("function 'graph' called but {} function was not found",
            functionName));
        return -1;
    }
    double (*FP)(double) =
        (double (*)(double))(intptr_t)Result->getAddress();
    if (!FP) {
    }

    for (int i = min; i < max; ++i) {
        std::cout << i << "\t" << FP(i) << std::endl;
    }
    return 0;
}
