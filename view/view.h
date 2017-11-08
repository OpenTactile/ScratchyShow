#ifndef VIEW_H
#define VIEW_H

#include <QObject>

class PositionQuery;
class TactileDisplay;
class Scene;

/* interface */ class View : public QObject
{
    Q_OBJECT

public:
    virtual void initialize() = 0;
    virtual void shutdown() = 0;

    virtual void setInputDevice(PositionQuery* query) = 0;
    virtual void setTactileDisplay(const TactileDisplay* display) = 0;

    virtual void showInfo(const QString& title, const QString& body) = 0;
    virtual void clearInfo() = 0;

    virtual void addMapEntry(const QString& title, const QString& metadata) = 0;
    virtual void showSelection(const QString& current, const QString& next) = 0;

signals:
    void onReset();
    void onExit();
    void onNext();
    void onPrevious();
    void onSelect(const QString& title);

public slots:
    virtual void updatePosition() = 0;
    virtual void changeScene(const Scene* scene) = 0;
};

#endif // VIEW_H
