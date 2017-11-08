#ifndef SCENE_H
#define SCENE_H

#include <QVector>
#include <QString>
#include <QRectF>
#include <QMap>
#include <QVector2D>
#include <QElapsedTimer>
#include "model/model.h"
#include "model/entity.h"
#include <vector>

class TactileMap;
class TactilePoint;
class TactileDisplay;

class PositionQuery;
class QPixmap;


class Scene
{
public:
    Scene(TactileDisplay* display, PositionQuery* position);

    bool loadScenario(const QString& filename);

    const TactileDisplay* getTactileDisplay() const;
    const std::vector<Entity> &getEntities() const;

    const QString& getTitle() const;
    QString getConfig(const QString& entry) const;

    void registerModel(const QString& name, ModelFactory factory);
    void queryMap(const PositionQuery* position, QVector<FrequencyTable> &out);
    void updatePosition(const Entity &e, QVector2D position);

    void enable();
    void disable();

private:
    TactileDisplay* display = nullptr;
    PositionQuery* position = nullptr;

    QMap<QString, ModelFactory> modelfactories;
    std::unique_ptr<Model> resetModel;

    std::vector<Entity> entities;
    QMap<QString, QString> configData;

    QString title;
    QElapsedTimer timer;
    bool enabled = true;
};

#endif // SCENE_H
