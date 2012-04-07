
#pragma once

#include <QObject>

#include "circa/circa.h"

class ScriptCenter : public QObject
{
    Q_OBJECT

public:
    caBranch* mainBranch;
    caStack* stack;

    ScriptCenter();
    void init();

    void call(const char* functionName, caValue* inputs);
};
