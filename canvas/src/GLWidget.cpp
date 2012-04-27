// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <QtGui>

#include "GLWidget.h"
#include "Scripts.h"
#include "main.h"

GLWidget::GLWidget(QWidget *parent)
     : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    setFixedSize(600, 600);
    setAutoFillBackground(false);
    setMouseTracking(true);
}

void GLWidget::animate()
{
    repaint();
}

void GLWidget::paintEvent(QPaintEvent*)
{
    scripts_refresh();

    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    scripts_pre_message_send();

    // call onPaintEvent
    caValue* msg = circa_alloc_list(2);

    circa_set_string(circa_index(msg, 0), "onPaintEvent");
    circa_set_typed_pointer(circa_index(msg, 1),
        circa_find_type(NULL, "Painter"), &painter);

    circa_actor_run_message(g_mainStack, "View", msg);

    circa_dealloc_value(msg);

    scripts_post_message_send();

    painter.end();
}

void GLWidget::mouseDoubleClickEvent ( QMouseEvent * qevent )
{
    caValue* event = circa_alloc_list(2);
    circa_set_int(circa_index(event, 0), 4);
    circa_set_vec2(circa_index(event, 1), qevent->x(), qevent->y());
    onInputEvent(event);
}
void GLWidget::mouseMoveEvent ( QMouseEvent * qevent )
{
    caValue* event = circa_alloc_list(2);
    circa_set_int(circa_index(event, 0), 3);
    circa_set_vec2(circa_index(event, 1), qevent->x(), qevent->y());
    onInputEvent(event);
}
void GLWidget::mousePressEvent ( QMouseEvent * qevent )
{
    caValue* event = circa_alloc_list(2);
    circa_set_int(circa_index(event, 0), 1);
    circa_set_vec2(circa_index(event, 1), qevent->x(), qevent->y());
    onInputEvent(event);
}
void GLWidget::mouseReleaseEvent ( QMouseEvent * qevent )
{
    caValue* event = circa_alloc_list(2);
    circa_set_int(circa_index(event, 0), 2);
    circa_set_vec2(circa_index(event, 1), qevent->x(), qevent->y());
    onInputEvent(event);
}
void GLWidget::keyPressEvent ( QKeyEvent * qevent )
{
    caValue* event = circa_alloc_list(2);
    onInputEvent(event);
}
void GLWidget::keyReleaseEvent ( QKeyEvent * qevent )
{
    caValue* event = circa_alloc_list(2);
    onInputEvent(event);
}
void GLWidget::onInputEvent(caValue* event)
{
    scripts_pre_message_send();

    caValue* msg = circa_alloc_list(2);

    circa_set_string(circa_index(msg, 0), "onInputEvent");

    circa_move(event, circa_index(msg, 1));
    circa_dealloc_value(event);

    circa_actor_run_message(g_mainStack, "View", msg);

    circa_dealloc_value(msg);
    scripts_post_message_send();
}
