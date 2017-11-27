#include "hardwareabstraction.h"

#include "view/view.h"
#include "model/tactiledisplay.h"
#include "model/actuator.h"

#include <itchy/tactilemousequery.h>
#include <scratchy/mousepositionquery.h>
#include <scratchy/graphicaldisplay.h>
#include <scratchy/signalmanager.h>

#include <unistd.h>
#include <mutex>
#include <QDebug>
#include <iostream>

static std::mutex hwlock;


#define float2q15(x) ((short)(x*32768))
#define q152float(x) ((((short)x)/32768.0f))
#define float2q5(x) ((short)(x*32))


HardwareAbstraction::HardwareAbstraction(View *view, bool perfMon):
    view(view)
{
    hwlock.lock();
    controller = nullptr;
    display = new GraphicalDisplay();
    hwlock.unlock();
    doStats = perfMon;
}

PositionQuery* HardwareAbstraction::connectInputdevice(PositionQuery* fallback)
{
    view->showInfo("Input dev.", "Searching for\nTactileMouse");
    //sleep(1);
    // Try tactileMouse

    PositionQuery* query = nullptr;

    hwlock.lock();
    query = new TactileMouseQuery(true);
    if(!query->initialize())
    {
        hwlock.unlock();
        delete query;
        view->showInfo("Input dev.", "Searching for\nSimpleMouse");
        //sleep(1);
        hwlock.lock();
        query = new MousePositionQuery();
        if(!query->initialize())
        {
            hwlock.unlock();
            delete query;
            view->showInfo("Input dev.", "Using fallback");
            query = fallback;
            sleep(2);
            view->clearInfo();
            return query;
        }
    }

    if(query)
        delete fallback;

    hwlock.unlock();

    view->clearInfo();
    return query;
}

void HardwareAbstraction::initializeSignalBoards()
{
    view->showInfo("Initialization", "Initializing SignalBoards");
    hwlock.lock();
    if(!controller)
        controller = new SignalManager();
    controller->maskDevice(60); // Skip OLED Display
    controller->initializeBoards(); // Default initialization to runlevel 3
    hwlock.unlock();
    view->clearInfo();
}

bool HardwareAbstraction::checkAdressLayout(const TactileDisplay *tactileDisplay)
{
#ifdef FAKEMODE
    return true;
#endif

    hwlock.lock();
    auto devices = controller->scanDevices();
    for(auto device : devices)
        actuatorMapping.insert(device, std::array<FrequencyTable, 4>());


    usleep(100000);
    hwlock.unlock();

    // Check if selected display matches current hardware configuration
    for(const Actuator& actuator : tactileDisplay->actuators())
    {
        // Check if Actuator exists
        if(!actuatorMapping.contains(actuator.PortID))
        {
            return false;
        }
    }
    return true;
}

HardwareAbstraction::DisplayButton HardwareAbstraction::currentButton()
{
    hwlock.lock();
    if(display->isPressed(Button::Up))
    {
        hwlock.unlock();
        usleep(200000);
        return DisplayButton::Up;
    }

    if(display->isPressed(Button::Down))
    {
        hwlock.unlock();
        usleep(200000);
        return DisplayButton::Down;
    }

    if(display->isPressed(Button::Select))
    {
        hwlock.unlock();
        usleep(200000);
        return DisplayButton::Select;
    }

    if(display->isPressed(Button::Back))
    {
        hwlock.unlock();
        sleep(1);
        hwlock.lock();
        if(display->isPressed(Button::Back))
        {
            hwlock.unlock();
            usleep(200000);
            return DisplayButton::Reset;
        }
        hwlock.unlock();
        return DisplayButton::Back;
    }

    hwlock.unlock();
    return DisplayButton::None;
}

void HardwareAbstraction::shutdown()
{
    view->showInfo("Exiting", "Shutting down SignalBoards");
    // Send Shutdown signal to all teensys
    hwlock.lock();
    for(SignalGenerator& g : controller->generators())
    {
        g.shutdown();
    }

    // Clear screen before exiting program
    display->clear();
    hwlock.unlock();
}

void HardwareAbstraction::sendTables(const TactileDisplay* display,
                                     const QVector<FrequencyTable>& data)
{
#ifdef FAKEMODE
    return;
#endif

    hwlock.lock();
    for(int i = 0; i < data.size(); i++)
    {
        const Actuator& actuator = display->actuators()[i];
        actuatorMapping[actuator.PortID][actuator.PinID] = data[i];
    }

    for(SignalGenerator& g : controller->generators())
        g.send(actuatorMapping[g.address()]);

    hwlock.unlock();

    if(doStats)
    {
        if(!statsDone)
        {
            //this->display->detach();
            statsDone = true;
            timer.start();
        }

        if(ticks > 0 && ticks % 1000 == 0)
        {
            //float us = timer.nsecsElapsed()/float(ticks) * 1.0e-9;
            //qDebug() << "s: " << us << "\tHz: " << 1.0/us;
            ticks = 0;
            timer.restart();
        }

        ticks++;
    }

}
