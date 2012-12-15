
#include <cstdio>

#include "ppapi/cpp/var.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"

#include "circa/circa.h"

class CircaInstance : public pp::Instance {
    /* The Instance class.  One of these exists for each instance of your NaCl
    * module on the web page.  The browser will ask the Module object to create
    * a new Instance for each occurrence of the <embed> tag that has certain
    * attributes.
    */
    
public:
    explicit CircaInstance(PP_Instance instance) : pp::Instance(instance) {
        printf("created CircaInstance\n");
    }
    virtual ~CircaInstance() {}

    virtual void HandleMessage(const pp::Var& var_message)
    {
        PostMessage(pp::Var("Message received"));
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
