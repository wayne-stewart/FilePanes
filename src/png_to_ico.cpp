
// 4668 warns when replacing macro definitions with 0.
// 4458 warns when one definition hides another.
// I don't need to be warned about what is in Microsoft's header files.
#pragma warning(push)
#pragma warning(disable:4668 4458)

#include <windows.h>
#include <stdio.h>

#pragma warning(pop)

// 4820 is an informative warning about how much padding is added to a struct
// for memory alignment. this is fine.
#pragma warning(disable:4820)

// 4711 is an informative warning about functions selected for automatic
// inline expansion. this is fine.
#pragma warning(disable:4711)

// 5045 is an informative warning that the compiler will insert Spectre mitigations
// when /Qspecter switch is applied to the cl command. this is fine.
#pragma warning(disable:5045)

// 4710 is an informative warning that a function was not inlined.
// this is fine.
#pragma warning(disable:4710)

void write_little_endian_u16(unsigned char *buffer, uint16_t value)
{
    buffer[0] = (unsigned char)(value & 0xFF);
    buffer[1] = (unsigned char)((value >> 8) & 0xFF);
}

void write_little_endian_u32(unsigned char *buffer, uint32_t value)
{
    buffer[0] = (unsigned char)(value & 0xFF);
    buffer[1] = (unsigned char)((value >> 8) & 0xFF);
    buffer[2] = (unsigned char)((value >> 16) & 0xFF);
    buffer[3] = (unsigned char)((value >> 24) & 0xFF);
}

int main(int argc, char *argv[])
{
    const int BUFFER_SIZE = 4096;
    char *path_arg = argv[1];
    char path_buffer[BUFFER_SIZE] = {0};
    char update_buffer[BUFFER_SIZE] = {0};
    //char read_buffer[BUFFER_SIZE] = {0};
    unsigned char write_buffer[BUFFER_SIZE] = {0};
    char *home_dir = nullptr;
    char *file_name = nullptr;
    size_t dir_size = 0;
    size_t path_size = 0;
    size_t file_name_size = 0;
    DWORD dw = 0;
    size_t size_result = 0;
    int int_result = 0;
    //errno_t err_result = 0;

    if (argc != 2) {
        printf("Usage: ptoi.exe <png file> %d", argc);
        return 1;
    }

    // the path passed in should at least be 5 characters long
    // because it should end with .png and have at least 1 character for the rest of the name
    // if size_result = the size of the buffer, i don't support it
    size_result = strnlen(path_arg, BUFFER_SIZE);
    if (size_result < 5 || size_result == BUFFER_SIZE) {
        printf("Invalid path Argument");
        return 1;
    }

    if (_strnicmp(path_arg + size_result - 4, ".png", 4) != 0) {
        printf("File must be a .png file");
        return 1;
    }

    if (path_arg[0] == '~') {
        size_t home_dir_size = 0;
        if (_dupenv_s(&home_dir, &home_dir_size, "userprofile") != 0 || home_dir == nullptr) {
            printf("HOME environment variable not set");
            return 1;
        }

        size_result = strnlen(home_dir, BUFFER_SIZE);
        if (size_result == 0 || size_result == BUFFER_SIZE) {
            printf("Invalid HOME environment variable");
            return 1;
        }

        int_result = snprintf(path_buffer, BUFFER_SIZE, "%s%s", home_dir, path_arg + 1);
        if (int_result == 0 || int_result == BUFFER_SIZE) {
            printf("Invalid HOME environment variable");
            return 1;
        }
    } else {
        int_result = snprintf(path_buffer, BUFFER_SIZE, "%s", path_arg);
        if (int_result == 0 || int_result == BUFFER_SIZE) {
            printf("Failed to write path_arg into path_buffer");
            return 1;
        }
    }

    dw = GetFileAttributes(path_buffer);
    if (dw == INVALID_FILE_ATTRIBUTES || (dw & FILE_ATTRIBUTE_DIRECTORY)) {
        printf("File not found: %s", path_buffer);
        return 1;
    }

    file_name = strrchr(path_buffer, '\\') + 1;
    strncpy_s(update_buffer, BUFFER_SIZE, path_buffer, BUFFER_SIZE);

    dir_size = (size_t)(file_name - path_buffer);
    file_name_size = strnlen(file_name, BUFFER_SIZE);
    path_size = dir_size + file_name_size;

    strncpy_s(update_buffer + path_size - 4,  BUFFER_SIZE - path_size + 4, ".ico", 4);

    // replace - with of the file name only_
    for (size_t i = dir_size; i < path_size - 4; i++) {
        if (update_buffer[i] == '-') {
            update_buffer[i] = '_';
        }
    }

    // printf("PNG: %s\n", path_buffer);
    // printf("ICO: %s\n", update_buffer);

    HANDLE src_file = CreateFile(path_buffer, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (src_file == INVALID_HANDLE_VALUE) {
        printf("Failed to open file: %s", path_buffer);
        return 1;
    }

    HANDLE dst_file = CreateFile(update_buffer, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (dst_file == INVALID_HANDLE_VALUE) {
        printf("Failed to create file: %s", update_buffer);
        return 1;
    }

    dw = GetFileSize(src_file, NULL);
    if (dw == INVALID_FILE_SIZE) {
        printf("Failed to get file size: %s", path_buffer);
        return 1;
    }

    // ICONDIR header ( 6 bytes )
    write_little_endian_u16(write_buffer, 0);    // reserved 0
    write_little_endian_u16(&write_buffer[2], 1);    // 1 for ico, 2 for cur ( 2 bytes )
    write_little_endian_u16(&write_buffer[4], 1);    // number of images ( 2 bytes )

    // ICONDIRENTRY header ( 16 bytes )
    write_buffer[6] = 16;   // image width
    write_buffer[7] = 16;   // image height
    write_buffer[8] = 0;    // number of colors - 0 for no color palette
    write_buffer[9] = 0;    // reserved 0
    write_little_endian_u16(&write_buffer[10], 1);   // ico color planes OR cur hotspot x ( 2 bytes )
    write_little_endian_u16(&write_buffer[12], 32);  // ico bits per pixel OR cur hotspot y ( 2 bytes )
    write_little_endian_u32(&write_buffer[14], dw); // image data size ( 4 bytes )
    write_little_endian_u32(&write_buffer[18], 22); // image data offset ( 4 bytes )

    // write the header
    WriteFile(dst_file, write_buffer, 22, &dw, NULL);

    // write the image data from the source file
    while(ReadFile(src_file, write_buffer, BUFFER_SIZE, &dw, NULL) && dw > 0)
    {
        WriteFile(dst_file, write_buffer, dw, &dw, NULL);
    }

    CloseHandle(src_file);
    CloseHandle(dst_file);

    return 0;
}