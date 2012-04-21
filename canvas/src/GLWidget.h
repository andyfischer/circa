
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

    virtual void mouseDoubleClickEvent ( QMouseEvent * event );
    virtual void mouseMoveEvent ( QMouseEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseReleaseEvent ( QMouseEvent * event );
    virtual void keyPressEvent ( QKeyEvent * event );
    virtual void keyReleaseEvent ( QKeyEvent * event );

protected:
    // Sends the event to App.onInputEvent, and deallocates the caValue.
    void onInputEvent(caValue* event);

    caValue* viewObj;
    caValue* onPaintEvent;

public slots: void animate();
};
