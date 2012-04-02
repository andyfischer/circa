
#pragma once

#include <QWidget>
#include <QGLWidget>

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget* parent);

    void animate();
    virtual void paintEvent(QPaintEvent *event);
};
