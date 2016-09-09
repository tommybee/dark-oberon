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
 *  @file tga.h
 *
 *  TGA Loader Library
 *
 *  @author Peter Knut
 *  @date 2002, 2003
 *
 *  @author Marcus Geelnard
 *  @date (c) ????
 */

#ifndef __tga_h_
#define __tga_h_


#include <stdio.h>

/**
 *  Valid pixel format.
 */
typedef enum {
  TGA_PIXFMT_GRAY,    //!< Grayscale pixel format.
  TGA_PIXFMT_RGB,     //!< RGB pixel format.
  TGA_PIXFMT_RGBA     //!< RGB-Alpha pixel format.
} EPixFormat;


/**
 *  TGA image file information.
 */
typedef struct {
  int width;              //!< Width of the image.
  int height;             //!< Height of the image.
  int original_width;     //!< Original width of the image. Different from #width when #TGA_RESCALE flag is set.
  int original_height;    //!< Original height of the image. Different from #height when #TGA_RESCALE flag is set.
  EPixFormat pixformat;   //!< Pixels' format.
  int bytesperpixel;      //!< Bytes per one pixel.
  unsigned char *data;    //!< Pointer to image data.
} TGA_INFO;


//========================================================================
// Flags for TGA_Read
//========================================================================

/**
 *  Forces tgaRead() to rescale images to closest larger @c 2^Nx2^M resolution.
 */
#define TGA_RESCALE   0x00000001

/**
 *  Forces the first pixel of the image to be the upper left corner of the
 *  image (default is the lower left corner of the image).
 */
#define TGA_ORIGIN_UL 0x00000004


//========================================================================
// Prototypes
//========================================================================

int tgaRead(FILE *f, TGA_INFO *t, int flags);

#endif // __tga_h_


//========================================================================
// END
//========================================================================
// vim:ts=2:sw=2:et:

