// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "circa/circa.h"

#include <QWidget>
#include <QtOpenGL/QGLWidget>
#include <QTime>

#include "engine/RenderTarget.h"
#include "engine/TextSprite.h"

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget* parent);

    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL();

    virtual void mouseDoubleClickEvent ( QMouseEvent * event );
    virtual void mouseMoveEvent ( QMouseEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseReleaseEvent ( QMouseEvent * event );
    virtual void keyPressEvent ( QKeyEvent * event );
    virtual void keyReleaseEvent ( QKeyEvent * event );

    // Temp for testing
    RenderTarget renderTarget;
    TextSprite* textSprite;

protected:
    // Sends the event to App.onInputEvent, and deallocates the caValue.
    void onInputEvent(caValue* event);

    QTime elapsedTime;

public slots: void animate();
};
