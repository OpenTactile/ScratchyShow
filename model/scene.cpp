#include "scene.h"
#include "model/actuator.h"
#include "model/tactiledisplay.h"

#include <QFile>
#include <QPixmap>
#include <QStringList>
#include "model/model.h"
#include <QDebug>
#include <QRegExp>
#include <QMap>
#include "model/nullmodel.h"

#include <mutex>
#include <algorithm>
#include <QTime>


static std::mutex mutex;

Scene::Scene(TactileDisplay *display, PositionQuery *position):
    display(display), position(position)
{
    resetModel = std::move(std::unique_ptr<Model>(new NullModel));
    resetModel->initialize(QStringList(), position, QRectF());
}

bool Scene::loadScenario(const QString& filename)
{
    std::lock_guard<std::mutex> lock(mutex);
    qsrand(QTime::currentTime().msecsSinceStartOfDay());

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    entities.clear();
    configData.clear();

    QString sceneData;
    while(!file.atEnd())
    {
        QString line = QString(file.readLine()).trimmed();

        if(line.startsWith('#') || line.isEmpty())
            continue;

        sceneData += line + "\n";
    }
    file.close();

    // Extract title
    title = sceneData.section('\n', 0, 0, QString::SectionSkipEmpty);

    // Extract substitution definitions
    QRegExp configExtract("[cC]onfig\\s*\\{\\s*([^}]*)\\s*\\}");
    QRegExp substExtract("[sS]ubstitute\\s*\\{\\s*([^}]*)\\s*\\}");
    QRegExp substSeparate("(\\S*)\\s*=\\s*\\(\\s*(.*)\\s*\\)");

    configExtract.indexIn(sceneData);
    QStringList configs = configExtract.capturedTexts()[1].split('\n', QString::SkipEmptyParts);
    for(const QString& line : configs)
    {
        QStringList splitted = line.split("=");
        if(splitted.length() == 2)
            configData[splitted[0].trimmed()] = splitted[1].trimmed();
        else
            qDebug() << "Warning:\tCould not interpret config line: " << line;
    }

    substExtract.indexIn(sceneData);
    QStringList substitutions = substExtract.capturedTexts()[1].split('\n', QString::SkipEmptyParts);

    QMap<QString,QStringList> substitute;
    for(const QString& line : substitutions)
    {
        substSeparate.indexIn(line);
        QString ident = substSeparate.capturedTexts()[1];
        QStringList values = substSeparate.capturedTexts()[2].split('|');
        for(QString& e : values) e = e.trimmed();
        std::random_shuffle(values.begin(), values.end());
        substitute.insert(ident, values);
    }

    // Extract scene definitions
    QRegExp sceneExtract("[sS]cene\\s*\\{\\s*([^}]*)\\s*\\}");

    sceneExtract.indexIn(sceneData);
    QString scene = sceneExtract.capturedTexts()[1];

    for(const QString& id : substitute.keys())
    {
        QStringList lst = substitute[id];
        for(int i = 0; i < lst.length(); i++)
        {
            // Apply randomized substitutions
            QString mask = QString("%") + id + QString::number(i) + "%";
            QString value = lst[i];
            scene.replace(mask, value);
        }
    }

    // Apply config
    for(const QString& key : configData.keys())
        scene.replace(key, configData[key]);


    QStringList lines = scene.split('\n', QString::SkipEmptyParts);
    int currentLine = 0;
    for(const QString& line: lines)
    {
        entities.push_back(Entity());
        Entity& e = entities.back();
        e.id = entities.size();

        QStringList splitted = line.split('|');

        // Parse position information
        QStringList geometry = splitted[0].split(';');
        for(QString& e : geometry) e = e.trimmed();
        e.area = QRectF(geometry[0].toDouble()*0.01, geometry[1].toDouble()*0.01,
                        geometry[2].toDouble()*0.01, geometry[3].toDouble()*0.01);

        // Load images
        QStringList imageFilenames = splitted[1].split(';');
        e.hiddenGraphic = QPixmap(QString(MAP_DIRECTORY) + imageFilenames[1].trimmed());
        e.visibleGraphic= QPixmap(QString(MAP_DIRECTORY) + imageFilenames[0].trimmed());

        // Select & initialize model
        QStringList modelData = splitted[2].split(';');
        QString modelName = modelData.takeFirst().trimmed();
        if(modelfactories.contains(modelName))
            e.model = modelfactories[modelName](modelData, position, e.area);
        else
            e.model = modelfactories["Null"](modelData, position, e.area);

        e.inactive = (dynamic_cast<NullModel*>(e.model.get()) != nullptr);

        // Parse drag'n'drop information
        QString dnd = splitted[3].trimmed().toLower();
        e.allowDrag = (dnd == "true");

        e.statistics.index = entities.size() - 1;
        currentLine++;
    }

    return true;
}

const TactileDisplay* Scene::getTactileDisplay() const
{
    std::lock_guard<std::mutex> lock(mutex);
    return display;
}

const std::vector<Entity>& Scene::getEntities() const
{
    std::lock_guard<std::mutex> lock(mutex);
    return entities;
}

const QString& Scene::getTitle() const
{
    std::lock_guard<std::mutex> lock(mutex);
    return title;
}

QString Scene::getConfig(const QString& entry) const
{
    if(configData.contains(entry))
        return configData[entry];
    else
        return "";
}

void Scene::registerModel(const QString& name, ModelFactory factory)
{
    modelfactories[name] = factory;
}

void Scene::queryMap(const PositionQuery* position, QVector<FrequencyTable>& out)
{
    std::lock_guard<std::mutex> lock(mutex);

    Entity* currentEntity = nullptr;    
    QVector2D anchor;

    double elapsed = timer.nsecsElapsed()*1e-6;

    for(const Actuator& ac : display->actuators())
    {
        QVector2D pos = ac.position;

        if(enabled)
        {
            for(Entity& e : entities)
            {
                if(e.inactive)
                    continue;

                const QRectF& r = e.area;
                if(r.contains(pos.x(), pos.y()))
                {
                    // TODO: Overlapping maps won't work this way (anyway, different models at the same time don't make sense at all)
                    currentEntity = &e;
                    anchor = QVector2D(r.topLeft());
                    // Update statistics
                    auto& st = e.statistics;
                    double vel = ac.velocity.length() * 1000.0;
                    if(vel < 500) // omit unrealistic values
                    {
                        st.accVelocity += vel;
                        st.cntVel++;
                        st.averageVelocity = st.accVelocity/st.cntVel;
                        st.hoverTime += elapsed / display->actuators().size(); // milliseconds, only rough estimation!
                    }
                }
            }
        }
    }
    timer.restart();

    if(currentEntity)
        currentEntity->model->apply(display, out);
    else
        resetModel->apply(display, out);
}

void Scene::updatePosition(const Entity& e, QVector2D position)
{
    entities[e.id].area.moveTo(position.toPointF());
}

void Scene::enable()
{
    enabled = true;
}

void Scene::disable()
{
    enabled = false;
}
