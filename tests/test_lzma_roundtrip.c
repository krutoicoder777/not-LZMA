#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include "lzma.h"

static double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static int compare_files(const char *f1, const char *f2) {
    FILE *a = fopen(f1, "rb");
    FILE *b = fopen(f2, "rb");
    if (!a || !b) { if(a) fclose(a); if(b) fclose(b); return 1; }
    
    int result = 0;
    while (1) {
        int ca = fgetc(a);
        int cb = fgetc(b);
        if (ca != cb) { result = 1; break; }
        if (ca == EOF) break;
    }
    fclose(a);
    fclose(b);
    return result;
}

static void test_file(const char *dir, const char *filename) {
    char in_path[512];
    char comp_path[512];
    char restored_path[512];
    
    snprintf(in_path, sizeof(in_path), "%s/%s", dir, filename);
    snprintf(comp_path, sizeof(comp_path), "%s/%s.lzma", dir, filename);
    snprintf(restored_path, sizeof(restored_path), "%s/%s.restored", dir, filename);

    FILE *in = fopen(in_path, "rb");
    if (!in) { printf("  [SKIP] Cannot open %s\n", in_path); return; }
    
    fseek(in, 0, SEEK_END);
    uint64_t original_size = ftell(in);
    fseek(in, 0, SEEK_SET);

    if (original_size == 0) {
        fclose(in);
        printf("  [SKIP] %s (empty file)\n", filename);
        return;
    }

    FILE *comp = fopen(comp_path, "wb");
    if (!comp) { fclose(in); return; }

    double start = get_time();
    fwrite(&original_size, sizeof(original_size), 1, comp);
    int res_enc = lzma_encode_stream(in, comp);
    double end = get_time();
    double comp_time = end - start;
    
    fclose(in);
    fclose(comp);

    if (res_enc != LZMA_OK) {
        printf("  [FAIL] %s: encode error %d\n", filename, res_enc);
        remove(comp_path);
        return;
    }

    comp = fopen(comp_path, "rb");
    fseek(comp, 0, SEEK_END);
    uint64_t compressed_size = ftell(comp);
    fclose(comp);

    comp = fopen(comp_path, "rb");
    FILE *restored = fopen(restored_path, "wb");
    
    uint64_t header_size;
    fread(&header_size, sizeof(header_size), 1, comp);
    
    start = get_time();
    int res_dec = lzma_decode_stream(comp, restored, header_size);
    end = get_time();
    double decomp_time = end - start;
    
    fclose(comp);
    fclose(restored);

    if (res_dec != LZMA_OK) {
        printf("  [FAIL] %s: decode error %d\n", filename, res_dec);
        remove(comp_path);
        remove(restored_path);
        return;
    }

    int diff = compare_files(in_path, restored_path);

    double ratio = (double)compressed_size / original_size * 100.0;
    printf("  [%s] %s: %llu B -> %llu B (%.1f%%) | C: %.3fs, D: %.3fs\n",
           diff == 0 ? "OK" : "FAIL", filename, 
           (unsigned long long)original_size, 
           (unsigned long long)compressed_size,
           ratio, comp_time, decomp_time);

    remove(comp_path);
    remove(restored_path);
}

int main(void) {
    const char *data_dir = "tests/data"; 
    
    DIR *dir = opendir(data_dir);
    if (!dir) {
        printf("Error: cannot open directory '%s'\n", data_dir);
        return 1;
    }

    printf("=== LZMA ROUNDTRIP TEST ===\n");
    printf("Directory: %s\n\n", data_dir);

    struct dirent *entry;
    int total = 0;
    int failed = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", data_dir, entry->d_name);
        struct stat st;
        if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) {
            test_file(data_dir, entry->d_name);
            total++;
        }
    }
    closedir(dir);

    printf("\n=== TESTS COMPLETED ===\n");
    return 0;
}
