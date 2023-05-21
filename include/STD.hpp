#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#include <fmt/core.h>
#include <fmt/os.h>

extern "C" DLLEXPORT double graph(double min, double max, double step, double n);
