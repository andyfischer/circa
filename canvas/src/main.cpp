
#include <qapplication.h>
#include <qlabel.h>

#include "ScriptCenter.h"
#include "Window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ScriptCenter scriptCenter;
    scriptCenter.init();

    Window window;
    window.show();
    return app.exec();
}

