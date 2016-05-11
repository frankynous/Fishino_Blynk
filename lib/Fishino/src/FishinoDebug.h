#ifndef __FISHINO_DEBUG_H
#define __FISHINO_DEBUG_H

#include <Arduino.h>

#include <Flash.h>

// #define FISHINO_MALLOC_DEBUG

// debug malloc support
#ifdef FISHINO_MALLOC_DEBUG
#define FISHINO_MALLOC(siz) _fishino_malloc(siz, F(FISHINO_MODULE), __LINE__)

void *_fishino_malloc(size_t siz, const __FlashStringHelper *file, int line);

#else
#define FISHINO_MALLOC(siz) malloc(siz)
#endif

#endif