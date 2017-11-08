#ifndef GRAPHICALOBJECTS_H
#define GRAPHICALOBJECTS_H

#include <QGraphicsItem>
#include <QPainter>

class PositionQuery;


class DisplayRepresentation : public QGraphicsItem
{
public:
    DisplayRepresentation(const PositionQuery* query);

    QRectF boundingRect() const;

    // overriding paint()
    void paint(QPainter * painter,
               const QStyleOptionGraphicsItem * option,
               QWidget * widget);
private:
    const PositionQuery* query;
};

#endif // GRAPHICALOBJECTS_H
