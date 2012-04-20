
#include <QtGui>

#include "GLWidget.h"
#include "Scripts.h"
#include "main.h"

GLWidget::GLWidget(QWidget *parent)
     : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    setFixedSize(600, 600);
    setAutoFillBackground(false);

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
