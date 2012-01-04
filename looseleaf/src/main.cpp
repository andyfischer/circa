
#include <QApplication>
#include <QDesktopWidget>

#include <QDir>
#include <QFileInfo>

#include "circa.h"

#include "BackgroundScript.h"
#include "Viewport.h"
#include "ScriptEnv.h"

using namespace circa;

// Setup functions that are implemented elsewhere:
void viewport_static_setup(Branch* branch);

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    printf("Started looseleaf\n");

    // Walk upwards till we find the "runtime" directory. This way things will work
    // fine when we are launched from an app bundle.
    int steps = 0;
    while (!QFileInfo("runtime").exists()) {

        QDir parent = QDir::current();
        parent.cdUp();

        QDir::setCurrent(parent.path());

        if (steps++ > 100) {
            printf("Couldn't find 'runtime' directory\n");
            return -1;
        }
    } 

    // Initialize Circa runtime
    circa_initialize();
    circa_use_standard_filesystem();
    circa_add_module_search_path("../libs");

    Branch kernel;
    BackgroundScript kernelRunner(&kernel);

    // Install our libraries
    Term* viewportModule = circa_load_module_from_file(circa_string_to_symbol("viewport"), "runtime/viewport.ca");
    viewport_static_setup(nested_contents(viewportModule));

    include_script(&kernel, "runtime/main.ca");

    Branch* filesBranch = create_branch_unevaluated(&kernel, "files");
    set_files_branch_global(filesBranch);

    if (has_static_errors(&kernel)) {
        print_static_errors_formatted(&kernel);
        return -1;
    }

    dump(&kernel);

    kernelRunner.start();

    printf("Calling app.exec..\n");
    int result = app.exec();

    printf("Shutting down, result = %d\n", result);

    clear_branch(&kernel);
    circa_shutdown();

    return result;
}
