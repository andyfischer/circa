// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <QtGui>

#include "GLWidget.h"
#include "Scripts.h"
#include "engine/Common.h"
#include "engine/FontBitmap.h"
#include "engine/ResourceManager.h"

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

#ifdef _MSC_VER
    // Fixes a font corruption issue on Windows
    QGL::setPreferredPaintEngine( QPaintEngine::OpenGL );
#endif
}

void GLWidget::animate()
{
    repaint();
}

#if 0
void GLWidget::paintEvent(QPaintEvent*)
{
    // Send a timeUpdate message
    circa::Value msg;
    circa_set_list(&msg, 2);
    circa_set_string(circa_index(&msg, 0), "timeUpdate");
    circa_set_float(circa_index(&msg, 1), elapsedTime.elapsed() / 1000.0f);

    circa_actor_run_message(g_world, "View", &msg);

    // Send an onPaintEvent message
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    scripts_pre_message_send();

    // call onPaintEvent
    circa_set_list(&msg, 2);

    circa_set_string(circa_index(&msg, 0), "onPaintEvent");
    circa_set_typed_pointer(circa_index(&msg, 1),
        circa_find_type(NULL, "Painter"), &painter);

    circa_actor_run_message(g_world, "View", &msg);

    scripts_post_message_send();

    painter.end();
}
#endif

void GLWidget::initializeGL()
{
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    check_gl_error();

    ResourceManager resourceManager;

    renderList.setup(&resourceManager);

    // Temp for testing
    int font = font_load("assets/jackinput.ttf", 24);
    textSprite = new TextSprite();
    textSprite->init(&renderList, font);
    textSprite->setText("hello");
    textSprite->setPosition(10, 10);
    textSprite->setColor(Color(0,0,1,1));
    check_gl_error();
}

void GLWidget::paintGL()
{
    check_gl_error();

    glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT);

    renderList.render();
}

void GLWidget::mouseDoubleClickEvent ( QMouseEvent * qevent )
{
    circa::Value event;
    circa_set_list(&event, 3);
    circa_set_int(circa_index(&event, 0), 4);
    circa_set_vec2(circa_index(&event, 1), qevent->x(), qevent->y());
    circa_set_int(circa_index(&event, 2), 0);
    onInputEvent(&event);
}
void GLWidget::mouseMoveEvent ( QMouseEvent * qevent )
{
    circa::Value event;
    circa_set_list(&event, 3);
    circa_set_int(circa_index(&event, 0), 3);
    circa_set_vec2(circa_index(&event, 1), qevent->x(), qevent->y());
    circa_set_int(circa_index(&event, 2), 0);
    onInputEvent(&event);
}
void GLWidget::mousePressEvent ( QMouseEvent * qevent )
{
    circa::Value event;
    circa_set_list(&event, 3);
    circa_set_int(circa_index(&event, 0), 1);
    circa_set_vec2(circa_index(&event, 1), qevent->x(), qevent->y());
    circa_set_int(circa_index(&event, 2), 0);
    onInputEvent(&event);
}
void GLWidget::mouseReleaseEvent ( QMouseEvent * qevent )
{
    circa::Value event;
    circa_set_list(&event, 3);
    circa_set_int(circa_index(&event, 0), 2);
    circa_set_vec2(circa_index(&event, 1), qevent->x(), qevent->y());
    circa_set_int(circa_index(&event, 2), 0);
    onInputEvent(&event);
}
void GLWidget::keyPressEvent ( QKeyEvent * qevent )
{
    circa::Value event;
    circa_set_list(&event, 3);
    circa_set_int(circa_index(&event, 0), 5);
    circa_set_vec2(circa_index(&event, 1), 0, 0);
    circa_set_int(circa_index(&event, 2), qevent->key());
    onInputEvent(&event);
}
void GLWidget::keyReleaseEvent ( QKeyEvent * qevent )
{
    circa::Value event;
    circa_set_list(&event, 3);
    circa_set_int(circa_index(&event, 0), 6);
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

    circa_actor_run_message(g_world, "View", &msg);

    scripts_post_message_send();
}
