
#include <qapplication.h>
#include <qlabel.h>
#include <QDir>
#include <QFontDatabase>
#include <QMessageBox>

#include "Scripts.h"
#include "Window.h"

bool fix_current_directory()
{
    // First step, we need to find the "ca" directory. If we're running from a Mac
    // bundle, then we'll need to walk up a few directories.

    while (true) {

        if (QDir("ca").exists())
            return true;

        // chdir to parent
        QDir current = QDir::current();
        current.cdUp();

        // If we reached the top, then fatal.
        if (current == QDir::current()) {
            QMessageBox msg;
            msg.setText("Fatal: Couldn't find the 'ca' directory");
            msg.exec();
            return false;
        }

        QDir::setCurrent(current.path());
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (!fix_current_directory())
        return -1;

    // Temp
    QFontDatabase::addApplicationFont("assets/AlteHaasGroteskBold.ttf");
    QFontDatabase::addApplicationFont("assets/AlteHaasGroteskRegular.ttf");

    scripts_initialize();

    Window window;
    window.show();
    return app.exec();
}

