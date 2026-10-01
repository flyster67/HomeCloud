/* backend.c — HomeCloud file I/O & compression engine
 *
 * Child process managed by homecloud.py via stdin/stdout pipes.
 * Handles file reading/writing and zlib compression.
 *
 * Build:
 *   Windows:  gcc backend.c -o backend.exe -lz
 *   Linux:    gcc backend.c -o backend -lz
 *
 * IPC Protocol:
 *   READ <path>\n                          -> OK <size>\n<bytes> | ERROR <msg>\n
 *   WRITE <size> <path>\n<bytes>          -> OK\n | ERROR <msg>\n
 *   COMPRESS <filename> <size>\n<bytes>    -> COMPRESSED <size>\n<bytes> | RAW\n
 *   DECOMPRESS <orig> <comp>\n<bytes>      -> OK <size>\n<bytes> | ERROR <msg>\n
 *   QUIT\n                                 -> exits
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#ifdef _WIN32
  #include <io.h>
  #include <fcntl.h>
#endif

#include <zlib.h>

/* Switch stdin/stdout to binary mode on Windows and disable stdout buffering */
static void init_io(void) {
    /* TODO:
     * - On Windows set binary mode on stdin/stdout (_setmode)
     * - setvbuf(stdout, NULL, _IONBF, 0)
     */
}

/* Read exactly n bytes from stdin into buf, handling short reads */
static int read_exact(uint8_t *buf, size_t n) {
    /* TODO: loop fread until n bytes are read */
    return 0;
}

/* Strip trailing \r and \n */
static void trim_newline(char *s) {
    /* TODO: trim from end */
}

/* Check file extension against already-compressed formats */
static int should_compress(const char *filename) {
    /* TODO: check extension (.zip, .jpg, .mp4, etc.) */
    return 1;
}

/* READ command: read file from disk and write to stdout */
static void cmd_read(const char *path) {
    /* TODO: fopen, ftell for size, read buffer, write "OK <size>\n" + data */
}

/* WRITE command: read size bytes from stdin and save to path */
static void cmd_write(const char *args) {
    /* TODO: parse size & path, read_exact payload, write to file */
}

/* COMPRESS command: compress buffer if worthwhile, otherwise signal RAW */
static void cmd_compress(const char *args) {
    /* TODO: parse filename + size, read payload, run zlib compress2, return COMPRESSED or RAW */
}

/* DECOMPRESS command: decompress payload back to original size */
static void cmd_decompress(const char *args) {
    /* TODO: parse sizes, read compressed data, uncompress with zlib, return OK <size> + data */
}

int main(void) {
    init_io();
    fprintf(stderr, "[backend] ready\n");

    char line[4096];
    while (fgets(line, sizeof(line), stdin)) {
        trim_newline(line);

        if (strncmp(line, "READ ", 5) == 0) {
            cmd_read(line + 5);
        } else if (strncmp(line, "WRITE ", 6) == 0) {
            cmd_write(line + 6);
        } else if (strncmp(line, "COMPRESS ", 9) == 0) {
            cmd_compress(line + 9);
        } else if (strncmp(line, "DECOMPRESS ", 11) == 0) {
            cmd_decompress(line + 11);
        } else if (strcmp(line, "QUIT") == 0) {
            break;
        } else {
            fprintf(stdout, "ERROR unknown command\n");
        }
    }

    fprintf(stderr, "[backend] exiting\n");
    return 0;
}
