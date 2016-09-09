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
 *  @file glgui.cpp        
 *
 *  Game interface classes, include drawing diferent types of windows (panels,
 *  buttons).
 *
 *  @author Peter Knut
 *
 *  @date 2003, 2004
 */


#include <stdlib.h>

#include "glgui.h"


//=========================================================================
// Definitions
//=========================================================================

#ifndef T_BYTE
#define T_BYTE unsigned char
#endif

#define GUI_MAX_CLIP_LEVEL  32
#define GUI_CLIP_EPSILON    0.1f

#define SET_COLOR(color, r, g, b) \
  do { \
    if (!color) color = GUI_NEW GLfloat[3]; \
    color[0] = r; color[1] = g; color[2] = b; \
  } while(0)


//=========================================================================
// Variables
//=========================================================================

TGUI *TGUI::self = NULL;

TGUI_BOX *TGUI::hover_box = NULL;
TGUI_BOX *TGUI::down_box = NULL;
TGUI_BOX *TGUI::focus_box = NULL;
TGUI_BOX *TGUI::modal_box = NULL;
TGUI_BOX *TGUI::float_box = NULL;
TGUI_BOX *TGUI::tooltip_box = NULL;

double hover_time = 0.0;

bool show_tooltip = true;
bool hover_box_hide = false; // :(((
bool rescan_mouse = false;

GLfloat clip_buffer[GUI_MAX_CLIP_LEVEL][4];
int clip_level = 0;

// clip planes equations - left, right, bottom, top
GLdouble clip_eqn[4][4] = {
  { 1,  0,  0, 0 },
  { -1, 0,  0, 0 },
  { 0,  1,  0, 0 },
  { 0,  -1, 0, 0 }
};


//=========================================================================
// Usefull functions
//=========================================================================

int TranslateKey(int key)   // translate numpad keys and combinations of keys to standart chars
{
  if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_9) key += '0' - GLFW_KEY_KP_0;
  else switch (key) {
    // numpad keys
    case GLFW_KEY_KP_MULTIPLY: key = '*'; break;
    case GLFW_KEY_KP_ADD:      key = '+'; break;
    case GLFW_KEY_KP_SUBTRACT: key = '-'; break;
    case GLFW_KEY_KP_DECIMAL:  key = '.'; break;
    case GLFW_KEY_KP_DIVIDE:   key = '/'; break;
    default:
      // small letters
      if (!glfwGetKey(GLFW_KEY_LSHIFT) && !glfwGetKey(GLFW_KEY_RSHIFT)) {
        if (key >= 'A' && key <='Z') key += 'a' - 'A';
      }
      // SHIFT + key
      else switch (key) {
        case '`':   key = '~'; break;
        case '1':   key = '!'; break;
        case '2':   key = '@'; break;
        case '3':   key = '#'; break;
        case '4':   key = '$'; break;
        case '5':   key = '%'; break;
        case '6':   key = '^'; break;
        case '7':   key = '&'; break;
        case '8':   key = '*'; break;
        case '9':   key = '('; break;
        case '0':   key = ')'; break;

        case ',':   key = '<'; break;
        case '.':   key = '>'; break;
        case '\'':  key = '"'; break;
        case '-':   key = '_'; break;
        case '=':   key = '+'; break;
        case '[':   key = '{'; break;
        case ']':   key = '}'; break;
        case ';':   key = ':'; break;
        case '/':   key = '?'; break;
        case '\\':  key = '|'; break;
      } // switch (key)
      break;
  }

  return key;
}


void DrawFaceRectangle(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLfloat *fcolor, GLfloat alpha, bool border, bool inverse)
{  
  glDisable(GL_TEXTURE_2D);

  // base rectangle
  glColor4f(fcolor[0], fcolor[1], fcolor[2], alpha);
  glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x, y + height);
    glVertex2f(x + width, y + height);
    glVertex2f(x + width, y);
  glEnd();

  if (border) {
    if (inverse) glColor4f(fcolor[0] * GUI_LIGHT_COEF, fcolor[1] * GUI_LIGHT_COEF, fcolor[2] * GUI_LIGHT_COEF, alpha);
    else glColor4f(fcolor[0] / GUI_LIGHT_COEF, fcolor[1] / GUI_LIGHT_COEF, fcolor[2] / GUI_LIGHT_COEF, alpha);

    glBegin(GL_LINE_STRIP);
      glVertex2f(x, y);
      glVertex2f(x + width - 1, y);
      glVertex2f(x + width - 1, y + height);
    glEnd();

    if (inverse) glColor4f(fcolor[0] / GUI_LIGHT_COEF, fcolor[1] / GUI_LIGHT_COEF, fcolor[2] / GUI_LIGHT_COEF, alpha);
    else glColor4f(fcolor[0] * GUI_LIGHT_COEF, fcolor[1] * GUI_LIGHT_COEF, fcolor[2] * GUI_LIGHT_COEF, alpha);

    glBegin(GL_LINE_STRIP);
      glVertex2f(x, y);
      glVertex2f(x, y + height - 1);
      glVertex2f(x + width, y + height - 1);
    glEnd();
  }

  glEnable(GL_TEXTURE_2D);
}


void DrawBgRectangle(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLfloat *fcolor, GLfloat *bgcolor, GLfloat alpha, bool border)
{  
  glDisable(GL_TEXTURE_2D);

  // base rectangle
  if (bgcolor) glColor4f(bgcolor[0], bgcolor[1], bgcolor[2], alpha);
  else glColor4f(1, 1, 1, alpha);

  glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x, y + height);
    glVertex2f(x + width, y + height);
    glVertex2f(x + width, y);
  glEnd();

  if (border) {
    glColor4f(fcolor[0] * GUI_LIGHT_COEF, fcolor[1] * GUI_LIGHT_COEF, fcolor[2] * GUI_LIGHT_COEF, alpha);

    glBegin(GL_LINE_STRIP);
      glVertex2f(x, y);
      glVertex2f(x + width - 1, y);
      glVertex2f(x + width - 1, y + height);
    glEnd();

    glColor4f(fcolor[0] / GUI_LIGHT_COEF, fcolor[1] / GUI_LIGHT_COEF, fcolor[2] / GUI_LIGHT_COEF, alpha);

    glBegin(GL_LINE_STRIP);
      glVertex2f(x, y);
      glVertex2f(x, y + height - 1);
      glVertex2f(x + width, y + height - 1);
    glEnd();
  }

  glEnable(GL_TEXTURE_2D);
}


void SetHoverBox(TGUI_BOX *box)
{
  if (TGUI::hover_box == box) return;

  TGUI::hover_box = box;
  hover_time = 0.0;
  show_tooltip = true;

  if (TGUI::tooltip_box) TGUI::tooltip_box = NULL;
}


//=========================================================================
// TGUI_TEXTURE
//=========================================================================

void TGUI_TEXTURE::DrawFrame(int frame, GLfloat w, GLfloat h)
{
  float fvwidth = (float)frame_width / width;    // frame virtual width in texture
  float fvheight = (float)frame_height / height; // frame virtual height in texture
  float fvhalfpix = 0.5f / height;                   // frame virtual size of half of pixel
  float fvx;        // frame virtual x position in texture
  float fvy;        // frame virtual y position in texture

  fvx = fvwidth * (frame % h_count);
  fvy = fvheight * (v_count - (frame / h_count) - 1);

  glBindTexture(GL_TEXTURE_2D, gl_id);

  glPushMatrix();

  switch (type) {
  case GUI_TT_RHOMBUS:
    glTranslated(-w/2, 0.0, 0.0);

    glBegin(GL_QUADS);
      // bottom
      glTexCoord2f(fvx + fvwidth/2, fvy + fvhalfpix);
      glVertex2f(w/2, 0);
      // right
      glTexCoord2f(fvx + fvwidth, fvy + fvheight/2);
      glVertex2f(w, h/2 - 0.5f);
      // top
      glTexCoord2f(fvx + fvwidth/2, fvy + fvheight - fvhalfpix);
      glVertex2f(w/2, h - 1);
      // left
      glTexCoord2f(fvx, fvy + fvheight/2);
      glVertex2f(0, h/2 - 0.5f);
    glEnd();
    break;

  case GUI_TT_NORMAL:
  default:
    glTranslated(point_x, point_y, 0.0);

    glBegin(GL_QUADS);
      // bottom left
      glTexCoord2f(fvx, fvy);
      glVertex2f(0, 0);
      // bottom right
      glTexCoord2f(fvx + fvwidth, fvy);
      glVertex2f(w, 0);
      // top right
      glTexCoord2f(fvx + fvwidth, fvy + fvheight);
      glVertex2f(w, h);
      // top left
      glTexCoord2f(fvx, fvy + fvheight);
      glVertex2f(0, h);
    glEnd();
    break;
  }

  glPopMatrix();
}


//=========================================================================
// TGUI_ANIMATION
//=========================================================================

/**
 *  Constructor.
 */
TGUI_ANIMATION::TGUI_ANIMATION(void)
{
  tex_item = NULL;

  act_frame = 0;
  shift_time = 0.0;
  speed_ratio = 1.0;
  frame_time = 0.0;

  state = GUI_AS_PLAY;
  visible = true;
  reverse = false;
  loop = true;
}


/**
 *  Constructor.
 *
 *  @param titem  Animation's texture item.
 */
TGUI_ANIMATION::TGUI_ANIMATION(TGUI_TEXTURE *titem)
{
  tex_item = NULL;

  act_frame = 0;
  shift_time = 0.0;
  speed_ratio = 1.0;
  frame_time = 0.0;

  state = GUI_AS_PLAY;
  visible = true;
  reverse = false;
  loop = true;

  SetTexItem(titem);
}



bool TGUI_ANIMATION::SetTexItem(TGUI_TEXTURE *titem)
{
  if (titem == tex_item) return false;

  tex_item = titem;

  act_frame = 0;
  shift_time = 0.0;
  speed_ratio = 1.0;
  if (tex_item) frame_time = tex_item->frame_time;
  else frame_time = 0;

  state = GUI_AS_PLAY;
  visible = true;
  reverse = false;
  loop = true;

  return true;
}


void TGUI_ANIMATION::SetRandomFrame()
{
  if (!tex_item) return;

  act_frame = rand() % tex_item->frames_count;
  shift_time = 0.0;
}


/**
 *  Updates animation according to @p time_shift.
 *
 *  @param time_shift  Time shift from the last update.
 */
void TGUI_ANIMATION::Update(double time_shift)
{
  if (state != GUI_AS_PLAY || !tex_item || !frame_time) return;

  shift_time += time_shift;

  while (shift_time > (frame_time * speed_ratio)) {
    if (reverse) {
      if (act_frame > 0) act_frame--;
      else {
        if (loop) act_frame = tex_item->frames_count - 1;
        else Pause();
      }
    }
    else {
      if (act_frame < tex_item->frames_count - 1) act_frame++;
      else {
        if (loop) act_frame = 0;
        else Pause();
      }
    }

    shift_time -= (frame_time * speed_ratio);
  }
}


//=========================================================================
// TGUI_BOX_ENVELOPE
//=========================================================================

void TGUI_BOX_ENVELOPE::TestBox(TGUI_BOX *box)
{
  if (!box->IsVisible()) return;

  if (box->GetPosX() < min_x) min_x = box->GetPosX();
  if (box->GetPosY() < min_y) min_y = box->GetPosY();

  if (box->GetPosX() + box->GetWidth() > max_x) max_x = box->GetPosX() + box->GetWidth();
  if (box->GetPosY() + box->GetHeight() > max_y) max_y = box->GetPosY() + box->GetHeight();
}


//=========================================================================
// TGUI_BOX
//=========================================================================


// constructor
TGUI_BOX::TGUI_BOX(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight)
{
  owner = bowner;
  key = bkey;

  x = bx; y = by;
  width = bwidth; height = bheight;
  padding = 0.0f;

  next = NULL; prev = NULL;

  on_mouse_move = NULL;
  on_mouse_down = on_mouse_up = NULL;
  on_mouse_click = NULL;
  on_draw = NULL;
  on_key_down = NULL;
  on_get_focus = NULL;
  on_lose_focus = NULL;
  on_show_tooltip = NULL;

  face_color = NULL;
  hover_color = NULL;
  SetAlpha(GUI_DEF_ALPHA);

  lock_mouse_down = false;
  enabled = true;
  visible = true;
  floating = false;

  can_focus = false;
  tooltip_text = NULL;
  tooltip = NULL;
}


TGUI_BOX::~TGUI_BOX(void)
{
  if (this == TGUI::modal_box)    TGUI::modal_box = NULL;
  if (this == TGUI::focus_box)    TGUI::focus_box = NULL;
  if (this == TGUI::hover_box)    SetHoverBox(NULL);
  if (this == TGUI::float_box)    TGUI::float_box = NULL;
  if (this == TGUI::down_box)     TGUI::down_box = NULL;
  if (this == TGUI::tooltip_box)  TGUI::tooltip_box = NULL;

  if (face_color) delete[] face_color;
  if (hover_color) delete[] hover_color;
  if (tooltip_text) delete[] tooltip_text;

  rescan_mouse = true;

  if (next) next->SetPrev(prev);
  if (prev) prev->SetNext(next);
}


void TGUI_BOX::SetPos(GLfloat bx, GLfloat by)
{
  TGUI::self->Lock();

  x = bx;
  y = by;
  if (owner) owner->RecalculateChildren(this);

  TGUI::self->Unlock();
}


void TGUI_BOX::SetPosX(GLfloat bx)
{
  TGUI::self->Lock();

  x = bx;
  if (owner) owner->RecalculateChildren(this);

  TGUI::self->Unlock();
}


void TGUI_BOX::SetPosY(GLfloat by)
{
  TGUI::self->Lock();

  y = by;
  if (owner) owner->RecalculateChildren(this);

  TGUI::self->Unlock();
}


void TGUI_BOX::SetSize(GLfloat bwidth, GLfloat bheight)
{
  TGUI::self->Lock();

  width = bwidth;
  height = bheight;
  if (owner) owner->RecalculateChildren(this);

  TGUI::self->Unlock();
}


void TGUI_BOX::SetWidth(GLfloat bwidth)
{
  TGUI::self->Lock();

  width = bwidth;
  if (owner) owner->RecalculateChildren(this);

  TGUI::self->Unlock();
}


void TGUI_BOX::SetHeight(GLfloat bheight)
{
  TGUI::self->Lock();
  height = bheight;
  if (owner) owner->RecalculateChildren(this);
  TGUI::self->Unlock();
}


void TGUI_BOX::OnMouseClick(void)
{
  if (on_mouse_click)
    TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_click, this));
}


bool TGUI_BOX::MouseMove(GLfloat mouse_x, GLfloat mouse_y)
{
  // cursor is not over box
  if (!(visible && InBox(mouse_x, mouse_y))) return false;

  if (on_mouse_move && enabled)
    TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_move, this, mouse_x - x, mouse_y - y));

  SetHoverBox(this);
  return true;
}


bool TGUI_BOX::MouseDown(GLfloat mouse_x, GLfloat mouse_y, int button)
{
  // cursor is not over box
  if (!(visible && enabled && InBox(mouse_x, mouse_y))) return false;

  Focus();

  if (on_mouse_down)
    TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_down, this, mouse_x - x, mouse_y - y, button));

  if (lock_mouse_down && button == GLFW_MOUSE_BUTTON_LEFT) TGUI::down_box = this;

  return true;
}


bool TGUI_BOX::MouseUp(GLfloat mouse_x, GLfloat mouse_y, int button)
{
  // cursor is not over box
  if (!(visible && enabled && InBox(mouse_x, mouse_y))) return false;

  if (on_mouse_up)
    TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_up, this, mouse_x - x, mouse_y - y, button));

  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (TGUI::down_box == this && on_mouse_click)
      TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_click, this));

    TGUI::down_box = NULL;
  }

  return true;
}


bool TGUI_BOX::KeyDown(int key)
{
  if (on_key_down)
    TGUI::self->callback_list.PushBack(TCALLBACK(on_key_down, this, key));

  return false;
}


void TGUI_BOX::Focus()
{
  if (!can_focus) return;

  TGUI::self->Lock();

  if (TGUI::focus_box) TGUI::focus_box->Unfocus();

  TGUI::focus_box = this;
  if (on_get_focus)
    TGUI::self->callback_list.PushBack(TCALLBACK(on_get_focus, this));

  TGUI::self->Unlock();
}


void TGUI_BOX::Unfocus()
{
  TGUI::self->Lock();

  if (TGUI::focus_box == this) {
    TGUI::focus_box = NULL;
    if (on_lose_focus)
      TGUI::self->callback_list.PushBack(TCALLBACK(on_lose_focus, this));
  }

  TGUI::self->Unlock();
}


void TGUI_BOX::SetColor(GLfloat r, GLfloat g, GLfloat b)
{
  TGUI::self->Lock();

  SetFaceColor(r, g, b);
  SetHoverColor(r * GUI_HOVER_COEF, g * GUI_HOVER_COEF, b * GUI_HOVER_COEF);

  TGUI::self->Unlock();
}


void TGUI_BOX::SetFaceColor(GLfloat r, GLfloat g, GLfloat b)
{
  TGUI::self->Lock();
  SET_COLOR(face_color, r, g, b);
  TGUI::self->Unlock();
}


void TGUI_BOX::SetHoverColor(GLfloat r, GLfloat g, GLfloat b)
{
  TGUI::self->Lock();
  SET_COLOR(hover_color, r, g, b);
  TGUI::self->Unlock();
}


void TGUI_BOX::SetVisible(bool vis)
{
  TGUI::self->Lock();

  visible = vis;

  if (!vis) {
    if (this == TGUI::modal_box)    TGUI::modal_box = NULL;
    if (this == TGUI::focus_box)    TGUI::focus_box = NULL;
    if (this == TGUI::hover_box)    SetHoverBox(NULL);
    if (this == TGUI::float_box)    TGUI::float_box = NULL;
    if (this == TGUI::down_box)     TGUI::down_box = NULL;
    if (this == TGUI::tooltip_box)  TGUI::tooltip_box = NULL;
  }

  else if (floating) {
    if (TGUI::float_box) TGUI::float_box->Hide();
    TGUI::float_box = this;
  }

  if (owner) owner->RecalculateChildren(this);

  TGUI::self->RescanMouse();
  TGUI::self->Unlock();
}


void TGUI_BOX::SetEnabled(bool en)
{
  TGUI::self->Lock();
  enabled = en;

  if (!en) {
    if (TGUI::down_box == this) TGUI::down_box = NULL;
  }

  TGUI::self->Unlock();
}


void TGUI_BOX::Show(void)
{
  TGUI::self->Lock();

  SetVisible(true);
  Focus();

  TGUI::self->Unlock();
}


void TGUI_BOX::ShowModal(void)
{
  TGUI::self->Lock();

  TGUI::modal_box = this;
  Show();

  TGUI::self->Unlock();
}


void TGUI_BOX::SetTooltipText(char *text)
{
  TGUI::self->Lock();

  if (tooltip_text) delete[] tooltip_text;

  if (text && *text) {
    int len = strlen(text);

    tooltip_text = GUI_NEW char[len + 1];
    strcpy(tooltip_text, text);
  }

  TGUI::self->Unlock();
}


void TGUI_BOX::ShowTooltip()
{
  TGUI::self->Lock();

  if (tooltip) TGUI::tooltip_box = tooltip;
  else if (tooltip_text) {
    TGUI::self->GetDefTooltip()->SetCaption(tooltip_text);
    TGUI::tooltip_box = TGUI::self->GetDefTooltip();
  }

  if (on_show_tooltip)
    on_show_tooltip(this);

  TGUI::self->Unlock();
}


void TGUI_BOX::Draw(GLfloat dx, GLfloat dy)
{
  if (!visible) return;

  TGUI::self->SetClipRectangle(x + dx, y + dy, width, height, 0.0f);

  if (on_draw) on_draw(this);

  TGUI::self->UnsetClipRectangle();
}


//=========================================================================
// TGUI_PANEL
//=========================================================================

TGUI_PANEL::~TGUI_PANEL()
{
  Clear();

  if (animation) delete animation;
}


void TGUI_PANEL::SetVisible(bool vis)
{
  TGUI::self->Lock();

  bool old = visible;

  TGUI_BOX::SetVisible(vis);

  if (vis && !old && on_show)
    TGUI::self->callback_list.PushBack(TCALLBACK(on_show, this));

  TGUI::self->Unlock();
}


bool TGUI_PANEL::MouseMove(GLfloat mouse_x, GLfloat mouse_y)
{
  // cursor is not over box
  if (!(visible && InBox(mouse_x, mouse_y))) return false;

  TGUI_BOX *box;
  bool over = false;

  // check if cursor is over child box
  for (box = last_child; box && !over; box = box->GetPrev()) {
    over = box->MouseMove(mouse_x - x - padding, mouse_y - y - padding);
  }

  if (!over) {
    if (on_mouse_move && enabled)
      TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_move, this, mouse_x - x, mouse_y - y));

    SetHoverBox(this);
  }

  return true;
}


bool TGUI_PANEL::MouseDown(GLfloat mouse_x, GLfloat mouse_y, int button)
{
  // cursor is not over box
  if (!(visible && enabled && InBox(mouse_x, mouse_y))) return false;

  TGUI_BOX *box;
  bool over = false;

  // check if cursor is over child box
  for (box = last_child; box && !over; box = box->GetPrev()) {
    over = box->MouseDown(mouse_x - x - padding, mouse_y - y - padding, button);
  }

  if (!over) {
    Focus();
    if (on_mouse_down)
      TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_down, this, mouse_x - x, mouse_y - y, button));

    if (lock_mouse_down && button == GLFW_MOUSE_BUTTON_LEFT) TGUI::down_box = this;
  }

  return true;
}


bool TGUI_PANEL::MouseUp(GLfloat mouse_x, GLfloat mouse_y, int button)
{
  // cursor is not over box
  if (!(visible && enabled && InBox(mouse_x, mouse_y))) return false;

  TGUI_BOX *box;
  bool over = false;

  // check if cursor is over child box
  for (box = last_child; box && !over; box = box->GetPrev()) {
    over = box->MouseUp(mouse_x - x - padding, mouse_y - y - padding, button);
  }

  if (!over) {
    if (on_mouse_up)
      TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_up, this, mouse_x - x, mouse_y - y, button));

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
      if (TGUI::down_box == this && on_mouse_click)
        TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_click, this));

      TGUI::down_box = NULL;
    }
  }

  return true;
}


TGUI_BOX *TGUI_PANEL::AddChild(TGUI_BOX *box)
{
  TGUI::self->Lock();

  if (!child_list) child_list = last_child = box;
  else {
    last_child->SetNext(box);
    box->SetPrev(last_child);

    last_child = box;
  }

  box->SetOwner(this);

  RecalculateChildren(box);

  TGUI::self->Unlock();

  return box;
}


TGUI_BOX *TGUI_PANEL::GetChild(int key)
{
  TGUI::self->Lock();

  TGUI_BOX *box;

  for (box = child_list; box; box = box->GetNext())
    if (box->key == key) {
      TGUI::self->Unlock();
      return box;
    }

  TGUI::self->Unlock();

  return NULL;
}


int TGUI_PANEL::GetChildOrder(TGUI_BOX *child)
{
  TGUI::self->Lock();

  TGUI_BOX *box;
  int i;

  for (i = 0, box = child_list; box; box = box->GetNext(), i++)
    if (box == child) {
      TGUI::self->Unlock();
      return i;
    }

  TGUI::self->Unlock();

  return -1;
}


TGUI_PANEL *TGUI_PANEL::AddPanel(int pkey, GLfloat px, GLfloat py, GLfloat pwidth, GLfloat pheight)
{
  TGUI_PANEL *panel;

  panel = GUI_NEW TGUI_PANEL(this, pkey, px, py, pwidth, pheight);
  AddChild(panel);

  return panel;
}


TGUI_PANEL *TGUI_PANEL::AddPanel(int pkey, GLfloat px, GLfloat py, TGUI_TEXTURE *tex)
{
  TGUI_PANEL *panel;

  panel = GUI_NEW TGUI_PANEL(this, pkey, px, py, tex);
  AddChild(panel);

  return panel;
}


TGUI_PANEL *TGUI_PANEL::AddPanel(int pkey, GLfloat px, GLfloat py, GLfloat pwidth, GLfloat pheight, TGUI_TEXTURE *tex)
{
  TGUI_PANEL *panel;

  panel = GUI_NEW TGUI_PANEL(this, pkey, px, py, tex);
  panel->SetSize(pwidth, pheight);
  AddChild(panel);

  return panel;
}


TGUI_BUTTON *TGUI_PANEL::AddButton(int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight, const char *bcaption)
{
  TGUI_BUTTON *button;

  button = GUI_NEW TGUI_BUTTON(this, bkey, bx, by, bwidth, bheight, bcaption, GUI_BS_BUTTON, -1);
  AddChild(button);

  return button;
}


TGUI_BUTTON *TGUI_PANEL::AddButton(int bkey, GLfloat bx, GLfloat by, TGUI_TEXTURE *tex)
{
  TGUI_BUTTON *button;

  button = GUI_NEW TGUI_BUTTON(this, bkey, bx, by, tex, GUI_BS_BUTTON, -1);
  AddChild(button);

  return button;
}


TGUI_BUTTON *TGUI_PANEL::AddCheckButton(int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight, const char *bcaption)
{
  TGUI_BUTTON *button;

  button = GUI_NEW TGUI_BUTTON(this, bkey, bx, by, bwidth, bheight, bcaption, GUI_BS_CHECK, -1);
  AddChild(button);

  return button;
}


TGUI_BUTTON *TGUI_PANEL::AddGroupButton(int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight, const char *bcaption, int bgroup)
{
  TGUI_BUTTON *button;

  button = GUI_NEW TGUI_BUTTON(this, bkey, bx, by, bwidth, bheight, bcaption, GUI_BS_GROUP, bgroup);
  AddChild(button);

  return button;
}


TGUI_BUTTON *TGUI_PANEL::AddGroupButton(int bkey, GLfloat bx, GLfloat by, TGUI_TEXTURE *tex, int bgroup)
{
  TGUI_BUTTON *button;

  button = GUI_NEW TGUI_BUTTON(this, bkey, bx, by, tex, GUI_BS_GROUP, bgroup);
  AddChild(button);

  return button;
}


TGUI_CHECK_BOX *TGUI_PANEL::AddCheckBox(int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight, const char *bcaption)
{
  TGUI_CHECK_BOX *box;

  box = GUI_NEW TGUI_CHECK_BOX(this, bkey, bx, by, bwidth, bheight, bcaption, GUI_BS_CHECK, -1);
  AddChild(box);

  return box;
}


TGUI_CHECK_BOX *TGUI_PANEL::AddGroupBox(int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight, const char *bcaption, int bgroup)
{
  TGUI_CHECK_BOX *box;

  box = GUI_NEW TGUI_CHECK_BOX(this, bkey, bx, by, bwidth, bheight, bcaption, GUI_BS_GROUP, bgroup);
  AddChild(box);

  return box;
}


TGUI_LABEL *TGUI_PANEL::AddLabel(int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight)
{
  TGUI_LABEL *label;

  label = GUI_NEW TGUI_LABEL(this, bkey, bx, by, bwidth, bheight);
  AddChild(label);
  
  return label;
}


TGUI_LABEL *TGUI_PANEL::AddLabel(int bkey, GLfloat bx, GLfloat by, TGUI_TEXTURE *tex)
{
  TGUI_LABEL *label;

  label = GUI_NEW TGUI_LABEL(this, bkey, bx, by, tex);
  AddChild(label);
  
  return label;
}


TGUI_LABEL *TGUI_PANEL::AddLabel(int bkey, GLfloat bx, GLfloat by, const char *bcaption)
{
  TGUI_LABEL *label;

  label = GUI_NEW TGUI_LABEL(this, bkey, bx, by, bcaption);
  AddChild(label);
  
  return label;
}


TGUI_SLIDER *TGUI_PANEL::AddSlider(int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight, TGUI_SLIDER_TYPE btype)
{
  TGUI_SLIDER *slider = GUI_NEW TGUI_SLIDER(this, bkey, bx, by, bwidth, bheight, btype);
  AddChild(slider);

  return slider;
}


TGUI_SCROLL_BOX *TGUI_PANEL::AddScrollBox(int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight)
{
  TGUI_SCROLL_BOX *list = GUI_NEW TGUI_SCROLL_BOX(this, bkey, bx, by, bwidth, bheight);
  AddChild(list);

  return list;
}


TGUI_LIST_BOX *TGUI_PANEL::AddListBox(int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight)
{
  TGUI_LIST_BOX *list = GUI_NEW TGUI_LIST_BOX(this, bkey, bx, by, bwidth, bheight);
  AddChild(list);

  return list;
}


TGUI_EDIT_BOX *TGUI_PANEL::AddEditBox(int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight, unsigned max_lenght)
{
  TGUI_EDIT_BOX *edit = GUI_NEW TGUI_EDIT_BOX(this, bkey, bx, by, bwidth, bheight, max_lenght);
  AddChild(edit);

  return edit;
}


TGUI_COMBO_BOX *TGUI_PANEL::AddComboBox(int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight)
{
  TGUI_COMBO_BOX *list = GUI_NEW TGUI_COMBO_BOX(this, bkey, bx, by, bwidth, bheight);
  AddChild(list);

  return list;
}


void TGUI_PANEL::Clear()
{
  TGUI_BOX *box;

  TGUI::self->Lock();

  while (child_list) {
    box = child_list;
    child_list = child_list->GetNext();     
    delete box;
  }

  child_list = last_child = NULL;

  TGUI::self->Unlock();
}


void TGUI_PANEL::SetTexture(TGUI_TEXTURE *tex)
{
  TGUI::self->Lock();

  if (animation) delete animation;
  animation = NULL;

  if (tex) animation = GUI_NEW TGUI_ANIMATION(tex);

  TGUI::self->Unlock();
}


void TGUI_PANEL::Update(double time_shift)
{
  TGUI_BOX *box;

  if (animation) animation->Update(time_shift);

  // updates children
  for (box = child_list; box; box = box->GetNext()) {
    box->Update(time_shift);
  }
}


void TGUI_PANEL::Draw(GLfloat dx, GLfloat dy)
{
  if (!visible) return;

  TGUI_BOX *box;
  GLfloat *fcolor = face_color;

  if (!fcolor) fcolor = TGUI::self->GetFaceColor();

  TGUI::self->SetClipRectangle(x + dx, y + dy, width, height, 0.0f);

  if (alpha) {
    // draws animation
    if (animation) {
      glColor4f(fcolor[0], fcolor[1], fcolor[2], alpha);
      animation->Draw(width, height);
    }

    // draws panel
    else DrawFaceRectangle(0, 0, width, height, fcolor, alpha, true, false);
  }

  // draws panel with user method
  if (on_draw) on_draw(this);

  // draws children
  if (child_list) {
    if (padding) TGUI::self->SetClipRectangle(0.0f, 0.0f, width, height, padding);
  
    for (box = child_list; box; box = box->GetNext()) {
      box->Draw(0, 0);
    }

    if (padding) TGUI::self->UnsetClipRectangle();
  }

  TGUI::self->UnsetClipRectangle();
}


//=========================================================================
// TGUI_LABEL
//=========================================================================


TGUI_LABEL::TGUI_LABEL(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight)
  :TGUI_BOX(bowner, bkey, bx, by, bwidth, bheight)
{
  buff_size = 0;
  caption = NULL;
  lines = NULL;
  lines_count = 0;
  font = NULL;
  font_color = NULL;
  line_height = 18.0f;
  animation = NULL;
  autosize = false;
  transparent = false;
}


TGUI_LABEL::TGUI_LABEL(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, const char *bcaption)
  :TGUI_BOX(bowner, bkey, bx, by, 0, 0)
{
  buff_size = 0;
  caption = NULL;
  lines = NULL;
  lines_count = 0;
  font = NULL;
  font_color = NULL;
  line_height = 18.0f;
  animation = NULL;
  autosize = true;
  transparent = true;

  SetCaption(bcaption);
}


TGUI_LABEL::TGUI_LABEL(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, TGUI_TEXTURE *tex)
  :TGUI_BOX(bowner, bkey, bx, by, 0, 0)
{
  buff_size = 0;
  caption = NULL;
  lines = NULL;
  lines_count = 0;
  font = NULL;
  font_color = NULL;
  line_height = 18.0f;
  animation = NULL;
  autosize = true;
  transparent = false;

  SetTexture(tex);
  if (animation) {
    SetColor(1.0f, 1.0f, 1.0f);
    width = (GLfloat)animation->GetFrameWidth();
    height = (GLfloat)animation->GetFrameHeight();
  }
}


void TGUI_LABEL::SetCaption(const char *bcaption)
{
  TGUI::self->Lock();

  // delete lines
  if (lines) {
    delete[] lines;
    lines = NULL;
    lines_count = 0;
  }

  // if new caption is emty, reset buffer
  if (!bcaption || !*bcaption) {
    if (caption) *caption = 0;
    TGUI::self->Unlock();
    return;
  }

  int len = strlen(bcaption);

  // prepare buffer for caption
  if (len >= buff_size) {
    if (caption) delete[] caption;
    caption = GUI_NEW char[len + 1];
    buff_size = len + 1;
  }

  // fill caption
  strcpy(caption, bcaption);

  int max_len = 0;
  int i, j;
  GLFfont *fnt = font;
  GLfloat nw = 0, nh = 0;

  if (!fnt) fnt = TGUI::self->GetFont();

  // transform to lines
  for (i = 0; i < len; i++) if (caption[i] == '\n') lines_count++;
  lines = GUI_NEW char *[++lines_count];
  lines[0] = caption;
  j = 1;

  for (i = 0; i < len; i++) if (caption[i] == '\n') {
    caption[i] = 0;
    lines[j++] = caption + i + 1;
  }

  if (fnt) {
    // compute new width and height (autosize)
    for (i = 0; i < lines_count; i++) {
      if ((len = strlen(lines[i])) > max_len) max_len = len;
    }

    nw = GLfloat(max_len * fnt->fSpacing + 2 * padding);
    nh = GLfloat(lines_count * line_height + 2 * padding);
  }

  SetSize(nw, nh);

  TGUI::self->Unlock();
}


void TGUI_LABEL::SetTexture(TGUI_TEXTURE *tex)
{
  TGUI::self->Lock();

  if (animation) delete animation;
  animation = NULL;

  if (!tex) {
    TGUI::self->Unlock();
    return;
  }

  animation = GUI_NEW TGUI_ANIMATION(tex);

  if (autosize) {
    // compute new width and height (autosize)
    width = (GLfloat)animation->GetFrameWidth();
    height = (GLfloat)animation->GetFrameHeight();
  }

  TGUI::self->Unlock();
}


void TGUI_LABEL::SetFontColor(GLfloat r, GLfloat g, GLfloat b)
{
  TGUI::self->Lock();
  SET_COLOR(font_color, r, g, b);
  TGUI::self->Unlock();
}


void TGUI_LABEL::Draw(GLfloat dx, GLfloat dy)
{
  if (!visible) return;

  GLFfont *fnt = font;
  if (!fnt) fnt = TGUI::self->GetFont();

  TGUI::self->SetClipRectangle(x + dx, y + dy, width, height, 0.0f);

  if (!transparent && alpha) {
    GLfloat *fcolor = face_color;
    if (!fcolor) fcolor = TGUI::self->GetFaceColor();

    // draw animation
    if (animation) {
      glColor4f(fcolor[0], fcolor[1], fcolor[2], alpha);
      animation->Draw(GLfloat(width), GLfloat(height));
    }

    // draws panel
    else DrawFaceRectangle(0, 0, width, height, fcolor, alpha, false, false);
  }

  if (padding) TGUI::self->SetClipRectangle(0, 0, width, height, padding);

  if (fnt && lines_count) {
    GLfloat *fncolor = font_color;
    if (!fncolor) fncolor = TGUI::self->GetFontColor();

    glfDisable(GLF_RESET_PROJECTION);

    glColor3fv(fncolor);

    glTranslatef(0, GLfloat(int(line_height - fnt->fHeight) / 2) , 0);

    for (int i = lines_count - 1; i >= 0; i--) {
      glfPrint(fnt, 0, 0, lines[i], false);
      glTranslatef(0, line_height, 0);
    }

    glfEnable(GLF_RESET_PROJECTION);
  }

  if (padding) TGUI::self->UnsetClipRectangle();

  if (on_draw) on_draw(this);

  TGUI::self->UnsetClipRectangle();
}


//=========================================================================
// TGUI_LIST
//=========================================================================


void TGUI_LIST::SetCaption(const char *bcaption)
{
  TGUI::self->Lock();

  TGUI_LABEL::SetCaption(bcaption);

  SetSize(width, height);

  if (bcaption && *bcaption) item_index = 0;
  else item_index = -1;

  TGUI::self->Unlock();
}


void TGUI_LIST::SetSelFontColor(GLfloat r, GLfloat g, GLfloat b)
{
  TGUI::self->Lock();
  SET_COLOR(sel_font_color, r, g, b);
  TGUI::self->Unlock();
}


void TGUI_LIST::SetSelColor(GLfloat r, GLfloat g, GLfloat b)
{
  TGUI::self->Lock();
  SET_COLOR(sel_color, r, g, b);
  TGUI::self->Unlock();
}


void TGUI_LIST::Draw(GLfloat dx, GLfloat dy)
{
  if (!visible) return;

  GLFfont *fnt = font;
  if (!fnt) fnt = TGUI::self->GetFont();

  TGUI::self->SetClipRectangle(x + dx, y + dy, width, height, 0.0f);
  
  if (alpha) {
    GLfloat *fcolor = face_color;
    if (!fcolor) fcolor = TGUI::self->GetFaceColor();

    // draws the animation
    if (animation) {
      glColor4f(fcolor[0], fcolor[1], fcolor[2], alpha);
      animation->Draw(GLfloat(width), GLfloat(height));
    }

    // draws background rectangle
    else DrawBgRectangle(0, 0, width, height, NULL, NULL, alpha, false);
  }

  if (fnt && caption) {
    GLfloat *fncolor = font_color;
    GLfloat *sfcolor = sel_font_color;
    GLfloat *scolor = sel_color;

    if (!fncolor) fncolor = TGUI::self->GetFontColor();
    if (!sfcolor) sfcolor = TGUI::self->GetFontColor();
    if (!scolor) scolor = TGUI::self->GetFaceColor();

    glfDisable(GLF_RESET_PROJECTION);

    DrawFaceRectangle(0, height - line_height * (item_index+1) - padding, width, line_height, scolor, 1, false, false);

    if (padding) TGUI::self->SetClipRectangle(0.0f, 0.0f, width, height, padding);
  
    
    glColor3fv(fncolor);

    for (int i = lines_count - 1; i >= 0; i--) {
      if (i == item_index) {
        glColor3fv(sfcolor);
        glfPrint(fnt, 0, 0, lines[i], false);
        glColor3fv(fncolor);
      }
      else glfPrint(fnt, 0, 0, lines[i], false);
    
      glTranslatef(0, line_height, 0);
    }

    glfEnable(GLF_RESET_PROJECTION);

    if (padding) TGUI::self->UnsetClipRectangle();
  }

  if (on_draw) on_draw(this);

  TGUI::self->UnsetClipRectangle();
}


bool TGUI_LIST::MouseDown(GLfloat mouse_x, GLfloat mouse_y, int button)
{
  // cursor is not over box
  if (!(visible && enabled && InBox(mouse_x, mouse_y))) return false;

  int ii;

  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    ii = lines_count - int((mouse_y - y - padding) / line_height) - 1;
    if (ii < 0) ii = 0;
    if (ii >= lines_count) ii = lines_count - 1;

    if (ii != item_index) {
      item_index = ii;
      if (on_change)
        TGUI::self->callback_list.PushBack(TCALLBACK(on_change, this, ii));
    }
  }

  Focus();
  if (on_mouse_down)
    TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_down, this, mouse_x - x, mouse_y - y, button));

  return true;
}

/**
 *  Changes selected line in the list to the one specified by @p line.
 *
 *  @return @c false when specified line was not found; @c true upon
 *          successful change.
 */
bool TGUI_LIST::SetItem (const char *line)
{
  TGUI::self->Lock();

  for (int i = 0; i < GetLinesCount (); i++) {
    if (strcmp (line, lines[i]) == 0) {
      item_index = i;
      TGUI::self->Unlock();
      return true;
    }
  }

  TGUI::self->Unlock();
  return false;
}


//=========================================================================
// TGUI_BUTTON
//=========================================================================

TGUI_BUTTON::TGUI_BUTTON(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight, const char *bcaption, int bstyle, int bgroup)
:TGUI_BOX(bowner, bkey, bx, by, bwidth, bheight)
{
  caption = NULL;
  SetCaption(bcaption);

  for (int i = 0; i < GUI_BS_COUNT; i++) animation[i] = NULL;

  checked = false;
  lock_mouse_down = true;

  font = NULL;
  font_color = NULL;

  style = bstyle;
  group = bgroup;
  can_focus = (style == GUI_BS_BUTTON);

  on_check = NULL;
}


TGUI_BUTTON::TGUI_BUTTON(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, TGUI_TEXTURE *tex, int bstyle, int bgroup)
:TGUI_BOX(bowner, bkey, bx, by, 0, 0)
{
  caption = NULL;
  for (int i = 0; i < GUI_BS_COUNT; i++) animation[i] = NULL;

  SetTexture(GUI_BS_UP, tex);

  if (animation[GUI_BS_UP]) {
    SetColor(1.0f, 1.0f, 1.0f);

    width = (GLfloat)animation[GUI_BS_UP]->GetFrameWidth();
    height = (GLfloat)animation[GUI_BS_UP]->GetFrameHeight();
  }

  checked = false;
  lock_mouse_down = true;

  font = NULL;
  font_color = NULL;

  style = bstyle;
  group = bgroup;
  can_focus = (style == GUI_BS_BUTTON);

  on_check = NULL;
}


void TGUI_BUTTON::SetCaption(const char *cpt)
{
  TGUI::self->Lock();

  if (caption) delete[] caption;
  caption = NULL;

  if (cpt) { 
    caption = GUI_NEW char[strlen(cpt) + 1];
    strcpy(caption, cpt);
  }

  TGUI::self->Unlock();
}


void TGUI_BUTTON::SetTexture(int state, TGUI_TEXTURE *tex)
{
  TGUI::self->Lock();

  if (animation[state]) delete animation[state];
  animation[state] = NULL;

  if (tex) animation[state] = GUI_NEW TGUI_ANIMATION(tex);

  TGUI::self->Unlock();
}


void TGUI_BUTTON::SetFontColor(GLfloat r, GLfloat g, GLfloat b)
{
  TGUI::self->Lock();
  SET_COLOR(font_color, r, g, b);
  TGUI::self->Unlock();
}


void TGUI_BUTTON::Draw(GLfloat dx, GLfloat dy)
{
  if (!visible) return;

  bool down = checked || (TGUI::hover_box == this && TGUI::down_box == this);
  bool hover = (TGUI::hover_box == this) && (!TGUI::down_box || TGUI::down_box == this) && enabled;

  GLfloat *fcolor = face_color;
  GLfloat *hcolor = hover_color;
  GLFfont *fnt = font;
  GLfloat ralpha = alpha;

  if (!fcolor) fcolor = TGUI::self->GetFaceColor();
  if (!hcolor) hcolor = TGUI::self->GetHoverColor();
  if (!fnt) fnt = TGUI::self->GetFont();
  if (!enabled) ralpha = alpha / 3;

  TGUI::self->SetClipRectangle(x + dx, y + dy, width, height, 0.0f);

  if (ralpha) {
    TGUI_BUTTON_STATE anim_id;

    if (down) {
      if (hover && animation[GUI_BS_DOWN_HOVER]) anim_id = GUI_BS_DOWN_HOVER;
      else if (animation[GUI_BS_DOWN]) anim_id = GUI_BS_DOWN;
      else anim_id = GUI_BS_UP;
    }
    else {
      if (hover && animation[GUI_BS_UP_HOVER]) anim_id = GUI_BS_UP_HOVER;
      else anim_id = GUI_BS_UP;
    }

    if (animation[anim_id]) {
      if (hover) glColor4f(hcolor[0], hcolor[1], hcolor[2], ralpha);
      else glColor4f(fcolor[0], fcolor[1], fcolor[2], ralpha);

      animation[anim_id]->Draw(GLfloat(width), GLfloat(height));
    }
    else {
      if (hover) DrawFaceRectangle(0, 0, width, height, hcolor, alpha, true, down);
      else DrawFaceRectangle(0, 0, width, height, fcolor, alpha, true, down);
    }
  }

  if (fnt && caption) {
    GLfloat *fncolor = font_color;
    int len = strlen(caption);
    GLfloat x = short((width - len * fnt->fSpacing) / 2);
    GLfloat y = short((height - fnt->fHeight) / 2);

    if (!fncolor) fncolor = TGUI::self->GetFontColor();

    if (enabled) glColor3fv(fncolor);
    else glColor3f(fcolor[0] / GUI_LIGHT_COEF, fcolor[1] / GUI_LIGHT_COEF, fcolor[2] / GUI_LIGHT_COEF);

    glfDisable(GLF_RESET_PROJECTION);

    if (down) glfPrint(fnt, x + 1, y - 1, caption, false);
    else glfPrint(fnt, x, y, caption, false);

    glfEnable(GLF_RESET_PROJECTION);
  }

  if (on_draw) on_draw(this);

  TGUI::self->UnsetClipRectangle();
}


bool TGUI_BUTTON::MouseUp(GLfloat mouse_x, GLfloat mouse_y, int button)
{
  // cursor is not over box
  if (!(visible && enabled && InBox(mouse_x, mouse_y))) return false;

  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (TGUI::down_box == this) {
      if (!(style == GUI_BS_GROUP && checked)) {
        if (on_mouse_click)
          TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_click, this));

        if (checked) SetChecked(false);
        else SetChecked(true);
      }
    }

    TGUI::down_box = NULL;
  }

  if (on_mouse_up)
    TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_up, this, mouse_x - x, mouse_y - y, button));

  return true;
}


void TGUI_BUTTON::SetChecked(bool ch)
{
  if (ch == checked) return;

  bool last_checked = checked;

  TGUI::self->Lock();

  if (style == GUI_BS_GROUP && ch && !checked) {
    TGUI_BUTTON *butt;
    TGUI_BOX *box;
    bool ok = false;

    // find checked button in group left from this
    for (box = prev; box; box = box->GetPrev()) {
      butt = dynamic_cast<TGUI_BUTTON *>(box);
      if (!(butt && butt->GetStyle() == GUI_BS_GROUP && butt->GetGroup() == group)) break;
      else if (butt->checked) {
        butt->SetChecked(false);
        ok = true;
        break;
      }
    }

    // find checked button in group left from this
    if (!ok) for (box = next; box; box = box->GetNext()) {
      butt = dynamic_cast<TGUI_BUTTON *>(box);
      if (!(butt && butt->GetStyle() == GUI_BS_GROUP && butt->GetGroup() == group)) break;
      else if (butt->checked) {
        butt->SetChecked(false);
        break;
      }
    }

    checked = true;
  }

  else if (style == GUI_BS_CHECK || style == GUI_BS_GROUP) checked = ch;

  if (last_checked != checked && on_check)
    TGUI::self->callback_list.PushBack(TCALLBACK(on_check, this, checked));

  TGUI::self->Unlock();
}


//=========================================================================
// TGUI_CHECK_BOX
//=========================================================================

void TGUI_CHECK_BOX::Draw(GLfloat dx, GLfloat dy)
{
  if (!visible) return;

  bool down = checked || (TGUI::hover_box == this && TGUI::down_box == this);
  bool hover = (TGUI::hover_box == this) && (!TGUI::down_box || TGUI::down_box == this) && enabled;

  GLFfont *fnt = font;
  GLfloat *fcolor = face_color;
  GLfloat *hcolor = hover_color;
  GLfloat ralpha = alpha;

  GLfloat pdy = GLfloat(int(height - GUI_CHECK_BOX_SIZE) / 2 + 1);

  if (!fcolor) fcolor = TGUI::self->GetFaceColor();
  if (!hcolor) hcolor = TGUI::self->GetHoverColor();
  if (!fnt) fnt = TGUI::self->GetFont();
  if (!enabled) ralpha = alpha / 3;

  TGUI::self->SetClipRectangle(x + dx, y + dy, width, height, 0.0f);

  if (ralpha) {
    TGUI_BUTTON_STATE anim_id;

    if (down) {
      if (hover && animation[GUI_BS_DOWN_HOVER]) anim_id = GUI_BS_DOWN_HOVER;
      else if (animation[GUI_BS_DOWN]) anim_id = GUI_BS_DOWN;
      else anim_id = GUI_BS_UP;
    }
    else {
      if (hover && animation[GUI_BS_UP_HOVER]) anim_id = GUI_BS_UP_HOVER;
      else anim_id = GUI_BS_UP;
    }

    if (animation[anim_id]) {
      if (hover) glColor4f(hcolor[0], hcolor[1], hcolor[2], ralpha);
      else glColor4f(fcolor[0], fcolor[1], fcolor[2], ralpha);

      animation[anim_id]->Draw(GLfloat(width), GLfloat(height));
    }
    else {
      if (hover) DrawFaceRectangle(0, pdy, GUI_CHECK_BOX_SIZE, GUI_CHECK_BOX_SIZE, hcolor, alpha, true, down);
      else DrawFaceRectangle(0, pdy, GUI_CHECK_BOX_SIZE, GUI_CHECK_BOX_SIZE, fcolor, alpha, true, down);

      // check point
      if (checked) {
        GLfloat fr = 4.0f;

        glDisable(GL_TEXTURE_2D);
        glColor3f(0, 0, 0);

        glBegin(GL_QUADS);
          glVertex2f(fr, pdy + fr);
          glVertex2f(fr, pdy + GUI_CHECK_BOX_SIZE - fr);
          glVertex2f(GUI_CHECK_BOX_SIZE - fr, pdy + GUI_CHECK_BOX_SIZE - fr);
          glVertex2f(GUI_CHECK_BOX_SIZE - fr, pdy + fr);
        glEnd();

        glEnable(GL_TEXTURE_2D);
      }
    }
  } // if alpha

  // caption
  if (fnt && caption) {
    GLfloat *fncolor = font_color;
    GLfloat y = GLfloat(int((height - fnt->fHeight) / 2) + 1);

    if (!fncolor) fncolor = TGUI::self->GetFontColor();

    if (enabled) glColor3fv(fncolor);
    else glColor3f(fcolor[0] / GUI_LIGHT_COEF, fcolor[1] / GUI_LIGHT_COEF, fcolor[2] / GUI_LIGHT_COEF);

    glfDisable(GLF_RESET_PROJECTION);
    glfPrint(fnt, GUI_CHECK_BOX_SIZE + 10, y, caption, false);
    glfEnable(GLF_RESET_PROJECTION);
  }

  if (on_draw) on_draw(this);

  TGUI::self->UnsetClipRectangle();
}


//=========================================================================
// TGUI_EDIT_BOX
//=========================================================================

TGUI_EDIT_BOX::TGUI_EDIT_BOX(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight, unsigned max_lenght)
  :TGUI_BOX(bowner, bkey, bx, by, bwidth, bheight)
{ 
  text = NULL; font = NULL; font_color = NULL; cursor_pos = 0;
  max_len = max_lenght; text_len = 0; text_dx = 0.0f;
  padding = 5.0f; 
  text = GUI_NEW char[max_len + 1]; *text = 0;

  on_change = NULL;
  can_focus = true;
};


void TGUI_EDIT_BOX::SetText(char *txt)
{
  TGUI::self->Lock();

  if (!txt || !*txt) {
    *text = 0;
    text_len = 0;
    text_dx = 0.0f;
    cursor_pos = 0;
    TGUI::self->Unlock();
    return;
  }

  unsigned len = strlen(txt);
  if (len > max_len) len = max_len;

  strncpy(text, txt, len);
  text[len] = 0;

  text_len = len;
  text_dx = 0; cursor_pos = 0;

  TGUI::self->Unlock();
}


void TGUI_EDIT_BOX::SetFontColor(GLfloat r, GLfloat g, GLfloat b)
{
  TGUI::self->Lock();
  SET_COLOR(font_color, r, g, b);
  TGUI::self->Unlock();
}


bool TGUI_EDIT_BOX::MouseDown(GLfloat mouse_x, GLfloat mouse_y, int button)
{
  // cursor is not over box
  if (!(visible && enabled && InBox(mouse_x, mouse_y))) return false;

  Focus();
  if (on_mouse_down)
    TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_down, this, mouse_x - x, mouse_y - y, button));

  if (lock_mouse_down && button == GLFW_MOUSE_BUTTON_LEFT) TGUI::down_box = this;

  GLFfont *fnt = font;
  if (!fnt) fnt = TGUI::self->GetFont();

  GLfloat pos = (mouse_x - x - text_dx - (fnt->fSpacing/2)) / fnt->fSpacing;

  if (pos < 0) cursor_pos = 0;
  else if (pos > text_len) cursor_pos = text_len;
  else cursor_pos = (unsigned)pos;

  return true;
}


bool TGUI_EDIT_BOX::KeyDown(int key)
{
  unsigned i;
  unsigned key2;
  bool change = false;
  bool edit = false;

  switch (key) {
  case GLFW_KEY_RIGHT: if (cursor_pos < text_len) cursor_pos++; edit = true; break;
  case GLFW_KEY_LEFT:  if (cursor_pos > 0) cursor_pos--; edit = true; break;
  case GLFW_KEY_END:   cursor_pos = text_len; edit = true; break;
  case GLFW_KEY_HOME:  cursor_pos = 0; edit = true; break;

  case GLFW_KEY_BACKSPACE:
    if (cursor_pos > 0) {
      cursor_pos--;
      for (i = cursor_pos; i < text_len; i++) text[i] = text[i + 1];
      text_len--;
      change = edit = true;
    }
    break;

  case GLFW_KEY_DEL:
    if (cursor_pos < text_len) {
      for (i = cursor_pos; i < text_len; i++) text[i] = text[i + 1];
      text_len--;
      change = edit = true;
    }
    break;

  default:
    key2 = TranslateKey(key);

    if (key2 >= ' ' && key2 <= '~' && text_len < max_len ) {
      // shift chars
      for (i = text_len + 1; i > cursor_pos; i--) text[i] = text[i - 1];

      // add new char
      text[cursor_pos] = key2;
      cursor_pos++;
      text_len++;
      change = edit = true;
    }
    break;
  }

  if (edit) {
    GLFfont *fnt = font;
    if (!fnt) fnt = TGUI::self->GetFont();
    GLfloat cpos = padding + text_dx + cursor_pos * fnt->fSpacing;

    if (cpos > width - padding) text_dx = width - 2*padding - cursor_pos * fnt->fSpacing;
    if (cpos < padding) {
      text_dx = -(GLfloat)cursor_pos * fnt->fSpacing;
    }

    if (change && on_change)
      TGUI::self->callback_list.PushBack(TCALLBACK(on_change, this, text));
  }

  if (on_key_down)
    TGUI::self->callback_list.PushBack(TCALLBACK(on_key_down, this, key));

  return edit;
}


void TGUI_EDIT_BOX::Draw(GLfloat dx, GLfloat dy)
{
  if (!visible) return;

  GLFfont *fnt = font;
  GLfloat *fcolor = face_color;

  if (!fcolor) fcolor = TGUI::self->GetFaceColor();
  if (!fnt) fnt = TGUI::self->GetFont();

  TGUI::self->SetClipRectangle(x + dx, y + dy, width, height, 0.0f);

  if (alpha) DrawBgRectangle(0, 0, width, height, fcolor, NULL, alpha, true);

  if (fnt) {
    GLfloat *fncolor = font_color;
    GLfloat x = text_dx;
    GLfloat y = GLfloat(int(height - 2*padding - fnt->fHeight) / 2);

    if (!fncolor) fncolor = TGUI::self->GetFontColor();

    if (padding) TGUI::self->SetClipRectangle(0.0f, 0.0f, width, height, padding);

    if (enabled) glColor3fv(fncolor);
    else glColor3f(fcolor[0] / GUI_LIGHT_COEF, fcolor[1] / GUI_LIGHT_COEF, fcolor[2] / GUI_LIGHT_COEF);

    // draws text
    if (text) {
      glfDisable(GLF_RESET_PROJECTION);

      glfPrint(fnt, x, y, text, false);

      glfEnable(GLF_RESET_PROJECTION);
    }

    // draws cursor
    if (TGUI::focus_box == this) {
      glDisable(GL_TEXTURE_2D);

      glBegin(GL_LINES);
        glVertex2f(x + cursor_pos * fnt->fSpacing, y);
        glVertex2f(x + cursor_pos * fnt->fSpacing, y + fnt->fHeight);
      glEnd();

      glEnable(GL_TEXTURE_2D);
    }

    if (padding) TGUI::self->UnsetClipRectangle();
  }

  if (on_draw) on_draw(this);

  TGUI::self->UnsetClipRectangle();
}


//=========================================================================
// TGUI_SLIDER
//=========================================================================

TGUI_SLIDER::TGUI_SLIDER(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight, TGUI_SLIDER_TYPE btype)
  :TGUI_BOX(bowner, bkey, bx, by, bwidth, bheight)
{ 
  type = btype;
  position = 0.0f;
  padding = 1;

  ComputeSliderSize();
  
  lock_mouse_down = true;
  on_change = NULL;
}


void TGUI_SLIDER::ComputeSliderSize()
{
  if (type == GUI_ST_HORIZONTAL) {
    swidth = GUI_SLIDER_SIZE - 2*padding;
    if (swidth > width) swidth = 2;
    sheight = height - 2*padding;
  }
  else {
    sheight = GUI_SLIDER_SIZE - 2*padding;
    if (sheight > height) height = 2;
    swidth = width - 2*padding;
  }
}


void TGUI_SLIDER::SetPosition(GLfloat pos)
{
  if (pos < 0) pos = 0.0f;
  else if (pos > 1) pos = 1.0f;

  position = pos;

  if (on_change)
    TGUI::self->callback_list.PushBack(TCALLBACK(on_change, this, position));
}


void TGUI_SLIDER::SetWidth(GLfloat bwidth)
{
  TGUI::self->Lock();

  width = bwidth;
  ComputeSliderSize();

  if (owner) owner->RecalculateChildren(this);

  TGUI::self->Unlock();
}


void TGUI_SLIDER::SetHeight(GLfloat bheight)
{
  TGUI::self->Lock();

  height = bheight;
  ComputeSliderSize();

  if (owner) owner->RecalculateChildren(this);

  TGUI::self->Unlock();
}


void TGUI_SLIDER::SetSize(GLfloat bwidth, GLfloat bheight)
{
  TGUI::self->Lock();

  width = bwidth;
  height = bheight;
  ComputeSliderSize();

  if (owner) owner->RecalculateChildren(this);

  TGUI::self->Unlock();
}


bool TGUI_SLIDER::MouseDown(GLfloat mouse_x, GLfloat mouse_y, int button)
{
  // cursor is not over box
  if (!(visible && enabled && InBox(mouse_x, mouse_y))) return false;

  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (type == GUI_ST_HORIZONTAL) SetPosition((mouse_x - x - swidth/2) / (width - swidth));
    else SetPosition((mouse_y - y - sheight/2) / (height - sheight));
  }

  Focus();
  if (on_mouse_down)
    TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_down, this, mouse_x - x, mouse_y - y, button));

  if (lock_mouse_down && button == GLFW_MOUSE_BUTTON_LEFT) TGUI::down_box = this;

  return true;
}


bool TGUI_SLIDER::MouseMove(GLfloat mouse_x, GLfloat mouse_y)
{
  // cursor is not over box
  if (!(visible && InBox(mouse_x, mouse_y))) return false;

  if (enabled) {
    if (TGUI::down_box == this) {
      if (type == GUI_ST_HORIZONTAL) SetPosition((mouse_x - x - swidth/2) / (width - swidth));
      else SetPosition((mouse_y - y - sheight/2) / (height - sheight));
    }

    if (on_mouse_move)
      TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_move, this, mouse_x - x, mouse_y - y));
  }

  SetHoverBox(this);
  return true;
}


void TGUI_SLIDER::Draw(GLfloat dx, GLfloat dy)
{
  if (!visible) return;

  GLfloat *fcolor = face_color;
  GLfloat *hcolor = hover_color;
  GLfloat ralpha = alpha;
  GLfloat sx, sy;
  
  if (type == GUI_ST_HORIZONTAL) {
    sx = GLfloat(int((width - 2*padding - swidth) * position + padding));
    sy = padding;
  }
  else {
    sx = padding;
    sy = GLfloat(int((height - 2*padding - sheight) * position + padding));
  }

  bool hover = (TGUI::hover_box == this) && (!TGUI::down_box || TGUI::down_box == this) && enabled;

  if (!fcolor) fcolor = TGUI::self->GetFaceColor();
  if (!hcolor) hcolor = TGUI::self->GetHoverColor();
  if (!enabled) ralpha = alpha / 3;

  TGUI::self->SetClipRectangle(x + dx, y + dy, width, height, 0.0f);

  // base rectangle
  if (ralpha) DrawFaceRectangle(0, 0, width, height, fcolor, ralpha, true, true);

  // slider rectangle
  if (hover) DrawFaceRectangle(sx, sy, swidth, sheight, hcolor, 1, true, false);
  else DrawFaceRectangle(sx, sy, swidth, sheight, fcolor, 1, true, false);

  // black square
  if (swidth > 8 && sheight > 8) {
    GLfloat fr = 4.0f;

    glDisable(GL_TEXTURE_2D);
    glColor3f(0, 0, 0);

    glBegin(GL_QUADS);
      glVertex2f(sx + fr, sy + fr);
      glVertex2f(sx + fr, sy + sheight - fr);
      glVertex2f(sx + swidth - fr, sy + sheight - fr);
      glVertex2f(sx + swidth - fr, sy + fr);
    glEnd();
    glEnable(GL_TEXTURE_2D);
  }

  if (on_draw) on_draw(this);

  TGUI::self->UnsetClipRectangle();
}


//=========================================================================
// TGUI_SCROLL_BOX
//=========================================================================

void TGUI_SCROLL_BOX::ResetSliders()
{
  TGUI::self->Lock();

  sliders[GUI_ST_HORIZONTAL]->SetPosition(0);
  sliders[GUI_ST_VERTICAL]->SetPosition(1);

  TGUI::self->Unlock();
}


void TGUI_SCROLL_BOX::ShowSlider(TGUI_SLIDER_TYPE type)
{
  TGUI::self->Lock();

  if (sliders[type]->IsVisible()) {
    TGUI::self->Unlock();
    return;
  }

  GLfloat pom;

  switch (type) {
  case GUI_ST_HORIZONTAL:

    if (sliders[GUI_ST_VERTICAL]->IsVisible()) { 
      sliders[GUI_ST_VERTICAL]->SetPosY(sliders[GUI_ST_HORIZONTAL]->GetHeight());
      sliders[GUI_ST_VERTICAL]->SetHeight(height - sliders[GUI_ST_HORIZONTAL]->GetHeight());

      sliders[GUI_ST_HORIZONTAL]->SetWidth(width - sliders[GUI_ST_VERTICAL]->GetWidth());
    }
    else sliders[GUI_ST_HORIZONTAL]->SetWidth(width);
   
    break;

  case GUI_ST_VERTICAL:

    if (sliders[GUI_ST_HORIZONTAL]->IsVisible()) {
      sliders[GUI_ST_VERTICAL]->SetPosY(sliders[GUI_ST_HORIZONTAL]->GetHeight());
      sliders[GUI_ST_VERTICAL]->SetHeight(height - sliders[GUI_ST_HORIZONTAL]->GetHeight());

      sliders[GUI_ST_HORIZONTAL]->SetWidth(width - sliders[GUI_ST_VERTICAL]->GetWidth());
    }
    else {
      sliders[GUI_ST_VERTICAL]->SetPosY(0);
      sliders[GUI_ST_VERTICAL]->SetHeight(height);
    }

    break;

  default: break;
  }

  // show slider
  sliders[type]->Show();

  // compute new sliders position
  if (sliders[GUI_ST_HORIZONTAL]->IsVisible()) {
    pom = ch_envelope.GetMaxX() - width + 2 * padding;
    if (sliders[GUI_ST_VERTICAL]) pom += sliders[GUI_ST_VERTICAL]->GetWidth();
    if (pom > 0) pom = ch_dx / pom;
    else pom = 0.0f;

    sliders[GUI_ST_HORIZONTAL]->SetPosition(pom);
  }

  if (sliders[GUI_ST_VERTICAL]->IsVisible()) {
    pom = ch_envelope.GetMaxY() - height + 2 * padding;
    if (sliders[GUI_ST_HORIZONTAL]) pom += sliders[GUI_ST_HORIZONTAL]->GetHeight();
    if (ch_dy > 0) pom = ch_dy / pom;
    else pom = 0.0f;

    sliders[GUI_ST_VERTICAL]->SetPosition(1 - pom);
  }

  TGUI::self->Unlock();
}


void TGUI_SCROLL_BOX::HideSlider(TGUI_SLIDER_TYPE type)
{
  TGUI::self->Lock();

  if (!sliders[type]->IsVisible()) {
    TGUI::self->Unlock();
    return;
  }

  if (type == GUI_ST_HORIZONTAL) {
    if (sliders[GUI_ST_VERTICAL]->IsVisible()) {
      sliders[GUI_ST_VERTICAL]->SetPosY(0);
      sliders[GUI_ST_VERTICAL]->SetHeight(height);
    }
  }
  else {
    if (sliders[GUI_ST_HORIZONTAL]->IsVisible()) {
      sliders[GUI_ST_HORIZONTAL]->SetWidth(width);
    }
  }

  sliders[type]->Hide();

  TGUI::self->Unlock();
}


void TGUI_SCROLL_BOX::SetWidth(GLfloat bwidth)
{
  TGUI::self->Lock();

  width = bwidth;

  if (sliders[GUI_ST_HORIZONTAL]->IsVisible()) {
    if (sliders[GUI_ST_VERTICAL]->IsVisible())
      sliders[GUI_ST_HORIZONTAL]->SetWidth(width - sliders[GUI_ST_VERTICAL]->GetWidth());
    else sliders[GUI_ST_HORIZONTAL]->SetWidth(width);
  }

  if (sliders[GUI_ST_VERTICAL]->IsVisible()) {
    sliders[GUI_ST_VERTICAL]->SetPosX(width - sliders[GUI_ST_VERTICAL]->GetWidth());
  }

  if (owner) owner->RecalculateChildren(this);

  TGUI::self->Unlock();
}


void TGUI_SCROLL_BOX::SetHeight(GLfloat bheight)
{
  TGUI::self->Lock();

  height = bheight;

  if (sliders[GUI_ST_VERTICAL]->IsVisible()) {
    if (sliders[GUI_ST_HORIZONTAL]->IsVisible())
      sliders[GUI_ST_VERTICAL]->SetHeight(height - sliders[GUI_ST_HORIZONTAL]->GetHeight());
    else sliders[GUI_ST_VERTICAL]->SetHeight(height);
  }

  if (owner) owner->RecalculateChildren(this);

  TGUI::self->Unlock();
}


void TGUI_SCROLL_BOX::SetSize(GLfloat bwidth, GLfloat bheight)
{
  TGUI::self->Lock();

  SetWidth(bwidth);
  SetHeight(bheight);

  TGUI::self->Unlock();
}


GLfloat TGUI_SCROLL_BOX::GetClientWidth()
{
  TGUI::self->Lock();
  GLfloat f = width - 2*padding  - (sliders[GUI_ST_VERTICAL]->IsVisible() ? sliders[GUI_ST_VERTICAL]->GetWidth() : 0.0f);
  TGUI::self->Unlock();

  return f;
}


GLfloat TGUI_SCROLL_BOX::GetClientHeight()
{
  TGUI::self->Lock();
  GLfloat f = height - 2*padding - (sliders[GUI_ST_HORIZONTAL]->IsVisible() ? sliders[GUI_ST_HORIZONTAL]->GetHeight() : 0.0f);
  TGUI::self->Unlock();

  return f;
}


void TGUI_SCROLL_BOX::Update(double time_shift)
{
  if (!visible) return;

  TGUI_BOX *box;

  sliders[GUI_ST_HORIZONTAL]->Update(time_shift);
  sliders[GUI_ST_VERTICAL]->Update(time_shift);

  // horizontal slider
  ch_dx = width - 2 * padding - ch_envelope.GetMaxX();
  if (sliders[GUI_ST_VERTICAL]->IsVisible()) ch_dx -= sliders[GUI_ST_VERTICAL]->GetWidth();
  if (ch_dx < 0) ch_dx += ch_envelope.GetMinX();
  else ch_dx = ch_envelope.GetMinX();

  ch_dx *= sliders[GUI_ST_HORIZONTAL]->GetPosition();
  ch_dx = short(ch_dx - ch_envelope.GetMinX());
  
  // vertical slider
  ch_dy = height - 2 * padding - ch_envelope.GetMaxY();
  if (sliders[GUI_ST_HORIZONTAL]->IsVisible()) ch_dy -= sliders[GUI_ST_HORIZONTAL]->GetHeight();
  if (ch_dy < 0) ch_dy += ch_envelope.GetMinY();
  else ch_dy = ch_envelope.GetMinY();

  ch_dy *= sliders[GUI_ST_VERTICAL]->GetPosition();
  ch_dy = short(ch_dy - ch_envelope.GetMinY());

  // update animation
  if (animation) animation->Update(time_shift);

  // update children
  for (box = child_list; box; box = box->GetNext()) {
    box->Update(time_shift);
  }
}


void TGUI_SCROLL_BOX::Draw(GLfloat dx, GLfloat dy)
{
  if (!visible) return;

  TGUI_BOX *box;
  GLfloat *fcolor = face_color;
  GLfloat pw = width;
  GLfloat ph = height;
  GLfloat px = 0.0f;
  GLfloat py = 0.0f;

  if (sliders[GUI_ST_VERTICAL]->IsVisible()) {
    pw -= sliders[GUI_ST_VERTICAL]->GetWidth();
  }

  if (sliders[GUI_ST_HORIZONTAL]->IsVisible()) {
    ph -= sliders[GUI_ST_HORIZONTAL]->GetHeight();
    py = sliders[GUI_ST_HORIZONTAL]->GetHeight();
  }

  if (!fcolor) fcolor = TGUI::self->GetFaceColor();

  TGUI::self->SetClipRectangle(x + dx, y + dy, width, height, 0.0f);

  if (alpha) DrawFaceRectangle(px, py, pw, ph, fcolor, alpha, true, true);

  if (on_draw) on_draw(this);

  if (child_list) {
    TGUI::self->SetClipRectangle(px, py, pw, ph, padding);

    // draws children
    for (box = child_list; box; box = box->GetNext()) {
      box->Draw(ch_dx, ch_dy);
    }

    TGUI::self->UnsetClipRectangle();
  }

  if (sliders[GUI_ST_HORIZONTAL]->IsVisible()) sliders[GUI_ST_HORIZONTAL]->Draw(0, 0);
  if (sliders[GUI_ST_VERTICAL]->IsVisible()) sliders[GUI_ST_VERTICAL]->Draw(0, 0);

  TGUI::self->UnsetClipRectangle();
}


bool TGUI_SCROLL_BOX::MouseMove(GLfloat mouse_x, GLfloat mouse_y)
{
  // cursor is not over box
  if (!(visible && InBox(mouse_x, mouse_y))) return false;

  TGUI_BOX *box;
  bool over = false;

  // check if cursor is over child box
  if (!over && sliders[GUI_ST_HORIZONTAL]->IsVisible()) over = sliders[GUI_ST_HORIZONTAL]->MouseMove(mouse_x - x, mouse_y - y);
  if (!over && sliders[GUI_ST_VERTICAL]->IsVisible()) over = sliders[GUI_ST_VERTICAL]->MouseMove(mouse_x - x, mouse_y - y);
  if (!over) {
    GLfloat pomy = mouse_y - y - padding - ch_dy;

    if (sliders[GUI_ST_HORIZONTAL]->IsVisible()) pomy -= sliders[GUI_ST_HORIZONTAL]->GetHeight();

    // check if cursor is over child box
    for (box = last_child; box && !over; box = box->GetPrev()) {
      over = box->MouseMove(mouse_x - x - padding - ch_dx, pomy);
    }
  }

  if (!over) {
    if (on_mouse_move && enabled)
      TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_move, this, mouse_x - x, mouse_y - y));
    SetHoverBox(this);
  }

  return true;
}


bool TGUI_SCROLL_BOX::MouseDown(GLfloat mouse_x, GLfloat mouse_y, int button)
{
  // cursor is not over box
  if (!(visible && enabled && InBox(mouse_x, mouse_y))) return false;

  TGUI_BOX *box;
  bool over = false;

  // check if cursor is over child box
  if (!over && sliders[GUI_ST_HORIZONTAL]->IsVisible()) over = sliders[GUI_ST_HORIZONTAL]->MouseDown(mouse_x - x, mouse_y - y, button);
  if (!over && sliders[GUI_ST_VERTICAL]->IsVisible()) over = sliders[GUI_ST_VERTICAL]->MouseDown(mouse_x - x, mouse_y - y, button);
  if (!over) {
    GLfloat pomy = mouse_y - y - padding - ch_dy;

    if (sliders[GUI_ST_HORIZONTAL]->IsVisible()) pomy -= sliders[GUI_ST_HORIZONTAL]->GetHeight();

    // check if cursor is over child box
    for (box = last_child; box && !over; box = box->GetPrev()) {
      over = box->MouseDown(mouse_x - x - padding - ch_dx, pomy, button);
    }

    if (floating) Hide();
  }

  Focus();

  if (!over) {
    if (on_mouse_down)
      TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_down, this, mouse_x - x, mouse_y - y, button));

    if (lock_mouse_down && button == GLFW_MOUSE_BUTTON_LEFT) TGUI::down_box = this;
  }

  return true;
}


bool TGUI_SCROLL_BOX::MouseUp(GLfloat mouse_x, GLfloat mouse_y, int button)
{
  // cursor is not over box
  if (!(visible && enabled && InBox(mouse_x, mouse_y))) return false;

  TGUI_BOX *box;
  bool over = false;

  // check if cursor is over child box
  if (!over && sliders[GUI_ST_HORIZONTAL]->IsVisible()) over = sliders[GUI_ST_HORIZONTAL]->MouseUp(mouse_x - x, mouse_y - y, button);
  if (!over && sliders[GUI_ST_VERTICAL]->IsVisible()) over = sliders[GUI_ST_VERTICAL]->MouseUp(mouse_x - x, mouse_y - y, button);
  if (!over) {
    GLfloat pomy = mouse_y - y - padding - ch_dy;

    if (sliders[GUI_ST_HORIZONTAL]->IsVisible()) pomy -= sliders[GUI_ST_HORIZONTAL]->GetHeight();

    // check if cursor is over child box
    for (box = last_child; box && !over; box = box->GetPrev()) {
      over = box->MouseUp(mouse_x - x - padding - ch_dx, pomy, button);
    }
  }

  if (!over) {
    if (on_mouse_up)
      TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_up, this, mouse_x - x, mouse_y - y, button));

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
      if (TGUI::down_box == this && on_mouse_click)
        TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_click, this));

      TGUI::down_box = NULL;
    }
  }

  return true;
}


TGUI_BOX *TGUI_SCROLL_BOX::AddChild(TGUI_BOX *box)
{
  TGUI::self->Lock();

  if (!TGUI_PANEL::AddChild(box)) {
    TGUI::self->Unlock();
    return NULL;
  }

  ch_envelope.TestBox(box);
  TGUI::self->Unlock();

  return box;
}


void TGUI_SCROLL_BOX::RecalculateChildren(TGUI_BOX *sender)
{  
  if (sender == sliders[GUI_ST_VERTICAL] && sliders[GUI_ST_VERTICAL]->IsVisible()) {
    if (sliders[GUI_ST_VERTICAL]->GetPosX() != width - sliders[GUI_ST_VERTICAL]->GetWidth()) {
      sliders[GUI_ST_VERTICAL]->SetPosX(width - sliders[GUI_ST_VERTICAL]->GetWidth());

      if (sliders[GUI_ST_HORIZONTAL]->IsVisible()) sliders[GUI_ST_HORIZONTAL]->SetWidth(width - sliders[GUI_ST_VERTICAL]->GetWidth());
    }
  }
  else if (sender == sliders[GUI_ST_HORIZONTAL] && sliders[GUI_ST_HORIZONTAL]->IsVisible()) {
    if (sliders[GUI_ST_VERTICAL]->IsVisible()) {
      sliders[GUI_ST_VERTICAL]->SetPosY(sliders[GUI_ST_HORIZONTAL]->GetHeight());
      sliders[GUI_ST_VERTICAL]->SetHeight(height - sliders[GUI_ST_HORIZONTAL]->GetHeight());
    }
  }
  else {
    TGUI_BOX *box;

    ch_envelope.Reset();

    // check if cursor is over child box
    for (box = last_child; box; box = box->GetPrev()) {
      ch_envelope.TestBox(box);
    }
  }
}


void TGUI_SCROLL_BOX::Clear()
{
  TGUI::self->Lock();

  TGUI_PANEL::Clear();
  ch_envelope.Reset();
  ch_dx = ch_dy = 0.0f;
  ResetSliders();

  TGUI::self->Unlock();
}


//=========================================================================
// TGUI_LIST_BOX
//=========================================================================

void TGUI_LIST_BOX::ResetSliders()
{
  TGUI::self->Lock();

  if (sliders[GUI_ST_HORIZONTAL]) sliders[GUI_ST_HORIZONTAL]->SetPosition(0);
  if (sliders[GUI_ST_VERTICAL]) sliders[GUI_ST_VERTICAL]->SetPosition(1);

  TGUI::self->Unlock();
}


void TGUI_LIST_BOX::ShowSlider(TGUI_SLIDER_TYPE type)
{
  if (sliders[type]) return;

  GLfloat w, h, x, y;
  GLfloat pom;

  TGUI::self->Lock();

  switch (type) {
  case GUI_ST_HORIZONTAL:
    x = 0;
    y = 0;
    h = GUI_SLIDER_SIZE;

    if (sliders[GUI_ST_VERTICAL]) { 
      w = width - GUI_SLIDER_SIZE;      

      sliders[GUI_ST_VERTICAL]->SetHeight(height - GUI_SLIDER_SIZE);
      sliders[GUI_ST_VERTICAL]->SetPos(width - GUI_SLIDER_SIZE, GUI_SLIDER_SIZE);
    }
    else w = width;

    //items->SetPos(0, height - items->GetHeight() - GUI_SLIDER_SIZE);
    
    break;

  case GUI_ST_VERTICAL:
    x = width - GUI_SLIDER_SIZE;
    w = GUI_SLIDER_SIZE;

    if (sliders[GUI_ST_HORIZONTAL]) {      
      y = GUI_SLIDER_SIZE;
      h = height - GUI_SLIDER_SIZE;
      
      sliders[GUI_ST_HORIZONTAL]->SetWidth(width - GUI_SLIDER_SIZE);
    }
    else {
      y = 0;
      h = height;
    }

    break;

  default:
    /* The default branch is here to avoid gcc warnings. Normally, this part
     * should be never reached. */
    w = h = x = y = 0;
  }

  // create new slider
  sliders[type] = GUI_NEW TGUI_SLIDER(this, type, x, y, w, h, type);

  // set new sliders position
  if (sliders[GUI_ST_HORIZONTAL]) {
    pom = ch_envelope.GetMaxX() - width + 2 * padding;
    if (sliders[GUI_ST_VERTICAL]) pom += GUI_SLIDER_SIZE;
    if (pom > 0) pom = ch_dx / pom;
    else pom = 0.0f;

    sliders[GUI_ST_HORIZONTAL]->SetPosition(pom);
  }

  if (sliders[GUI_ST_VERTICAL]) {
    pom = ch_envelope.GetMaxY() - height + 2 * padding;
    if (sliders[GUI_ST_HORIZONTAL]) pom += GUI_SLIDER_SIZE;
    if (ch_dy > 0) pom = ch_dy / pom;
    else pom = 0.0f;

    sliders[GUI_ST_VERTICAL]->SetPosition(1 - pom);
  }

  TGUI::self->Unlock();
}


void TGUI_LIST_BOX::HideSlider(TGUI_SLIDER_TYPE type)
{
  if (!sliders[type]) return;

  TGUI::self->Lock();

  if (type == GUI_ST_HORIZONTAL); //items->SetPos(0, height - items->GetHeight());
  else if (sliders[GUI_ST_HORIZONTAL]) sliders[GUI_ST_HORIZONTAL]->SetWidth(width);

  delete sliders[type];
  sliders[type] = NULL;

  TGUI::self->Unlock();
}


GLfloat TGUI_LIST_BOX::GetClientWidth()
{
  TGUI::self->Lock();
  GLfloat f = width - 2*padding  - (sliders[GUI_ST_VERTICAL] ? sliders[GUI_ST_VERTICAL]->GetWidth() : 0.0f);
  TGUI::self->Unlock();

  return f;
}


GLfloat TGUI_LIST_BOX::GetClientHeight()
{
  TGUI::self->Lock();
  GLfloat f = height - 2*padding - (sliders[GUI_ST_HORIZONTAL] ? sliders[GUI_ST_HORIZONTAL]->GetHeight() : 0.0f);
  TGUI::self->Unlock();

  return f;
}


void TGUI_LIST_BOX::Update(double time_shift)
{
  if (!visible) return;

  if (sliders[GUI_ST_HORIZONTAL]) sliders[GUI_ST_HORIZONTAL]->Update(time_shift);
  if (sliders[GUI_ST_VERTICAL]) sliders[GUI_ST_VERTICAL]->Update(time_shift);

  if (sliders[GUI_ST_HORIZONTAL]) {
    ch_dx = width - 2 * padding - ch_envelope.GetMaxX();
    if (sliders[GUI_ST_VERTICAL]) ch_dx -= GUI_SLIDER_SIZE;
    if (ch_dx < 0) ch_dx += ch_envelope.GetMinX();
    else ch_dx = ch_envelope.GetMinX();

    ch_dx *= sliders[GUI_ST_HORIZONTAL]->GetPosition();
    ch_dx = short(ch_dx - ch_envelope.GetMinX());
  }

  if (sliders[GUI_ST_VERTICAL]) {
    ch_dy = height - 2 * padding - ch_envelope.GetMaxY();
    if (sliders[GUI_ST_HORIZONTAL]) ch_dy -= GUI_SLIDER_SIZE;
    if (ch_dy < 0) ch_dy += ch_envelope.GetMinY();
    else ch_dy = ch_envelope.GetMinY();

    ch_dy *= sliders[GUI_ST_VERTICAL]->GetPosition();
    ch_dy = short(ch_dy - ch_envelope.GetMinY());
  }

  // update children
  items->Update(time_shift);
}


void TGUI_LIST_BOX::Draw(GLfloat dx, GLfloat dy)
{
  if (!visible) return;

  GLfloat *fcolor = face_color;
  GLfloat pw = width;
  GLfloat ph = height;
  GLfloat px = 0.0f;
  GLfloat py = 0.0f;

  if (sliders[GUI_ST_VERTICAL]) {
    pw -= GUI_SLIDER_SIZE;
  }

  if (sliders[GUI_ST_HORIZONTAL]) {
    ph -= GUI_SLIDER_SIZE;
    py = GUI_SLIDER_SIZE;
  }

  if (!fcolor) fcolor = TGUI::self->GetFaceColor();

  TGUI::self->SetClipRectangle(x + dx, y + dy, width, height, 0.0f);

  if (alpha) DrawBgRectangle(px, py, pw, ph, fcolor, NULL, alpha, true);

  TGUI::self->SetClipRectangle(px, py, pw, ph, padding);
  
  items->Draw(ch_dx, ch_dy);
  if (on_draw) on_draw(this);

  TGUI::self->UnsetClipRectangle();

  if (sliders[GUI_ST_HORIZONTAL]) sliders[GUI_ST_HORIZONTAL]->Draw(0, 0);
  if (sliders[GUI_ST_VERTICAL]) sliders[GUI_ST_VERTICAL]->Draw(0, 0);

  TGUI::self->UnsetClipRectangle();
}


bool TGUI_LIST_BOX::MouseMove(GLfloat mouse_x, GLfloat mouse_y)
{
  // cursor is not over box
  if (!(visible && InBox(mouse_x, mouse_y))) return false;

  bool over = false;

  // check if cursor is over child box
  if (!over && sliders[GUI_ST_HORIZONTAL]) over = sliders[GUI_ST_HORIZONTAL]->MouseMove(mouse_x - x, mouse_y - y);
  if (!over && sliders[GUI_ST_VERTICAL]) over = sliders[GUI_ST_VERTICAL]->MouseMove(mouse_x - x, mouse_y - y);
  if (!over) {
    GLfloat pomy = mouse_y - y - padding - ch_dy;

    if (sliders[GUI_ST_HORIZONTAL]) pomy -= GUI_SLIDER_SIZE;

    // check if cursor is over child box
    over = items->MouseMove(mouse_x - x - padding - ch_dx, pomy);
  }

  if (!over) {
    if (on_mouse_move && enabled)
      TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_move, this, mouse_x - x, mouse_y - y));
    SetHoverBox(this);
  }

  return true;
}


bool TGUI_LIST_BOX::MouseDown(GLfloat mouse_x, GLfloat mouse_y, int button)
{
  // cursor is not over box
  if (!(visible && enabled && InBox(mouse_x, mouse_y))) return false;

  bool over = false;

  // check if cursor is over child box
  if (!over && sliders[GUI_ST_HORIZONTAL]) over = sliders[GUI_ST_HORIZONTAL]->MouseDown(mouse_x - x, mouse_y - y, button);
  if (!over && sliders[GUI_ST_VERTICAL]) over = sliders[GUI_ST_VERTICAL]->MouseDown(mouse_x - x, mouse_y - y, button);
  if (!over) {
    int ii = items->GetItemIndex();
    GLfloat pomy = mouse_y - y - padding - ch_dy;

    if (sliders[GUI_ST_HORIZONTAL]) pomy -= GUI_SLIDER_SIZE;

    over = items->MouseDown(mouse_x - x - padding - ch_dx, pomy, button);

    if (over && ii != items->GetItemIndex()) {
      if (on_change)
        TGUI::self->callback_list.PushBack(TCALLBACK(on_change, this, items->GetItemIndex()));
    }

    if (floating) Hide();
  }

  
  if (!floating) Focus();

  if (!over) {
    if (on_mouse_down)
      TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_down, this, mouse_x - x, mouse_y - y, button));

    if (lock_mouse_down && button == GLFW_MOUSE_BUTTON_LEFT) TGUI::down_box = this;
  }

  return true;
}


bool TGUI_LIST_BOX::MouseUp(GLfloat mouse_x, GLfloat mouse_y, int button)
{
  // cursor is not over box
  if (!(visible && enabled && InBox(mouse_x, mouse_y))) return false;

  bool over = false;

  // check if cursor is over child box
  if (!over && sliders[GUI_ST_HORIZONTAL]) over = sliders[GUI_ST_HORIZONTAL]->MouseUp(mouse_x - x, mouse_y - y, button);
  if (!over && sliders[GUI_ST_VERTICAL]) over = sliders[GUI_ST_VERTICAL]->MouseUp(mouse_x - x, mouse_y - y, button);
  if (!over) {
    GLfloat pomy = mouse_y - y - padding - ch_dy;

    if (sliders[GUI_ST_HORIZONTAL]) pomy -= GUI_SLIDER_SIZE;

    over = items->MouseUp(mouse_x - x - padding - ch_dx, pomy, button);
  }

  if (!over) {
    if (on_mouse_up)
      TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_up, this, mouse_x - x, mouse_y - y, button));

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
      if (TGUI::down_box == this && on_mouse_click)
        TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_click, this));

      TGUI::down_box = NULL;
    }
  }

  return true;
}


void TGUI_LIST_BOX::SetItems(const char *itm)
{
  if (!items) return;
 
  TGUI::self->Lock();

  items->SetCaption(itm);
  items->SetPosY(GetClientHeight() - items->GetHeight());

  TGUI::self->Unlock();
}


void TGUI_LIST_BOX::RecalculateChildren(TGUI_BOX *sender)
{
  if (!items) return;

  ch_envelope.Reset();
  ch_envelope.TestBox(items);
}


//=========================================================================
// TGUI_COMBO_BOX
//=========================================================================

TGUI_COMBO_BOX::TGUI_COMBO_BOX(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight)
:TGUI_BOX(bowner, bkey, bx, by, bwidth, bheight)
{
  GLfloat y = GetAbsPosY() - 100;

  if (y < 0) y += 100 + bheight;

  list = TGUI::self->AddListBox(0, GetAbsPosX(), y, bwidth, 100);
  list->Hide();
  list->SetFloating(true);

  font = NULL;
  font_color = NULL;
  padding = 5.0f;
}


void TGUI_COMBO_BOX::SetFontColor(GLfloat r, GLfloat g, GLfloat b)
{
  SET_COLOR(font_color, r, g, b);
  list->SetFontColor(r, g, b);
}


void TGUI_COMBO_BOX::Draw(GLfloat dx, GLfloat dy)
{
  if (!visible) return;

  GLFfont *fnt = font;
  GLfloat *fcolor = face_color;
  GLfloat *hcolor = hover_color;
  GLfloat ralpha = alpha;

  GLfloat sx = width - 1 - GUI_SLIDER_SIZE;
  GLfloat sy = 1.0f;
  
  bool hover = (TGUI::hover_box == this) && (!TGUI::down_box || TGUI::down_box == this) && enabled;

  if (!fnt) fnt = TGUI::self->GetFont();
  if (!fcolor) fcolor = TGUI::self->GetFaceColor();
  if (!hcolor) hcolor = TGUI::self->GetHoverColor();
  if (!enabled) ralpha = alpha / 3;

  TGUI::self->SetClipRectangle(x + dx, y + dy, width, height, 0.0f);

  // base rect
  if (ralpha) DrawBgRectangle(0, 0, width, height, fcolor, NULL, ralpha, true);

  // arrow rect
  if (hover) DrawFaceRectangle(sx, sy, GUI_SLIDER_SIZE, height - 2, hcolor, alpha, true, false);
  else DrawFaceRectangle(sx, sy, GUI_SLIDER_SIZE, height - 2, fcolor, alpha, true, false);

  // black triangle
  {
    GLfloat fr = 4.0f;

    glDisable(GL_TEXTURE_2D);
    glColor3f(0, 0, 0);

    glBegin(GL_TRIANGLES);
      glVertex2f(sx + GUI_SLIDER_SIZE / 2 + 1, sy + fr);
      glVertex2f(sx + fr + 1, sy + height - 3 - fr);
      glVertex2f(sx + GUI_SLIDER_SIZE - fr + 1, sy + height - 3 - fr);
    glEnd();

    glEnable(GL_TEXTURE_2D);
  }

  if (fnt && list->GetItemIndex() >= 0) {
    GLfloat *fncolor = font_color;
    GLfloat y = height - 2*padding - fnt->fHeight;

    if (y < 0 && int(y) % 2) y--;
    y = GLfloat(int(y) / 2);

    if (padding) TGUI::self->SetClipRectangle(0.0f, 0.0f, width - GUI_SLIDER_SIZE, height, padding);

    if (!fncolor) fncolor = TGUI::self->GetFontColor();

    if (enabled) glColor3fv(fncolor);
    else glColor3f(fcolor[0] / GUI_LIGHT_COEF, fcolor[1] / GUI_LIGHT_COEF, fcolor[2] / GUI_LIGHT_COEF);

    // draws text
    if (list->GetItemIndex() >= 0) {
      glfDisable(GLF_RESET_PROJECTION);

      glfPrint(fnt, 0, y, list->GetSelected(), false);

      glfEnable(GLF_RESET_PROJECTION);
    }

    if (padding) TGUI::self->UnsetClipRectangle();
  }

  if (on_draw) on_draw(this);

  TGUI::self->UnsetClipRectangle();
}


bool TGUI_COMBO_BOX::MouseDown(GLfloat mouse_x, GLfloat mouse_y, int button)
{
  // cursor is not over box
  if (!(visible && enabled && InBox(mouse_x, mouse_y))) return false;

  if (button == GLFW_MOUSE_BUTTON_LEFT && !hover_box_hide) {
    list->ToggleVisible();
  }

  if (on_mouse_down)
    TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_down, this, mouse_x, mouse_y, button));

  return true;
}


//========================================================================
// TGUI_MESSAGE_BOX
//========================================================================

TGUI_MESSAGE_BOX::TGUI_MESSAGE_BOX(TGUI_BOX *bowner)
:TGUI_PANEL(bowner, 0, 0, 0, 300, 100)
{
  text = AddLabel(0, 0, 70, "MessageBox");

  buttons[0] = AddButton(GUI_MB_OK,     0, 0, 100, 25, "OK");
  buttons[1] = AddButton(GUI_MB_YES,    0, 0, 100, 25, "Yes");
  buttons[2] = AddButton(GUI_MB_NO,     0, 0, 100, 25, "No");
  buttons[3] = AddButton(GUI_MB_CANCEL, 0, 0, 100, 25, "Cancel");

  buttons[0]->SetCanFocus(false);
  buttons[1]->SetCanFocus(false);
  buttons[2]->SetCanFocus(false);
  buttons[3]->SetCanFocus(false);

  Hide();
  padding = 10;
  text_spacing = 20;
  can_focus=  true;
}


void TGUI_MESSAGE_BOX::Show(const char *txt, int butt)
{
  GLfloat w = 0;
  GLfloat gw = 0;
  GLfloat gh = 0;
  GLfloat dx = 0;

  TGUI::self->Lock();

  text->SetCaption(txt);

  buttons[0]->SetVisible((butt & GUI_MB_OK) == GUI_MB_OK);
  buttons[1]->SetVisible((butt & GUI_MB_YES) == GUI_MB_YES);
  buttons[2]->SetVisible((butt & GUI_MB_NO) == GUI_MB_NO);
  buttons[3]->SetVisible((butt & GUI_MB_CANCEL) == GUI_MB_CANCEL);

  // global width
  if ((butt & GUI_MB_OK) == GUI_MB_OK) w += buttons[0]->GetWidth() + padding;
  if ((butt & GUI_MB_YES) == GUI_MB_YES) w += buttons[1]->GetWidth() + padding;
  if ((butt & GUI_MB_NO) == GUI_MB_NO) w += buttons[2]->GetWidth() + padding;
  if ((butt & GUI_MB_CANCEL) == GUI_MB_CANCEL) w += buttons[3]->GetWidth() + padding;

  if (text->GetWidth() > (w - padding)) gw = text->GetWidth() + 2 * padding;
  else gw = w + padding;

  // global height
  gh = buttons[0]->GetHeight() + text->GetHeight() + 2 * text_spacing + padding;

  // set box properties
  SetSize(gw, gh);
  SetPos(GLfloat(int(owner->GetWidth() - gw) / 2), GLfloat(int(owner->GetHeight() - gh) / 2));

  // buttons positions
  dx = GLfloat(int(gw - w - padding) / 2);

  if ((butt & GUI_MB_OK) == GUI_MB_OK) {
    buttons[0]->SetPosX(dx);
    dx += buttons[0]->GetWidth() + padding;
  }
  if ((butt & GUI_MB_YES) == GUI_MB_YES) {
    buttons[1]->SetPosX(dx);
    dx += buttons[1]->GetWidth() + padding;
  }
  if ((butt & GUI_MB_NO) == GUI_MB_NO) {
    buttons[2]->SetPosX(dx);
    dx += buttons[2]->GetWidth() + padding;
  }
  if ((butt & GUI_MB_CANCEL) == GUI_MB_CANCEL) {
    buttons[3]->SetPosX(dx);
  }

  // text position
  text->SetPos(
    GLfloat(int(gw - text->GetWidth() - 2 * padding) / 2),
    GLfloat(buttons[0]->GetHeight() + text_spacing)
  );

  // show window
  ShowModal();
  Focus();

  TGUI::self->Unlock();
}


bool TGUI_MESSAGE_BOX::KeyDown(int key)
{
  switch (key) {
  case GLFW_KEY_ENTER:
  case GLFW_KEY_KP_ENTER:
    if (buttons[0]->IsVisible()) buttons[0]->OnMouseClick();
    else if (buttons[1]->IsVisible()) buttons[1]->OnMouseClick();
    return true;

  case GLFW_KEY_ESC:
    if (buttons[3]->IsVisible()) buttons[3]->OnMouseClick();
    else if (buttons[2]->IsVisible()) buttons[2]->OnMouseClick();
    else if (buttons[0]->IsVisible()) buttons[0]->OnMouseClick();
    return true;

  case 'O':
    if (buttons[0]->IsVisible()) buttons[0]->OnMouseClick(); return true;
  case 'Y':
    if (buttons[1]->IsVisible()) buttons[1]->OnMouseClick(); return true;
  case 'N':
    if (buttons[2]->IsVisible()) buttons[2]->OnMouseClick(); return true;
  case 'C':
    if (buttons[3]->IsVisible()) buttons[3]->OnMouseClick(); return true;
  }

  return false;
}


//=========================================================================
// TGUI
//=========================================================================

TGUI::TGUI()
:TGUI_PANEL(NULL, GUI_KEY, 0, 0, 10000, 10000)
{
  rlock = GUI_NEW TRECURSIVE_LOCK();

  self = this;
  mouse_over_box = false;
  lock_mouse_down = true;

  font = NULL;
  font_color = NULL;

  mice_x = mice_y = 0;
  cursor_height = 0;

  Reset();

  def_tooltip = GUI_NEW TGUI_LABEL(this, 10, 10, 100, "");
  def_tooltip->SetTransparent(false);
  def_tooltip->SetColor(1, 1, 0.9f);
  def_tooltip->SetPadding(3);

  message_box = GUI_NEW TGUI_MESSAGE_BOX(this);
}


TGUI::~TGUI()
{
  if (font_color) delete[] font_color;
  if (def_tooltip) delete def_tooltip;
  if (message_box) delete message_box;

  delete rlock;
  rlock = NULL;
}


void TGUI::SetFontColor(GLfloat r, GLfloat g, GLfloat b)
{
  Lock();

  SET_COLOR(font_color, r, g, b);

  Unlock();
}


void TGUI::Update(double time_shift)
{
  Lock();

  if (rescan_mouse) {
    RescanMouse();
    rescan_mouse = false;
  }

  hover_time += time_shift;

  TGUI_PANEL::Update(time_shift);

  if (TGUI::tooltip_box) TGUI::tooltip_box->Update(time_shift);

  // hide tooltip
  if (TGUI::tooltip_box && hover_time >= GUI_TOOLTIP_TIME) {
    TGUI::tooltip_box = NULL;
  }

  // show tooltip
  else if (TGUI::hover_box && !TGUI::tooltip_box && show_tooltip && hover_time >= GUI_TOOLTIP_DELAY && hover_time < GUI_TOOLTIP_TIME) {
    TGUI::hover_box->ShowTooltip();

    if (TGUI::tooltip_box) {
      GLfloat tx = mice_x - x;
      GLfloat ty = mice_y - y - TGUI::tooltip_box->GetHeight() - cursor_height;

      if(tx + TGUI::tooltip_box->GetWidth() > x + width) tx = x + width - TGUI::tooltip_box->GetWidth();
      if(ty < 0) ty = mice_y - y + 3; // +3 aby to nebolo nalepene ka kurzore

      TGUI::tooltip_box->SetPos(tx, ty);
    }
  }

  Unlock();
}


void TGUI::Draw(GLfloat dx, GLfloat dy)
{
  if (!visible) return;

  TGUI_BOX *box;

  Lock();

  glDisable(GL_DEPTH_TEST);

  SetClipRectangle(x + dx, y + dy, width, height, 0);

  if (animation && alpha) {
    glColor4f(face_color[0], face_color[1], face_color[2], alpha);
    animation->Draw(GLfloat(width), GLfloat(height));
  }

  if (on_draw) on_draw(this);

  if (child_list) {
    if (padding) SetClipRectangle(0, 0, width, height, padding);

    // draws children
    for (box = child_list; box; box = box->GetNext()) {
      box->Draw(0, 0);
    }

    if (padding) UnsetClipRectangle();
  }

  if (message_box) message_box->Draw(0, 0);
  if (TGUI::tooltip_box) TGUI::tooltip_box->Draw(0, 0);

  UnsetClipRectangle();

  Unlock();
}


void TGUI::PollEvents(void)
{
  TCALLBACK call;

  while (callback_list.PopFront(call)) {
    switch (call.type) {
    case NONE:
      call.none.func(call.none.p1);
      break;

    case FFI:
      call.ffi.func(call.ffi.p1, call.ffi.p2, call.ffi.p3, call.ffi.p4);
      break;

    case FF:
      call.ff.func(call.ff.p1, call.ff.p2, call.ff.p3);
      break;

    case F:
      call.f.func(call.f.p1, call.f.p2);
      break;

    case I:
      call.i.func(call.i.p1, call.i.p2);
      break;

    case B:
      call.b.func(call.b.p1, call.b.p2);
      break;

    case S:
      call.s.func(call.s.p1, call.s.p2);
      break;

    case NOTSET:
      /* should never happen. */
      break;
    }
  }
}


void TGUI::Reset(void)
{
  Lock();
  Clear();

  on_mouse_move = NULL;
  on_mouse_down = on_mouse_up = NULL;
  on_mouse_click = NULL;

  mice_x = mice_y = 0.0f;
  padding = 0.0f;

  SetTexture(NULL);
  SetColor(GUI_DEF_COLOR, GUI_DEF_COLOR, GUI_DEF_COLOR);
  SetFontColor(GUI_DEF_FONT_COLOR, GUI_DEF_FONT_COLOR, GUI_DEF_FONT_COLOR);
  font = NULL;

  Focus();
  SetHoverBox(NULL);
  TGUI::down_box = NULL;

  Unlock();
}


bool TGUI::SetClipRectangle(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLfloat padding)
{
  Lock();

  if (clip_level >= GUI_MAX_CLIP_LEVEL) {
    clip_level++;
    Unlock();
    return false;
  }

  clip_buffer[clip_level][0] = 0 + GUI_CLIP_EPSILON;
  clip_buffer[clip_level][1] = width - 2*padding + GUI_CLIP_EPSILON;
  clip_buffer[clip_level][2] = 0 + GUI_CLIP_EPSILON;
  clip_buffer[clip_level][3] = height - 2*padding + GUI_CLIP_EPSILON;

  if (clip_level) {
    if (-x - padding + clip_buffer[clip_level][0] > clip_buffer[clip_level - 1][0])
      clip_buffer[clip_level][0] = clip_buffer[clip_level - 1][0] + x + padding;

    if (x + padding + clip_buffer[clip_level][1] > clip_buffer[clip_level - 1][1])
      clip_buffer[clip_level][1] = clip_buffer[clip_level - 1][1] - x - padding;

    if (-y - padding + clip_buffer[clip_level][2] > clip_buffer[clip_level - 1][2])
      clip_buffer[clip_level][2] = clip_buffer[clip_level - 1][2] + y + padding;

    if (y + padding + clip_buffer[clip_level][3] > clip_buffer[clip_level - 1][3])
      clip_buffer[clip_level][3] = clip_buffer[clip_level - 1][3] - y - padding;
  }
  else {
    glLoadIdentity();
    glEnable(GL_CLIP_PLANE0);
    glEnable(GL_CLIP_PLANE1);
    glEnable(GL_CLIP_PLANE2);
    glEnable(GL_CLIP_PLANE3);
  }

  clip_eqn[0][3] = clip_buffer[clip_level][0];
  clip_eqn[1][3] = clip_buffer[clip_level][1];
  clip_eqn[2][3] = clip_buffer[clip_level][2];
  clip_eqn[3][3] = clip_buffer[clip_level][3];

  glPushMatrix();
  glTranslatef(x + padding, y + padding, 0.0f);

  glClipPlane(GL_CLIP_PLANE0, clip_eqn[0]);
  glClipPlane(GL_CLIP_PLANE1, clip_eqn[1]);
  glClipPlane(GL_CLIP_PLANE2, clip_eqn[2]);
  glClipPlane(GL_CLIP_PLANE3, clip_eqn[3]);

  clip_level++;

  Unlock();

  return true;
}


void TGUI::UnsetClipRectangle()
{
  if (!clip_level) return;

  Lock();

  clip_level--;

  if (clip_level >= GUI_MAX_CLIP_LEVEL) {
    Unlock();
    return;
  }

  glPopMatrix();

  if (!clip_level) {
    glDisable(GL_CLIP_PLANE0);
    glDisable(GL_CLIP_PLANE1);
    glDisable(GL_CLIP_PLANE2);
    glDisable(GL_CLIP_PLANE3);

    Unlock();
    return;
  }

  clip_eqn[0][3] = clip_buffer[clip_level - 1][0];
  clip_eqn[1][3] = clip_buffer[clip_level - 1][1];
  clip_eqn[2][3] = clip_buffer[clip_level - 1][2];
  clip_eqn[3][3] = clip_buffer[clip_level - 1][3];

  glClipPlane(GL_CLIP_PLANE0, clip_eqn[0]);
  glClipPlane(GL_CLIP_PLANE1, clip_eqn[1]);
  glClipPlane(GL_CLIP_PLANE2, clip_eqn[2]);
  glClipPlane(GL_CLIP_PLANE3, clip_eqn[3]);

  Unlock();
}


bool TGUI::RescanMouse(void)
{
  if (!visible) return false;

  bool over = false;

  Lock();

  // check if cursor is over child box
  if (TGUI::modal_box) {
    TGUI::modal_box->MouseMove(mice_x - x, mice_y - y);
    over = true;
  }
  else {
    TGUI_BOX *box;

    for (box = last_child; box && !over; box = box->GetPrev()) {
      over = box->MouseMove(mice_x - x, mice_y - y);
    }

    if (!over) {
      if (on_mouse_move && enabled)
        TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_move, this, mice_x - x, mice_y - y));

      SetHoverBox(this);
    }
  }

  mouse_over_box = over;

  Unlock();

  return true;
}


bool TGUI::MouseMove(GLfloat mouse_x, GLfloat mouse_y)
{
  Lock();

  mice_x = mouse_x;
  mice_y = mouse_y;

  bool b = RescanMouse();

  Unlock();

  return b;
}


bool TGUI::MouseDown(GLfloat mouse_x, GLfloat mouse_y, int button)
{
  Lock();

  // cursor is not over box
  if (!(visible && enabled && InBox(mouse_x, mouse_y))) {
    Unlock();
    return false;
  }

  bool over = false;

  if (TGUI::tooltip_box) {
    TGUI::tooltip_box = NULL;
    show_tooltip = false;
  }

  // check if cursor is over child box
  if (TGUI::modal_box) {
    over = TGUI::modal_box->MouseDown(mouse_x - x, mouse_y - y, button);
  }
  else {
    TGUI_BOX *box;

    if (TGUI::float_box) {
      over = TGUI::float_box->MouseDown(mouse_x - x, mouse_y - y, button);
      if (!over) {
        TGUI::float_box->Hide();
        hover_box_hide = true;
      }
    }

    for (box = last_child; box && !over; box = box->GetPrev()) {
      over = box->MouseDown(mouse_x - x, mouse_y - y, button);
    }

    hover_box_hide = false;

    if (!over) {
      Focus();
      if (on_mouse_down)
        TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_down, this, mouse_x - x, mouse_y - y, button));

      if (lock_mouse_down && button == GLFW_MOUSE_BUTTON_LEFT) TGUI::down_box = this;
    }
  }

  Unlock();

  return true;
}


bool TGUI::MouseUp(GLfloat mouse_x, GLfloat mouse_y, int button)
{
  Lock();

  // cursor is not over box
  if (!(visible && enabled && InBox(mouse_x, mouse_y))) {
    Unlock();
    return false;
  }

  bool over = false;

  // check if cursor is over child box
  if (TGUI::modal_box) {
    over = TGUI::modal_box->MouseUp(mouse_x - x, mouse_y - y, button);
  }
  else {
    TGUI_BOX *box;

    for (box = last_child; box && !over; box = box->GetPrev()) {
      over = box->MouseUp(mouse_x - x, mouse_y - y, button);
    }

    if (!over) {
      if (on_mouse_up)
        TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_up, this, mouse_x - x, mouse_y - y, button));

      if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (TGUI::down_box == this && on_mouse_click)
          TGUI::self->callback_list.PushBack(TCALLBACK(on_mouse_click, this));

        TGUI::down_box = NULL;
      }
    }
  }

  Unlock();

  return true;
}


bool TGUI::KeyDown(int key)
{
  bool edit = false;

  Lock();

  if (TGUI::focus_box && TGUI::focus_box != this) {
    edit = TGUI::focus_box->KeyDown(key);
    if (TGUI::float_box && TGUI::float_box != TGUI::focus_box) TGUI::float_box->Hide();
    if (TGUI::modal_box) edit = true;
  }
  else {
    if (TGUI::float_box) TGUI::float_box->Hide();
    if (on_key_down)
      TGUI::self->callback_list.PushBack(TCALLBACK(on_key_down, this, key));
  }

  Unlock();

  return edit;
}


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

