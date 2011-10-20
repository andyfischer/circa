#pragma once

#include <QTimer>

#include "scriptenv.h"

const int updateInterval = 16;

class BackgroundScript : public QObject
{
    Q_OBJECT

public:
    ScriptEnv scriptEnv;

    BackgroundScript()
    {
        init();
    }
    BackgroundScript(circa::Branch* branch)
      : scriptEnv(branch)
    {
        init();
    }
    void init()
    {
        QObject::connect(&updateTimer, SIGNAL(timeout()), this, SLOT(tick()));
    }

    virtual ~BackgroundScript() {}

    void start()
    {
        updateTimer.start(updateInterval);
    }

public slots:
    void tick()
    {
        scriptEnv.tick();
    }

protected:
    QTimer updateTimer;
};
