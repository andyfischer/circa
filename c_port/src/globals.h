#ifndef __GLOBALS_INCLUDED__
#define __GLOBALS_INCLUDED__


Term* GetGlobal(string name);

extern "C" {

Term* GetGlobal(const char* name);

}

#endif
