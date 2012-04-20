
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
    if (!circa_push_function_by_name(g_mainStack, "View.onPaintEvent"))
        return;

    circa_copy(viewObj, circa_input(g_mainStack, 0));
    circa_set_typed_pointer(circa_input(g_mainStack, 1),
        circa_find_type(NULL, "Painter"), &painter);

    scripts_run();

    painter.end();
}
