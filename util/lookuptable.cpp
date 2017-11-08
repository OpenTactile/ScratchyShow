#include "lookuptable.h"

#include <QFile>
#include <QStringList>
#include <QDebug>

LookupTable::LookupTable()
{
    // Initialize data
    for(unsigned int n = 0; n < values.size(); n++)
        values[n] = 0.0;
}

LookupTable::LookupTable(const QString &filename, int column)
{
    // Initialize data
    for(unsigned int n = 0; n < values.size(); n++)
        values[n] = 0.0;

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical() << "Could not read file" << filename << "- skipping";
    }
    else
    {
        int index = 0;
        while(!file.atEnd() && index <= 1000)
        {
            QString line(file.readLine());
            if(line.trimmed().startsWith('#'))
                continue;

            QStringList spl = line.split(';');
            values[index++] = spl[column].toDouble();
        }

        file.close();
    }
}

