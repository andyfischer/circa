
#include <QtGui>

#include "GLWidget.h"

#include "Window.h"

Window::Window()
{
    GLWidget *openGL = new GLWidget(this);

    QGridLayout *layout = new QGridLayout;
    layout->setMargin(0);
    layout->addWidget(openGL, 0, 0);
    setLayout(layout);

    setWindowTitle(tr("circaCanvas"));
}
