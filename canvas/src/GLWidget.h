
#pragma once

#include "circa/circa.h"

#include <QWidget>
#include <QtOpenGL/QGLWidget>

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget* parent);

    virtual void paintEvent(QPaintEvent *event);

private:
    caValue* viewObj;

public slots: void animate();
};
