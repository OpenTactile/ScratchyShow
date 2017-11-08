#ifndef VIEWENTITY_H
#define VIEWENTITY_H

#include <model/entity.h>
#include <QGraphicsItem>

struct ViewEntity
{
    QGraphicsItem* visibleItem;
    QGraphicsItem* hiddenItem;
    const Entity* entity;
    bool visible = false;

    void update()
    {
        QRectF area = entity->area;
        QPointF newPos(area.x(), area.y());
        visibleItem->setPos(newPos);
        hiddenItem->setPos(newPos);
    }

    void free()
    {
        delete visibleItem;
        delete hiddenItem;
    }
};


#endif // VIEWENTITY_H
