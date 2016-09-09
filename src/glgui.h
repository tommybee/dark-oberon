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
 *  @file glgui.h        
 *
 *  Game interface classes, include drawing diferent types of windows (panels,
 *  buttons).
 *
 *  @author Peter Knut
 *
 *  @date 2003, 2004, 2005
 */

#ifndef __glgui_h__
#define __glgui_h__

//=========================================================================
// Forward declarations
//=========================================================================

class TGUI_TEXTURE;
class TGUI_ANIMATION;
class TGUI_BOX;
class TGUI_LABEL;
class TGUI_BUTTON;
class TGUI_PANEL;
class TGUI_LIST_BOX;
class TGUI_COMBO_BOX;
class TGUI_SCROLL_BOX;
class TGUI_SLIDER;
class TGUI;

class TGUI_MESSAGE_BOX;


//=========================================================================
// Defines
//=========================================================================

#ifndef GUI_NEW
#define GUI_NEW new
#endif

#define GUI_KEY             -1

#define GUI_DEF_COLOR       0.7f
#define GUI_DEF_FONT_COLOR  0.0f
#define GUI_DEF_ALPHA       1.0f

#define GUI_LIGHT_COEF      1.4f
#define GUI_HOVER_COEF      1.15f

// button styles
#define GUI_BS_BUTTON       0
#define GUI_BS_CHECK        1
#define GUI_BS_GROUP        2

// animation state
#define GUI_AS_STOP         0        //!< Animation is stopped.
#define GUI_AS_PLAY         1        //!< Animation is running.
#define GUI_AS_PAUSE        2        //!< Animation is paused.

// texture typers
enum TGUI_TEX_TYPE {
  GUI_TT_NORMAL,                     //!< Clasic rectangular texture.
  GUI_TT_RHOMBUS                     //!< Special texture that stores rhombus image.
};

// button state
enum TGUI_BUTTON_STATE {
  GUI_BS_UP,
  GUI_BS_DOWN,
  GUI_BS_UP_HOVER,
  GUI_BS_DOWN_HOVER
};

#define GUI_BS_COUNT        4

// slider type
enum TGUI_SLIDER_TYPE {
  GUI_ST_HORIZONTAL,
  GUI_ST_VERTICAL
};

#define GUI_ST_COUNT        2


// def values
#define GUI_CHECK_BOX_SIZE  15.0f
#define GUI_SLIDER_SIZE     17.0f
#define GUI_TOOLTIP_DELAY   1
#define GUI_TOOLTIP_TIME    4


//=========================================================================
// Included files
//=========================================================================

#ifdef NEW_GLFW3
#include <glfw3.h>
#else
#include <glfw.h>
#endif

#include <stdio.h>
#include <string.h>

#include "glfont.h"
#include "doipc.h"


//=========================================================================
// Typedefs
//=========================================================================

typedef int     GUI_TEX_ID;
typedef GLfloat *TGUI_COLOR;


//=========================================================================
// TTEX_TABLE
//=========================================================================

/**
 *  Stores one animated texture.
 */
class TGUI_TEXTURE {
public:
  char *id;             //!< Texture string id.

  int width;
  int height;

  int frame_width;      //!< Width of one frame. [pixels]
  int frame_height;     //!< Height of one frame. [pixels]

  GLubyte frames_count; //!< Count of frames in texture.
  GLubyte h_count;
  GLubyte v_count;

  double frame_time;    //!< Animation time for one frame. [seconds]

  GLfloat point_x;      //!< X position of base point in textre. <0, 1>
  GLfloat point_y;      //!< Y position of base point in textre. <0, 1>

  GLenum gl_id;         //!< Texture identifier.
  TGUI_TEX_TYPE type;   //!< Type of texture

  /** Constructor. */
  TGUI_TEXTURE(void) {
    id = NULL;
    width = height = 0;
    frame_width = frame_height = 0;
    frames_count = h_count = v_count = 0;
    frame_time = 0.0;
    point_x = point_y = 0;
    gl_id = 0;
    type = GUI_TT_NORMAL;
  };

  void DrawFrame(int frame) { DrawFrame(frame, GLfloat(frame_width), GLfloat(frame_height)); }
  void DrawFrame(int frame, GLfloat w, GLfloat h);

  /** Destructor */
  ~TGUI_TEXTURE(void) { if (id) delete[] id; glDeleteTextures(1, &gl_id); };
};


//=========================================================================
// ANIMATION
//=========================================================================


/**
 *  This class is used for animated textures.
 *  It contains methods for drawing and automatic animation update in time.
 */
class TGUI_ANIMATION {
public:
  void Stop()                                   //!< Stops the animation.
    { state = GUI_AS_STOP; shift_time = 0.0;
      if (reverse) act_frame = tex_item->frames_count - 1; else act_frame = 0; }
  void Pause() { state = GUI_AS_PAUSE; }        //!< Pauses the playing of the animation.
  void Play()  { state = GUI_AS_PLAY;  }        //!< Plays the animation.

  void Hide()  { visible = false;  }            //!< Hides animation.
  void Show()  { visible = true;   }            //!< Shows animation.
  void SetVisible(bool vis) { visible = vis; }  //!< Sets animation visibility.
  bool IsVisible() { return visible; }          //!< Returns animation visibility.

  void Update(double time_shift);

  /**
   *  Draws actual frame of the animation.
   */
  void Draw()
    { if (visible && tex_item) tex_item->DrawFrame(act_frame); }
  /**
   *  Draws actual frame of the animation with different size.
   *
   *  @param width    New width.
   *  @param height   New height.
   */
  void Draw(GLfloat w, GLfloat h)
    { if (visible && tex_item) tex_item->DrawFrame(act_frame, w, h); }

  //! Assigns texture item @p titem to animation.
  bool SetTexItem(TGUI_TEXTURE *titem);

  void SetSpeedRatio(double ratio) { speed_ratio = ratio; }
  void ResetSpeedRatio() { speed_ratio = 1.0f; }

  void SetAnimTime(double atime) { frame_time = atime / tex_item->frames_count; }
  void ResetAnimTime() { frame_time = tex_item->frame_time; }

  void SetReverse(bool rev, bool reset_frame)
    { reverse = rev; if (reset_frame) act_frame = tex_item->frames_count - 1; }
  void SetLoop(bool setloop) { loop = setloop; }

  void SetRandomFrame();

  //! Returns texture item.
  TGUI_TEXTURE *GetTexItem() { return tex_item; }

  //! Returns frame width.
  int    GetFrameWidth()  { return tex_item ? tex_item->frame_width : 0;  }
  //! Returns frame height.
  int    GetFrameHeight() { return tex_item ? tex_item->frame_height : 0; }
  //! Returns texture ID.
  GLenum GetTexId()       { return tex_item ? tex_item->gl_id : 0;        }
  

  TGUI_ANIMATION(void);
  TGUI_ANIMATION(TGUI_TEXTURE *titem);

private:
  int state;                //!< State of animation. [AS_*]
  double shift_time;        //!< Time shift from last drawing. [seconds]
  double speed_ratio;       //!< Speed ratio for animation.
  double frame_time;        //!< Animation time for one frame. [seconds]
  int act_frame;            //!< Actual animation frame.
  bool reverse;             //!< If animation is playing in reverse order.
  bool loop;                //!< Play animation in loop.

  bool visible;             //!< Animation visibility.

  TGUI_TEXTURE *tex_item;   //!< Pointer to associated texture.
};


//=========================================================================
// TGUI_BOX_ENVELOPE
//=========================================================================

class TGUI_BOX_ENVELOPE {
private:
  GLfloat min_x;
  GLfloat max_x;
  GLfloat min_y;
  GLfloat max_y;

public:
  TGUI_BOX_ENVELOPE() { Reset(); }

  void Reset() { min_x = min_y = 0; max_x = max_y = 0; }
  void TestBox(TGUI_BOX *box);

  GLfloat GetMinX() { return min_x; }
  GLfloat GetMinY() { return min_y; }
  GLfloat GetMaxX() { return max_x; }
  GLfloat GetMaxY() { return max_y; }
};


//=========================================================================
// CALLBACKS
//=========================================================================

enum TCALLBACK_TYPE {
  NOTSET,
  NONE,
  FFI,
  FF,
  F,
  I,
  B,
  S
};


struct TCALLBACK {
  TCALLBACK_TYPE type;

  union {
    struct {
      void (*func) (TGUI_BOX *);
      TGUI_BOX *p1;
    } none;

    struct {
      void (*func) (TGUI_BOX *, GLfloat, GLfloat, int);
      TGUI_BOX *p1;
      GLfloat p2;
      GLfloat p3;
      int p4;
    } ffi;

    struct {
      void (*func) (TGUI_BOX *, GLfloat, GLfloat);
      TGUI_BOX *p1;
      GLfloat p2;
      GLfloat p3;
    } ff;

    struct {
      void (*func) (TGUI_BOX *, GLfloat);
      TGUI_BOX *p1;
      GLfloat p2;
    } f;

    struct {
      void (*func) (TGUI_BOX *, int);
      TGUI_BOX *p1;
      int p2;
    } i;

    struct {
      void (*func) (TGUI_BOX *, bool);
      TGUI_BOX *p1;
      bool p2;
    } b;

    struct {
      void (*func) (TGUI_BOX *, char *);
      TGUI_BOX *p1;
      char *p2;
    } s;
  };

  TCALLBACK() {
    type = NOTSET;
  }

  TCALLBACK(void (*func) (TGUI_BOX *), TGUI_BOX *p1) {
    type = NONE;
    none.func = func;
    none.p1 = p1;
  }

  TCALLBACK(void (*func) (TGUI_BOX *, GLfloat, GLfloat, int), TGUI_BOX *p1, GLfloat p2, GLfloat p3, int p4) {
    type = FFI;
    ffi.func = func;
    ffi.p1 = p1;
    ffi.p2 = p2;
    ffi.p3 = p3;
    ffi.p4 = p4;
  }

  TCALLBACK(void (*func) (TGUI_BOX *, GLfloat, GLfloat), TGUI_BOX *p1, GLfloat p2, GLfloat p3) {
    type = FF;
    ff.func = func;
    ff.p1 = p1;
    ff.p2 = p2;
    ff.p3 = p3;
  }

  TCALLBACK(void (*func) (TGUI_BOX *, GLfloat), TGUI_BOX *p1, GLfloat p2) {
    type = F;
    f.func = func;
    f.p1 = p1;
    f.p2 = p2;
  }

  TCALLBACK(void (*func) (TGUI_BOX *, int), TGUI_BOX *p1, int p2) {
    type = I;
    i.func = func;
    i.p1 = p1;
    i.p2 = p2;
  }

  TCALLBACK(void (*func) (TGUI_BOX *, bool), TGUI_BOX *p1, bool p2) {
    type = B;
    b.func = func;
    b.p1 = p1;
    b.p2 = p2;
  }

  TCALLBACK(void (*func) (TGUI_BOX *, char *), TGUI_BOX *p1, char *p2) {
    type = S;
    s.func = func;
    s.p1 = p1;
    s.p2 = p2;
  }
};


//=========================================================================
// TGUI_BOX
//=========================================================================

class TGUI_BOX {                                          //!represents common graphic window 
  friend class TGUI_TEXTURE;
  friend class TGUI_ANIMATION;
  friend class TGUI_LABEL;
  friend class TGUI_BUTTON;
  friend class TGUI_PANEL;
  friend class TGUI_LIST_BOX;
  friend class TGUI_COMBO_BOX;
  friend class TGUI_SCROLL_BOX;
  friend class TGUI_SLIDER;
  friend class TGUI;

protected:
  int key;

  TGUI_BOX *owner;
  int lock_mouse_down;

  GLfloat x, y;                                           //!< Coordinates of the window.
  GLfloat width, height;                                  //!< width,height of the window
  GLfloat padding;

  TGUI_BOX *next;                                         //!<instances of TGUI_BOX are connected to the linked list,used in TGUI_PANEL class as a list of its children
  TGUI_BOX *prev;

  TGUI_COLOR face_color;
  TGUI_COLOR hover_color;
  GLfloat alpha;

  bool can_focus;

  bool enabled;
  bool visible;
  bool floating;

  char *tooltip_text;
  TGUI_BOX *tooltip;

  void (*on_mouse_move)   (TGUI_BOX *sender, GLfloat mouse_x, GLfloat mouse_y);
  void (*on_mouse_down)   (TGUI_BOX *sender, GLfloat mouse_x, GLfloat mouse_y, int button);
  void (*on_mouse_up)     (TGUI_BOX *sender, GLfloat mouse_x, GLfloat mouse_y, int button);
  void (*on_mouse_click)  (TGUI_BOX *sender);
  void (*on_key_down)     (TGUI_BOX *sender, int key);
  void (*on_draw)         (TGUI_BOX *sender);
  void (*on_get_focus)    (TGUI_BOX *sender);
  void (*on_lose_focus)   (TGUI_BOX *sender);
  void (*on_show_tooltip) (TGUI_BOX *sender);

  bool InBox(GLfloat bx, GLfloat by)
  { return (bx >= x && bx <= (x + width) && by >= y && by <= (y + height)); }

  // list of boxes
  void SetNext(TGUI_BOX *n) { next = n; }                //!<sets instance of the class given as parameter to be the next class in the linked list
  void SetPrev(TGUI_BOX *p) { prev = p; }                //!<sets instance of the class given as parameter to be the previous class in the linked list

  virtual void RecalculateChildren(TGUI_BOX *sender) {}

  void OnMouseClick(void);

  virtual bool MouseMove(GLfloat mouse_x, GLfloat mouse_y);
  virtual bool MouseDown(GLfloat mouse_x, GLfloat mouse_y, int button);
  virtual bool MouseUp  (GLfloat mouse_x, GLfloat mouse_y, int button);

  virtual bool KeyDown  (int key);

  // drawing
  virtual void Update(double time_shift) {};
  virtual void Draw(GLfloat dx, GLfloat dy);

public:
  // constructing
  TGUI_BOX(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight);
  virtual ~TGUI_BOX(void);

  // position, size
  void SetPos(GLfloat bx, GLfloat by);
  void SetPosX(GLfloat bx);
  void SetPosY(GLfloat by);
  virtual void SetSize(GLfloat bwidth, GLfloat bheight);
  virtual void SetWidth(GLfloat bwidth);
  virtual void SetHeight(GLfloat bheight);

  void SetPadding(GLfloat bpadding) { padding = bpadding; };

  GLfloat GetPosX()    { return x; };
  GLfloat GetPosY()    { return y; };
  GLfloat GetAbsPosX() { return owner ? owner->GetAbsPosX() + owner->padding + x : x; };
  GLfloat GetAbsPosY() { return owner ? owner->GetAbsPosY() + owner->padding + y : y; };
  GLfloat GetWidth()   { return width; };
  GLfloat GetHeight()  { return height; };

  virtual GLfloat GetClientWidth() { return width - 2*padding; }
  virtual GLfloat GetClientHeight() { return height - 2*padding; }

  // appearance
  void SetColor(GLfloat r, GLfloat g, GLfloat b);
  void SetFaceColor(GLfloat r, GLfloat g, GLfloat b);
  void SetHoverColor(GLfloat r, GLfloat g, GLfloat b);
  void SetAlpha(GLfloat a) { alpha = a; };

  TGUI_COLOR GetFaceColor(void) { return face_color; };
  TGUI_COLOR GetHoverColor(void) { return hover_color; };
  GLfloat GetAlpha(void) { return alpha; };

  // properties
  void SetKey(int bkey) { key = bkey; }
  int GetKey(void) { return key; }

  TGUI_BOX *GetNext(void) { return next; };              //!<gets next item from the linked list
  TGUI_BOX *GetPrev(void) { return prev; };              //!<gets previous item from the linked list

  // behaviour
  virtual void Focus();
  virtual void Unfocus();

  void SetEnabled(bool en);
  bool IsEnabled(void) { return enabled; };
  void ToggleEnabled(void) { SetEnabled(!enabled); };

  virtual void SetVisible(bool vis);
  bool IsVisible(void) { return visible; };
  void Hide(void) { SetVisible(false); };
  void Show(void);
  void ShowModal(void);
  void ToggleVisible(void) { SetVisible(!visible); };

  void SetCanFocus(bool canf) { can_focus = canf; };
  void SetFloating(bool fl) { floating = fl; };
  void SetOwner(TGUI_BOX *bowner) { owner = bowner; };

  void SetTooltipText(char *text);
  void SetTooltipBox(TGUI_BOX *box) { tooltip = box; }
  void ShowTooltip();

  // events
  void SetOnMouseMove(void (*func)(TGUI_BOX *, GLfloat, GLfloat))
  { on_mouse_move = func; };
  void SetOnMouseDown(void (*func)(TGUI_BOX *, GLfloat, GLfloat, int))
  { on_mouse_down = func; };
  void SetOnMouseUp(void (*func)(TGUI_BOX *, GLfloat, GLfloat, int))
  { on_mouse_up = func; };
  void SetOnMouseClick(void (*func)(TGUI_BOX *))
  { on_mouse_click = func; };
  void SetOnKeyDown(void (*func)(TGUI_BOX *, int))
  { on_key_down = func; };
  void SetOnDraw(void (*func)(TGUI_BOX *))
  { on_draw = func; };
  void SetOnGetFocus(void (*func)(TGUI_BOX *))
  { on_get_focus = func; };
  void SetOnLoseFocus(void (*func)(TGUI_BOX *))
  { on_lose_focus = func; };
  void SetOnShowTooltip(void (*func)(TGUI_BOX *))
  { on_show_tooltip = func; };
};


//=========================================================================
// TGUI_LABEL
//=========================================================================

class TGUI_LABEL: public TGUI_BOX {
protected:
  int buff_size;
  char *caption;
  char **lines;
  int lines_count;
  bool autosize;
  bool transparent;

  GLFfont *font;
  TGUI_ANIMATION *animation;
  TGUI_COLOR font_color;
  GLfloat line_height;

  // drawing
  virtual void Update(double time_shift) { if (animation) animation->Update(time_shift); };
  virtual void Draw(GLfloat dx, GLfloat dy);

public:
  // constructing
  TGUI_LABEL(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight);
  TGUI_LABEL(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, const char *bcaption);
  TGUI_LABEL(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, TGUI_TEXTURE *tex);
  virtual ~TGUI_LABEL() {
    if (caption) delete[] caption;
    if (font_color) delete[] font_color;
    if (animation) delete animation;
    if (lines) delete[] lines;
  };

  virtual void SetCaption(const char *bcaption);
  char *GetLine(int id) { return id < lines_count && id >= 0 ? lines[id] : NULL; };
  int GetLinesCount() { return lines_count; }
  GLfloat GetLineHeight() { return line_height; }

  void SetTexture(TGUI_TEXTURE *tex);
  void SetFont(GLFfont *gui_font) { font = gui_font; };
  void SetFontColor(GLfloat r, GLfloat g, GLfloat b);
  void SetLineHeight(GLfloat lheight) { line_height = lheight; };
  void SetAutoSize(bool autos) { autosize = autos; };
  void SetTransparent(bool trans) { transparent = trans; }

  GLFfont *GetFont(void) { return font; };
  TGUI_COLOR GetFontColor(void) { return font_color; };
};


//=========================================================================
// TGUI_LIST
//=========================================================================

class TGUI_LIST: public TGUI_LABEL {
  friend class TGUI_LIST_BOX;

protected:
  TGUI_COLOR sel_font_color;
  TGUI_COLOR sel_color;

  int item_index;

  void (*on_change) (TGUI_BOX *sender, int i_index);

  // events
  bool MouseDown(GLfloat mouse_x, GLfloat mouse_y, int button);

  // drawing
  virtual void Draw(GLfloat dx, GLfloat dy);

public:

  TGUI_LIST(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, char *bitems)
  :TGUI_LABEL(bowner, bkey, bx, by, "") {
    sel_font_color = sel_color = NULL; item_index = -1; padding = 5.0f;
    SetItems(bitems); on_change = NULL;
    can_focus = true;
  }

  ~TGUI_LIST() {
    if (sel_font_color) delete[] sel_font_color;
    if (sel_color) delete[] sel_color;
  }

  void SetItems(const char *bitems) { SetCaption(bitems); };
  virtual void SetCaption(const char *bcaption);
  char *GetItem(int id) { return GetLine(id); };
  char *GetSelected() { return GetLine(item_index); };
  bool SetItem (const char *line);

  void SetSelFontColor(GLfloat r, GLfloat g, GLfloat b);
  void SetSelColor(GLfloat r, GLfloat g, GLfloat b);
  int GetItemIndex() { return item_index; };

  void SetOnChange(void (*func)(TGUI_BOX *, int)) { on_change = func; }
};


//=========================================================================
// TGUI_BUTTON
//=========================================================================

class TGUI_BUTTON: public TGUI_BOX {
  friend class TGUI_MESSAGE_BOX;

protected:
  TGUI_ANIMATION *animation[GUI_BS_COUNT];

  bool checked;
  char *caption;

  GLFfont *font;
  TGUI_COLOR font_color;

  int style;
  int group;

  void (*on_check) (TGUI_BOX *sender, bool checked);

  // events
  virtual bool MouseUp(GLfloat mouse_x, GLfloat mouse_y, int button);

  // drawing
  virtual void Update(double time_shift)
  { for (int i = 0; i < GUI_BS_COUNT; i++) if (animation[i]) animation[i]->Update(time_shift); };
  virtual void Draw(GLfloat dx, GLfloat dy);

public:
  // constructing
  TGUI_BUTTON(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight, const char *bcaption, int bstyle, int bgroup);
  TGUI_BUTTON(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, TGUI_TEXTURE *tex, int bstyle, int bgroup);

  virtual ~TGUI_BUTTON() {
    if (caption) delete[] caption;
    for (int i = 0; i < GUI_BS_COUNT; i++) if (animation[i]) delete animation[i];
    if (font_color) delete[] font_color;
  };

  // appearance
  void SetCaption(const char *cpt);
  void SetTexture(int state, TGUI_TEXTURE *tex);
  void SetFont(GLFfont *gui_font) { font = gui_font; };
  void SetFontColor(GLfloat r, GLfloat g, GLfloat b);

  GLFfont *GetFont(void) { return font; };
  TGUI_COLOR GetFontColor(void) { return font_color; };

  int GetStyle() { return style; };
  int GetGroup() { return group; };

  void SetChecked(bool ch);
  bool IsChecked(void) { return checked; };

  void SetOnCheck(void (*func)(TGUI_BOX *, bool)) { on_check = func; }
};


//=========================================================================
// TGUI_CHECK_BOX
//=========================================================================

class TGUI_CHECK_BOX: public TGUI_BUTTON {
protected:
  // drawing
  virtual void Draw(GLfloat dx, GLfloat dy);

public:
  // constructing
  TGUI_CHECK_BOX(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight, const char *bcaption, int bstyle, int bgroup)
    :TGUI_BUTTON(bowner, bkey, bx, by, bwidth, bheight, bcaption, bstyle, bgroup) {};
  TGUI_CHECK_BOX(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, TGUI_TEXTURE *tex, int bstyle, int bgroup)
    :TGUI_BUTTON(bowner, bkey, bx, by, tex, bstyle, bgroup) {};
};


//=========================================================================
// TGUI_EDIT_BOX
//=========================================================================

class TGUI_EDIT_BOX: public TGUI_BOX {
protected:
  char *text;

  GLFfont *font;
  TGUI_COLOR font_color;

  unsigned cursor_pos;
  unsigned max_len;
  unsigned text_len;

  GLfloat text_dx;

  void (*on_change) (TGUI_BOX *sender, char *txt);

  virtual bool MouseDown(GLfloat mouse_x, GLfloat mouse_y, int button);
  virtual bool KeyDown(int key);

  // drawing
  virtual void Draw(GLfloat dx, GLfloat dy);

public:
  TGUI_EDIT_BOX(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight, unsigned max_lenght);

  virtual ~TGUI_EDIT_BOX() { delete[] text; if (font_color) delete[] font_color; };

  // properties
  void SetText(char *txt);
  void SetFont(GLFfont *gui_font) { font = gui_font; };
  void SetFontColor(GLfloat r, GLfloat g, GLfloat b);
  char *GetText() { return text; };

  // behavior
  virtual void Unfocus() { text_dx = 0; cursor_pos = 0; TGUI_BOX::Unfocus(); };

  // events
  void SetOnChange(void (*func)(TGUI_BOX *, char *))
  { on_change = func; };
};


//=========================================================================
// TGUI_SLIDER
//=========================================================================

class TGUI_SLIDER: public TGUI_BOX {
  friend class TGUI_SCROLL_BOX;
  friend class TGUI_LIST_BOX;

protected:
  TGUI_SLIDER_TYPE type;
  GLfloat position;

  GLfloat swidth;
  GLfloat sheight;

  void (*on_change) (TGUI_BOX *sender, GLfloat position);

  void ComputeSliderSize();

  bool MouseDown(GLfloat mouse_x, GLfloat mouse_y, int button);
  bool MouseMove(GLfloat mouse_x, GLfloat mouse_y);

  // drawing
  virtual void Draw(GLfloat dx, GLfloat dy);

public:
  // constructing
  TGUI_SLIDER(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight, TGUI_SLIDER_TYPE btype);

  // properties
  void SetPosition(GLfloat pos);
  GLfloat GetPosition() { return position; }

  virtual void SetWidth(GLfloat bwidth);
  virtual void SetHeight(GLfloat bheight);
  virtual void SetSize(GLfloat bwidth, GLfloat bheight);

  // events
  void SetOnChange(void (*func)(TGUI_BOX *, GLfloat))
  { on_change = func; };
};


//=========================================================================
// TGUI_PANEL
//=========================================================================

class TGUI_PANEL: public TGUI_BOX {
  friend class TGUI;

protected:
  TGUI_BOX *child_list;
  TGUI_BOX *last_child;

  TGUI_ANIMATION *animation;

  void (*on_show) (TGUI_BOX *sender);

  // events
  virtual bool MouseMove(GLfloat mouse_x, GLfloat mouse_y);
  virtual bool MouseDown(GLfloat mouse_x, GLfloat mouse_y, int button);
  virtual bool MouseUp  (GLfloat mouse_x, GLfloat mouse_y, int button);

  // drawing
  virtual void Update(double time_shift);
  virtual void Draw(GLfloat dx, GLfloat dy);

public:
  // constructing
  TGUI_PANEL(TGUI_BOX *bowner, int pkey, GLfloat px, GLfloat py, GLfloat pwidth, GLfloat pheight)
    :TGUI_BOX(bowner, pkey, px, py, pwidth, pheight)
  { animation = NULL; child_list = last_child = NULL; padding = 5.0f; on_show = NULL; };


  TGUI_PANEL(TGUI_BOX *bowner, int pkey, GLfloat px, GLfloat py, TGUI_TEXTURE *tex)
    :TGUI_BOX(bowner, pkey, px, py, 0, 0)
  {
    animation = NULL; child_list = last_child = NULL;
    SetTexture(tex);
    SetColor(1, 1, 1);
    width = (GLfloat)animation->GetFrameWidth();
    height = (GLfloat)animation->GetFrameHeight();
    on_show = NULL;
  };

  virtual ~TGUI_PANEL();

  // properties
  void SetTexture(TGUI_TEXTURE *tex);
  virtual void SetVisible(bool vis);

  // children
  virtual TGUI_BOX  *AddChild(TGUI_BOX *box);
  TGUI_BOX          *GetChild(int key);
  int               GetChildOrder(TGUI_BOX *child);

  TGUI_PANEL      *AddPanel(int pkey, GLfloat px, GLfloat py, GLfloat pwidth, GLfloat pheight);
  TGUI_PANEL      *AddPanel(int pkey, GLfloat px, GLfloat py, TGUI_TEXTURE *tex);
  TGUI_PANEL      *AddPanel(int pkey, GLfloat px, GLfloat py, GLfloat pwidth, GLfloat pheight, TGUI_TEXTURE *tex);

  TGUI_BUTTON     *AddButton(int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight, const char *bcaption);
  TGUI_BUTTON     *AddButton(int bkey, GLfloat bx, GLfloat by, TGUI_TEXTURE *tex);

  TGUI_BUTTON     *AddCheckButton(int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight, const char *bcaption);

  TGUI_BUTTON     *AddGroupButton(int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight, const char *bcaption, int bgroup);
  TGUI_BUTTON     *AddGroupButton(int bkey, GLfloat bx, GLfloat by, TGUI_TEXTURE *tex, int bgroup);

  TGUI_CHECK_BOX  *AddCheckBox(int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight, const char *bcaption);
  TGUI_CHECK_BOX  *AddGroupBox(int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight, const char *bcaption, int bgroup);

  TGUI_LABEL      *AddLabel(int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight);
  TGUI_LABEL      *AddLabel(int bkey, GLfloat bx, GLfloat by, const char *bcaption);
  TGUI_LABEL      *AddLabel(int bkey, GLfloat bx, GLfloat by, TGUI_TEXTURE *tex);

  TGUI_SLIDER     *AddSlider   (int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight, TGUI_SLIDER_TYPE btype);

  TGUI_SCROLL_BOX *AddScrollBox(int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight);
  TGUI_LIST_BOX   *AddListBox  (int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight);
  TGUI_EDIT_BOX   *AddEditBox  (int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight, unsigned max_lenght);
  TGUI_COMBO_BOX  *AddComboBox (int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight);

  void SetOnShow(void (*func)(TGUI_BOX *))
  { on_show = func; };

  virtual void Clear();
};


//=========================================================================
// TGUI_SCROLL_BOX
//=========================================================================

class TGUI_SCROLL_BOX: public TGUI_PANEL {
protected:
  TGUI_SLIDER *sliders[GUI_ST_COUNT];

  TGUI_BOX_ENVELOPE ch_envelope;
  GLfloat ch_dx;
  GLfloat ch_dy;

  // events
  virtual bool MouseMove(GLfloat mouse_x, GLfloat mouse_y);
  virtual bool MouseDown(GLfloat mouse_x, GLfloat mouse_y, int button);
  virtual bool MouseUp  (GLfloat mouse_x, GLfloat mouse_y, int button);

  virtual void RecalculateChildren(TGUI_BOX *sender);

  // drawing
  virtual void Update(double time_shift);
  virtual void Draw(GLfloat dx, GLfloat dy);

public:
  TGUI_SCROLL_BOX(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight)
    :TGUI_PANEL(bowner, bkey, bx, by, bwidth, bheight)
  { 
    sliders[GUI_ST_HORIZONTAL] = GUI_NEW TGUI_SLIDER(this, GUI_ST_HORIZONTAL, 0, 0, bwidth - GUI_SLIDER_SIZE, GUI_SLIDER_SIZE, GUI_ST_HORIZONTAL);
    sliders[GUI_ST_VERTICAL] = GUI_NEW TGUI_SLIDER(this, GUI_ST_VERTICAL, bwidth - GUI_SLIDER_SIZE, GUI_SLIDER_SIZE, GUI_SLIDER_SIZE, bheight - GUI_SLIDER_SIZE, GUI_ST_VERTICAL);
    sliders[GUI_ST_VERTICAL]->SetPosition(1);
    ch_dx = ch_dy = 0.0f;
  }

  virtual ~TGUI_SCROLL_BOX() {
    delete sliders[GUI_ST_HORIZONTAL];
    delete sliders[GUI_ST_VERTICAL];
  }

  // sliders
  void ResetSliders();
  void ShowSlider(TGUI_SLIDER_TYPE type);
  void HideSlider(TGUI_SLIDER_TYPE type);
  TGUI_SLIDER *GetSlider(TGUI_SLIDER_TYPE type) { return sliders[type]; };
  TGUI_SLIDER *GetHSlider() { return sliders[GUI_ST_HORIZONTAL]; };
  TGUI_SLIDER *GetVSlider() { return sliders[GUI_ST_VERTICAL]; };

  virtual void SetWidth(GLfloat bwidth);
  virtual void SetHeight(GLfloat bheight);
  virtual void SetSize(GLfloat bwidth, GLfloat bheight);

  virtual GLfloat GetClientWidth();
  virtual GLfloat GetClientHeight();

  // children
  virtual TGUI_BOX *AddChild(TGUI_BOX *box);

  virtual void Clear();
};


//=========================================================================
// TGUI_LIST_BOX
//=========================================================================


class TGUI_LIST_BOX: public TGUI_BOX {
protected:
  TGUI_SLIDER *sliders[GUI_ST_COUNT];
  TGUI_LIST *items;
  TGUI_BOX_ENVELOPE ch_envelope;

  GLfloat ch_dx;
  GLfloat ch_dy;

  void (*on_change) (TGUI_BOX *sender, int i_index);

  // drawing
  virtual void Update(double time_shift);
  virtual void Draw(GLfloat dx, GLfloat dy);

public:
  TGUI_LIST_BOX(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight)
    :TGUI_BOX(bowner, bkey, bx, by, bwidth, bheight)
  { 
    sliders[GUI_ST_HORIZONTAL] = sliders[GUI_ST_VERTICAL] = NULL;
    ch_dx = ch_dy = 0.0f;
    padding = 1;
    on_change = NULL;
    can_focus = true;

    items = NULL;
    items = GUI_NEW TGUI_LIST(this, 0, 0, 0, const_cast<char*>(""));
    ShowSlider(GUI_ST_VERTICAL);
  }

  virtual ~TGUI_LIST_BOX() {
    if (sliders[GUI_ST_HORIZONTAL]) delete sliders[GUI_ST_HORIZONTAL];
    if (sliders[GUI_ST_VERTICAL]) delete sliders[GUI_ST_VERTICAL];
    if (items) delete items;
  }

  // sliders
  void ResetSliders();
  void ShowSlider(TGUI_SLIDER_TYPE type);
  void HideSlider(TGUI_SLIDER_TYPE type);

  // properties
  int GetItemIndex() { return items->GetItemIndex(); }
  char *GetItem(int id) { return items->GetLine(id); }
  int GetItemsCount() { return items->GetLinesCount(); }
  char *GetSelected() { return items->GetSelected(); }

  void SetItems(const char *itm);
  bool SetItem (const char *line) { return items->SetItem (line); }

  void SetFontColor(GLfloat r, GLfloat g, GLfloat b) { items->SetFontColor(r, g, b); };
  void SetSelFontColor(GLfloat r, GLfloat g, GLfloat b) { items->SetSelFontColor(r, g, b); };
  void SetSelColor(GLfloat r, GLfloat g, GLfloat b) { items->SetSelColor(r, g, b); };

  virtual GLfloat GetClientWidth();
  virtual GLfloat GetClientHeight();

  // events
  virtual bool MouseMove(GLfloat mouse_x, GLfloat mouse_y);
  virtual bool MouseDown(GLfloat mouse_x, GLfloat mouse_y, int button);
  virtual bool MouseUp  (GLfloat mouse_x, GLfloat mouse_y, int button);

  void SetOnChange(void (*func)(TGUI_BOX *, int))
  { on_change = func; };

  virtual void RecalculateChildren(TGUI_BOX *sender);
};


//=========================================================================
// TGUI_COMBO_BOX
//=========================================================================

class TGUI_COMBO_BOX: public TGUI_BOX {
protected:
  TGUI_LIST_BOX *list;
  GLFfont *font;
  TGUI_COLOR font_color;

  virtual bool MouseDown(GLfloat mouse_x, GLfloat mouse_y, int button);

  // drawing
  virtual void Draw(GLfloat dx, GLfloat dy);

public:
  TGUI_COMBO_BOX(TGUI_BOX *bowner, int bkey, GLfloat bx, GLfloat by, GLfloat bwidth, GLfloat bheight);

  virtual ~TGUI_COMBO_BOX() { if (font_color) delete[] font_color; };

  // properties
  void SetFont(GLFfont *gui_font) { font = gui_font; };
  void SetFontColor(GLfloat r, GLfloat g, GLfloat b);

  void SetItems(const char *itm) { list->SetItems(itm); };
  bool SetItem (const char *line) { return list->SetItem (line); }

  char *GetItem(int id) { return list->GetItem(id); };
  int GetItemsCount() { return list->GetItemsCount(); }
  char *GetText() { return list->GetSelected(); };
  int GetItemIndex() { return list->GetItemIndex(); };

  TGUI_LIST_BOX *GetList () { return list; }

  // events
  void SetOnChange(void (*func)(TGUI_BOX *, int))
  { list->SetOnChange (func); }
};


//========================================================================
// TGUI_MESSAGE_BOX
//========================================================================

#define GUI_MB_BUTTONS_COUNT  4
#define GUI_MB_OK             0x1001
#define GUI_MB_YES            0x1002
#define GUI_MB_NO             0x1004
#define GUI_MB_CANCEL         0x1008


class TGUI_MESSAGE_BOX: public TGUI_PANEL {
private:
  TGUI_LABEL *text;
  TGUI_BUTTON *buttons[GUI_MB_BUTTONS_COUNT];

  GLfloat text_spacing;

  virtual bool KeyDown  (int key);

public:
  TGUI_MESSAGE_BOX(TGUI_BOX *bowner);

  void Show(const char *txt, int butt);

  TGUI_BUTTON *GetButton(int butt) {
    switch (butt) {
    case GUI_MB_OK:     return buttons[0];
    case GUI_MB_YES:    return buttons[1];
    case GUI_MB_NO:     return buttons[2];
    case GUI_MB_CANCEL: return buttons[3];
    default:            return NULL;
    }
  }
};


//=========================================================================
// TGUI
//=========================================================================

class TGUI: public TGUI_PANEL {
protected:
  GLFfont *font;
  TGUI_COLOR font_color;
  TGUI_LABEL *def_tooltip;
  TGUI_MESSAGE_BOX *message_box;
  TRECURSIVE_LOCK *rlock;

  GLfloat mice_x;
  GLfloat mice_y;
  int cursor_height;

  bool mouse_over_box;      //!< If mouse cursor is over some visible box.

public:
  static TGUI *self;

  static TGUI_BOX *hover_box;
  static TGUI_BOX *down_box;
  static TGUI_BOX *focus_box;
  static TGUI_BOX *modal_box;
  static TGUI_BOX *float_box;
  static TGUI_BOX *tooltip_box;
  
  TSAVE_LIST<TCALLBACK> callback_list;

  // constructing
  TGUI();
  ~TGUI();

  void Reset();
  bool MouseOverBox(void) { return mouse_over_box; }

  void SetFont(GLFfont *gui_font) { font = gui_font; }
  GLFfont *GetFont(void) { return font; };

  void SetFontColor(GLfloat r, GLfloat g, GLfloat b);
  TGUI_COLOR GetFontColor(void) { return font_color; }

  void SetCursorHeight(int cur_height) { cursor_height = cur_height; }

  // tooltip
  TGUI_LABEL *GetDefTooltip() { return def_tooltip; }

  // message box
  TGUI_MESSAGE_BOX *GetMessageBox() { return message_box; }
  void ShowMessageBox(const char *txt, int butt) { message_box->Show(txt, butt); }
  void HideMessageBox() { message_box->Hide(); }

  // events
  virtual bool MouseMove(GLfloat mouse_x, GLfloat mouse_y);
  virtual bool MouseDown(GLfloat mouse_x, GLfloat mouse_y, int button);
  virtual bool MouseUp  (GLfloat mouse_x, GLfloat mouse_y, int button);
  virtual bool KeyDown  (int key);

  bool RescanMouse(void);

  // mutex
  void Lock(void) { if (rlock) rlock->Lock(); }
  void Unlock(void) { if (rlock) rlock->Unlock(); }

  // drawing
  virtual void Update(double time_shift);
  void Draw() { Draw(0, 0); };
  virtual void Draw(GLfloat dx, GLfloat dy);
  void PollEvents(void);

  bool SetClipRectangle(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLfloat padding);
  void UnsetClipRectangle();
};


#endif    //__glgui_h__


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

