
#include <qapplication.h>
#include <qlabel.h>

#include "Scripts.h"
#include "Window.h"

int main(int argc, char *argv[])
{
    scripts_initialize();

    QApplication app(argc, argv);

    Window window;
    window.show();
    return app.exec();
}

