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
 *  @file domouse.cpp
 *
 *  Mouse.
 *
 *  @author Peter Knut
 *
 *  @date 2003, 2004
 */

#include "domouse.h"
#include "doselection.h"


//=========================================================================
// Variables
//=========================================================================

/**
 *  This variable represents the mouse.
 */
TMOUSE mouse;

GLfloat circle_color[MC_COUNT][3] = {
  {1.0f, 1.0f, 1.0f},   // MC_ARROW
  {1.0f, 1.0f, 0.0f},   // MC_SELECT
  {1.0f, 1.0f, 0.0f},   // MC_SELECT_PLUS
  {0.0f, 1.0f, 0.0f},   // MC_CAN_MOVE
  {1.0f, 0.1f, 0.0f},   // MC_CANT_MOVE
  {1.0f, 0.1f, 0.0f},   // MC_CAN_ATTACK
  {1.0f, 0.1f, 0.0f},   // MC_CANT_ATTACK
  {1.0f, 1.0f, 1.0f},   // MC_CAN_MINE
  {1.0f, 0.1f, 0.0f},   // MC_CANT_MINE
  {1.0f, 1.0f, 1.0f},   // MC_CAN_REPAIR
  {1.0f, 0.1f, 0.0f},   // MC_CANT_REPAIR
  {1.0f, 1.0f, 1.0f},   // MC_CAN_BUILD
  {1.0f, 0.1f, 0.0f},   // MC_CANT_BUILD
  {1.0f, 1.0f, 1.0f},   // MC_CAN_HIDE
  {1.0f, 0.1f, 0.0f},   // MC_CANT_HIDE
  {1.0f, 1.0f, 1.0f},   // MC_CANT_EJECT
};


//=========================================================================
// Functions
//=========================================================================

/**
 *  Converts real mouse position to real map position.
 *
 *  @todo Comment.
 *  
 *  @todo Ak sa nieco vymysli s cyklickymi zavislostami, tak urobit tuto
 *        funkciu inline.
 */
void MouseToMap(GLfloat rpos_x, GLfloat rpos_y, double *map_x, double *map_y)
{
  double pomdx, pomdy;

  pomdx = ((rpos_x - config.scr_width / 2) * projection.game_h_coef - map.dx) / DAT_MAPEL_STRAIGHT_SIZE;
  pomdy = ((rpos_y - config.scr_height / 2) * projection.game_v_coef - (map.dy + 0.5f)) / DAT_MAPEL_DIAGONAL_SIZE;

  *map_x = pomdy + pomdx;
  *map_y = pomdy - pomdx;
}

//=========================================================================
// Bool field
//=========================================================================

/**
 *  Deletes 2D array of bool identifiers.
 *
 *  @param width  Width of field.
 */
static void DeleteBoolField(bool **bool_field, int width)
{
  if (!bool_field) return;

  int i;

  for (i = 0; i < width; i++)
    if (bool_field[i]) 
      delete[] bool_field[i];

  delete[] bool_field;
}


/**
 *  Creates 2D array of bool.
 *
 *  @param width  Width of field.
 *  @param height Height of field.
 */
static bool **CreateBoolField(int width, int height)
{
  bool **bool_field = NULL;
  int i, j;

  bool ok = true;

  if (!(bool_field = NEW bool *[width])) ok = false;

  if (ok) for (i = 0; i < width; i++) {
    bool_field[i] = NULL;
    if (!(bool_field[i] = NEW bool[height])) ok = false;

    if (ok) for (j = 0; j < height; j++) bool_field[i][j] = false;
  }

  if (ok) return bool_field;
  else {
    if (bool_field) DeleteBoolField(bool_field, width);
    return NULL;
  }
}


//=========================================================================
// Class TMOUSE
//=========================================================================

/**
 *  Constructor.
 */
TMOUSE::TMOUSE(void) {
  x = y = 0; sel_x = sel_y = 0; rx = ry = 0; rpos_x = rpos_y = 0.0f;
  down_x = down_y = 0;
  draw_selection = false;
  cursor_id = MC_ARROW;
  circle_id = MCC_NONE;
  action    = UA_NONE;
  over_unit = NULL;
  build_map = CreateBoolField(RAC_MAX_UNIT_SIZE, RAC_MAX_UNIT_SIZE);
}


/**
 *  Load textures from file.
 *
 *  @param path  Filename of the dat file.
 *
 *  @return @c true on success, otherwise @c false.
 */
bool TMOUSE::LoadData(const char *path)
{
  bool ok = tex_set.Load(path, GL_NEAREST, GL_NEAREST);

  if (ok && tex_set.groups[DAT_TGID_MOUSE_CURSORS].count < MC_COUNT + MCC_COUNT) {
    Critical("Not enough textures for mouse cursors.");
    DeleteData();

    ok = false;
  }

  if (ok) {
    int i;
    // cursor animations
    for (i = 0; i < MC_COUNT; i++)
      cursors_anim[i].SetTexItem(&tex_set.groups[DAT_TGID_MOUSE_CURSORS].textures[i]);

    // circles animations
    for (i = 0; i < MCC_COUNT; i++)
      circles_anim[i].SetTexItem(&tex_set.groups[DAT_TGID_MOUSE_CURSORS].textures[MC_COUNT + i]);
  }

  return ok;
}

/**
 *  Deletes textures set.
 */
void TMOUSE::DeleteData(void)
{
  mouse.tex_set.Clear();
}


/**
 *  Updates mouse.
 *
 *  @param time_shift  Time shift from the last update.
 *  @param in_game     Specifies, whether we are in game.
 */
void TMOUSE::Update(bool in_game, double time_shift)
{
  // real position
  rpos_x = (GLfloat)x;
  rpos_y = (GLfloat)y;

  // selection position
  if (!draw_selection) {
    sel_x = rpos_x;
    sel_y = rpos_y;
  }

  if (in_game) {
    T_BYTE last_cuid = cursor_id;
    T_BYTE last_ccid = circle_id;

    // map moving
    if (!map.mouse_moving && !map.drag_moving) {
      if (mouse.x == 0) map.MouseMove(MAP_MOUSE_MOVE_RIGHT);
      if (mouse.x == config.scr_width - 1) map.MouseMove(MAP_MOUSE_MOVE_LEFT);
      if (mouse.y == 0) map.MouseMove(MAP_MOUSE_MOVE_UP);
      if (mouse.y == config.scr_height - 1) map.MouseMove(MAP_MOUSE_MOVE_DOWN);
    }

    // position in map
    MouseToMap(rpos_x, rpos_y, &rmap_x, &rmap_y);

    if (!map.IsInMap(rmap_x, rmap_y)) {
      map_pos.SetPosition(LAY_UNAVAILABLE_POSITION, LAY_UNAVAILABLE_POSITION);
    }
    else map_pos.SetPosition((T_SIMPLE)rmap_x, (T_SIMPLE)rmap_y);

    // update cursor type
    if (gui->MouseOverBox() && (!mouse.draw_selection || gui->modal_box))
      ResetCursor();
    else
      UpdateCursorID();

    // if cursor has been changed, reset animations
    if (cursor_id != last_cuid) {
      cursors_anim[last_cuid].Stop();
      cursors_anim[cursor_id].Play();

      if (last_ccid != MCC_NONE) circles_anim[last_ccid].Stop();
      if (circle_id != MCC_NONE) circles_anim[circle_id].Play();
    }

    // else update animations
    else {
      cursors_anim[cursor_id].Update(time_shift);
      if (circle_id != MCC_NONE) circles_anim[circle_id].Update(time_shift);
    }
  }

  // not in_game
  else {
    ResetCursor();

    // update animation
    cursors_anim[cursor_id].Update(time_shift);
  }
}


/**
 *  Finds and sets identifier of mouse cursor. It depends on what is under cursor.
 */
void TMOUSE::UpdateCursorID(void)
{
  // if we draw selection, only basic arrow cursor is drawn
  if (draw_selection) {
    if (abs(x - down_x) > MC_SELECT_RADIUS || abs(y - down_y) > MC_SELECT_RADIUS)
    {
      if (glfwGetKey(GLFW_KEY_LCTRL))
        cursor_id = MC_SELECT_PLUS;
      else
        cursor_id = MC_SELECT;

      circle_id = MCC_NONE;
    }
    else {
      if (down_cursor_id == MC_SELECT && glfwGetKey(GLFW_KEY_LCTRL))
        cursor_id = MC_SELECT_PLUS;
      else
        cursor_id = down_cursor_id;

      circle_id = down_circle_id;
    }

    return;
  }

  bool all_seg = (view_segment == DRW_ALL_SEGMENTS);

  T_BYTE act_seg;
  
  int i;

  // initializes segment variables
  if (all_seg) act_seg = DAT_SEGMENTS_COUNT - 1;
  else act_seg = view_segment;

  // if cursor is under map
  if (rmap_x < 0 || rmap_y < 0) {
    switch (action) {
    case UA_MOVE:
      cursor_id = MC_CANT_MOVE;
      break;
    case UA_ATTACK:
      cursor_id = MC_CANT_ATTACK;
      break;
    case UA_MINE:
      cursor_id = MC_CANT_MINE;
      break;
    case UA_REPAIR:
      cursor_id = MC_CANT_REPAIR;
      break;
    case UA_BUILD:
      cursor_id = MC_CANT_BUILD;
      break;
    default:
      if (selection->TestCanMove()) cursor_id = MC_CANT_MOVE;
      else cursor_id = MC_ARROW;
      break;
    }
  }

  // cursor is over or in map
  else {
  
    T_BYTE map_state [DAT_SEGMENTS_COUNT];
    bool   is_in_map = (rmap_x < map.width && rmap_y < map.height);

    // initialize map state fields
    if (is_in_map) {
      for (i = 0; i < DAT_SEGMENTS_COUNT; i++)
        map_state[i] = myself->GetLocalMap()->map[i][map_pos.x][map_pos.y].state;
    }

    // try to find some unit through all segments
    if (all_seg) {
      ReleaseCountedPointer(over_unit);
      for (i = act_seg; i >= 0; i--) {
        if (!is_in_map || map_state[i] != WLK_UNKNOWN_AREA) FindOverUnit(i);
        if (over_unit) break;
      }
    }
    else if (!is_in_map || map_state[act_seg] != WLK_UNKNOWN_AREA) FindOverUnit(act_seg);
    else ReleaseCountedPointer(over_unit);

    // test action and compute new cursor_id
    switch (action) {
    case UA_MOVE:
      if (over_unit) {
        if (selection->TestCanHide(over_unit)) cursor_id = MC_CAN_HIDE;
        else cursor_id = MC_CAN_MOVE;
      }

      else if (is_in_map) cursor_id = MC_CAN_MOVE;

      else  cursor_id = MC_CANT_MOVE;

      break;

    case UA_ATTACK:
      if (over_unit && selection->TestCanAttack(over_unit)) cursor_id = MC_CAN_ATTACK;

      else cursor_id = MC_CANT_ATTACK;

      break;

    case UA_MINE:
      if (over_unit && 
        over_unit->TestItemType(IT_SOURCE) && 
        selection->TestCanMine((TSOURCE_UNIT *)over_unit)
      )
        cursor_id = MC_CAN_MINE;

      else if (over_unit && 
        (over_unit->TestItemType(IT_BUILDING) || over_unit->TestItemType(IT_FACTORY)) &&
        selection->TestCanUnload((TBUILDING_UNIT *)over_unit)
      )
        cursor_id = MC_CAN_MINE;

      else cursor_id = MC_CANT_MINE;

      break;

    case UA_REPAIR:
      if (over_unit && 
        (over_unit->TestItemType(IT_WORKER) || over_unit->TestItemType(IT_FORCE) || over_unit->TestItemType(IT_BUILDING) || over_unit->TestItemType(IT_FACTORY)) &&
        selection->TestCanBuildOrRepair((TBASIC_UNIT *)over_unit)
      ) 
        cursor_id = MC_CAN_REPAIR;
      else cursor_id = MC_CANT_REPAIR;

      break;

    case UA_BUILD:
      if (selection->TestCanBuild(myself->build_item, map_pos, build_map)) cursor_id = MC_CAN_BUILD;
      else cursor_id = MC_CANT_BUILD;
      break;

    case UA_NONE:
      // cursor is over unit
      if (over_unit) {

        // CTRL key is pressed
        if (glfwGetKey(GLFW_KEY_LCTRL) == GLFW_PRESS && !over_unit->TestState(US_GHOST)) cursor_id = MC_SELECT_PLUS;

        // ejecting
        else if (over_unit->IsSelected() && over_unit->CanEject()) cursor_id = MC_EJECT;

        // if units in selection are not my (or selection is empty)
        else if (!selection->IsMy()) {
          if (over_unit->TestState(US_GHOST)) cursor_id = MC_ARROW;
          else cursor_id = MC_SELECT;
        }

        // my units are in the selection (and it is not empty)
        else {
          // mining
          if (over_unit->TestItemType(IT_SOURCE) && selection->TestCanMine((TSOURCE_UNIT *)over_unit))
            cursor_id = MC_CAN_MINE;

          // unloading
          else if ((over_unit->TestItemType(IT_BUILDING) || over_unit->TestItemType(IT_FACTORY)) && 
            selection->TestCanUnload((TBUILDING_UNIT *)over_unit)
          )
            cursor_id = MC_CAN_MINE;

          // repairing
          else if ((over_unit->TestItemType(IT_WORKER) || over_unit->TestItemType(IT_FORCE) || over_unit->TestItemType(IT_BUILDING) || over_unit->TestItemType(IT_FACTORY)) &&
            selection->TestCanBuildOrRepair((TBASIC_UNIT *)over_unit)
          )
            cursor_id = MC_CAN_REPAIR;

          // hiding
          else if (selection->TestCanHide(over_unit))
            cursor_id = MC_CAN_HIDE;         

          // attacking
          else if (over_unit->GetPlayer() != myself && over_unit->GetPlayer() != hyper_player &&
            selection->TestCanAttack(over_unit)
          )
            cursor_id = MC_CAN_ATTACK;

          // moving
          else if (over_unit->TestState(US_GHOST)) {
            if (selection->TestCanMove() && is_in_map) cursor_id = MC_CAN_MOVE;
            else cursor_id = MC_ARROW;
          }

          // none special action could by done
          else cursor_id = MC_SELECT;
        }
      }

      // cursor is not over unit and selected units can move
      else if (selection->TestCanMove()) {
        if (is_in_map) cursor_id = MC_CAN_MOVE;
        else cursor_id = MC_CANT_MOVE;
      }

      // else
      else cursor_id = MC_ARROW;
      break;

    default:
      cursor_id = MC_ARROW;
      break;
    } // switch action

  } // cusor over or in map

  // update cursor circle type
  switch (cursor_id) {
  case MC_SELECT:
  case MC_SELECT_PLUS:
  case MC_CAN_MOVE:
  case MC_CAN_ATTACK:
  case MC_CAN_MINE:
  case MC_CAN_REPAIR:
  case MC_CAN_BUILD:
  case MC_CAN_HIDE:
    circle_id = MCC_ARROWS_IN;
    break;

  case MC_EJECT:
    circle_id = MCC_ARROWS_OUT;
    break;

  case MC_CANT_MOVE:
  case MC_CANT_ATTACK:
  case MC_CANT_MINE:
  case MC_CANT_REPAIR:
  case MC_CANT_BUILD:
  case MC_CANT_HIDE:
    circle_id = MCC_CROSS;
    break;

  default:
    circle_id = MCC_NONE;
    break;
  }
}


/**
 *  Finds and returns unit under mouse cursor in segment seg.
 */
void TMOUSE::FindOverUnit(int seg)
{
  TMAP_UNIT *units[100];
  TMAP_UNIT *last_unit = NULL;
  int count = 0;

  bool left;
  int act_x, act_y;
  int steps;

  int i;

  bool go = true;
  bool found = false;

  // sets start position and direction
  act_x = (int)mouse.rmap_x;
  act_y = (int)mouse.rmap_y;
  left = ((mouse.rmap_y - (int)mouse.rmap_y) - (mouse.rmap_x - (int)mouse.rmap_x) > 0);

  // finds lowest position in the map from mouse position restrict to max. height of texture
  steps = act_x;
  if (act_y < steps) steps = act_y;
  if (DAT_MAX_TEX_MAPELS < steps) steps = DAT_MAX_TEX_MAPELS;

  act_x -= steps;
  act_y -= steps;

  // goes up from lowest position and finds all units
  glfwLockMutex(delete_mutex);
  while (go) {
    // unit is found
    if (map.IsInMap(act_x, act_y)
        && (
          (((units[count] = map.segments[seg].surface[act_x][act_y].unit)) && (units[count]->IsVisible()))
          || ((units[count] = map.segments[seg].surface[act_x][act_y].ghost))
        )
        && (units[count] != last_unit)
       )
    {
      units[count] = units[count]->AcquirePointer();
      if (units[count])
      {
        last_unit = units[count];
        count++;
      }
    }

    // goes to next mapel
    if (act_x != (int)mouse.rmap_x || act_y != (int)mouse.rmap_y) {
      if (left) act_y++;
      else act_x++;
      left = !left;
    }
    else go = false;
  }
  glfwUnlockMutex(delete_mutex);


  // if we have some units, tests selection
  if (count) {
    GLfloat pixel[3] = { -1.0f, -1.0f, -1.0f };
    GLfloat fwidth, fheight;

    // white quad under mouse cursor
    glLoadIdentity();
    glColor3f(1.0f, 1.0f, 1.0f);
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
      glVertex2d(mouse.x - 5, mouse.y - 5);
      glVertex2d(mouse.x + 5, mouse.y - 5);
      glVertex2d(mouse.x + 5, mouse.y + 5);
      glVertex2d(mouse.x - 5, mouse.y + 5);
    glEnd();
    glEnable(GL_TEXTURE_2D);

    projection.SetProjection(PRO_GAME);
    
    // draw units
    for (i = 0; i < count; i++) {
      //!!! tuto nastava chyba ze units[i] == NULL napriek tomu, ze je to pocitany 
      // pointer. Ak sa to nevyriesi inak, moze sa tu dat test a aspon to nespadne.
      SetMapPosition(units[i]->GetRealPositionX(), units[i]->GetRealPositionY());

      fwidth = (GLfloat)units[i]->GetAnimation()->GetFrameWidth();
      fheight = (GLfloat)units[i]->GetAnimation()->GetFrameHeight();

      // draws black silhoulette of unit
      glColor3f(0.0f, 0.0f, 0.0f);
      units[i]->GetAnimation()->Draw();

      // read color from back buffer
      glReadPixels(mouse.x, mouse.y, 1, 1, GL_RGB, GL_FLOAT, &pixel);

      // color is not white -> we have a unit
      if (pixel[0] < DAT_MAX_COLOR_TESTED || pixel[1] < DAT_MAX_COLOR_TESTED || pixel[2] < DAT_MAX_COLOR_TESTED) {
        if (over_unit != units[i]) {
          ReleaseCountedPointer(over_unit);
          over_unit = units[i]->AcquirePointer();
        }

        found = true;
        break;
      }
    }

    // release couted pointers
    for (i = 0; i < count; i++) ReleaseCountedPointer(units[i]);
  }

  if (!found) ReleaseCountedPointer(over_unit);
}


/**
 *  Select all moveable units from mouse selection.
 *  @return @c True if at least one unit is selected.
 */
bool TMOUSE::RectSelect()
{
  GLfloat mx1, mx2, my1, my2;         // mouse selection
  double rect[4][2];                  // selection rectangle in mapels
  double left, right, top, bottom;    // envelope
  T_SIMPLE l, r, t, b;                // envelope

  float ux, uy;
  TMAP_UNIT *unit;
  TMAP_UNIT *first_found = NULL;
  TMAP_ITEM *item;

  int i, j, seg;

  // converts mouse selection to clock-wise direction
  if (((rpos_x > sel_x && rpos_y < sel_y)) || 
      ((rpos_x < sel_x && rpos_y > sel_y))) {
    mx1 = sel_x;
    my1 = rpos_y;
    mx2 = rpos_x;
    my2 = sel_y;
  }
  else {
    mx1 = rpos_x;
    my1 = rpos_y;
    mx2 = sel_x;
    my2 = sel_y;
  }

  // converts mouse selection into map axis rectangle
  MouseToMap(mx1, my1, &rect[0][0], &rect[0][1]);
  MouseToMap(mx1, my2, &rect[1][0], &rect[1][1]);
  MouseToMap(mx2, my2, &rect[2][0], &rect[2][1]);
  MouseToMap(mx2, my1, &rect[3][0], &rect[3][1]);

  // finds envelope
  left = right = rect[0][0];
  top = bottom = rect[0][1];

  for (i = 1; i < 4; i++) {
    if (rect[i][0] < left) left = rect[i][0];
    if (rect[i][0] > right) right = rect[i][0];
    if (rect[i][1] < bottom) bottom = rect[i][1];
    if (rect[i][1] > top) top = rect[i][1];
  }

  // selection doesnt interfere with map
  if ((right < 1) || (left > (map.width - 1)) || (top < 1) || (bottom > (map.height - 1)))
    return false;

  // restrits envelope to map
  if (left < 0) left = 0;
  if (bottom < 0) bottom = 0;

  if (right >= map.width) right = map.width;
  if (top >= map.height) top = map.height;

  l = (T_SIMPLE)left;
  b = (T_SIMPLE)bottom;
  r = (T_SIMPLE)right;
  t = (T_SIMPLE)top;

  glfwLockMutex(delete_mutex);

  // goes through whole envelope and tests units
  for (seg = DAT_SEGMENTS_COUNT - 1; seg >=0 ; seg--)
    if (view_segment == DRW_ALL_SEGMENTS || seg == view_segment) {
      for (j = b; j < t; j++)
        for (i = l; i < r; i++) {
      
          unit = map.segments[seg].surface[i][j].unit;

          // unit must by my, not selected and moveable
          if (unit && (unit->GetPlayer() == myself) && !unit->GetSelected() 
              && (item = static_cast<TMAP_ITEM*>(unit->GetPointerToItem()))->CanMove()) 
          {
            ux = (float)unit->GetRealPositionX() + float(item->GetWidth()) / 2;
            uy = (float)unit->GetRealPositionY() + float(item->GetHeight()) / 2;

            // if unit is in selection rectangle
            if (((rect[1][0] - rect[0][0]) * (uy - rect[0][1]) <= (rect[1][1] - rect[0][1]) * (ux - rect[0][0])) &&
                ((rect[2][0] - rect[1][0]) * (uy - rect[1][1]) <= (rect[2][1] - rect[1][1]) * (ux - rect[1][0])) &&
                ((rect[3][0] - rect[2][0]) * (uy - rect[2][1]) <= (rect[3][1] - rect[2][1]) * (ux - rect[2][0])) &&
                ((rect[0][0] - rect[3][0]) * (uy - rect[3][1]) <= (rect[0][1] - rect[3][1]) * (ux - rect[3][0])))
            {
              if (first_found) {
                if (unit != first_found)
                  selection->AddUnit(unit, false);
              }
              else first_found = unit;
            }
          }
        }
    } // if

  // first found unit is added to selection as last with selection sound
  if (first_found)
    selection->AddUnit(first_found, true);

  glfwUnlockMutex(delete_mutex);

  return first_found != NULL;
}


/**
 *  Draws mouse cursor and seletion rectangle.
 */
void TMOUSE::Draw(void)
{
  glLoadIdentity();
  glTranslated(rpos_x, rpos_y, 0);

  // draws circle animation
  if (circle_id != MCC_NONE) {
    glColor3fv(circle_color[cursor_id]);
    circles_anim[circle_id].Draw();
  }

  // draws cursor  
  glColor3f(1.0, 1.0, 1.0);
  cursors_anim[cursor_id].Draw();
}


/**
 * Draws selection recangle.
 */
void TMOUSE::DrawSelection(void)
{
  // draws selection rectangle
  if (!draw_selection) return;
  if (abs(x - down_x) <= MC_SELECT_RADIUS && abs(y - down_y) <= MC_SELECT_RADIUS) return;

  glLoadIdentity();

  glDisable(GL_TEXTURE_2D);  
  glColor3f(0.0f, 1.0f, 0.0f);

  glBegin(GL_LINE_LOOP);
    glVertex2d(rpos_x, rpos_y);
    glVertex2d(sel_x, rpos_y);
    glVertex2d(sel_x, sel_y);
    glVertex2d(rpos_x, sel_y);
  glEnd();

  glEnable(GL_TEXTURE_2D);
}


/**
 *  Draws build map - where I can build.
 */
void TMOUSE::DrawBuildMap(void)
{
  if (!myself->build_item || (mouse.cursor_id != MC_CAN_BUILD && mouse.cursor_id != MC_CANT_BUILD)
    || map_pos.x == LAY_UNAVAILABLE_POSITION || map_pos.y == LAY_UNAVAILABLE_POSITION
    ) return;

  T_SIMPLE w = myself->build_item->GetWidth();
  T_SIMPLE h = myself->build_item->GetHeight();
  int i, j;

  glDisable(GL_TEXTURE_2D);
  SetMapPosition(map_pos.x - w / 2 - 1, map_pos.y - h / 2 - 1);

  // quads inside build map
  for (i = map_pos.x; i < map_pos.x + w; i++) {
    glTranslated(DAT_MAPEL_STRAIGHT_SIZE_2, DAT_MAPEL_DIAGONAL_SIZE_2, 0);
    glPushMatrix();

    for (j = map_pos.y; j < map_pos.y + h; j++) {
      glTranslated(-DAT_MAPEL_STRAIGHT_SIZE_2, DAT_MAPEL_DIAGONAL_SIZE_2, 0);

      if (build_map[i - map_pos.x][j - map_pos.y])
        glColor4f(0, 1, 0, DRW_BUILD_MAP_ALPHA);
      else
        glColor4f(1, 0.1f, 0, DRW_BUILD_MAP_ALPHA);

      glBegin(GL_QUADS);
        glVertex2d(0.0, 0.0);
        glVertex2d(DAT_MAPEL_STRAIGHT_SIZE_2, DAT_MAPEL_DIAGONAL_SIZE_2);
        glVertex2d(0, DAT_MAPEL_DIAGONAL_SIZE);
        glVertex2d(-DAT_MAPEL_STRAIGHT_SIZE_2, DAT_MAPEL_DIAGONAL_SIZE_2);
      glEnd();
    }

    glPopMatrix();
  }

  // line around build map
  glColor3f(0.9f, 0.9f, 0.9f);
  SetMapPosition(map_pos.x - w / 2, map_pos.y - h / 2);

  glBegin(GL_LINE_LOOP);
    glVertex2d(0.0, 0.0);
    glVertex2d(w * DAT_MAPEL_STRAIGHT_SIZE_2, w * DAT_MAPEL_DIAGONAL_SIZE_2);
    glVertex2d((w - h) * DAT_MAPEL_STRAIGHT_SIZE_2, (w + h) * DAT_MAPEL_DIAGONAL_SIZE_2);
    glVertex2d(-h * DAT_MAPEL_STRAIGHT_SIZE_2, h * DAT_MAPEL_DIAGONAL_SIZE_2);
  glEnd();

  glEnable(GL_TEXTURE_2D);

  // draw building texture
  glColor4f(1, 1, 1, DRW_BUILD_MAP_ALPHA);
  myself->race->tex_table.groups[myself->build_item->tg_stay_id].textures[0].DrawFrame(0);
}


/**
 *  Centers mouse cursor.
 */
void TMOUSE::Center(void)
{
  x = rx = config.scr_width / 2;
  y = ry = config.scr_height / 2;
  glfwSetMousePos(x, y);
}


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

