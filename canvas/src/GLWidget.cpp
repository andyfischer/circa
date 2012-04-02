
#include <QtGui>

#include "GLWidget.h"

GLWidget::GLWidget(QWidget *parent)
     : QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
 {
     elapsed = 0;
     setFixedSize(600, 600);
     setAutoFillBackground(false);
 }

 void GLWidget::animate()
 {
     repaint();
 }

 void GLWidget::paintEvent(QPaintEvent *event)
 {
     QPainter painter;
     painter.begin(this);
     painter.setRenderHint(QPainter::Antialiasing);
     //TODO paint
     painter.end();
 }
