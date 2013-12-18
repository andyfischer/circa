
// This file includes some code borrowed from libtar: http://www.feep.net/libtar

#include "common_headers.h"

#include "blob.h"

const int tarHeaderSize = 512;

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
        output += output* 8 + (str[i] - '0');
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

static void advance_to_next_file(char** data)
{
    int size = file_size(*data);
    *data += (((size - 1) / 512) + 1) * 512;
}

namespace circa {

void tar_read_file(caValue* tarBlob, caValue* filename, caValue* fileOut)
{
    char* data = as_blob(tarBlob);
}

void tar_debug_list_every_file(caValue* tarBlob)
{
    char* data = as_blob(tarBlob);
    char* end = data + blob_size(tarBlob);

    int pos = 0;

    while (data < end) {
        printf("file: %s\n", file_name(data));
        printf("  size: %d\n", file_size(data));
        advance_to_next_file(&data);
    }
}

} // namespace circa
