
// This file includes some code borrowed from libtar: http://www.feep.net/libtar

#include "common_headers.h"

#include "blob.h"
#include "tagged_value.h"

namespace circa {

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

static int parse_octal_string(char* str, int len)
{
    int output = 0;
    for (int i=0; i < len; i++) {
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

static void file_copy_contents(char* data, caValue* out)
{
    int size = file_size(data);
    char* file = data + 512;
    set_blob(out, size);
    memcpy(as_blob(out), file, size);
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
    return file_name(data)[0] == 0;
}

void tar_read_file(caValue* tarBlob, const char* filename, caValue* fileOut)
{
    char* data = as_blob(tarBlob);

    while (!eof(data)) {
        if (strcmp(file_name(data), filename) == 0) {
            file_copy_contents(data, fileOut);
            return;
        }

        advance_to_next_file(&data);
    }

    set_null(fileOut);
}

void tar_debug_dump_listing(caValue* tarBlob)
{
    char* data = as_blob(tarBlob);

    while (!eof(data)) {
        printf("file: %s\n", file_name(data));
        printf("  size: %d\n", file_size(data));
        advance_to_next_file(&data);
    }
}

} // namespace circa
