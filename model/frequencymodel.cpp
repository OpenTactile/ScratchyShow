#include "frequencymodel.h"

#include <QColor>
#include <QStringList>
#include <QString>

#include "model/tactiledisplay.h"
#include "model/actuator.h"
#include <scratchy/positionquery.h>

FrequencyModel::FrequencyModel()
{

}

FrequencyModel::~FrequencyModel()
{

}

void FrequencyModel::initialize(const QStringList& options, PositionQuery* position, QRectF bounds)
{
    frequency = options[0].toFloat();
    amplitude = options[1].toFloat();

    if(options[2].trimmed() == "true")
        scaleFrequency = true;

    if(options[3].trimmed() == "true")
        scaleAmplitude = true;

    colorHue = options[4].toFloat();
    this->position = position;
}

void FrequencyModel::apply(const TactileDisplay* display, QVector<FrequencyTable> &tables)
{
    static int cnt = 0;
    cnt++;

    const float referenceVelocityInv = 1.0/0.025;

    // TODO: Create & Send tables manually
    /*
    for(int i = 0; i < data.size(); i++)
    {
        const Actuator& actuator = display->actuators()[i];
        float velocity = actuator.velocity.length();

        float factor = velocity * referenceVelocityInv;

        for(int j = 0; j < FREQUENCYCOUNT; j++)
        {
            data[i].absoluteOffset = 0.0;
            if(j == 0)
            {
                data[i].frequency[j] = frequency;
                data[i].amplitude[j] = amplitude;
            }
            else
            {
                data[i].frequency[j] = 0.0;
                data[i].amplitude[j] = 0.0;
                continue;
            }

            if(scaleFrequency) data[i].frequency[j] *= factor;
            if(scaleAmplitude) data[i].amplitude[j] *= factor;

            if(data[i].frequency[j] > 1000.0)
            {
                data[i].frequency[j] = 1000.0;
            }

            if(data[i].amplitude[j] > 1.0)
            {
                data[i].amplitude[j] = 1.0;
            }
        }
    }*/

    if(cnt % 100 == 0)
    {
        QColor cl = QColor::fromHsvF(colorHue, 1.0, 1.0);
        position->feedback(cl.red(),cl.green(),cl.blue());
    }
}
