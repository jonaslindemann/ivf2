#include <ivfvis/ticks.h>

#include <algorithm>
#include <cmath>
#include <cstdio>

using namespace ivfvis;

double ivfvis::niceNumber(double x, bool round)
{
    if (!(x > 0.0) || !std::isfinite(x))
        return 0.0;
    const double exponent = std::floor(std::log10(x));
    const double power = std::pow(10.0, exponent);
    const double f = x / power;
    double nice;
    if (round)
        nice = f < 1.5 ? 1.0 : (f < 3.0 ? 2.0 : (f < 7.0 ? 5.0 : 10.0));
    else
        nice = f <= 1.0 ? 1.0 : (f <= 2.0 ? 2.0 : (f <= 5.0 ? 5.0 : 10.0));
    return nice * power;
}

double ivfvis::niceTickStep(double min, double max, int count)
{
    if (!(max > min))
        return 0.0;
    count = std::max(count, 2);
    const double range = niceNumber(max - min, false);
    return niceNumber(range / static_cast<double>(count - 1), true);
}

std::vector<double> ivfvis::niceTicks(double min, double max, int count)
{
    std::vector<double> ticks;
    if (min > max || !std::isfinite(min) || !std::isfinite(max))
        return ticks;
    if (min == max)
        return {min};
    const double step = niceTickStep(min, max, count);
    if (step <= 0.0)
        return {min, max};
    const double eps = step * 1e-6;
    const double first = std::ceil((min - eps) / step) * step;
    for (int i = 0;; ++i)
    {
        double v = first + static_cast<double>(i) * step;
        if (v > max + eps)
            break;
        if (std::abs(v) < eps)
            v = 0.0;
        ticks.push_back(v);
    }
    return ticks;
}

std::vector<double> ivfvis::logTicks(double min, double max)
{
    if (!(min > 0.0) || !(max > min))
        return niceTicks(min, max);
    const int e0 = static_cast<int>(std::ceil(std::log10(min) - 1e-9));
    const int e1 = static_cast<int>(std::floor(std::log10(max) + 1e-9));
    std::vector<double> ticks;
    for (int e = e0; e <= e1; ++e)
        ticks.push_back(std::pow(10.0, e));
    if (ticks.size() >= 2)
        return ticks;

    ticks.clear();
    const int lo = static_cast<int>(std::floor(std::log10(min))) - 1;
    for (int e = lo; e <= e1 + 1; ++e)
        for (double m : {1.0, 2.0, 5.0})
        {
            const double v = m * std::pow(10.0, e);
            if (v >= min * (1.0 - 1e-9) && v <= max * (1.0 + 1e-9))
                ticks.push_back(v);
        }
    if (ticks.size() >= 2)
        return ticks;
    return niceTicks(min, max);
}

std::string ivfvis::formatTick(double value, double step)
{
    step = std::abs(step);
    if (step > 0.0 && std::abs(value) < step * 1e-6)
        return "0";
    if (value == 0.0)
        return "0";

    char buffer[64];
    const double magnitude = std::max(std::abs(value), step);
    if (magnitude >= 1e6 || (step > 0.0 && step < 1e-4))
    {
        // Mantissa digits: enough to tell ticks apart
        int digits = 0;
        if (step > 0.0)
            digits = std::clamp(static_cast<int>(std::floor(std::log10(std::abs(value)))) -
                                    static_cast<int>(std::floor(std::log10(step))),
                                0, 6);
        std::snprintf(buffer, sizeof(buffer), "%.*e", digits, value);
        return buffer;
    }

    // Smallest number of decimals that shows the step exactly
    int decimals = 0;
    if (step > 0.0)
        while (decimals < 6)
        {
            const double scaled = step * std::pow(10.0, decimals);
            if (std::abs(scaled - std::round(scaled)) <= 1e-6 * std::max(1.0, scaled))
                break;
            ++decimals;
        }
    std::snprintf(buffer, sizeof(buffer), "%.*f", decimals, value);
    return buffer;
}
