#ifndef MODEL_H
#define MODEL_H

#include <QVector>
#include <QStringList>
#include <functional>
#include <QRectF>
#include <memory>
#include <scratchy/signalgenerator.h>

class TactileDisplay;
class PositionQuery;
class Actuator;

/* interface */ class Model
{
public:
    virtual ~Model(){}
    virtual void initialize(const QStringList& options, PositionQuery* position, QRectF bounds) = 0;    
    virtual void apply(const TactileDisplay* display, QVector<FrequencyTable>& tables) = 0;
};


using ModelFactory = std::function<std::unique_ptr<Model>(const QStringList&, PositionQuery*, QRectF)>;

template<class M>
ModelFactory Factory()
{
    return [](const QStringList& options, PositionQuery* position, QRectF bounds)
    {
        std::unique_ptr<Model> model(new M);
        model->initialize(options, position, bounds);
        return model;
    };
}

#endif // MODEL_H
