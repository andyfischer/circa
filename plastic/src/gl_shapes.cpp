// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>
#include <importing_macros.h>

#include "app.h"
#include "gl_util.h"
#include "plastic_common_headers.h"

using namespace circa;

namespace gl_shapes {

struct MultipurposeBuffer
{
    static const int staticMemorySize = 5000;
    static GLfloat staticArray[staticMemorySize];
    GLfloat *heapArray;

    MultipurposeBuffer(int size)
    {
        if (size <= staticMemorySize)
            heapArray = NULL;
        else
            heapArray = new GLfloat(size);
    }
    ~MultipurposeBuffer()
    {
        if (heapArray)
            delete heapArray;
    }
    GLfloat* get()
    {
        if (heapArray)
            return heapArray;
        else
            return staticArray;
    }
    GLfloat& operator[] (int index) { return get()[index]; }
};

GLfloat MultipurposeBuffer::staticArray[MultipurposeBuffer::staticMemorySize];

GLint current_program = 0;

void set_gl_color(TValue* color)
{
    glColor4f(color->getIndex(0)->toFloat(),
              color->getIndex(1)->toFloat(),
              color->getIndex(2)->toFloat(),
              color->getIndex(3)->toFloat());
}

void clear_gl_color()
{
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void load_2d_vector_list_to_buffer(TValue* list, GLfloat* buffer)
{
    int numElements = list->numElements();
    int write = 0;
    for (int i=0; i < numElements; i++) {
        float x = list->getIndex(i)->getIndex(0)->toFloat();
        float y = list->getIndex(i)->getIndex(1)->toFloat();

        buffer[write++] = x;
        buffer[write++] = y;
        buffer[write++] = 0;
    }
}

CA_FUNCTION(background)
{
    gl_clear_error();

    TValue* color = INPUT(0);

    glClearColor(color->getIndex(0)->asFloat(),
            color->getIndex(1)->asFloat(),
            color->getIndex(2)->asFloat(),
            color->getIndex(3)->asFloat());

    glClear(GL_COLOR_BUFFER_BIT);

    gl_check_error(CONTEXT, CALLER);

    set_null(OUTPUT);
}

CA_FUNCTION(gl_triangles)
{
    TValue* list = INPUT(0);
    TValue* color = INPUT(1);

    set_gl_color(color);
    glBindTexture(GL_TEXTURE_2D, 0);

    int numElements = list->numElements();
    MultipurposeBuffer buffer(numElements * 3);

    load_2d_vector_list_to_buffer(list, buffer.get());

    glVertexPointer(3, GL_FLOAT, 0, buffer.get());
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_TRIANGLES, 0, numElements);

    clear_gl_color();
    glDisableClientState(GL_VERTEX_ARRAY);
    gl_check_error(CONTEXT, CALLER);
}

CA_FUNCTION(gl_line_strip)
{
    TValue* list = INPUT(0);
    TValue* color = INPUT(1);

    set_gl_color(color);

    int numElements = list->numElements();
    MultipurposeBuffer buffer(numElements * 3);

    load_2d_vector_list_to_buffer(list, buffer.get());
    
    glVertexPointer(3, GL_FLOAT, 0, buffer.get());
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINE_STRIP, 0, numElements);

    clear_gl_color();
    glDisableClientState(GL_VERTEX_ARRAY);
    gl_check_error(CONTEXT, CALLER);
}

CA_FUNCTION(gl_line_loop)
{
    TValue* list = INPUT(0);
    TValue* color = INPUT(1);

    set_gl_color(color);

    int numElements = list->numElements();
    MultipurposeBuffer buffer(numElements * 3);

    load_2d_vector_list_to_buffer(list, buffer.get());
    
    glVertexPointer(3, GL_FLOAT, 0, buffer.get());
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINE_LOOP, 0, numElements);

    clear_gl_color();
    glDisableClientState(GL_VERTEX_ARRAY);
    gl_check_error(CONTEXT, CALLER);
}

CA_FUNCTION(gl_lines)
{
    TValue* list = INPUT(0);
    TValue* color = INPUT(1);

    set_gl_color(color);

    int numElements = list->numElements();
    MultipurposeBuffer buffer(numElements * 3);

    load_2d_vector_list_to_buffer(list, buffer.get());
    
    glVertexPointer(3, GL_FLOAT, 0, buffer.get());
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINES, 0, numElements);

    clear_gl_color();
    glDisableClientState(GL_VERTEX_ARRAY);
    gl_check_error(CONTEXT, CALLER);
}

CA_FUNCTION(gl_points)
{
    TValue* list = INPUT(0);
    TValue* color = INPUT(1);

    set_gl_color(color);

    int numElements = list->numElements();
    MultipurposeBuffer buffer(numElements * 3);

    load_2d_vector_list_to_buffer(list, buffer.get());

    glVertexPointer(3, GL_FLOAT, 0, buffer.get());
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_POINTS, 0, numElements);

    clear_gl_color();
    glDisableClientState(GL_VERTEX_ARRAY);
    gl_check_error(CONTEXT, CALLER);
}

CA_FUNCTION(gl_circle)
{
    Point* loc = Point::checkCast(INPUT(0));
    float x = loc->getX();
    float y = loc->getY();
    float radius = FLOAT_INPUT(1);
    TValue* color = INPUT(2);
    set_gl_color(color);

    // Dumb guess on how many polygons to use
    int control_points = int(radius/3) + 10;
    if (control_points < 15) control_points = 15;

    MultipurposeBuffer buffer(control_points * 3);
    int write = 0;

    buffer[write++] = x;
    buffer[write++] = y;
    buffer[write++] = 0;

    for (int i=0; i <= control_points; i++) {
        float angle_0 = float(float(i) / control_points * M_PI * 2);
        float angle_1 = float(float(i+1) / control_points * M_PI * 2);

        buffer[write++] = x + radius * std::cos(angle_0);
        buffer[write++] = y + radius * std::sin(angle_0);
        buffer[write++] = 0;

        buffer[write++] = x + radius * std::cos(angle_1);
        buffer[write++] = y + radius * std::sin(angle_1);
        buffer[write++] = 0;
    }

    glVertexPointer(3, GL_FLOAT, 0, buffer.get());
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 1 + control_points*2);

    clear_gl_color();
    glDisableClientState(GL_VERTEX_ARRAY);
    gl_check_error(CONTEXT, CALLER);
}

CA_FUNCTION(gl_pie)
{
    Point* loc = Point::checkCast(INPUT(0));
    float x = loc->getX();
    float y = loc->getY();
    float radius = FLOAT_INPUT(1);
    float angle_start = FLOAT_INPUT(2);
    float angle_fin = FLOAT_INPUT(3);
    TValue* color = INPUT(4);
    set_gl_color(color);

    if (angle_start > angle_fin) {
        float swap = angle_start; angle_start = angle_fin; angle_fin = swap;
    }
    float angle_span = angle_fin - angle_start;

    // Dumb guess on how many polygons to use
    int control_points = int(radius/3) + 10;
    if (control_points < 15) control_points = 15;

    MultipurposeBuffer buffer(control_points * 3);
    int write = 0;

    buffer[write++] = x;
    buffer[write++] = y;
    buffer[write++] = 0;

    for (int i=0; i < control_points; i++) {

        float angle_0 = float(float(i) / control_points * angle_span + angle_start);
        float angle_1 = float(float(i+1) / control_points * angle_span + angle_start);

        // Convert from 0..1 to radians
        angle_0 *= M_PI * 2;
        angle_1 *= M_PI * 2;

        // Use (sin,-cos) so that angle 0 starts at the top and increases clockwise.
        buffer[write++] = x + radius * std::sin(angle_0);
        buffer[write++] = y + radius * -std::cos(angle_0);
        buffer[write++] = 0;

        buffer[write++] = x + radius * std::sin(angle_1);
        buffer[write++] = y + radius * -std::cos(angle_1);
        buffer[write++] = 0;
    }

    glVertexPointer(3, GL_FLOAT, 0, buffer.get());
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_TRIANGLE_FAN, 0, control_points*2 + 1);

    clear_gl_color();
    glDisableClientState(GL_VERTEX_ARRAY);
    gl_check_error(CONTEXT, CALLER);
}


void setup(Branch& branch)
{
    install_function(branch["background"], background);
    Branch& gl_ns = nested_contents(branch["gl"]);
    install_function(gl_ns["triangles"], gl_triangles);
    install_function(gl_ns["line_strip"], gl_line_strip);
    install_function(gl_ns["line_loop"], gl_line_loop);
    install_function(gl_ns["lines"], gl_lines);
    install_function(gl_ns["points"], gl_points);
    install_function(gl_ns["circle"], gl_circle);
    install_function(gl_ns["pie"], gl_pie);
}

} // namespace gl_shapes
