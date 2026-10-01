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

#define PATH_BUFFER 4096

#ifdef _WIN32
  #include <io.h>
  #include <fcntl.h>
#endif

#include <zlib.h>

// ---------------------- done ----------------------
/* Switch stdin/stdout to binary mode on Windows and disable stdout buffering */
static void init_io(void) {
    #ifdef _WIN32
        _setmode(_fileno(stdin),  _O_BINARY);
        _setmode(_fileno(stdout), _O_BINARY);
    #endif
        /* Disable buffering on stdout so responses are sent immediately.
        * Without this, fwrite output might sit in a buffer and Python
        * blocks forever waiting for data that's stuck in libc's buffer. */
        setvbuf(stdout, NULL, _IONBF, 0);
    
}

// ---------------------- done ----------------------
/* Read exactly n bytes from stdin into buf, handling short reads */
static int read_exact(uint8_t *buf, size_t n) {
    while (n > 0) {
        size_t got = fread(buf, 1, n, stdin);  /* reads from stdin pipe */
        if (got == 0) return 0;                /* EOF or pipe broken */
        buf += got;                            /* advance pointer */
        n   -= got;                            /* decrease remaining count */
    }
    return 1;  /* success */
}

// ---------------------- done ----------------------
/* Strip trailing \r and \n */
static void trim_newline(char *s) {
    s[strcspn(s, "\r\n")] = '\0';
}

/* Check file extension against already-compressed formats */
static int should_compress(const char *filename) {
    /* TODO: check extension (.zip, .jpg, .mp4, etc.) */
    return 1;
}

// ---------------------- done ----------------------
/* READ command: read file from disk and write to stdout */
static void cmd_read(const char *path) {
    FILE *fptr;
    uint64_t FileLen;

    fptr = fopen(path, "rb");
    if (!fptr) { fprintf(stdout, "ERROR cannot open file\n"); return; } /*checks for null for file that didnt open correctly*/

    fseek(fptr, 0, SEEK_END); /*goto end*/
    FileLen = ftell(fptr);/*get size*/
    fseek(fptr, 0, SEEK_SET);/*reset to start*/

    uint8_t *buf = (uint8_t *)malloc((size_t)FileLen);/*make buffer*/
    if (!buf) { fprintf(stdout, "ERROR out of memory\n"); fclose(fptr); return; }/*checks for null for a pointer that didnt malloc correctly*/

    size_t got = fread(buf, 1, (size_t)FileLen, fptr); /*read out the binary size out of the file*/
    fclose(fptr);/*file access isnt needed anymore; closing*/

    if (got != FileLen) { /*checks if got isnt equal to the file len. if it isnt, the read didnt go right.*/
        fprintf(stdout, "ERROR incomplete read\n");
        free(buf);
        return;
    }

    fprintf(stdout, "OK %llu\n", (unsigned long long)FileLen); /*printing that everything went okay*/
    fwrite(buf, 1, (size_t)FileLen, stdout);

    free(buf);/*Free the buffer's memory*/
    fprintf(stderr, "[backend] READ %s → %llu bytes\n", path, (unsigned long long)FileLen);/*printout any errors done.*/
}


/* WRITE command: read size bytes from stdin and save to path */
static void cmd_write(const char *args) {
    uint64_t size = 0;
    char *end = NULL;
    size = strtoull(args, &end, 10); /*get file size*/
    while (*end == ' ') end++;

    char path[PATH_BUFFER];
    strncpy(path, end, sizeof(path) - 1); /*copy into path*/
    path[sizeof(path) - 1] = '\0'; /*make into an actual string*/
    trim_newline(path);/*remove newline*/

    uint8_t *buf = (uint8_t *)malloc((size_t)size); /*make buffer*/
    if (!buf) { fprintf(stdout, "ERROR out of memory\n"); return; } /*buffer safety checking*/

    if (!read_exact(buf, (size_t)size)) {
        fprintf(stdout, "ERROR incomplete data\n"); /*read exact binary amount and compare to size to see reading was done proprely*/
        free(buf);
        return;
    }

    FILE *fptr = fopen(path, "wb"); /*open to begin writing binary*/
    if (!fptr) { fprintf(stdout, "ERROR cannot open file\n"); free(buf); return; } /*fptr safety check*/

    size_t written = fwrite(buf, 1, (size_t)size, fptr); /*check if writing was actually done, fwrite returns an int*/
    fclose(fptr); /*close file*/
    free(buf); /*free buffer*/

    if (written != size) { /*if the size of whats written is diffrent from the writing size*/
        fprintf(stdout, "ERROR incomplete write\n");
        return;
    }

    fprintf(stdout, "OK\n"); /*print OK to stdout(py frontend)*/
    fprintf(stderr, "[backend] WRITE %s ← %llu bytes\n", path, (unsigned long long)size);
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
