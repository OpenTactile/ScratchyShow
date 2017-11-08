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

    // TODO: Reset voltages to zero

    if(cnt % 100 == 0)
    {
        position->feedback(0,0,0);
    }
}
