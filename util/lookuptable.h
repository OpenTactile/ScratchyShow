#ifndef LOOKUPTABLE_H
#define LOOKUPTABLE_H

#include <array>
#include <QString>

class LookupTable
{
public:
    LookupTable();
    LookupTable(const QString& filename, int column = 0);

    inline float operator [](int index) const
    {
        if(index < 0) return values[0];
        if(static_cast<unsigned int>(index) >= values.size()) return *values.cend();
        return values[index];
    }

    inline float operator [](float index) const
    {
        if(index < 0.0) return values[0];
        if(index >= values.size()) return *values.cend();
        int idx = int(index);
        float frac = index - idx;
        return values[idx] * (1.0 - frac) + values[idx+1] * frac;
    }

private:
    std::array<float, 1001> values;
};

#endif // LOOKUPTABLE_H
