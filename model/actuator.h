#ifndef ACTUATOR_H
#define ACTUATOR_H

#include <QVector2D>

struct Actuator
{
    Actuator(QVector2D pos,
             QVector2D size = QVector2D(0,0),
             unsigned char map = 0,
             unsigned char port = 0,
             unsigned char pin = 0) :
        position(pos * 0.001), bounds(size * 0.001), MapID(map),
        PortID(port), PinID(pin) {}

    Actuator(){}

    QVector2D position = QVector2D(0.0, 0.0);
    QVector2D velocity = QVector2D(0.0, 0.0);
    QVector2D bounds = QVector2D(0.0,0.0);

    float radius = 0.5 * 0.001;

    unsigned char MapID  = 0;
    unsigned char PortID = 0;
    unsigned char PinID  = 0;

    // Additional parameters here...
};

#endif // ACTUATOR_H
