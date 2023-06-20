#include "STD.hpp"

extern "C" DLLEXPORT double graph(double min, double max, double step, double n) {
    constexpr char fileName[] = {"data.csv"};
    std::string functionName =  fmt::format("{}{:.0f}", "buildGraph", n);

    auto Result = CodeGenerator::TheJIT->lookup(functionName);
    if (auto E = Result.takeError()) {
        fmt::print("function 'graph' called but {} function was not found",
           functionName);
        return -1;
    }
    double (*FP)(double) = (double (*)(double))(intptr_t)Result->getAddress();

    auto out = fmt::output_file(fileName);

    try {
        for (double i = min; i < max; i += step) {
            out.print("{},{}\n", i, FP(i));
        }
        out.close();
    } catch (const std::system_error& e) {
        fmt::print("Error {}, meaning {}", e.code(), e.what());
        return -2;
    }

    return 0;
}
