#ifndef WAVE_H
#define WAVE_H
#include <stdint.h>

#define RIFF_ID      "RIFF"
#define WAVE_ID      "WAVE"
#define LIST_ID      "LIST"
#define LIST_INFO_ID "INFO"

#define FMT_ID       "fmt "
#define FMT_SIZE     16
#define DATA_ID      "data"

typedef struct {
    int16_t audioFormat;
    int16_t numChannels;
    int16_t blockAlign; 
    int16_t bitsPerSample;

    uint32_t sampleRate; 
    uint32_t byteRate;

    uint32_t dataSize;
    unsigned char *data;
} wave_info_t;

wave_info_t *wave_read_file(const char*);
void wave_free(wave_info_t**);
#endif
