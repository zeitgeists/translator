#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#include <iostream>

extern "C" DLLEXPORT double graph(double min, double max, double n);
