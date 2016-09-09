/*
 * -------------
 *  Dark Oberon
 * -------------
 * 
 * An advanced strategy game.
 *
 * Copyright (C) 2002 - 2005 Valeria Sventova, Jiri Krejsa, Peter Knut,
 *                           Martin Kosalko, Marian Cerny, Michal Kral
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (see docs/gpl.txt) as
 * published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 */

/**
 *  @file tga.cpp
 *
 *  TGA Loader Library
 *
 *  This library supports version 1 Targa images, with these restrictions:
 *    - Pixel format may only be 8, 24 or 32 bits
 *    - Colormaps must be no longer than 256 entries
 *
 *  @author Peter Knut
 *  @date 2002, 2003
 *  
 *  @author Marcus Geelnard
 *  @date (c) ????
 */

#include <stdlib.h>
#include <stdio.h>

#include "tga.h"

/**
 *  TGA file header information.
 *
 *  @todo mozno by chcelo niekde zistit, ktora zlozka co znamena...
 */
typedef struct {
  int idlen;                //!< 1 byte
  int cmaptype;             //!< Specifies, if there is a colormap. (1 byte)
  int imagetype;            //!< Specifies type of the image. (1 byte)
  int cmapfirstidx;         //!< 2 bytes
  int cmaplen;              //!< 2 bytes
  int cmapentrysize;        //!< 1 byte
  int xorigin;              //!< 2 bytes
  int yorigin;              //!< 2 bytes
  int width;                //!< 2 bytes
  int height;               //!< 2 bytes
  int bitsperpixel;         //!< 1 byte
  int imageinfo;            //!< 1 byte

  int _alphabits;           //!< (derived from imageinfo)
  int _origin;              //!< (derived from imageinfo)
} TGA_HEADER;


#define TGA_CMAPTYPE_NONE      0      //!< Value of TGA_HEADER::cmaptype, when there is no colormap present.
#define TGA_CMAPTYPE_PRESENT   1      //!< Value of TGA_HEADER::cmaptype, when there is a colormap present.

#define TGA_IMAGETYPE_NONE     0      //!< Value of TGA_HEADER::imagetype. @todo okomentovat vsetky TGA_IMAGETYPE_*
#define TGA_IMAGETYPE_CMAP     1      //!< Value of TGA_HEADER::imagetype.
#define TGA_IMAGETYPE_TC       2      //!< Value of TGA_HEADER::imagetype.
#define TGA_IMAGETYPE_GRAY     3      //!< Value of TGA_HEADER::imagetype.
#define TGA_IMAGETYPE_CMAP_RLE 9      //!< Value of TGA_HEADER::imagetype.
#define TGA_IMAGETYPE_TC_RLE   10     //!< Value of TGA_HEADER::imagetype.
#define TGA_IMAGETYPE_GRAY_RLE 11     //!< Value of TGA_HEADER::imagetype.

#define TGA_IMAGEINFO_ALPHA_MASK   0x0f    //!< Mask to get TGA_HEADER::_alphabits from TGA_HEADER::imageinfo;
#define TGA_IMAGEINFO_ALPHA_SHIFT  0       //!< Shift to get TGA_HEADER::_alphabits from TGA_HEADER::imageinfo;
#define TGA_IMAGEINFO_ORIGIN_MASK  0x30    //!< Mask to get TGA_HEADER::_origin from TGA_HEADER::imageinfo;
#define TGA_IMAGEINFO_ORIGIN_SHIFT 4       //!< Shift to get TGA_HEADER::_origin from TGA_HEADER::imageinfo;

#define _TGA_ORIGIN_BL 0      //!< Bottom left image origin.
#define _TGA_ORIGIN_BR 1      //!< Bottom right image origin.
#define _TGA_ORIGIN_UL 2      //!< Upper left image origin.
#define _TGA_ORIGIN_UR 3      //!< Upper right image origin.


/**
 *  Reads a TGA file header and checks that it is valid.
 *
 *  @return @c 1 when the TGA header was valid, @c 0 otherwise.
 */
static int tgaReadHeader(FILE *f, TGA_HEADER *h)
{
  unsigned char buf[18];
  int pos;

  // Read TGA file header from file
  pos = ftell(f);
  fread(buf, 18, 1, f);

  // Interpret header (endian independent parsing)
  h->idlen         = (int)buf[0];
  h->cmaptype      = (int)buf[1];
  h->imagetype     = (int)buf[2];
  h->cmapfirstidx  = (int)buf[3]  | (((int)buf[4]) << 8);
  h->cmaplen       = (int)buf[5]  | (((int)buf[6]) << 8);
  h->cmapentrysize = (int)buf[7];
  h->xorigin       = (int)buf[8]  | (((int)buf[9]) << 8);
  h->yorigin       = (int)buf[10] | (((int)buf[11]) << 8);
  h->width         = (int)buf[12] | (((int)buf[13]) << 8);
  h->height        = (int)buf[14] | (((int)buf[15]) << 8);
  h->bitsperpixel  = (int)buf[16];
  h->imageinfo     = (int)buf[17];

  // Extract alphabits and origin information
  h->_alphabits = (int)(h->imageinfo & TGA_IMAGEINFO_ALPHA_MASK) >>
                   TGA_IMAGEINFO_ALPHA_SHIFT;
  h->_origin    = (int)(h->imageinfo & TGA_IMAGEINFO_ORIGIN_MASK) >>
                   TGA_IMAGEINFO_ORIGIN_SHIFT;

  // Validate TGA header (is this a TGA file?)
  if ((h->cmaptype == 0 || h->cmaptype == 1) &&
      ((h->imagetype >= 1 && h->imagetype <= 3) || (h->imagetype >= 9 && h->imagetype <= 11)) &&
      (h->bitsperpixel == 8 || h->bitsperpixel == 24 || h->bitsperpixel == 32))
  {
    // Skip the ID field
    fseek( f, h->idlen, SEEK_CUR );
    // Indicate that the TGA header was valid
    return 1;
  }
  else {
    // Restore file position
    fseek( f, pos, SEEK_SET );

    // Indicate that the TGA header was invalid
    return 0;
  }
}


/**
 *  Reads Run-Length Encoded data.
 */
static void tgaReadRLE(unsigned char *buf, int size, int bpp, FILE *f)
{
  int repcount, bytes, k, n;
  unsigned char pixel[ 4 ];

  // Dummy check
  if (bpp > 4) return;

  while (size > 0) {
    // Get repetition count
    repcount = (unsigned int)fgetc(f);
    bytes = ((repcount & 127) + 1) * bpp;
    if (size < bytes) bytes = size;

    // Run-Length packet?
    if (repcount & 128) {
      for (k = 0; k < bpp; k ++)
        pixel[ k ] = (unsigned char)fgetc( f );
      
      for (n = 0; n < (repcount & 127) + 1; n++)
        for (k = 0; k < bpp; k++)
          *buf++ = pixel[k];
    }
    else {
      // It's a Raw packet
      fread( buf, bytes, 1, f );
      buf += bytes;
    }

    size -= bytes;
  } // while
}


/**
 *  Reads a TGA image from a file.
 *
 *  @param f      File to read the TGA image from.
 *  @param t      Pointer to information structure which will be filled by the
 *                function.
 *  @param flags  Flags. Combination of #TGA_RESCALE and #TGA_ORIGIN_UL.
 *
 *  @return @c 1 on success, @c 0 otherwise.
 */
int tgaRead(FILE *f, TGA_INFO *t, int flags)
{
  TGA_HEADER h;
  unsigned char *cmap, *pix, *data, tmp;
  int cmapsize, pixsize, pixsize2, datasize;
  int width, height, log2, bpp, bpp2, k, m, n, swapx, swapy;

  // Read TGA header
  if (!tgaReadHeader(f, &h)) return 0;

  // Is there a colormap?
  cmapsize = (h.cmaptype == TGA_CMAPTYPE_PRESENT ? 1 : 0) * h.cmaplen *
             ((h.cmapentrysize+7) / 8);

  if (cmapsize > 0) {
    // Is it a colormap that we can handle?
    if ((h.cmapentrysize != 24 && h.cmapentrysize != 32) ||
        h.cmaplen == 0 || h.cmaplen > 256 ) return 0;
    
    // Allocate memory for colormap
    cmap = (unsigned char *)malloc(cmapsize);
    if (cmap == NULL) return 0;

    // Read colormap from file
    fread (cmap, cmapsize, 1, f);
  }
  else cmap = NULL;

  // Size of pixel data
  pixsize = h.width * h.height * ((h.bitsperpixel + 7) / 8);

  // Bytes per pixel (pixel data - unexpanded)
  bpp = (h.bitsperpixel + 7) / 8;

  // Bytes per pixel (expanded pixels - not colormap indeces)
  if (cmap) bpp2 = (h.cmapentrysize + 7) / 8;
  else bpp2 = bpp;
  
  // For colormaped images, the RGB/RGBA image data may use more memory
  // than the stored pixel data
  pixsize2 = h.width * h.height * bpp2;

  // Allocate memory for pixel data
  pix = (unsigned char *)malloc(pixsize2);
  if (pix == NULL) {
    if (cmap) free(cmap);
    return 0;
  }

  // Read pixel data from file
  if (h.imagetype >= TGA_IMAGETYPE_CMAP_RLE) tgaReadRLE(pix, pixsize, bpp, f);
  else fread( pix, pixsize, 1, f );
    
  // If the image origin is not bottom left, re-arrange the pixels
  switch (h._origin) {
  default:
  case _TGA_ORIGIN_UL:
    swapx = 0;
    swapy = 1;
    break;

  case _TGA_ORIGIN_BL:
    swapx = 0;
    swapy = 0;
    break;

  case _TGA_ORIGIN_UR:
    swapx = 1;
    swapy = 1;
    break;

  case _TGA_ORIGIN_BR:
    swapx = 1;
    swapy = 0;
    break;
  } // switch

  if ((swapy && !(flags & TGA_ORIGIN_UL)) || (!swapy && (flags & TGA_ORIGIN_UL))) {
    for (n = 0; n < h.height / 2; n++)
      for (m = 0; m < h.width; m++)
        for (k = 0; k < bpp; k++) {
          tmp = pix[(n * h.width + m) * bpp + k];
          pix[(n * h.width + m) * bpp + k] = pix[((h.height - 1 - n) * h.width + m) * bpp + k];
          pix[((h.height - 1 - n) * h.width+m) * bpp + k ] = tmp;
        }
  }

  if (swapx) {
    for (n = 0; n < h.height; n++)
      for (m = 0; m < h.width / 2; m++)
        for (k = 0; k < bpp; k++) {
          tmp = pix[ (n*h.width+m)*bpp + k ];
          pix[ (n*h.width+m)*bpp + k ] = pix[ (n*h.width+h.width-1-m)*bpp + k ];
          pix[ (n*h.width+h.width-1-m)*bpp + k ] = tmp;
        }
  }

  // If we have a colormap, convert pixel data to image (BGR/BGRA) data
  if (cmap) {
    for (m = h.width * h.height - 1; m >= 0; m--) {
      n = pix[m];
      for (k = 0; k < bpp2; k++) pix[ m*bpp2 + k ] = cmap[ n*bpp2 + k ];
    }
  }

  // convert BGR to RGB
  for (n = 0; n < h.height * h.width * bpp2; n += bpp2) {
    tmp = pix[n];
    pix[n] = pix[n + 2];
    pix[n + 2] = tmp;
  }

  // Is the TGA_RESCALE flag set?
  if (flags & TGA_RESCALE) {
    // Calculate next larger 2^N width
    for (log2 = 0, width = h.width; width > 1; width >>= 1, log2++);
    width  = (int)1 << log2;
    if (width < h.width) width <<= 1;

    // Calculate next larger 2^M height
    for (log2 = 0, height = h.height; height > 1; height >>= 1, log2++);
    height = (int)1 << log2;
    if (height < h.height) height <<= 1;
  }
  else {
    width  = h.width;
    height = h.height;
  }

  // Do we need to rescale?
  if (width != h.width || height != h.height) {
    // Allocate memory for final image data
    datasize = width * height * bpp2;
    data = (unsigned char *)malloc(datasize);

    if (data == NULL) {
      free( pix );
      if (cmap) free( cmap );
      return 0;
    }

    // clear new data (black color, alpha 0)
    for (n = 0; n < datasize; n++) data[n] = 0;

    // copy pixel data
    for (m = 0; m < h.height; m++)
      for (n = 0; n < h.width * bpp2; n++) {
        data[(m)* (width * bpp2) + n] = pix[m * (h.width * bpp2) + n];
      }

    // Set pointer to image data in TGA information
    t->data = data;

    // Free memory for pixel data
    free( pix );
  }
  // No extra conversion is needed (image data = pixel data)
  else t->data = pix;

  // Set remaining TGA information
  t->width         = width;
  t->height        = height;
  t->original_width = h.width;
  t->original_height = h.height;
  t->bytesperpixel = bpp2;

  switch (bpp2) {
  default:
  case 1: t->pixformat = TGA_PIXFMT_GRAY; break;
  case 3: t->pixformat = TGA_PIXFMT_RGB; break;
  case 4: t->pixformat = TGA_PIXFMT_RGBA; break;
  }

  // Free memory for colormap
  if (cmap) free( cmap );

  return 1;
}


//========================================================================
// END
//========================================================================
// vim:ts=2:sw=2:et:

