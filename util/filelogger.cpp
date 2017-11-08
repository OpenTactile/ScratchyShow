#include "filelogger.h"

#include <QFile>
#include <QDir>
#include <QDateTime>


static QString workingDir;
static QFile file;
static Log::SessionID sessionID = 0;

void Log::setWorkingDirectory(const QString& dir)
{
    workingDir = dir;
}

Log::SessionID Log::startSession()
{
    Log::SessionID id = qrand()%9999;
    QString newFilename = QString("%1").arg(id, 4, 10, QChar('0')) + ".log";

    // Check if file is not already existing
    QDir dir(workingDir);
    QStringList entries = dir.entryList();
    while(entries.contains(newFilename) || id == 0) // Better not to have too many sessions...
    {
        id = qrand()%9999;
        newFilename = QString("%1").arg(id, 4, 10, QChar('0')) + ".log";
    }

    file.close();
    file.setFileName(dir.absoluteFilePath(newFilename));
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        exit(-1);
    }

    sessionID = id;
    timestamp();
    push("Started session No." + sessionString());
    return sessionID;
}

void Log::stopSession()
{
    timestamp();
    push("Ended session No." + sessionString());
    sessionID = 0;
    file.flush();
    file.close();
}

Log::SessionID Log::currentSession()
{
    return sessionID;
}

QString Log::sessionString()
{
    return QString("%1").arg(sessionID, 4, 10, QChar('0'));
}

void Log::timestamp()
{
    if(sessionID == 0) return;
    file.write(QDateTime::currentDateTime().toString("'['dd.MM.yyyy' - 'hh:mm:ss']\t'").toLocal8Bit());
}

void Log::push(const QString& message)
{
    if(sessionID == 0) return;
    file.write((message + "\n").toLocal8Bit());
    file.flush();
}
