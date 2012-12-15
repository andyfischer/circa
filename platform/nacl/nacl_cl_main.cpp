
#include <cstdio>

#include "ppapi/cpp/var.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"

#include "circa/circa.h"

void pp_to_circa(const pp::Var& in, caValue* out)
{
    if (in.is_string()) {
        circa_set_string(out, in.AsString().c_str());
    } else {
        circa_set_null(out);
    }
}

pp::Var circa_to_pp(caValue* in)
{
    circa::Value asStr;
    circa_to_string_repr(in, &asStr);
    return pp::Var(circa_string(&asStr));
}


class CircaInstance : public pp::Instance {
    /* The Instance class.  One of these exists for each instance of your NaCl
    * module on the web page.  The browser will ask the Module object to create
    * a new Instance for each occurrence of the <embed> tag that has certain
    * attributes.
    */
public:

    caWorld* world;
    caStack* stack;

    explicit CircaInstance(PP_Instance instance) : pp::Instance(instance) {
        printf("created CircaInstance\n");

        world = circa_initialize();
        stack = circa_alloc_stack(world);

        circa_repl_start(stack);
    }
    virtual ~CircaInstance() {}

    virtual void HandleMessage(const pp::Var& ppMsg)
    {
        circa::Value msg;
        pp_to_circa(ppMsg, &msg);

        if (circa_is_string(&msg)) {

            circa::Value replOutput;
            circa_repl_run_line(stack, &msg, &replOutput);

            PostMessage(circa_to_pp(&replOutput));
        }
    }
};

class CircaModule : public pp::Module {
    /* The Module class.  The browser calls the CreateInstance() method to create
     * an instance of your NaCl module on the web page.  The browser creates a new 
     * instance for each <embed> tag with type="application/x-nacl".
     */

public:
    CircaModule() : pp::Module() {
        printf("created CircaModule\n");
    }
    virtual ~CircaModule() {}

    virtual pp::Instance* CreateInstance(PP_Instance instance) {
        return new CircaInstance(instance);
    }
};

namespace pp {
    /* Factory function called by the browser when the module is first loaded.
    * The browser keeps a singleton of this module.
    */
    Module* CreateModule() {
        return new CircaModule();
    }
}
