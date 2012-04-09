
#include <QtGui>

#include "GLWidget.h"
#include "ScriptCenter.h"
#include "main.h"

GLWidget::GLWidget(QWidget *parent)
     : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    setFixedSize(600, 600);
    setAutoFillBackground(false);
}

void GLWidget::animate()
{
    repaint();
}

void GLWidget::paintEvent(QPaintEvent*)
{
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    ScriptCenter* scripts = GetScriptCenter();

    caValue* inputs = circa_alloc_value();
    circa_set_list(inputs, 1);
    circa_set_typed_pointer(circa_get_index(inputs, 0),
        circa_find_type(scripts->mainBranch, "Painter"), &painter);

    scripts->call("onPaintEvent", inputs);

    painter.end();

    circa_dealloc_value(inputs);
}
