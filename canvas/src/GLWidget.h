
#pragma once

#include <QWidget>
#include <QtOpenGL/QGLWidget>

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget* parent);

    virtual void paintEvent(QPaintEvent *event);

public slots: void animate();
};
