#include "decodeP.h"
#include "decode.h"
#include <stdlib.h>
#include <stdio.h>

/* initializes structure completely */
jDecoder initDecoder() {
  decoderData* dd = malloc(sizeof(decoderDesc));
  dd->width = 0;
  dd->height =  0;
  
  dd->compressedData = NULL;
  dd->dataSize = 0;
  dd->decompressedData = NULL;
  dd->decompressedDataSize = 0;
  return (jDecoder)dd;
}

/* free all allocated memory */
void destroyDecoder(jDecoder dec){
  decoderData *dd = (decoderData*)dec;
  if(dd->decompressedData)
    free(dd->decompressedData);
  free(dd);
}

#define GetJpegChar() *dec->compressedData++; dec->dataSize--

int Get2Bytes(decoderData* dec) 
{
  int qq = ((int)(*dec->compressedData)) << 8 | (int)(*(dec->compressedData+1));
  dec->compressedData+= 2;
  dec->dataSize-=2;
  return qq;
  
}

static void
SkipVariable (decoderData* dec)
{
    int length;

    length = Get2Bytes (dec) - 2;

    while (length--) {
	GetJpegChar();
    }
}

/* process sof header */
static void GetSof (decoderData* dec,int  code)
{
  int length;
  short ci;
  int c;

  
  length = Get2Bytes (dec);
  
  dec->precision = GetJpegChar();
  dec->height = Get2Bytes (dec);
  dec->width = Get2Bytes (dec);
  dec->numComponents = GetJpegChar();

  
  if (length != (dec->numComponents * 3 + 8)) {
    fprintf (stderr, "Bogus SOF length");
  }
  

  for (ci = 0; ci < dec->numComponents; ci++) {
    (void) GetJpegChar();
    (void)GetJpegChar();
    (void) GetJpegChar();   /* skip component decriptions ... nothing of relevance here */
  }
}


static void GetSos (decoderData* dec)
{
  int length;
  int i, ci, n, c, cc;

  
  length = Get2Bytes (dec);

    /* 
     * Get the number of image components.
     */
    n = GetJpegChar();
    length -= 3;

    if (length != (n * 2 + 3) || n < 1 || n > 4) {
	fprintf (stderr, "Bogus SOS length");
    }


    for (i = 0; i < n; i++) {
	cc = GetJpegChar();
	c = GetJpegChar();
	length -= 2;
	/* just skip what we do not need */
	/*	for (ci = 0; ci < dcPtr->numComponents; ci++)
	    if (cc == dcPtr->compInfo[ci].componentId) {
		break;
	    }

	if (ci >= dcPtr->numComponents) {
	    fprintf (stderr, "Invalid component number in SOS");
	    exit (1);
	}

	compptr = &dcPtr->compInfo[ci];
	dcPtr->curCompInfo[i] = compptr;
	compptr->dcTblNo = (c >> 4) & 15; */
    }

    /*
     * Get the PSV, skip Se, and get the point transform parameter.
     */
    /* make noops , as we do not support predictor and Pt */
    /*dcPtr->Ss = */(void)GetJpegChar(); 
    (void)GetJpegChar();
    c = GetJpegChar(); 
    /* dcPtr->Pt = c & 0x0F;*/
}


static void GetApp0 (decoderData* dec)
{
  int length;
  
  length = Get2Bytes (dec) - 2;
  while (length-- > 0)	/* skip any remaining data */
    (void)GetJpegChar();
}


static void GetDht (decoderData* dec)
{
  int length;
  unsigned char bits[17];
  unsigned char huffval[256];
  int i, index, count,l;
  unsigned short code,codeRoam,codeMask,codeSave;
  unsigned short ehuffco[256],ehuffsi[256];
  int huffsize[257];
  unsigned short huffcode[257];
  int p ,lastp , si;
  length = Get2Bytes (dec) - 2;
  
  while (length) {
    index = GetJpegChar();
    
    bits[0] = 0;
    count = 0;
    for (i = 1; i <= 16; i++) {
      bits[i] = GetJpegChar();
      count += bits[i];
    }
    
    if (count > 256) {
      fprintf (stderr, "Bogus DHT counts");
      exit (1);
    }
    
    for (i = 0; i < count; i++)
      huffval[i] = GetJpegChar();
    
    length -= 1 + 16 + count;
    
    if (index & 0x10) {	/* AC table definition */
      fprintf(stderr,"Huffman table for lossless JPEG is not defined.\n");
    } else {		/* DC table definition */
      /* compose ehuco / ehufsi */
      p = 0;
      for (l = 1; l <= 16; l++) {
        for (i = 1; i <= (int)bits[l]; i++)
	  huffsize[p++] = (char)l;
      }
      huffsize[p] = 0;
      lastp = p;   
   
      code = 0;
      si = huffsize[0];
      p = 0;
      while (huffsize[p]) {
	while (((int)huffsize[p]) == si) {
	  huffcode[p] = code;
	  /* here we produce decompression descritpors */
	  codeSave = codeRoam = code << (8  - huffsize[p]); /* shifts code to up of 8 bit */
	  codeMask = (0xff >> (8 - huffsize[p])) <<(8 - huffsize[p]);
	  for(;(codeRoam &codeMask) == codeSave;codeRoam++) {
	    dec->descr[codeRoam].bitsCode = huffsize[p];
	    dec->descr[codeRoam].bitsSym = huffval[p];
          }
	  p++;
	  code++;

	}
	code <<= 1;
	si++;
      }
    }
        /* ok , compose descriptors for us */
  }
}

int NextMarker (decoderData* dec)
{
  int c, nbytes;
  
  nbytes = 0;
    do {
	/*
	 * skip any non-FF bytes
	 */
	do {
	    nbytes++;
	    c = *dec->compressedData++; dec->dataSize--;
	} while (c != 0xFF);
	/*
	 * skip any duplicate FFs without incrementing nbytes, since
	 * extra FFs are legal
	 */
	do {
	    c = GetJpegChar();
	} while (c == 0xFF);
    } while (c == 0);		/* repeat if it was a stuffed FF/00 */

    return c;
}


char ProcessTables(decoderData* dec) 
{
    char c;

    while (1) {
	c = NextMarker (dec);

	switch (c) {
	case M_SOF0:
	case M_SOF1:
	case M_SOF2:
	case M_SOF3:
	case M_SOF5:
	case M_SOF6:
	case M_SOF7:
	case M_JPG:
	case M_SOF9:
	case M_SOF10:
	case M_SOF11:
	case M_SOF13:
	case M_SOF14:
	case M_SOF15:
	case M_SOI:
	case M_EOI:
	case M_SOS:
	    return ((JpegMarker)c);

	case M_DHT:
	    GetDht (dec);
	    break;

	case M_DQT:
            fprintf(stderr,"Not a lossless JPEG file.\n");
	    break;

	    /*	case M_DRI:
	    GetDri (dec);
	    break;*/

	case M_APP0:
	    GetApp0 (dec);
	    break;

	case M_RST0:		/* these are all parameterless */
	case M_RST1:
	case M_RST2:
	case M_RST3:
	case M_RST4:
	case M_RST5:
	case M_RST6:
	case M_RST7:
	case M_TEM:
	    fprintf (stderr, "Warning: unexpected marker 0x%02x", c);
	    break;

	default:		/* must be DNL, DHP, EXP, APPn, JPGn, COM,
				 * or RESn */
	    SkipVariable (dec);
	    break;
	}
    }
}


/* decode complete frame , setup all what necessary */
int decodeFrame(jDecoder dd, void* dataPtr,int dataSize,void* outPtr) {
  int c , c2;
  void* afterPtr;
  decoderData* dec = (decoderData*)dd;
  dec->compressedData = (char*)dataPtr;
  dec->dataSize = dataSize;

  /* helpfull macro */
  /* start with decoding ... */	
  c =*dec->compressedData++; dec->dataSize--;
  c2 = *dec->compressedData++; dec->dataSize--;
  if((c!= 0xff) || (c2 != M_SOI)) { /* DEMAND SOI ON THE START  */
    fprintf(stderr,"Wrong JPEG data , no SOI ... \n");
    return 0;
  }
  /* process tables */
  c =  ProcessTables(dec);
  
    switch (c) {
    case M_SOF0:
    case M_SOF1:
    case M_SOF3:
	GetSof (dec, c);
	break;

    default:
	fprintf (stderr, "Unsupported SOF marker type 0x%02x", c);
	break;
    }  
    /* at the moment , we got all data about farme , check if we have
       to alloc / realloc decompressed data pointer */

    c = ProcessTables (dec); 
    switch (c) {
    case M_SOS:
      GetSos (dec);
      break;
      
    case M_EOI:
      return 0;
      break;
    default:
      fprintf (stderr, "Unexpected marker 0x%02x", c);
      return 0 ;
      break;
    }
    /*
      we are here , we are just ready to decompress frame ,
      best time to check if everything went OK 
      */
    if(!dec->width || !dec->height){
      fprintf(stderr,"Zero width / height for frame \n");
      return 0;
    }
    /*if(dec->width * dec->height != dec->decompressedDataSize) {
      /* take care to allocate frame storage anew */
    /* if(dec->decompressedData)
	free(dec->decompressedData);
      printf("before malloc\n");
      dec->decompressedData = malloc((size_t)(dec->width * dec->height));
      printf("after malloc\n");
      if(!dec->decompressedData){
	fprintf(stderr,"UNable to malloc storage for the decompressed frame\n");
	return 0;
      }
    }*/
    dec->decompressedData = outPtr;
        afterPtr = decodeScan(dec->width,dec->height,dec->compressedData,dec->decompressedData,
			  dec->descr);
    return 1;
}


