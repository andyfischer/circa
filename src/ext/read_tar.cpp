
// This file includes some code borrowed from libtar: http://www.feep.net/libtar

#include "common_headers.h"

#include "blob.h"
#include "debug.h"
#include "string_type.h"
#include "tagged_value.h"
#include "read_tar.h"

namespace circa {

#pragma pack(push,1)

struct TarHeader
{
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char chksum[8];
	char typeflag;
	char linkname[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[155];
	char padding[12];
	char *gnu_longname;
	char *gnu_longlink;
};

#pragma pack(pop)

static int parse_octal_string(char* str, int len)
{
    int output = 0;
    for (int i=0; i < len; i++) {
        if (str[i] == 0 || str[i] == ' ')
            break;
        output = output * 8 + (str[i] - '0');
    }
    return output;
}

static int file_size(char* data)
{
    TarHeader* header = (TarHeader*) data;
    return parse_octal_string(header->size, 11);
}

static const char* file_name(char* data)
{
    TarHeader* header = (TarHeader*) data;
    return header->name;
}

static void file_copy_contents(char* data, Value* out)
{
    int size = file_size(data);
    char* file = data + 512;
    set_blob(out, size);
    memcpy(blob_data_flat(out), file, size);
}

static void advance_to_next_file(char** data)
{
    int size = file_size(*data);
    *data += 512;
    if (size > 0)
        *data += (((size-1) / 512) + 1) * 512;
}

static bool eof(char* data)
{
    if (data == NULL)
        return true;
    return file_name(data)[0] == 0;
}

void tar_read_file(Value* tarBlob, const char* filename, Value* fileOut)
{
    ca_assert(is_string(tarBlob));
    char* data = blob_data_flat(tarBlob);

    while (!eof(data)) {
        if (strcmp(file_name(data), filename) == 0) {
            file_copy_contents(data, fileOut);
            return;
        }

        advance_to_next_file(&data);
    }

    set_null(fileOut);
}

bool tar_file_exists(Value* tarBlob, const char* filename)
{
    char* data = blob_data_flat(tarBlob);
    while (!eof(data)) {
        if (strcmp(file_name(data), filename) == 0)
            return true;
        advance_to_next_file(&data);
    }
    return false;
}

CIRCA_EXPORT void circa_load_tar_in_memory(World* world, char* data, uint32_t numBytes)
{
    char* dataStart = data;

    while (!eof(data) && (data - dataStart) < numBytes) {
        Value filename;
        set_string(&filename, file_name(data));

        Value contents;
        file_copy_contents(data, &contents);

        circa_load_file_in_memory(world, &filename, &contents);
        advance_to_next_file(&data);
    }
}

void tar_debug_dump_listing(Value* tarBlob)
{
    char* data = blob_data_flat(tarBlob);

    while (!eof(data)) {
        printf("file: %s\n", file_name(data));
        printf("  size: %d\n", file_size(data));
        advance_to_next_file(&data);
    }
}

} // namespace circa
