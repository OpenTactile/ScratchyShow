#ifndef TACTILEDISPLAY_H
#define TACTILEDISPLAY_H

#include <QVector>
#include <QRectF>

class Actuator;
class PositionQuery;

class TactileDisplay
{
public:
    TactileDisplay();
    ~TactileDisplay();

    void transform(const PositionQuery* position);
    const QVector<Actuator> &actuators() const;
    QRectF boundingBox() const;
    const PositionQuery* position() const;

    TactileDisplay& operator<<(const Actuator &rhs);

private:
    QVector<Actuator> actuatorsReference;
    QVector<Actuator> actuatorsCurrent;
    const PositionQuery* query;
};

#endif // TACTILEDISPLAY_H
