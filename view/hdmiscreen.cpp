#include "view/hdmiscreen.h"

#include "ui_xlessgui.h"
#include "ui_infobox.h"

#include <cmath>

#include <QApplication>
#include <QLabel>
#include <QDesktopWidget>
#include <QComboBox>
#include <QLineEdit>

#include <scratchy/positionquery.h>
#include "model/tactiledisplay.h"
#include "model/scene.h"
#include "model/actuator.h"

#include <QDebug>
#include <QVector2D>

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsColorizeEffect>
#include <QPushButton>
#include <util/filelogger.h>

// TODO: Make this configurable
const float screenWidth = 0.52f;
const float screenHeight = 0.25f;

HDMIScreen::HDMIScreen():
    uiMain(new Ui::MainWindow),
    uiInfo(new Ui::InfoBox)
{
    fullscreenWindow = new QMainWindow();
    uiMain->setupUi(fullscreenWindow);
    fullscreenWindow->setWindowFlags(Qt::FramelessWindowHint | Qt::Window);

    infoWindow = new QWidget();
    uiInfo->setupUi(infoWindow);
    infoWindow->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | Qt::WindowStaysOnTopHint);

    QRect screen = QApplication::desktop()->screenGeometry();
    int x = (screen.width() - infoWindow->width()) / 2;
    int y = (screen.height() - infoWindow->height()) / 2;
    screenResolution = QVector2D(screen.width(), screen.height());
    infoWindow->move(x, y);
}

HDMIScreen::~HDMIScreen()
{

}

void HDMIScreen::initialize()
{
    fullscreenWindow->showFullScreen();

    scene = new QGraphicsScene();
    sceneRect = QRectF(0.0, 0.0, screenWidth, screenHeight);
    uiMain->canvas->setScene(scene);
    //uiMain->canvas->setSceneRect(sceneRect);
    uiMain->canvas->setSceneRect(QRectF(-screenWidth, -screenHeight, screenWidth*3, screenHeight*3));
    uiMain->canvas->centerOn(0.0,0.0);

    connect(uiMain->comboNext, SIGNAL(activated(QString)), this, SIGNAL(onSelect(QString)));
    connect(this, SIGNAL(mousePressed(QVector2D)), this, SLOT(selectMap(QVector2D)));
    connect(this, SIGNAL(mouseMoved(QVector2D)), this, SLOT(moveSelected(QVector2D)));
    connect(this, SIGNAL(mouseReleased()), this, SLOT(deselectMap()));

    connect(uiMain->buttonShow, SIGNAL(toggled(bool)), this, SLOT(toggleVisibility(bool)));

    connect(uiMain->buttonRecord, &QPushButton::toggled, [&](bool pressed){
        if(pressed)
        {
            Log::startSession();
            uiMain->labelID->setText(QString("ID: <b>") + Log::sessionString() + "</b>");
        }
        else
        {
            emit(onSelect(uiMain->comboNext->currentText()));
            Log::stopSession();
            uiMain->labelID->setText(QString("ID: <b>") + Log::sessionString() + "</b>");
        }
    });
}

void HDMIScreen::shutdown()
{
    infoWindow->close();
    fullscreenWindow->close();
}

void HDMIScreen::setInputDevice(PositionQuery* query)
{
    pQuery = query;
}

void HDMIScreen::setTactileDisplay(const TactileDisplay* display)
{
    QBrush displayBrush(QColor(255,128,0));
    QPen displayPen(QBrush(QColor(128,64,0)), 0.0);

    if(displayGraphic)
        delete displayGraphic;

    QGraphicsItemGroup* displayGroup = new QGraphicsItemGroup();

    for(const Actuator& actuator : display->actuators())
        displayGroup->addToGroup(scene->addRect(actuator.position.x()- actuator.bounds.x()*0.5,
                                               actuator.position.y()- actuator.bounds.y()*0.5,
                                               actuator.bounds.x(),
                                               actuator.bounds.y(),
                                               displayPen, displayBrush));

    scene->addItem(displayGroup);
    displayGraphic = displayGroup;
    displayGraphic->setZValue(99999);

    displayColor = new QGraphicsColorizeEffect();
    displayColor->setColor(Qt::red);
    displayColor->setStrength(1.0);
    displayGraphic->setGraphicsEffect(displayColor);
    displayColor->setEnabled(false);    
}

void HDMIScreen::showInfo(const QString& title, const QString& body)
{
    uiInfo->titleText->setText(title);
    uiInfo->bodyText->setText(body);    
    infoWindow->showNormal();
    infoWindow->update();
    qApp->processEvents();
}

void HDMIScreen::clearInfo()
{
    infoWindow->hide();
    qApp->processEvents();
}

void HDMIScreen::addMapEntry(const QString& title, const QString& metadata)
{
    uiMain->comboNext->addItem(title, metadata);
}

void HDMIScreen::showSelection(const QString& current, const QString& next)
{
    uiMain->labelCurrent->setText(current);
    uiMain->comboNext->setEditText(next);
    qApp->processEvents();
}

void HDMIScreen::updatePosition()
{
    static float avgVel = 0.0;

    if(!displayGraphic)
        return;

    QVector2D devicePosition = pQuery->position();
    avgVel = avgVel * 0.95f;

    // Do not redraw if nothing has changed or if invalid coordinates appeared
    if(devicePosition == QVector2D(0.0, 0.0))
        return;

    if(qIsNaN(devicePosition.x()) || qIsNaN(devicePosition.y()))
        return;

    if(devicePosition == lastPosition && lastButton == pQuery->buttonPressed())
        return;

    QVector2D position = devicePosition;

    displayGraphic->setPos(position.x(), position.y());
    displayGraphic->setRotation(pQuery->orientation()/M_PI * 180.0);
    avgVel += pQuery->velocity().length() * 0.05;
    uiMain->velocityLabel->setText(QString("Velocity: ") + QString::number(int(round(avgVel*1000))) + " mm/s");
    lastPosition = devicePosition;

    bool button = pQuery->buttonPressed();

    if(button != lastButton)
    {
        if(button)
            emit mousePressed(pQuery->position());
        else
            emit mouseReleased();
    }

    if(button)
    {
        displayColor->setEnabled(true);
        emit mouseMoved(pQuery->position());
    }
    else
        displayColor->setEnabled(false);

    lastButton = button;
}

void HDMIScreen::changeScene(const Scene* scene)
{
    for(ViewEntity& e : entities)
        e.free();
    entities.clear();

    const int pixpercm = 100;

    for(const Entity& e : scene->getEntities())
    {
        ViewEntity ve;
        ve.entity = &e;

        QRectF area = e.area;
        const QPixmap* imageVisible = &e.visibleGraphic;
        const QPixmap* imageHidden= &e.hiddenGraphic;

        int cropWidth = int(area.width()*100 * pixpercm);
        int cropHeight = int(area.height()*100 * pixpercm);

        QPixmap croppedVisible = imageVisible->copy(0,0,cropWidth,cropHeight).transformed(QTransform().scale(1, -1));
        QGraphicsPixmapItem* itemVisible = this->scene->addPixmap(croppedVisible);
        itemVisible->setTransformationMode(Qt::SmoothTransformation);
        itemVisible->setScale(1.0/(100 * pixpercm));
        itemVisible->setPos(area.x(), area.y());

        QPixmap croppedHidden = imageHidden->copy(0,0,cropWidth,cropHeight).transformed(QTransform().scale(1, -1));
        QGraphicsPixmapItem* itemHidden = this->scene->addPixmap(croppedHidden);
        itemHidden->setTransformationMode(Qt::SmoothTransformation);
        itemHidden->setScale(1.0/(100 * pixpercm));        
        itemHidden->setPos(area.x(), area.y());
        itemHidden->setVisible(false);
    }

    // Ensure correct scaling
    QTransform fitMat(1,0,0,0,1,0,0,0,1);
    fitMat.scale(uiMain->canvas->width() / sceneRect.width(), uiMain->canvas->height() / sceneRect.height());
    fitMat.scale(1,-1);
    float trX = scene->getConfig("Translate[0]").toFloat();
    float trY = scene->getConfig("Translate[1]").toFloat();

    uiMain->canvas->setTransform(fitMat);    
    fullscreenWindow->setStyleSheet(QString("background: '")+ scene->getConfig("BackgroundColor") + "'");
    uiMain->canvas->setStyleSheet("background: transparent");
    uiMain->canvas->centerOn(trX,trY);

    bool disableButton = (scene->getConfig("DisableSecretButton").toLower() == "true");
    uiMain->buttonShow->setEnabled(!disableButton);

    bool hideToolbar = (scene->getConfig("HideToolbar").toLower() == "true");
    uiMain->toolbar->setVisible(!hideToolbar);

}

void HDMIScreen::updateScene()
{
    for(ViewEntity& e : entities)
        e.update();
}

void HDMIScreen::selectMap(QVector2D pos)
{
    QVector2D screenvec = pos;
    for(ViewEntity& e : entities)
    {
        if(!e.entity->allowDrag)
            continue;

        if(e.visibleItem->contains(e.visibleItem->mapFromScene(screenvec.toPointF())))
        {
            selectedEntity = &e;
            selectionStart = pos;
            selectionOriginal = QVector2D(e.visibleItem->pos());
            emit mapSelected();
            return;
        }

        if(e.hiddenItem->contains(e.hiddenItem->mapFromScene(screenvec.toPointF())))
        {
            selectedEntity = &e;
            selectionStart = pos;
            selectionOriginal = QVector2D(e.hiddenItem->pos());
            emit mapSelected();
            return;
        }
    }

    selectedEntity = nullptr;
}

void HDMIScreen::deselectMap()
{
    if(selectedEntity)
    {
        emit mapRepositioned(*selectedEntity->entity, QVector2D(selectedEntity->visibleItem->pos()), false);
    }
    selectedEntity = nullptr;
}

void HDMIScreen::moveSelected(QVector2D pos)
{
    if(selectedEntity)
    {
        QVector2D diff = pos - selectionStart;
        QPointF newPos = (selectionOriginal + diff).toPointF();
        selectedEntity->visibleItem->setPos(newPos);
        selectedEntity->hiddenItem->setPos(newPos);
    }
}

void HDMIScreen::toggleVisibility(bool visible)
{
    for(ViewEntity& e : entities)
    {
        e.visibleItem->setVisible(!visible);
        e.hiddenItem->setVisible(visible);
    }
}
