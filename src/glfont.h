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
 *  @file glfont.h
 *
 *  GLFont Library
 *
 *  @author Peter Knut
 *
 *  @date 2002
 *
 *  @todo Je potrebne okomentovat vsetko v glfont.cpp a glfont.h.
 */

#ifndef __glfont_h_
#define __glfont_h_

//========================================================================
// Defines
//========================================================================

#define GLF_RESET_PROJECTION    1

//========================================================================
// Included files
//========================================================================

#ifdef NEW_GLFW3
#include <glfw3.h>
#else
#include <glfw.h>
#endif


//========================================================================
// Font information
//========================================================================

typedef struct {
  GLuint fTexture[1];
  GLuint fBase;
  
  int    ftWidth;
  int    ftHeight;
  
  int    fxCount;
  int    fyCount;

  int    fWidth;
  int    fHeight;
  int    fSpacing;

  int    fdWidth;
  int    fdHeight;  

  int    fStartPosFG;
  int    fStartPosBG;
} GLFfont;


//========================================================================
// New, delete font
//========================================================================

GLFfont *glfNewFont(GLuint Tex,
                    int tWidth, int tHeight,
                    int xCount, int yCount,
                    int Width,  int Height,   int Spacing,
                    int dWidth, int dHeight);

void glfDeleteFont(GLFfont *font);


//========================================================================
// Font settings
//========================================================================

void glfSetFontBase(GLFfont *font, int BaseFG, int BaseBG);
void glfSetFontDisplayMode(GLFfont *font, int dWidth, int dHeight);


void glfDisable(int flag);
void glfEnable(int flag);


//========================================================================
// Printing
//========================================================================

void glfPrint(GLFfont *font, GLfloat x, GLfloat y, char *string, bool outline);


#endif // __glfont_h_

//========================================================================
// END
//========================================================================
// vim:ts=2:sw=2:et:

