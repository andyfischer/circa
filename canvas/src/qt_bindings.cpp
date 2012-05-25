
#include "circa/circa.h"

#include <QBrush>
#include <QPen>
#include <QFont>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>

void BrushRelease(caValue* value)
{
    delete (QBrush*) circa_get_pointer(value);
}
void PenRelease(caValue* value)
{
    delete (QPen*) circa_get_pointer(value);
}
void FontMetricsRelease(caValue* value)
{
    delete (QFontMetrics*) circa_get_pointer(value);
}
void FontRelease(caValue* value)
{
    delete (QFont*) circa_get_pointer(value);
}
void LinearGradientRelease(caValue* value)
{
    delete (QLinearGradient*) circa_get_pointer(value);
}
void PainterPathRelease(caValue* value)
{
    delete (QPainterPath*) circa_get_pointer(value);
}

QPointF to_qpoint(caValue* value)
{
    float x,y;
    circa_vec2(value, &x, &y);
    return QPoint(x,y);
}

QColor to_qcolor(caValue* value)
{
    float r,g,b,a;
    circa_vec4(value, &r, &g, &b, &a);
    return QColor(r * 255, g * 255, b * 255, a * 255);
}

QRectF to_qrect(caValue* value)
{
    float x1,y1,x2,y2;
    circa_vec4(value, &x1, &y1, &x2, &y2);
    return QRectF(x1, y1, x2 - x1, y2 - y1);
}

void create_brush(caStack* stack)
{
    caValue* color = circa_input(stack, 0);
    caValue* out = circa_create_default_output(stack, 0);
    circa_handle_set_object(out, new QBrush(to_qcolor(color)), BrushRelease);
}

void Brush__setColor(caStack* stack)
{
    QBrush* brush = (QBrush*) circa_get_pointer(circa_input(stack, 0));
    caValue* color = circa_input(stack, 1);

    brush->setColor(to_qcolor(color));
}
void create_pen(caStack* stack)
{
    caValue* c = circa_input(stack, 0);
    float width = circa_float(circa_input(stack, 1));
    caValue* style = circa_input(stack, 2);

    QPen* pen = new QPen();
    pen->setColor(to_qcolor(c));
    pen->setWidth(width);
    style; // TODO: support style

    caValue* out = circa_create_default_output(stack, 0);
    circa_handle_set_object(out, pen, PenRelease);
}

void Pen__setColor(caStack* stack)
{
    QPen* pen = (QPen*) circa_get_pointer(circa_input(stack, 0));
    caValue* color = circa_input(stack, 1);

    pen->setColor(to_qcolor(color));
}
void Pen__setStyle(caStack* stack)
{
    QPen* pen = (QPen*) circa_get_pointer(circa_input(stack, 0));
    caValue* style = circa_input(stack, 1);

    pen; style; // TODO: support style
}
void Pen__setWidth(caStack* stack)
{
    QPen* pen = (QPen*) circa_get_pointer(circa_input(stack, 0));
    float width = circa_float(circa_input(stack, 1));

    pen->setWidth(width);
}
void Pen__setDashPattern(caStack* stack)
{
    QPen* pen = (QPen*) circa_get_pointer(circa_input(stack, 0));
    caValue* dashes = circa_input(stack, 1);
}
void FontMetrics__width(caStack* stack)
{
    QFontMetrics* fontMetrics = (QFontMetrics*) circa_get_pointer(circa_input(stack, 0));
    const char* text = circa_string(circa_input(stack, 1));

    circa_set_float(circa_output(stack, 0), fontMetrics->width(text));
}
void FontMetrics__height(caStack* stack)
{
    QFontMetrics* fontMetrics = (QFontMetrics*) circa_get_pointer(circa_input(stack, 0));

    circa_set_float(circa_output(stack, 0), fontMetrics->height());
}
void create_font(caStack* stack)
{
    const char* name = circa_string(circa_input(stack, 0));
    float size = circa_float(circa_input(stack, 1));

    QFont* font = new QFont(name, int(size));

    caValue* out = circa_create_default_output(stack, 0);
    circa_handle_set_object(out, font, FontRelease);
}
void Font__setPixelSize(caStack* stack)
{
    QFont* font = (QFont*) circa_get_pointer(circa_input(stack, 0));
    float size = circa_float(circa_input(stack, 1));

    font->setPixelSize(size);
}
void Font__fontMetrics(caStack* stack)
{
    QFont* font = (QFont*) circa_get_pointer(circa_input(stack, 0));

    QFontMetrics* metrics = new QFontMetrics(*font);

    caValue* out = circa_create_default_output(stack, 0);
    circa_handle_set_object(out, metrics, FontMetricsRelease);
}
void PainterPath__moveTo(caStack* stack)
{
    QPainterPath* painterPath = (QPainterPath*) circa_get_pointer(circa_input(stack, 0));
    caValue* p = circa_input(stack, 1);

    painterPath->moveTo(to_qpoint(p));
}
void PainterPath__cubicTo(caStack* stack)
{
    QPainterPath* painterPath = (QPainterPath*) circa_get_pointer(circa_input(stack, 0));
    caValue* a = circa_input(stack, 1);
    caValue* b = circa_input(stack, 2);
    caValue* c = circa_input(stack, 3);

    painterPath->cubicTo(to_qpoint(a), to_qpoint(b), to_qpoint(c));
}
void PainterPath__addText(caStack* stack)
{
    QPainterPath* painterPath = (QPainterPath*) circa_get_pointer(circa_input(stack, 0));
    caValue* p = circa_input(stack, 1);
    QFont* f = (QFont*) circa_get_pointer(circa_input(stack, 2));
    const char* text = circa_string(circa_input(stack, 3));

    painterPath->addText(to_qpoint(p), *f, text);
}
void create_linear_gradient(caStack* stack)
{
    caValue* start = circa_input(stack, 0);
    caValue* stop = circa_input(stack, 1);
    caValue* out = circa_create_default_output(stack, 0);

    QLinearGradient* gradient = new QLinearGradient(to_qpoint(start), to_qpoint(stop));

    circa_handle_set_object(out, gradient, LinearGradientRelease);
}
void LinearGradient__setColorAt(caStack* stack)
{
    QLinearGradient* linearGradient = (QLinearGradient*) circa_get_pointer(circa_input(stack, 0));
    float ratio = circa_float(circa_input(stack, 1));
    caValue* c = circa_input(stack, 2);

    linearGradient->setColorAt(ratio, to_qcolor(c));
}
void Painter__save(caStack* stack)
{
    QPainter* painter = (QPainter*) circa_get_pointer(circa_input(stack, 0));
    painter->save();
}
void Painter__restore(caStack* stack)
{
    QPainter* painter = (QPainter*) circa_get_pointer(circa_input(stack, 0));
    painter->restore();
}
void Painter__setBackground(caStack* stack)
{
    QPainter* painter = (QPainter*) circa_get_pointer(circa_input(stack, 0));
    QBrush* brush = (QBrush*) circa_get_pointer(circa_input(stack, 1));

    painter->setBackground(*brush);
}
void Painter__setBrush(caStack* stack)
{
    QPainter* painter = (QPainter*) circa_get_pointer(circa_input(stack, 0));
    QBrush* brush = (QBrush*) circa_get_pointer(circa_input(stack, 1));

    painter->setBrush(*brush);
}
void Painter__setPen(caStack* stack)
{
    QPainter* painter = (QPainter*) circa_get_pointer(circa_input(stack, 0));
    QPen* pen = (QPen*) circa_get_pointer(circa_input(stack, 1));

    painter->setPen(*pen);
}
void Painter__setColor(caStack* stack)
{
    QPainter* painter = (QPainter*) circa_get_pointer(circa_input(stack, 0));
    caValue* color = circa_input(stack, 1);
    painter->setPen(to_qcolor(color));
}
void Painter__setFont(caStack* stack)
{
    QPainter* painter = (QPainter*) circa_get_pointer(circa_input(stack, 0));
    QFont* font = (QFont*) circa_get_pointer(circa_input(stack, 1));

    painter->setFont(*font);
}
void Painter__rotate(caStack* stack)
{
    QPainter* painter = (QPainter*) circa_get_pointer(circa_input(stack, 0));
    float radians = circa_float(circa_input(stack, 1));

    painter->rotate(radians);
}
void Painter__translate(caStack* stack)
{
    QPainter* painter = (QPainter*) circa_get_pointer(circa_input(stack, 0));
    caValue* delta = circa_input(stack, 1);

    painter->translate(to_qpoint(delta));
}
void Painter__boundingRect(caStack* stack)
{
    QPainter* painter = (QPainter*) circa_get_pointer(circa_input(stack, 0));
    caValue* r = circa_input(stack, 1);
    int flags = circa_int_input(stack, 2);
    const char* text = circa_string(circa_input(stack, 3));

    QRectF rect = painter->boundingRect(to_qrect(r), flags, text);
    circa_set_vec4(circa_output(stack, 0), rect.left(), rect.top(), rect.right(), rect.bottom());
}
void Painter__drawEllipse(caStack* stack)
{
    QPainter* painter = (QPainter*) circa_get_pointer(circa_input(stack, 0));
    caValue* r = circa_input(stack, 1);

    painter->drawEllipse(to_qrect(r));
}
void Painter__drawText(caStack* stack)
{
    QPainter* painter = (QPainter*) circa_get_pointer(circa_input(stack, 0));
    caValue* r = circa_input(stack, 1);
    int flags = circa_int_input(stack, 2);
    const char* text = circa_string(circa_input(stack, 3));

    painter->drawText(to_qrect(r), flags, text);

    // Save the bounding rect as output
    QRectF rect = painter->boundingRect(to_qrect(r), flags, text);
    circa_set_vec4(circa_output(stack, 0), rect.left(), rect.top(), rect.right(), rect.bottom());
}
void Painter__drawLine(caStack* stack)
{
    QPainter* painter = (QPainter*) circa_get_pointer(circa_input(stack, 0));
    caValue* a = circa_input(stack, 1);
    caValue* b = circa_input(stack, 2);

    painter->drawLine(to_qpoint(a), to_qpoint(b));
}
void Painter__drawRect(caStack* stack)
{
    QPainter* painter = (QPainter*) circa_get_pointer(circa_input(stack, 0));
    caValue* r = circa_input(stack, 1);

    painter->drawRect(to_qrect(r));
}
void Painter__drawPath(caStack* stack)
{
    QPainter* painter = (QPainter*) circa_get_pointer(circa_input(stack, 0));
    QPainterPath* path = (QPainterPath*) circa_get_pointer(circa_input(stack, 1));

    painter->drawPath(*path);
}
void Painter__drawPolygon(caStack* stack)
{
    QPainter* painter = (QPainter*) circa_get_pointer(circa_input(stack, 0));
    caValue* points = circa_input(stack, 1);
    caValue* color = circa_input(stack, 2);

    int count = circa_count(points);
    QPolygonF polygon(count);

    for (int i=0; i < count; i++) {
        float x,y;
        circa_vec2(circa_index(points, i), &x, &y);
        polygon[i] = QPointF(x, y);
    }

    painter->setBrush(QBrush(to_qcolor(color), Qt::SolidPattern));
    painter->setPen(to_qcolor(color));
    painter->drawPolygon(polygon);
}
void Painter__drawRoundRect(caStack* stack)
{
    QPainter* painter = (QPainter*) circa_get_pointer(circa_input(stack, 0));
    caValue* r = circa_input(stack, 1);

    painter->drawRoundRect(to_qrect(r));
}
void Painter__fillRect(caStack* stack)
{
    QPainter* painter = (QPainter*) circa_get_pointer(circa_input(stack, 0));
    caValue* r = circa_input(stack, 1);
    QBrush* b = (QBrush*) circa_get_pointer(circa_input(stack, 2));

    painter->fillRect(to_qrect(r), *b);
}
void Painter__fillRectColor(caStack* stack)
{
    QPainter* painter = (QPainter*) circa_get_pointer(circa_input(stack, 0));
    caValue* r = circa_input(stack, 1);
    caValue* color = circa_input(stack, 2);

    painter->fillRect(to_qrect(r), to_qcolor(color));
}

void Painter__fontMetrics(caStack* stack)
{
    QPainter* painter = (QPainter*) circa_get_pointer(circa_input(stack, 0));

    QFontMetrics* metrics = new QFontMetrics(painter->fontMetrics());

    caValue* out = circa_create_default_output(stack, 0);
    circa_handle_set_object(out, metrics, FontMetricsRelease);
}

void Painter__viewport(caStack* stack)
{
    QPainter* painter = (QPainter*) circa_get_pointer(circa_input(stack, 0));
    QRect rect = painter->viewport();

    caValue* out = circa_create_default_output(stack, 0);
    circa_set_vec4(out, rect.left(), rect.top(), rect.right(), rect.bottom());
}

static const caFunctionBinding IMPORTS[] = {
    {"create_brush", create_brush},
    {"Brush.setColor", Brush__setColor},
    {"create_pen", create_pen},
    {"Pen.setColor", Pen__setColor},
    {"Pen.setStyle", Pen__setStyle},
    {"Pen.setWidth", Pen__setWidth},
    {"Pen.setDashPattern", Pen__setDashPattern},
    {"FontMetrics.width", FontMetrics__width},
    {"FontMetrics.height", FontMetrics__height},
    {"create_font", create_font},
    {"Font.setPixelSize", Font__setPixelSize},
    {"Font.fontMetrics", Font__fontMetrics},
    {"PainterPath.moveTo", PainterPath__moveTo},
    {"PainterPath.cubicTo", PainterPath__cubicTo},
    {"PainterPath.addText", PainterPath__addText},
    {"create_linear_gradient", create_linear_gradient},
    {"LinearGradient.setColorAt", LinearGradient__setColorAt},
    {"Painter.save", Painter__save},
    {"Painter.restore", Painter__restore},
    {"Painter.setBackground", Painter__setBackground},
    {"Painter.setBrush", Painter__setBrush},
    {"Painter.setColor", Painter__setColor},
    {"Painter.setFont", Painter__setFont},
    {"Painter.setPen", Painter__setPen},
    {"Painter.rotate", Painter__rotate},
    {"Painter.translate", Painter__translate},
    {"Painter.boundingRect", Painter__boundingRect},
    {"Painter.drawEllipse", Painter__drawEllipse},
    {"Painter.drawText", Painter__drawText},
    {"Painter.drawLine", Painter__drawLine},
    {"Painter.drawRect", Painter__drawRect},
    {"Painter.drawPath", Painter__drawPath},
    {"Painter.drawPolygon", Painter__drawPolygon},
    {"Painter.drawRoundRect", Painter__drawRoundRect},
    //{"Painter.fillRect", Painter__fillRect},
    {"Painter.fillRect", Painter__fillRectColor},
    {"Painter.fontMetrics", Painter__fontMetrics},
    {"Painter.viewport", Painter__viewport},
    {NULL, NULL}
};

void qt_bindings_install(caBranch* branch)
{
    circa_install_function_list(branch, IMPORTS);
}

