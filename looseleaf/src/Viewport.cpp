// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <QtGui>
#include <QtOpenGL>

#include "circa.h"

#include "MouseState.h"
#include "Viewport.h"

using namespace circa;

circa::HandleWrapper<Viewport> viewport_t;

const int updateInterval = 16;

Viewport::Viewport(QWidget* parent)
 : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    QObject::connect(&updateTimer, SIGNAL(timeout()), this, SLOT(tick()));
    updateTimer.start(updateInterval);


    // Place a pointer to this Viewport in the EvalContext, so that the script can
    // access us.
    // TODO: This should use a handle, it doesn't b/c this object is owned
    // elsewhere.
    set_opaque_pointer(scriptEnv.context.argumentList.append(), this);
}

Viewport::~Viewport()
{
}

void Viewport::initializeGL()
{
    //qglClearColor();

    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_LIGHTING);

    glPolygonMode(GL_FRONT, GL_FILL);
    glDisable(GL_CULL_FACE);
}
void Viewport::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
     
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1000.0f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
void Viewport::mouseMoveEvent(QMouseEvent *event)
{
    mouseState.x = event->x();
    mouseState.y = event->y();
}
void Viewport::tick()
{
    updateGL();
}
void Viewport::saveScript()
{
    printf("saving\n");
    save_script(scriptEnv.branch);
}
void Viewport::paintGL()
{
    try {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        scriptEnv.tick();
    } catch (std::exception& e) {
        std::cout << "Caught exception in Viewport::paintGL: " << e.what() << std::endl;
    }
}
circa::Branch* Viewport::loadScript(const char* filename)
{
    return scriptEnv.loadScript(filename);
}

void Viewport::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
        close();
    else if (e->key() == Qt::Key_S && (e->modifiers() & Qt::ControlModifier))
        saveScript();
    else
        QWidget::keyPressEvent(e);
}

CA_FUNCTION(create_window)
{
    std::cout << "create_window: " << STRING_INPUT(0) << std::endl;

    Viewport* viewport = new Viewport();
    std::string title;
    title += "looseleaf : ";
    title += STRING_INPUT(0);
    viewport->setWindowTitle(title.c_str());
    viewport->setMouseTracking(true);
    viewport->loadScript(STRING_INPUT(0));
    viewport_t.set(OUTPUT, viewport);
    viewport->show();
}

CA_FUNCTION(Viewport__resize)
{
    float x,y;
    get_point(INPUT(1), &x, &y);
    viewport_t.get(INPUT(0))->resize(QSize(x,y));
}

CA_FUNCTION(viewport__size)
{
    Viewport* window = (Viewport*) as_opaque_pointer(CONTEXT->argumentList[0]);
    QSize size = window->size();
    set_point(OUTPUT, size.width(), size.height());
}

CA_FUNCTION(viewport__mouse)
{
    Viewport* window = (Viewport*) as_opaque_pointer(CONTEXT->argumentList[0]);
    MouseState* mouseState = &window->mouseState;
    set_point(OUTPUT, mouseState->x, mouseState->y);
}

void viewport_static_setup(Branch* branch)
{
    viewport_t.initialize(branch, "Viewport");
    install_function(branch, create_window, "create_window");
    install_function(branch, Viewport__resize, "Viewport.resize");
    install_function(branch, viewport__mouse, "viewport:mouse");
    install_function(branch, viewport__size, "viewport:size");
}
