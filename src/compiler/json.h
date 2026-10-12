#ifndef REGEXLAB_JSON_H
#define REGEXLAB_JSON_H
#include "compiler.h"
#include "../analysis/benchmark.h"
#include "../analysis/optimizer.h"
#include <string>
std::string jsonQuote(const std::string& text);
std::string compilationJSON(const std::string& regex, const std::string& input, const Compilation& c);
std::string comparisonJSON(const Comparison& c);
std::string optimizationJSON(const OptimizationResult& result);
std::string equivalenceJSON(const EquivalenceResult& result);
#endif
