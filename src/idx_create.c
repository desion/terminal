/***
 * this is for idx generate
 */
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "hdict.h"
#include "crc64.h"

static void usage(void) {
    printf("-i     <input path>    the raw file path for generate idx\n"
        "-o     <output path>   generated idx file path\n"
        "-h                     help\n"
        "-b                     binary file\n"
        "-l     <label>         idx label\n"
        "-t     <idx type>      0:the idx key is integer,idx with binary search. 1:idx key is string, idx with hash\n");
    return;
}

static int find_index(const char* str, char c){
    int index = 0;
    while(*str){
        if(c == *str){
            return index;
        }
        index++;
        str++;
    }
    return -1;
}

int main(int argc, char* argv[]){
    int c;
    char *input;
    char *output;
    uint8_t idx_type = 0;
    char* label;
    while ((c = getopt(argc, argv, "i:o:ht:l:")) != -1) {
        switch (c) {
        case 'i':
            input = optarg;
            break;
        case 'o':
            output = optarg;
            break;
        case 'l':
            label = optarg;
            break;
        case 'h':
            usage();
            exit(EXIT_SUCCESS);
        case 't':
            idx_type = 1;
            break;
        default:
            fprintf(stderr, "Illegal argument \"%c\"\n", c);
            return 1;
        }
    }
    meta_t idx_meta;
    idx_t idx_data;
    idx_meta.version = time(NULL);
    idx_meta.idx_type = idx_type;
    strcpy(idx_meta.label, label);
    char buf[4096];
    int index;
    uint64_t crc = crc64(0, &idx_meta, sizeof(meta_t));
    FILE *out_fp = fopen(output, "wb");
    if (NULL == out_fp){
        fprintf(stderr, "Illegal Output file %s\n", output);
        return 1;
    }

    fwrite(&idx_meta, sizeof(idx_meta), 1, out_fp);
    fwrite(&crc, sizeof(uint64_t), 1, out_fp);
    FILE *in_fp;
    //uint64_t id;
    long off;

    in_fp = fopen(input, "r");
    if (NULL == in_fp){
        fprintf(stderr, "Illegal Input file %s\n", input);
        return 1;
    }
    off = ftell(in_fp);
    while(fgets(buf, 4096, in_fp) != NULL){
        index = find_index(buf, ':');
        if (index == -1){
            continue;
        }
        off = off + index + 1;
        idx_data.key = strtoull(buf, NULL, 10);
        idx_data.pos = ((strlen(buf) - index - 1) << 40) | off;
        fwrite(&idx_data, sizeof(idx_t), 1, out_fp);
        off = ftell(in_fp);
    }
    fclose(out_fp);
    fclose(in_fp);
    return 0;
}
