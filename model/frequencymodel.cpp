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
    Q_UNUSED(bounds)

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

    const float referenceVelocityInv = 1.0f/referenceVelocity;

    for(int i = 0; i < tables.size(); i++)
    {
        const Actuator& actuator = display->actuators()[i];
        float velocity = actuator.velocity.length();

        float factor = velocity * referenceVelocityInv;

        FrequencyTable& table = tables[i];

        for(unsigned int j = 0; j < table.frequency.size(); j++)
        {            
            if(j == 0)
            {
                table.frequency[j] = (scaleFrequency)?
                            frequency * factor :
                            frequency ;
                table.amplitude[j] = (scaleAmplitude)?
                            amplitude * factor :
                            amplitude;

                table.frequency[j] = std::min(table.frequency[j],
                                              fixed_q5(1000.0f));
                table.amplitude[j] = std::min(table.amplitude[j],
                                              fixed_q15(1.0f));
            }
            else
            {
                table.frequency[j] = 0.0;
                table.amplitude[j] = 0.0;
                continue;
            }
        }
    }

    if(cnt % 100 == 0)
    {
        QColor cl = QColor::fromHsvF(colorHue, 1.0, 1.0);
        position->feedback(cl.red(),cl.green(),cl.blue());
    }
}
