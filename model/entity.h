#ifndef ENTITY_H
#define ENTITY_H

#include <QPixmap>
#include <memory>
class Model;


struct MapStatistics
{
    int index = 0;
    double averageVelocity = 0.0;
    double hoverTime = 0.0;

    double accVelocity = 0.0;
    unsigned long cntVel = 0;
};

struct Entity
{
    QPixmap visibleGraphic;
    QPixmap hiddenGraphic;

    bool visible;
    bool allowDrag;
    bool inactive;
    QRectF area;
    std::unique_ptr<Model> model;
    int id;

    MapStatistics statistics;
};

#endif // ENTITY_H
