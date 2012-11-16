// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <QtGui>
#include <QtCore>

#include "GLWidget.h"
#include "Scripts.h"
#include "Common.h"
#include "FontBitmap.h"
#include "ResourceManager.h"

GLWidget::GLWidget(QWidget *parent)
     : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    setFixedSize(800, 800);
    setAutoFillBackground(false);
    setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(animate()));
    timer->start(16);

    elapsedTime.start();
}

void GLWidget::animate()
{
    updateGL();
}

void GLWidget::initializeGL()
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0,0.0,0.0,0.0);

    check_gl_error();

    ResourceManager resourceManager;

    renderTarget.setup(&resourceManager);

    check_gl_error();
}

void GLWidget::resizeGL(int w, int h)
{
    renderTarget.setViewportSize(w, h);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    scripts_pre_message_send();

    // Send a timeUpdate message
    circa::Value msg;
    circa_set_list(&msg, 2);
    circa_set_string(circa_index(&msg, 0), "timeUpdate");
    circa_set_float(circa_index(&msg, 1), elapsedTime.elapsed() / 1000.0f);

    circa_actor_run_message(g_world, "Main", &msg);

    // Send a paintGL message
    circa_set_list(&msg, 2);

    circa_set_string(circa_index(&msg, 0), "paintGL");
    caType* renderTargetType = circa_find_type(g_world, "EngineBindings:RenderTarget");
    assert(renderTargetType != NULL);
    circa_set_typed_pointer(circa_index(&msg, 1), renderTargetType, &renderTarget);

    circa_actor_run_message(g_world, "Main", &msg);

    scripts_post_message_send();

    // Execute GL commands
    renderTarget.render();
}

void GLWidget::mouseDoubleClickEvent ( QMouseEvent * qevent )
{
    circa::Value event;
    circa_set_list(&event, 3);
    circa_set_name(circa_index(&event, 0), circa_to_name("MouseDoubleClick"));
    circa_set_vec2(circa_index(&event, 1), qevent->x(), qevent->y());
    circa_set_int(circa_index(&event, 2), 0);
    onInputEvent(&event);
}
void GLWidget::mouseMoveEvent ( QMouseEvent * qevent )
{
    circa::Value event;
    circa_set_list(&event, 3);
    circa_set_name(circa_index(&event, 0), circa_to_name("MouseMove"));
    circa_set_vec2(circa_index(&event, 1), qevent->x(), qevent->y());
    circa_set_int(circa_index(&event, 2), 0);
    onInputEvent(&event);
}
void GLWidget::mousePressEvent ( QMouseEvent * qevent )
{
    circa::Value event;
    circa_set_list(&event, 3);

    if (qevent->button() == Qt::LeftButton)
        circa_set_name(circa_index(&event, 0), circa_to_name("MouseDown"));
    else if (qevent->button() == Qt::RightButton)
        circa_set_name(circa_index(&event, 0), circa_to_name("RightMouseDown"));
    else
        return;

    circa_set_vec2(circa_index(&event, 1), qevent->x(), qevent->y());
    circa_set_int(circa_index(&event, 2), 0);
    onInputEvent(&event);
}
void GLWidget::mouseReleaseEvent ( QMouseEvent * qevent )
{
    circa::Value event;
    circa_set_list(&event, 3);

    if (qevent->button() == Qt::LeftButton)
        circa_set_name(circa_index(&event, 0), circa_to_name("MouseUp"));
    else if (qevent->button() == Qt::RightButton)
        circa_set_name(circa_index(&event, 0), circa_to_name("RightMouseUp"));
    else
        return;

    circa_set_vec2(circa_index(&event, 1), qevent->x(), qevent->y());
    circa_set_int(circa_index(&event, 2), 0);
    onInputEvent(&event);
}
void GLWidget::keyPressEvent ( QKeyEvent * qevent )
{
    circa::Value event;
    circa_set_list(&event, 3);
    circa_set_name(circa_index(&event, 0), circa_to_name("KeyDown"));
    circa_set_vec2(circa_index(&event, 1), 0, 0);
    circa_set_int(circa_index(&event, 2), qevent->key());
    onInputEvent(&event);
}
void GLWidget::keyReleaseEvent ( QKeyEvent * qevent )
{
    circa::Value event;
    circa_set_list(&event, 3);
    circa_set_name(circa_index(&event, 0), circa_to_name("KeyUp"));
    circa_set_vec2(circa_index(&event, 1), 0, 0);
    circa_set_int(circa_index(&event, 2), qevent->key());
    onInputEvent(&event);
}
void GLWidget::onInputEvent(caValue* event)
{
    scripts_pre_message_send();

    circa::Value msg;
    circa_set_list(&msg, 2);

    circa_set_string(circa_index(&msg, 0), "onInputEvent");
    circa_move(event, circa_index(&msg, 1));

    circa_actor_run_message(g_world, "Main", &msg);

    scripts_post_message_send();
}
