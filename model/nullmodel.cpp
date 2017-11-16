#include "nullmodel.h"

#include "model/tactiledisplay.h"
#include <scratchy/positionquery.h>

NullModel::NullModel()
{

}

NullModel::~NullModel()
{

}

void NullModel::initialize(const QStringList& options, PositionQuery* position, QRectF bounds)
{
    this->position = position;
}

void NullModel::apply(const TactileDisplay* display, QVector<FrequencyTable> &tables)
{
    static int cnt = 0;
    cnt++;

    for(FrequencyTable& tab : tables)
    {
        for(fixed_q5& frequency : tab.frequency)
            frequency = 0.0;
        for(fixed_q15& amplitude : tab.amplitude)
            amplitude = 0.0;
    }

    if(cnt % 100 == 0)
    {
        position->feedback(0,0,0);
    }
}
