#ifndef DECODE_H
#define DECODE_H

/* opaque pointer to internal data */
typedef void* jDecoder;

/* create and destroy decoder struct */
jDecoder initDecoder();
void destroyDecoder(jDecoder);

/* decodes single frame , returns 0 on fail */
/* data to store decompressed image will allocated internally */
int decodeFrame(jDecoder,
                void*, /* data ptr */
		int, /* data size */
		void*  /* decompressed data pointer , must be big enough */
		);

void* getDataPtr(jDecoder); /* returns pointer to decompressed data structure */
/* return dimensions of decompressed image */
int getWidth(jDecoder);
int getWeight(jDecoder);



#endif /* DECODE_H*/
