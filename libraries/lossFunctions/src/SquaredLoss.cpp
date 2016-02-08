// SquaredLoss.cpp

#include "SquaredLoss.h"

#include <cmath>

namespace lossFunctions
{
    double SquaredLoss::Evaluate(double prediction, double label) const
    {
        double residual = prediction - label;
        return 0.5 * residual * residual;
    }

    double SquaredLoss::GetDerivative(double prediction, double label) const
    {
        double residual = prediction - label;
        return residual;
    }
}