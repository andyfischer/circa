// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#ifndef CIRCA_OBJECTS_H_INCLUDED
#define CIRCA_OBJECTS_H_INCLUDED

typedef void (*caObjectReleaseFunc)(void* object);

bool circa_is_object(caValue* value);

// Access the contents of an object value
void* circa_object_contents(caValue* value);

// Convenience func; access a stack input as an object value
void* circa_object_input(caStack* stack, int input);

// Convenience func; create an object value in the given output slot, with the appropriate
// declared type. Returns a pointer to the object's contents.
void* circa_create_object_output(caStack* stack, int output);

// Configure the given type as an object-based type.
void circa_setup_object_type(caType* type, size_t objectSize, caObjectReleaseFunc release);

#endif
