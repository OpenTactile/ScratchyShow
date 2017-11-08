#ifndef FREQUENCYMODEL_H
#define FREQUENCYMODEL_H

#include "model/model.h"

class FrequencyModel : public Model
{
public:
    FrequencyModel();
    virtual ~FrequencyModel();
    virtual void initialize(const QStringList& options,PositionQuery* position, QRectF bounds);
    virtual void apply(const TactileDisplay* display, QVector<FrequencyTable>& tables);

private:
    float frequency = 200.0;
    float amplitude = 1.0;
    bool scaleFrequency = false;
    bool scaleAmplitude = false;
    float colorHue = 0.0;
    PositionQuery* position = nullptr;
};

#endif // FREQUENCYMODEL_H
