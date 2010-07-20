// Copyright (c) 2010 Paul Hodge. All rights reserved.

#ifdef USE_OPENFRAMEWORKS

#include "ofMain.h"
#include "ofGraphics.h"
#include "ofTrueTypeFont.h"

#include <circa.h>
#include <importing_macros.h>

#include "app.h"

namespace of_bindings {

CA_FUNCTION(background)
{
    circa::TaggedValue* color = INPUT(0);
	ofBackground(color->getIndex(0)->toFloat() * 255,
        color->getIndex(1)->toFloat() * 255,
        color->getIndex(2)->toFloat() * 255);
}

// todo: don't use a global font.
ofTrueTypeFont *mainFont = NULL;

CA_FUNCTION(load_font)
{
    if (mainFont != NULL)
        return;

    mainFont = new ofTrueTypeFont();
    mainFont->loadFont(STRING_INPUT(0), INT_INPUT(1));
}

CA_FUNCTION(draw_string)
{
    circa::TaggedValue *loc = INPUT(1);
    circa::TaggedValue *color = INPUT(2);

    ofSetColor(color->getIndex(0)->toFloat() * 255,
        color->getIndex(1)->toFloat() * 255,
        color->getIndex(2)->toFloat() * 255,
        color->getIndex(3)->toFloat() * 255);

    mainFont->drawString(STRING_INPUT(0),
            loc->getIndex(0)->toFloat(),
            loc->getIndex(1)->toFloat());
}

void setup()
{
    circa::Branch& branch = app::runtime_branch();

    install_function(branch["load_font"], load_font);
    install_function(branch["draw_string"], draw_string);
}

} // namespace of_bindings

#endif
;
