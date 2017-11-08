#include "tactiledisplay.h"

#include "model/actuator.h"
#include <scratchy/positionquery.h>

#include <cmath>

TactileDisplay::TactileDisplay()
{

}

TactileDisplay::~TactileDisplay()
{

}

void TactileDisplay::transform(const PositionQuery* position)
{
    query = position;
    QVector2D translation = position->position();
    //float alpha = position->orientation();
    //QMatrix2x2 rotMat(cos(a), -sin(a), sin(a), cos(a));

    for(int i = 0; i < actuatorsReference.size(); i++)
    {
        //actuatorsCurrent[i].position = rotMat * actuatorsReference[i].position;
        actuatorsCurrent[i].position = actuatorsReference[i].position + translation;
        actuatorsCurrent[i].velocity = position->velocity();
    }
}

const PositionQuery* TactileDisplay::position() const
{
    return query;
}

const QVector<Actuator>& TactileDisplay::actuators() const
{
    return actuatorsCurrent;
}

QRectF TactileDisplay::boundingBox() const
{
    float xMin = INFINITY; float yMin = INFINITY;
    float xMax = -INFINITY; float yMax = -INFINITY;

    for(const Actuator& a : actuatorsReference)
    {
        xMin = qMin(xMin, a.position.x() - a.radius);
        yMin = qMin(yMin, a.position.y() - a.radius);
        xMax = qMax(xMax, a.position.x() + a.radius);
        yMax = qMax(yMax, a.position.y() + a.radius);
    }

    return QRectF(QPointF(xMin, yMin), QPointF(xMax, yMax));
}

TactileDisplay& TactileDisplay::operator<<(const Actuator &rhs)
{
    actuatorsCurrent << rhs;
    actuatorsReference << rhs;
    return *this;
}
