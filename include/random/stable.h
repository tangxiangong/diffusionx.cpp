#ifndef STABLE_H
#define STABLE_H
#include <vector>
#include <format>
#include "../error.hpp"


using std::vector;
using std::format;

Result<double> rand_stable(double alpha, double beta = 0.0, double sigma = 1.0, double mu = 0.0);

Result<vector<double> > rand_stable(size_t n, double alpha, double beta = 0.0,
                                    double sigma = 1.0, double mu = 0.0);


#endif //STABLE_H
