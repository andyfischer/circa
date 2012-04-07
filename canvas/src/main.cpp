
#include <qapplication.h>
#include <qlabel.h>

#include "ScriptCenter.h"
#include "Window.h"

ScriptCenter* g_scriptCenter = NULL;

ScriptCenter* GetScriptCenter()
{
    return g_scriptCenter;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    g_scriptCenter = new ScriptCenter();
    g_scriptCenter->init();

    Window window;
    window.show();
    return app.exec();
}

