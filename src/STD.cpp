#include "STD.hpp"
#include "AST.hpp"

extern "C" DLLEXPORT double graph(double min, double max, double step, double n) {
    //std::string functionName =  std::format("{}{}", "func", n);
    std::string functionNum = std::to_string(n);
    std::string functionName = "func" + std::to_string(n);

    auto Result = TheJIT->lookup(functionName);
    if (auto E = Result.takeError()) {
        Logger::LogError(
           // std::format("function 'graph' called but {} function was not found",
           // functionName)
           "function 'graph' called but " + functionName + " function was not found");
        return -1;
    }
    double (*FP)(double) =
        (double (*)(double))(intptr_t)Result->getAddress();
    if (!FP) {
    }

    std::ofstream outputFile("data.txt");

    if (outputFile.is_open()) {
        for (double i = min; i < max; i += step) {
            outputFile << i << "," << FP(i) << std::endl;
        }
        outputFile.close();
    } else {
    
    std::cout << "Unable to open the file." << std::endl;
    }
    return 0;
}
