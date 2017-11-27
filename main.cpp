#include <iostream>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <pthread.h>

#include <QFileInfo>
#include <QApplication>
#include <QElapsedTimer>
#include <QDebug>
#include <QDir>
#include <QTimer>
#include <QTime>

#include <scratchy/constantvelocityquery.h>

#include "model/tactiledisplay.h"
#include "model/actuator.h"
#include "model/scene.h"

#include "model/model.h"
#include "model/frequencymodel.h"
#include "model/nullmodel.h"

#include "view/hdmiscreen.h"
#include "util/hardwareabstraction.h"

#include "util/filelogger.h"

#define VERSION_STRING "v0.5"

//#define REALTIME_PRIORITY

const int targetHz = 1000;

int main(int argc, char* argv[])
{

#ifdef REALTIME_PRIORITY
    // Realtime priority (Dangerous, may freeze system!)
    __pid_t pid = getpid();
    struct sched_param param;
    param.__sched_priority = 10;
    qDebug() << sched_setscheduler(pid, SCHED_FIFO, &param);
#endif

    QApplication app(argc, argv);

    Log::setWorkingDirectory(QString(MAP_DIRECTORY) + "../logs/");

    View* view = new HDMIScreen();
    view->initialize();

    HardwareAbstraction HAL(view, true);


    view->showInfo("ScratchyShow", QString("Version: ") + VERSION_STRING);

    // Initialize mouse
    PositionQuery* currentPosition = HAL.connectInputdevice(new ConstantVelocityQuery(0.05, 0.305));

    // TODO: Parse from file
    // Load tactile display definition
    TactileDisplay tactileDisplay;

    tactileDisplay << Actuator(QVector2D(-4, 0.0),      QVector2D(0.71, 12.5),  0, 1, 2)
                   << Actuator(QVector2D(-2, 0.0),      QVector2D(0.71, 12.5),  0, 1, 0)
                   << Actuator(QVector2D( 0.0, 0.0),    QVector2D(0.71, 12.5),  0, 2, 0)
                   << Actuator(QVector2D(2, 0.0),       QVector2D(0.71, 12.5),  0, 1, 1)
                   << Actuator(QVector2D(4, 0.0),       QVector2D(0.71, 12.5),  0, 1, 3);

    HAL.initializeSignalBoards();

    if(!HAL.checkAdressLayout(&tactileDisplay))
    {
        view->showInfo("Error", "Physical configuration does not match software model");
        sleep(4);
        HAL.shutdown();
        exit(0);
    }


    // Search for tactile scene files (*.tsc) within the map directory
    QVector<QPair<QString, QString>> tactileScenes;

    QDir tactileDir(MAP_DIRECTORY);
    QStringList filters; filters << "*.tsc";
    for(const QString& file : tactileDir.entryList(filters))
    {
        QFile fl(QString(MAP_DIRECTORY) + file);
        if(!fl.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;

        QString title;
        while(!fl.atEnd())
        {
            QString line = QString(fl.readLine()).trimmed();
            if(line.startsWith('#'))
                continue;

            title = line;
            break;
        }
        fl.close();

        tactileScenes << qMakePair(QString(MAP_DIRECTORY) + file, title);
        view->addMapEntry(title, QString(MAP_DIRECTORY) + file);
        qDebug() << file << "->" <<  title;
    }

    if(tactileScenes.size() == 0)
    {
        qDebug() << "No tactile scenes found. Aborting.";
        exit(-1);
    }

    int currentMap = 0;
    int nextMap = 0;
    view->showSelection(tactileScenes[currentMap].second, tactileScenes[nextMap].second);

    Scene scene(&tactileDisplay, currentPosition);

    scene.registerModel("Frequency", Factory<FrequencyModel>());
    scene.registerModel("Null", Factory<NullModel>());
    // TODO: Add python model

    scene.loadScenario(tactileScenes[currentMap].first);
    view->changeScene(&scene);
    view->setTactileDisplay(&tactileDisplay);
    view->setInputDevice(currentPosition);

    if(HDMIScreen* vw = dynamic_cast<HDMIScreen*>(view))
    {
        QObject::connect(vw, &HDMIScreen::mapRepositioned, [&](const Entity& e, QVector2D pos, bool silent){
            scene.updatePosition(e, pos);
            if(!silent)
            {
                vw->updateScene();
                scene.enable();                
                Log::timestamp();
                Log::push(QString("User repositioned map ID") + QString::number(e.id) + " - New position: (" + QString::number(pos.x()) + ", " + QString::number(pos.x()) + ")");
            }
        });

        QObject::connect(vw, &HDMIScreen::mapSelected, [&](){
            scene.disable();
        });

        QObject::connect(vw, &HDMIScreen::onSelect, [&](const QString& title){
            int index = 0;
            for(int n = 0; n < tactileScenes.size(); n++)
            {
                if(tactileScenes[n].second == title)
                {
                    index = n;
                }
            }

            // Scene statistics
            Log::push("---- Map usage statistics ----");
            for(const auto& e : scene.getEntities())
            {
                Log::push(QString::number(e.statistics.index) + ": Average velocity = "
                          + QString::number(e.statistics.averageVelocity, 'f', 4)
                          + ", Hover Time = "+ QString::number(e.statistics.hoverTime*0.001, 'f', 4));
            }

            currentMap = index; nextMap = index;
            view->showSelection(tactileScenes[currentMap].second, tactileScenes[nextMap].second);
            scene.loadScenario(tactileScenes[currentMap].first);
            view->changeScene(&scene);

            Log::timestamp();
            Log::push(QString("Scene changed to '") + title + "'");
        });
    }

    std::atomic<bool> running(true);
    std::thread modelThread([&]()
    {
        QVector<FrequencyTable> query;
        query.resize(tactileDisplay.actuators().size());

        QElapsedTimer perfcounter;
        int cnt = 0; qint64 worst = 0;
        perfcounter.start();

        while(running.load())
        {            
            cnt++; perfcounter.restart();
            currentPosition->update();

            // Move virtual display to current coordinates and update velocities:
            tactileDisplay.transform(currentPosition);
            scene.queryMap(currentPosition, query);
            HAL.sendTables(&tactileDisplay, query);
            // ----

            qint64 elapsed = perfcounter.nsecsElapsed();
            const int wait = 1e6/targetHz - elapsed/1000;
            if(wait > 0)
                usleep(wait);

            worst = qMax(worst, perfcounter.nsecsElapsed());
            if(cnt % 1000 == 0)
            {
                qDebug() << "FPS:" << 1/(worst*1.0e-09);
                worst = 0;                
            }
        }

        qDebug() << "Model thread ended normally";
    });

    // Main Loop
    view->clearInfo();

    // Adjust scheduling
    sched_param sch_params;
    sch_params.sched_priority = 99;
    pthread_setschedparam(modelThread.native_handle(), SCHED_RR, &sch_params);

    auto ButtonHandler = [&]()
    {
        switch(HAL.currentButton())
        {
        case HardwareAbstraction::DisplayButton::Up:
            nextMap--;
            if(nextMap < 0)
                nextMap = tactileScenes.size() - 1;

            view->showSelection(tactileScenes[currentMap].second, tactileScenes[nextMap].second);
            break;

        case HardwareAbstraction::DisplayButton::Down:
            nextMap++;
            if(nextMap >= tactileScenes.size())
                nextMap = 0;

            view->showSelection(tactileScenes[currentMap].second, tactileScenes[nextMap].second);
            break;

        case HardwareAbstraction::DisplayButton::Select:
            currentMap = nextMap;
            view->showSelection(tactileScenes[currentMap].second, tactileScenes[nextMap].second);
            scene.loadScenario(tactileScenes[currentMap].first);
            view->changeScene(&scene);
            break;

        case HardwareAbstraction::DisplayButton::Back:
            HAL.initializeSignalBoards(); // Reset Signalboards
            break;

        case HardwareAbstraction::DisplayButton::Reset: // Shutdown
            running = false;
            sleep(1);
            HAL.shutdown();
            sleep(1);
            view->shutdown();
            app.exit();
            break;

        case HardwareAbstraction::DisplayButton::None:
            break;
        }
    };

    QTimer guiClock;
    guiClock.setInterval(5);

    QObject::connect(&guiClock, &QTimer::timeout, ButtonHandler);
    QObject::connect(&guiClock, SIGNAL(timeout()), view, SLOT(updatePosition()));

    guiClock.start();
    return app.exec();
}

