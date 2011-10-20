// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include <QTimer>
#include <QWidget>
#include <QGLWidget>

#include "MouseState.h"
#include "ScriptEnv.h"

class GLWidget;

class Viewport : public QGLWidget
{
    Q_OBJECT

public:
    Viewport(QWidget* parent = 0);
    ~Viewport();

    circa::Branch* loadScript(const char* filename);
    void keyPressEvent(QKeyEvent *e);

public slots:
    void tick();
    void saveScript();

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int,int);
    //void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    ScriptEnv scriptEnv;
    QTimer updateTimer;

public:
    MouseState mouseState;
};
