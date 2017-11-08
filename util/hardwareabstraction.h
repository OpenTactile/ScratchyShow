#ifndef HARDWAREABSTRACTION_H
#define HARDWAREABSTRACTION_H

class PositionQuery;
class View;
class GraphicalDisplay;
class SignalManager;
class TactileDisplay;
class TactilePoint;

#include <scratchy/signalgenerator.h>
#include <QHash>
#include <QVarLengthArray>
#include <QElapsedTimer>
#include <scratchy/signalmanager.h>
#include <array>

class HardwareAbstraction
{
public:
    HardwareAbstraction(View* view, bool perfMon = false);

    PositionQuery* connectInputdevice(PositionQuery* fallback);

    void initializeSignalBoards();
    bool checkAdressLayout(const TactileDisplay* tactileDisplay);
    void shutdown();

    void sendTables(const TactileDisplay* display, const QVector<FrequencyTable> &data);

    enum class DisplayButton
    {
        Up, Down, Select, Back, Reset, None
    };

    DisplayButton currentButton();
    SignalManager* controller;

private:
    QHash<u_int8_t, std::array<FrequencyTable, 4>> actuatorMapping;
    View* view;
    GraphicalDisplay* display;

    bool doStats = false;
    bool statsDone = false;
    QElapsedTimer timer;
    qint64 ticks = 0;
};

#endif // HARDWAREABSTRACTION_H
