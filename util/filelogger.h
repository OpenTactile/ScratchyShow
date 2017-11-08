#ifndef FILELOGGER_H
#define FILELOGGER_H

#include <QString>

namespace Log
{
    using SessionID = unsigned int;
    extern void setWorkingDirectory(const QString& dir);

    extern SessionID startSession();
    extern void stopSession();
    extern SessionID currentSession();
    extern QString sessionString();

    extern void timestamp();
    extern void push(const QString& message);
}

#endif // FILELOGGER_H
