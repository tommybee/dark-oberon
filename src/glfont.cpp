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
 *  @file glfont.cpp
 *
 *  GLFont Library
 *
 *  @author Peter Knut
 *
 *  @date 2002, 2003
 */

#include <string.h>
#include <stdlib.h>

#include "glfont.h"


//========================================================================
// Variables
//========================================================================

bool reset_projection = true;


//========================================================================
// BuildFont - internal function
//========================================================================

void _glfBuildFont(GLFfont *font)  // Build Our Font Display List
{
  int   loop;
  float cx;    // Holds Our X Character Coord
  float cy;    // Holds Our Y Character Coord
  float cwx;   // CharWidth in texture units
  float cwy;   // CharHeight in texture units

  cwx = (1.0f / font->ftWidth) * font->fWidth;
  cwy = (1.0f / font->ftHeight) * font->fHeight;
  font->fBase = glGenLists(font->fxCount * font->fyCount);        // Creating Display Lists
  glBindTexture(GL_TEXTURE_2D, font->fTexture[0]);                // Select Our Font Texture

  for (loop = 0; loop < (font->fxCount * font->fyCount); loop++)  // Loop Through All Lists
  {
    cx = (float)(loop % font->fxCount) * cwx;     // X Position Of Current Character
    cy = (float)(loop / font->fxCount) * cwy;     // Y Position Of Current Character

    glNewList(font->fBase + loop, GL_COMPILE);    // Start Building A List
      glBegin(GL_QUADS);                          // Use A Quad For Each Character
        glTexCoord2f(cx, 1 - cy - cwy);           // Texture Coord (Bottom Left)
        glVertex2i(0, 0);                         // Vertex Coord (Bottom Left)
        glTexCoord2f(cx + cwx, 1 - cy - cwy);     // Texture Coord (Bottom Right)
        glVertex2i(font->fWidth, 0);              // Vertex Coord (Bottom Right)
        glTexCoord2f(cx + cwx, 1 - cy);           // Texture Coord (Top Right)
        glVertex2i(font->fWidth, font->fHeight);  // Vertex Coord (Top Right)
        glTexCoord2f(cx, 1 - cy);                 // Texture Coord (Top Left)
        glVertex2i(0, font->fHeight);             // Vertex Coord (Top Left)
      glEnd();                                    // Done Building Our Quad (Character)
      glTranslated(font->fSpacing, 0, 0);         // Move To The Right Of The Character
    glEndList();                                  // Done Building The Display List
  } // for
}


//========================================================================
// New, Delete font
//========================================================================

GLFfont *glfNewFont(GLuint Tex,
                    int tWidth, int tHeight,
                    int xCount, int yCount,
                    int Width, int Height, int Spacing,
                    int dWidth, int dHeight)
{
  GLFfont *font;

  if (!(font = new GLFfont)) return NULL;

  font->fTexture[0] = Tex;

  font->ftWidth     = tWidth;
  font->ftHeight    = tHeight;

  font->fxCount     = xCount;
  font->fyCount     = yCount;

  font->fWidth      = Width;
  font->fHeight     = Height;
  font->fSpacing    = Spacing; 
  
  font->fdWidth     = dWidth;
  font->fdHeight    = dHeight;

  font->fStartPosFG = 32;
  font->fStartPosBG = 32;

  _glfBuildFont(font);

  return font;
}


void glfDeleteFont(GLFfont *font) // Delete The Font From Memory
{
  glDeleteLists(font->fBase, font->fxCount * font->fyCount);  // Delete Allocated Display Lists
  delete font;
  font = NULL;
}


//========================================================================
// Print
//========================================================================

void glfPrint(GLFfont *font, GLfloat x, GLfloat y, char *string, bool outline)  // Where The Printing Happens
{
  GLfloat color[4];
  int  blend_src, blend_dst;
  GLboolean  enable_blend, enable_depth;

  // blending
  glGetBooleanv(GL_BLEND, &enable_blend);             // pouziva sa blending?
  if (!enable_blend) glEnable(GL_BLEND);              // ak nie, zapne ho
  else {                                              // ak ano, zalohuje funkcie 
    glGetIntegerv(GL_BLEND_SRC, &blend_src);
    glGetIntegerv(GL_BLEND_DST, &blend_dst);
  }

  // dept testing
  glGetBooleanv(GL_DEPTH_TEST, &enable_depth);        // pouziva sa dept test?
  glDisable(GL_DEPTH_TEST);                           // Disables Depth Testing

  glBindTexture(GL_TEXTURE_2D, font->fTexture[0]);    // Select Our Font Texture
  
  if (reset_projection) {
    // ulozenie aktualnych matic
    glMatrixMode(GL_PROJECTION);                      // Select The Projection Matrix
    glPushMatrix();                                   // Store The Projection Matrix
    glLoadIdentity();                                 // Reset The Projection Matrix
    glOrtho(0, font->fdWidth, 
            0, font->fdHeight,
            -1, 1);                                   // Set Up An Ortho Screen

    glMatrixMode(GL_MODELVIEW);                       // Select The Modelview Matrix
    glPushMatrix();                                   // Store The Modelview Matrix
    glLoadIdentity();
  }
  else {
    glMatrixMode(GL_MODELVIEW);                       // Select The Modelview Matrix
    glPushMatrix();                                   // Store The Modelview Matrix
  }

  glGetFloatv(GL_CURRENT_COLOR, color);               // ulozi povodnu farbu  
  glTranslatef(x, y, 0);                              // Position The Text (0,0 - Bottom Left)  

  // vykresli sa cierna silueta
  if (outline) glListBase(font->fBase - font->fStartPosBG);
  else glListBase(font->fBase - font->fStartPosFG);
  
  glColor3f(1.0f, 1.0f, 1.0f);
  
  glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

  glPushMatrix();
  glCallLists(strlen(string), GL_BYTE, string);
  glPopMatrix();

  // vykresli sa text
  glColor3f(color[0], color[1], color[2]);            // nastavi farbu na povodnu
  glListBase(font->fBase - font->fStartPosFG);        // Choose The Font Set (0 or 1)
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
  glCallLists(strlen(string), GL_BYTE, string);       // Write The Text To The Screen

  // navrat povodnych matic
  if (reset_projection) {
    glMatrixMode(GL_PROJECTION);                      // Select The Projection Matrix
    glPopMatrix();                                    // Restore The Old Projection Matrix
    glMatrixMode(GL_MODELVIEW);                       // Select The Modelview Matrix
    glPopMatrix();                                    // Restore The Old Projection Matrix
  }
  else {
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();                                    // Restore The Old Projection Matrix
  }

  if (enable_depth) glEnable(GL_DEPTH_TEST);          // Enables Depth Testing
  if (!enable_blend) glDisable(GL_BLEND);             // vrati povodny stav
  else glBlendFunc(blend_src, blend_dst);             // vrati povodne funkcie
}


//========================================================================
// Font settings
//========================================================================

void glfSetFontBase(GLFfont *font, int BaseFG, int BaseBG)
{
  font->fStartPosFG = BaseFG;
  font->fStartPosBG = BaseBG;
}


void glfSetFontDisplayMode(GLFfont *font, int dWidth, int dHeight)
{
  font->fdWidth  = dWidth;
  font->fdHeight = dHeight;
}


void glfSetEnable(int flag, bool enable)
{
  switch (flag) {
  case GLF_RESET_PROJECTION:
    reset_projection = enable;
    break;
  }
}


void glfEnable(int flag)
{
  glfSetEnable(flag, true);
}


void glfDisable(int flag)
{
  glfSetEnable(flag, false);
}


//========================================================================
// END
//========================================================================
// vim:ts=2:sw=2:et:

