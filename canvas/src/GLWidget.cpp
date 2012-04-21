
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

    // Create View object
    viewObj = circa_alloc_value();
    circa_push_function_by_name(g_mainStack, "create_view");
    scripts_run();
    circa_move(circa_output(g_mainStack, 0), viewObj);
    circa_pop(g_mainStack);

    onPaintEvent = circa_alloc_value();
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

    // call onPaintEvent
    caStack* stack = g_mainStack;
    if (!circa_push_function_by_name(stack, "View.onPaintEvent"))
        return;

    circa_copy(viewObj, circa_input(stack, 0));
    circa_set_typed_pointer(circa_input(stack, 1),
        circa_find_type(NULL, "Painter"), &painter);

    // State
    circa_copy(onPaintEvent, circa_input(stack, 2));

    scripts_run();

    // State
    circa_copy(circa_output(stack, 1), onPaintEvent);

    circa_pop(stack);

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
    caStack* stack = g_mainStack;

    if (!circa_push_function_by_name(stack, "View.onInputEvent"))
        return;

    circa_copy(viewObj, circa_input(stack, 0));
    circa_copy(event, circa_input(stack, 1));

    if (!scripts_run())
        return;

    circa_copy(circa_output(stack, 0), viewObj);
    circa_pop(stack);
    circa_dealloc_value(event);
}

