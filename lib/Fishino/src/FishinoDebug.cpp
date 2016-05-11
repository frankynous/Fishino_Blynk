#include "FishinoDebug.h"

#ifdef FISHINO_MALLOC_DEBUG
// debug malloc, to see allocation problems
void *_fishino_malloc(size_t siz, const __FlashStringHelper *file, int line)
{
	Serial << F("Alloc ") << siz << F(" bytes, file:") << file << ", line:" << line << "\n";
	void *ptr = malloc(siz);
	if(!ptr)
		Serial << F("Failed to allocate ") << siz << F(" bytes, file:") << file << ", line:" << line << "\n";
//	Serial.flush();
	return ptr;
}
#endif

