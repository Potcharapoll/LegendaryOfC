#include "wave.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

wave_info_t *wave_read_file(const char *file_path) {
  FILE *fp;
  char id[4];
  uint32_t chunkSize;
  wave_info_t *wave_info;

  fp = fopen(file_path, "rb");
  if (fp == NULL) abort();

  wave_info = malloc(sizeof(*wave_info));
  
  fread(id, 1, 4, fp);
  if (strcmp(id, RIFF_ID) != 0)  {
    fprintf(stderr, "Invalid ID (RIFF) [%s]\n", id);
    abort();
  }

  fread(&chunkSize, 4, 1, fp);

  fread(id, 1, 4, fp);
  if (strcmp(id, WAVE_ID) != 0)  {
    fprintf(stderr, "Invalid ID (WAVE) [%s]\n", id);
    abort();
  }

  fread(id, 1, 4, fp);
  if (strcmp(id, FMT_ID) != 0)  {
    fprintf(stderr, "Invalid ID (fmt ) [%s]\n", id);
    abort();
  }

  fread(&chunkSize, 4, 1, fp);

  fread(&wave_info->audioFormat, 2, 1, fp);
  fread(&wave_info->numChannels, 2, 1, fp);
  fread(&wave_info->sampleRate, 4, 1, fp);
  fread(&wave_info->byteRate, 4, 1, fp);
  fread(&wave_info->blockAlign, 2, 1, fp);
  fread(&wave_info->bitsPerSample, 2, 1, fp);

  fread(id, 1, 4, fp);
  if (strcmp(id, LIST_ID) == 0)  {
    fread(&chunkSize, 4, 1, fp);
    fseek(fp, chunkSize, SEEK_CUR);
  }

  fread(id, 1, 4, fp);
  if (strcmp(id, DATA_ID) != 0)  {
    fprintf(stderr, "Invalid ID (data) [%s]\n", id);
    abort();
  }

  fread(&chunkSize, 4, 1, fp);

  wave_info->dataSize = chunkSize;
  wave_info->data     = malloc(chunkSize);

  fread(wave_info->data, 1, chunkSize, fp);

  fclose(fp);

  return wave_info;
}

void wave_free(wave_info_t **self) {
  free((*self)->data);

  free(*self);
  *self = NULL;
}

