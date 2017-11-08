#ifndef HDMISCREEN_H
#define HDMISCREEN_H

#include "view/view.h"

#include <QMainWindow>
#include <QVector2D>
#include <QRectF>

#include "view/viewentity.h"

namespace Ui {
class MainWindow;
class InfoBox;
}

class QLabel;
class QGraphicsScene;
class QGraphicsItem;
class QGraphicsColorizeEffect;


class HDMIScreen : public View
{
    Q_OBJECT

public:
    HDMIScreen();
    virtual ~HDMIScreen();

    virtual void initialize();
    virtual void shutdown();

    virtual void setInputDevice(PositionQuery* query);
    virtual void setTactileDisplay(const TactileDisplay* display);

    virtual void showInfo(const QString& title, const QString& body);
    virtual void clearInfo();

    virtual void addMapEntry(const QString& title, const QString& metadata);
    virtual void showSelection(const QString& current, const QString& next);

signals:
    void onReset();
    void onExit();
    void onNext();
    void onPrevious();
    void onSelect(const QString& title);

    void mousePressed(QVector2D pos);
    void mouseReleased();
    void mouseMoved(QVector2D pos);

    void mapSelected();
    void mapRepositioned(const Entity& e, QVector2D position, bool silent);

public slots:
    virtual void updatePosition();
    virtual void changeScene(const Scene* scene);
    virtual void updateScene();

    virtual void selectMap(QVector2D pos);
    virtual void deselectMap();
    virtual void moveSelected(QVector2D pos);

    virtual void toggleVisibility(bool visible);

private:
    Ui::MainWindow* uiMain;
    Ui::InfoBox* uiInfo;
    QMainWindow* fullscreenWindow;
    QWidget* infoWindow;

    QVector2D screenResolution;
    QRectF sceneRect;
    QGraphicsScene* scene;
    QVector<ViewEntity> entities;

    ViewEntity* selectedEntity = nullptr;
    QVector2D selectionStart;
    QVector2D selectionOriginal;

    QGraphicsItem* displayGraphic = nullptr;
    QGraphicsColorizeEffect* displayColor = nullptr;
    PositionQuery* pQuery = nullptr;

    QVector2D lastPosition;
    bool lastButton;
};

#endif // HDMISCREEN_H
