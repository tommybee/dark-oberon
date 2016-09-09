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
 *  @file dounits.cpp
 *
 *  Working with units.
 *
 *  @author Peter Knut
 *  @author Valeria Sventova
 *  @author Jiri Krejsa
 *
 *  @date 2002, 2003, 2004
 */



//=========================================================================
// Included files
//=========================================================================

#include <cstdlib>
#include <math.h>
#include <stdarg.h>

#include "dodata.h"
#include "dodraw.h"
#include "dologs.h"
#include "dounits.h"
#include "doselection.h"
#include "dosimpletypes.h"
#include "domouse.h"
#include "dopool.h"
#include "donet.h"
#include "dohost.h"
#include "doengine.h"


//=========================================================================
// Definitions
//=========================================================================

#define SET_FRAME_COLOR \
  do { \
    if (player == myself) glColor3f(0.5f, 0.8f, 0.5f); \
    else if (player == hyper_player) glColor3f(0.8f, 0.8f, 0.5f); \
    else glColor3f(1.0f, 0.5f, 0.5f); \
  } while(0)


#define SET_RADAR_COLOR \
  do { \
    if (player == myself) glColor3f(0, 0.8f, 0); \
    else if (player == hyper_player) glColor3f(0.8f, 0.8f, 0); \
    else glColor3f(1, 0.2f, 0.2f); \
  } while(0)


//=========================================================================
// Usefull methods.
//=========================================================================

inline void DrawStatusCube(GLfloat x, GLfloat y, GLfloat l, GLfloat p, GLfloat r, GLfloat g, GLfloat b)
{
  GLfloat lp = l * p;

  glBegin(GL_QUADS);
  if (p > 0) {
    // life quad
    glColor3f(r, g, b);
    glVertex2d(x, y + lp);
    glVertex2d(x - UNI_LIFE_BAR_SIZE, y + UNI_LIFE_BAR_SIZE_2 + lp);
    glVertex2d(x, y + UNI_LIFE_BAR_SIZE + lp);
    glVertex2d(x + UNI_LIFE_BAR_SIZE, y + UNI_LIFE_BAR_SIZE_2 + lp);

    glColor3f(r * 0.6f, g * 0.6f, b * 0.6f);
    glVertex2d(x, y);
    glVertex2d(x, y + lp);
    glVertex2d(x - UNI_LIFE_BAR_SIZE, y + UNI_LIFE_BAR_SIZE_2 + lp);
    glVertex2d(x - UNI_LIFE_BAR_SIZE, y + UNI_LIFE_BAR_SIZE_2);

    glColor3f(r * 0.8f, g * 0.8f, b * 0.8f);
    glVertex2d(x, y);
    glVertex2d(x, y + lp);
    glVertex2d(x + UNI_LIFE_BAR_SIZE, y + UNI_LIFE_BAR_SIZE_2 + lp);
    glVertex2d(x + UNI_LIFE_BAR_SIZE, y + UNI_LIFE_BAR_SIZE_2);
  }
  else {
    glColor4f(r, g, b, 0.4f);
    glVertex2d(x, y + lp);
    glVertex2d(x - UNI_LIFE_BAR_SIZE, y + UNI_LIFE_BAR_SIZE_2 + lp);
    glVertex2d(x, y + UNI_LIFE_BAR_SIZE + lp);
    glVertex2d(x + UNI_LIFE_BAR_SIZE, y + UNI_LIFE_BAR_SIZE_2 + lp);
  }

  if (p < 1) {
    // upper life quad
    glColor4f(r, g, b, 0.4f);
    glVertex2d(x, y + l);
    glVertex2d(x - UNI_LIFE_BAR_SIZE, y + UNI_LIFE_BAR_SIZE_2 + l);
    glVertex2d(x, y + UNI_LIFE_BAR_SIZE + l);
    glVertex2d(x + UNI_LIFE_BAR_SIZE, y + UNI_LIFE_BAR_SIZE_2 + l);

    glColor4f(r * 0.6f, g * 0.6f, b * 0.6f, 0.4f);
    glVertex2d(x, y + lp);
    glVertex2d(x, y + l);
    glVertex2d(x - UNI_LIFE_BAR_SIZE, y + UNI_LIFE_BAR_SIZE_2 + l);
    glVertex2d(x - UNI_LIFE_BAR_SIZE, y + UNI_LIFE_BAR_SIZE_2 + lp);

    glColor4f(r * 0.8f, g * 0.8f, b * 0.8f, 0.4f);
    glVertex2d(x, y + lp);
    glVertex2d(x, y + l);
    glVertex2d(x + UNI_LIFE_BAR_SIZE, y + UNI_LIFE_BAR_SIZE_2 + l);
    glVertex2d(x + UNI_LIFE_BAR_SIZE, y + UNI_LIFE_BAR_SIZE_2 + lp);
  }
  glEnd();
}


inline void DrawStatusQuad(GLfloat x, GLfloat y, GLfloat w, GLfloat p, GLfloat r, GLfloat g, GLfloat b)
{
  GLfloat wp = w * p;

  glBegin(GL_QUADS);

  if (p > 0) {
    glColor3f(r, g, b);
    glVertex2f(x, y);
    glVertex2f(x, y + 3);
    glVertex2f(x + wp, y + 3);
    glVertex2f(x + wp, y);
  }

  if (p < 1) {
    glColor4f(r, g, b, 0.4f);
    glVertex2f(x + wp, y);
    glVertex2f(x + wp, y + 3);
    glVertex2f(x + w, y + 3);
    glVertex2f(x + w, y);
  }

  glEnd();
}


//=========================================================================
// Methods definitions of the class TMAP_UNIT.
//=========================================================================

/**
 *  Constructor. Only zeroize the values.
 */
TMAP_UNIT::TMAP_UNIT()
{
  life = 0;
  is_in_map = selected = false;
  group_id = -1;
  hided_count = 0;
  myself_units = 0;

  shot = NULL;
  target = last_target = NULL;
  pointer_counter = 0;
  burn_animation = sign_animation = NULL;
  will_be_deleted = false;
  ghost_owner = NULL;
  aggressivity = AM_IGNORE;
  auto_attack = false;
}


/**
 *  Constructor. Set position and pointer to unit kind.
 *
 *  @param ux The x coordinate.
 *  @param uy The y coordinate.
 *  @param uz The z coordinate.
 *  @param mi Pointer to the unit kind.
 *  @param new_unit_id If new_unit_id != 0, unit_id = new_unit_id, else new unique unit_id is generated according to global_unit.
 */
TMAP_UNIT::TMAP_UNIT(int uplayer, int ux, int uy, int uz, TMAP_ITEM *mi, int new_unit_id, bool global_unit)
:TPLAYER_UNIT(uplayer, ux, uy, uz, mi, new_unit_id, global_unit)
{
  life = 0;
  is_in_map = selected = false;
  group_id = -1;
  hided_count = 0;
  myself_units = 0;

  shot = NULL;
  target = last_target = NULL;

  pointer_counter = 0;

  sign_animation = NULL;
  will_be_deleted = false;
  ghost_owner = NULL;
  aggressivity = mi->GetAggressivity();
  auto_attack = false;

  ChangeAnimation();

  // burning animation
  int burning_id = ((TMAP_ITEM *)pitem)->tg_burning_id;

  if (burning_id > -1) {
    burn_animation = NEW TGUI_ANIMATION(player->race->tex_table.GetTexture(burning_id, 0));
    burn_animation->SetRandomFrame();
    burn_animation->Hide();
  }
  else burn_animation = NULL;
}


/**
 * Destructor.
 */
TMAP_UNIT::~TMAP_UNIT()
{
  hided_units.DestroyList();
  working_units.DestroyList();

  if (burn_animation) delete burn_animation;

  ReleaseCountedPointer(ghost_owner);
}


void TMAP_UNIT::SetLife(const float value)
{
  TEVENT * hlp;
  TMAP_ITEM *pit = (TMAP_ITEM *)pitem;
  int ml = pit->GetMaxLife();
  int old_life = (int)life;

  if (value < 0) life = 0;
  if (value > ml) life = (float)ml;
  else life = value;

  
  if (((int)life) != old_life) {
    // only not remote (local) units send message about change of life
    if (!player_array.IsRemote(this->GetPlayerID())) {
      // send info about unit's actual life to all not local players
      hlp = pool_events->GetFromPool();
      hlp->SetEventProps(GetPlayerID(), GetUnitID(), false, glfwGetTime(), RQ_SYNC_LIFE, US_NONE, -1, 0, 0, 0, 0, 0, 0, (int)life);
      SendNetEvent(hlp, all_players);
      pool_events->PutToPool(hlp);
    }
  }

  ChangeAnimation();

  if (burn_animation) {
    if (life == 0) burn_animation->Hide();
    else if (!TestState(US_IS_BEING_BUILT)) {
      if (life < ml * UNI_BURNING_COEF2) {
        if (burn_animation->SetTexItem(player->race->tex_table.GetTexture(pit->tg_burning_id, 1))) {
          burn_animation->SetRandomFrame();
        }

        burn_animation->Show();
      }
      else if (life < ml * UNI_BURNING_COEF1) {
        if (burn_animation->SetTexItem(player->race->tex_table.GetTexture(pit->tg_burning_id, 0))) {
          burn_animation->SetRandomFrame();
        }

        burn_animation->Show();
      }
      else burn_animation->Hide();
    }
  }
}

/** 
 * Injure unit for the value of the parameter.
 *
 * @param injury Value of injury. If the param is lesser than zero function doesn't take any efect and return false. 
 * @return If life is smaller than zero, call function Dead and return true. Otherwise return false.
 */
bool TMAP_UNIT::Injure(const float injury)
{
  float result_life;

  if ((injury < 0) || ((life == 0) && !TestState(US_IS_BEING_BUILT)) || (TestState(US_DYING)) || (TestState(US_ZOMBIE)) || (TestState(US_DELETE))) return false; 
  
  result_life = life - injury;

  if (result_life < 0)
    SetLife(0);
  else
    SetLife(result_life);
  
  if (life == 0) {
    Dead(true); 
    return true;
  } 
  else 
    return false;
}


/** 
 * Heal unit for the value of the parameter but outside to maximum life of unit kind. 
 * If unit is completly healed return true, otherwise return false.
 *
 * @param value Value of healing. If the param is lesser than zero function doesn't take any efect and return false.
 */
float TMAP_UNIT::Heal(const float value) 
{ 
  float ret_val;
  float aux_life, max_life;

  if (value < 0) return 0; 
  
  aux_life = value + life;
  max_life = (float)((TMAP_ITEM *)pitem)->GetMaxLife();
  
  if (aux_life <= max_life) {
    SetLife(aux_life); 
    ret_val = value;
  }
  else { 
    aux_life = life; 
    SetLife(max_life); 
    ret_val = max_life - aux_life;
  }

  return ret_val;
}


/**
 *  Method draws unit.
 */
void TMAP_UNIT::Draw(T_BYTE style)
{
  if (!(visible || TestState(US_GHOST)) || !in_active_area) return;

  bool use_depth = false;
  TMAP_ITEM *pit = static_cast<TMAP_ITEM *>(pitem);
  T_BYTE w = pit->GetWidth();
  T_BYTE h = pit->GetHeight();
  T_BYTE l = pit->selection_height;

  if (view_segment == DRW_ALL_SEGMENTS && pos.segment == 1) {
    if (TestItemType(IT_BUILDING) || TestItemType(IT_FACTORY) || TestItemType(IT_SOURCE)) {
      use_depth = (((TMAP_ITEM *)pitem)->GetExistSegments().max == 1);
    }
    else use_depth = true;
  }

  if (TestState(US_GHOST)) {
    // set color accorting to style
    SetUnitColor(style);

    if (use_depth) glEnable(GL_DEPTH_TEST);

    // draw unit
    SetMapPosition(rpos_x, rpos_y);
    animation->Draw();

    if (use_depth) glDisable(GL_DEPTH_TEST);
    return;
  }
  else if (!visible) return;

  SetMapPosition(rpos_x, rpos_y);

  if (use_depth) glEnable(GL_DEPTH_TEST);

  // quad under selected unit
  if (mouse.over_unit == this) {
    glDisable(GL_TEXTURE_2D);

    glColor4f(1, 1, 0, 0.4f);
    glBegin(GL_QUADS);
      glVertex2d(0, 0);
      glVertex2d(w * DAT_MAPEL_STRAIGHT_SIZE_2, w * DAT_MAPEL_DIAGONAL_SIZE_2);
      glVertex2d((w - h) * DAT_MAPEL_STRAIGHT_SIZE_2, (w + h) * DAT_MAPEL_DIAGONAL_SIZE_2);
      glVertex2d(-h * DAT_MAPEL_STRAIGHT_SIZE_2, h * DAT_MAPEL_DIAGONAL_SIZE_2);
    glEnd();

    glEnable(GL_TEXTURE_2D);
  }

  // quad under building unit in progress
  if (TestState(US_IS_BEING_BUILT) && (life == 0)) {
    if (!selected) {
      glDisable(GL_TEXTURE_2D);

      glColor3f(1, 1, 1);
      glBegin(GL_LINE_LOOP);
        glVertex2d(0, 0);
        glVertex2d(w * DAT_MAPEL_STRAIGHT_SIZE_2, w * DAT_MAPEL_DIAGONAL_SIZE_2);
        glVertex2d((w - h) * DAT_MAPEL_STRAIGHT_SIZE_2, (w + h) * DAT_MAPEL_DIAGONAL_SIZE_2);
        glVertex2d(-h * DAT_MAPEL_STRAIGHT_SIZE_2, h * DAT_MAPEL_DIAGONAL_SIZE_2);
      glEnd();

      glEnable(GL_TEXTURE_2D);
    }

    style = DS_BUILDING;
  }

  // draw background selection
  if (selected) DrawBGSelection(style);
  
  // set color accorting to style
  SetUnitColor(style);

  // draw animation
  animation->Draw();

  // burn over building
  if (burn_animation && burn_animation->IsVisible()) {
    glPushMatrix();

    MapPosition(w * 0.5f, h * 0.5f);
    glTranslatef(pit->burning_x, pit->burning_y, 0);
    burn_animation->Draw();

    glPopMatrix();
  }

  // need picture over building
  if (sign_animation) {
    glPushMatrix();

    if (!TestItemType(IT_FORCE) && !TestItemType(IT_WORKER)) MapPosition(w * 0.5f, h * 0.5f);
    glTranslated(0, l * 0.5f, 0);
    sign_animation->Draw();

    glPopMatrix();
  }

  // draw foreground selection
  if (selected) {
    DrawFGSelection(style);

    // draw group id
    if (group_id >= 0) {
      char txt[3];

      SetUnitColor(style);

      glfDisable(GLF_RESET_PROJECTION);
      sprintf(txt, "%d", group_id + 1);
      glfPrint(font0, 12.0f, -6.0f, txt, true);
      glfEnable(GLF_RESET_PROJECTION);
    }
  }

  if (use_depth) glDisable(GL_DEPTH_TEST);
}


/**
 *  Method draws unit into radar.
 */
void TMAP_UNIT::DrawToRadar(void)
{
  if (!(visible || TestState(US_GHOST))) return;

  GLfloat zoom = radar.zoom;
  GLfloat w = pitem->GetWidth() * zoom;
  GLfloat h = pitem->GetHeight() * zoom;

  if (w < DRW_MIN_RADAR_SIZE) w = DRW_MIN_RADAR_SIZE;
  if (h < DRW_MIN_RADAR_SIZE) h = DRW_MIN_RADAR_SIZE;

  glPushMatrix();
  RadarPosition(rpos_x, rpos_y);

  SET_RADAR_COLOR;

  glDisable(GL_TEXTURE_2D);

  glBegin(GL_QUADS);
    glVertex2d(0, 0);
    glVertex2d(w, w);
    glVertex2d(w - h, w + h);
    glVertex2d(-h, h);
  glEnd();

  glEnable(GL_TEXTURE_2D);
  glPopMatrix();
}

/**
 *  Method draws background unit selection unit.
 */
void TMAP_UNIT::DrawBGSelection(T_BYTE style)
{
  TMAP_ITEM *pit = static_cast<TMAP_ITEM *>(pitem);
  T_BYTE w = pit->GetWidth();
  T_BYTE h = pit->GetHeight();
  T_BYTE l = pit->selection_height; 

  glDisable(GL_TEXTURE_2D);

  if (pit->GetItemType() == IT_FORCE || pit->GetItemType() == IT_WORKER) {
    SET_FRAME_COLOR;

    // main frame
    glBegin(GL_LINE_LOOP);
      glVertex2d(0, 0);
      glVertex2d(w * DAT_MAPEL_STRAIGHT_SIZE_2, w * DAT_MAPEL_DIAGONAL_SIZE_2);
      glVertex2d((w - h) * DAT_MAPEL_STRAIGHT_SIZE_2, (w + h) * DAT_MAPEL_DIAGONAL_SIZE_2);
      glVertex2d(-h * DAT_MAPEL_STRAIGHT_SIZE_2, h * DAT_MAPEL_DIAGONAL_SIZE_2);
    glEnd();
  }

  else {
    SET_FRAME_COLOR;

    // main frame
    glBegin(GL_LINE_STRIP);
      glVertex2d((w - h) * DAT_MAPEL_STRAIGHT_SIZE_2, (w + h) * DAT_MAPEL_DIAGONAL_SIZE_2);
      glVertex2d(-h * DAT_MAPEL_STRAIGHT_SIZE_2, h * DAT_MAPEL_DIAGONAL_SIZE_2);
      glVertex2d(0, 0);
      glVertex2d(w * DAT_MAPEL_STRAIGHT_SIZE_2, w * DAT_MAPEL_DIAGONAL_SIZE_2);
      glVertex2d((w - h) * DAT_MAPEL_STRAIGHT_SIZE_2, (w + h) * DAT_MAPEL_DIAGONAL_SIZE_2);    
      glVertex2d((w - h) * DAT_MAPEL_STRAIGHT_SIZE_2, (w + h) * DAT_MAPEL_DIAGONAL_SIZE_2 + l);
      glVertex2d(w * DAT_MAPEL_STRAIGHT_SIZE_2, w * DAT_MAPEL_DIAGONAL_SIZE_2 + l);
    glEnd();

    glBegin(GL_LINES);
      glVertex2d((w - h) * DAT_MAPEL_STRAIGHT_SIZE_2, (w + h) * DAT_MAPEL_DIAGONAL_SIZE_2 + l);
      glVertex2d(-h * DAT_MAPEL_STRAIGHT_SIZE_2, h * DAT_MAPEL_DIAGONAL_SIZE_2 + l);
    glEnd();
  }

  glEnable(GL_TEXTURE_2D);
}


/**
 *  Method draws foreground unit selection unit.
 */
void TMAP_UNIT::DrawFGSelection(T_BYTE style)
{
  TMAP_ITEM *pit = static_cast<TMAP_ITEM *>(pitem);

  T_BYTE w = pit->GetWidth();
  T_BYTE h = pit->GetHeight();
  T_BYTE l = pit->selection_height;

  float r, g, b;
  float lifep = float(life) / pit->GetMaxLife();

  if (lifep > UNI_LIFE_LIMIT1)      { r = b = 0; g = 1; }
  else if (lifep > UNI_LIFE_LIMIT2) { r = g = 1; b = 0; }
  else                              { g = b = 0; r = 1; }

  glDisable(GL_TEXTURE_2D);

  if (pit->GetItemType() == IT_FORCE || pit->GetItemType() == IT_WORKER) {
    GLfloat dx = -w * DAT_MAPEL_STRAIGHT_SIZE_2 + 3;
    GLfloat d1 = w * DAT_MAPEL_STRAIGHT_SIZE - 6;

    // status of hided units
    if (GetPlayer() == myself && AcceptsHidedUnits()) {
      DrawStatusQuad(dx, l, d1, float(hided_count) / pit->GetMaxHidedUnits(), 1, 1, 1);
      l += 3;
    }

    // life quad
    DrawStatusQuad(dx, l, d1, lifep, r, g, b);
  }

  else {
    GLfloat d1 = w * DAT_MAPEL_STRAIGHT_SIZE_2;
    GLfloat d2 = w * DAT_MAPEL_DIAGONAL_SIZE_2;

    // life cube
    DrawStatusCube(d1 - UNI_LIFE_BAR_SIZE, d2 - UNI_LIFE_BAR_SIZE_2, l, lifep, r, g, b);

    // status of hided units
    if (GetPlayer() == myself && AcceptsHidedUnits()) {
      lifep = float(hided_count) / pit->GetMaxHidedUnits();

      DrawStatusCube(d1 - 2 * UNI_LIFE_BAR_SIZE, d2 - 2 * UNI_LIFE_BAR_SIZE_2, l, lifep, 1, 1, 1);
    }

    SET_FRAME_COLOR;

    // main border
    glBegin(GL_LINE_STRIP);
      glVertex2f(-h * DAT_MAPEL_STRAIGHT_SIZE_2, h * DAT_MAPEL_DIAGONAL_SIZE_2);
      glVertex2f(-h * DAT_MAPEL_STRAIGHT_SIZE_2, h * DAT_MAPEL_DIAGONAL_SIZE_2 + l);
      glVertex2f(0, l);
      glVertex2f(0, 0);
    glEnd();

    glBegin(GL_LINE_STRIP);
      glVertex2f(0, l);
      glVertex2f(d1, d2 + l);
      glVertex2f(d1, d2);
    glEnd();
  }

  glEnable(GL_TEXTURE_2D);
}


/**
 *  Method is calling when new event is get from queue. Returns true if unit does something.
 *
 *  @param Pointer to processed event.
 */
void TMAP_UNIT::ProcessEvent(TEVENT * proc_event)
{
  double new_time_stamp = 0.0;

  TATTACK_INFO attack_info;
  TMAP_ITEM *itm = static_cast<TMAP_ITEM *>(pitem);
  TMAP_UNIT * new_target;

  /****************** Put state of event ********************************/
  
  // test if event is event (else it is request)
  if (proc_event->GetEvent() < RQ_FIRST) PutState(proc_event->GetEvent());
  
  /****************** Synchronisation and setup (ALL UNITS) *************************/

  // RQ_DELETE
  if (proc_event->TestEvent(RQ_DELETE))
    PutState(US_DELETE);

  // RQ_ZOMBIE,
  if (proc_event->TestEvent(RQ_ZOMBIE))
    PutState(US_ZOMBIE);

  // RQ_DYING
  if (proc_event->TestEvent(RQ_DYING))
    PutState(US_DYING);

  // RQ_FEEDING
  if (proc_event->TestEvent(RQ_FEEDING))
    if (TestState(US_ATTACKING)) PutState(US_FEEDING);
  
  //US_START_ATTACK
  if (proc_event->TestEvent(US_START_ATTACK) || proc_event->TestEvent(US_NEXT_ATTACK))
  {
    // translate target from id to pointer
    new_target = static_cast<TMAP_UNIT*>(players[proc_event->simple6]->hash_table_units.GetUnitPointer(proc_event->int1));
    if (new_target != GetTarget()) 
    {
      ReleaseCountedPointer(target);
      if (new_target) 
        SetTarget(static_cast<TMAP_UNIT*>(new_target->AcquirePointer()));
    }
  }

  // US_END_ATTACK
  if (proc_event->TestEvent(US_END_ATTACK))
  {
    last_target = target; // not counted pointer, but it is OK, because last_targed will be compared only to new target
    ClearActions(); 
   
    if (!player_array.IsRemote(proc_event->GetPlayerID())) {
      SendEvent(false, proc_event->GetTimeStamp() + TS_MIN_EVENTS_DIFF, US_STAY, -1, pos.x, pos.y, pos.segment);
      SetAutoAttack(false);
    }
  }

  // RQ_SYNC_LIFE
  if (proc_event->TestEvent(RQ_SYNC_LIFE))
  {
    SetLife((float)proc_event->int1);
  }

  /****************** Compute new values - textures, times (ALL UNITS) ********************************/
  
  // US_DYING, RQ_DYING
  if (proc_event->TestEvent(US_DYING) || proc_event->TestEvent(RQ_DYING)) {
  #if SOUND    
    // play sound
    if (visible) {
      if (!((TMAP_ITEM *)pitem)->snd_dead.IsEmpty())
        PlaySound(&((TMAP_ITEM *)pitem)->snd_dead, 1);
      else {
        if (TestItemType(IT_BUILDING) || TestItemType(IT_FACTORY) || TestItemType(IT_SOURCE))
          PlaySound(&player->race->snd_explosion, 1);
        else
          PlaySound(&player->race->snd_dead, 1);
      }
    }
  #endif
  }
  
  // US_DELETE, RQ_DELETE
  if (proc_event->TestEvent(US_DELETE) || proc_event->TestEvent(RQ_DELETE)) {
    DeleteFromSegments();
    UnitToDelete(true);
    return;
  }
 
  // RQ_CREATE_PROJECTILE
  if (proc_event->TestEvent(RQ_CREATE_PROJECTILE))
  {
    TGUN *p_gun = itm->GetArmament()->GetOffensive();
    TPROJECTILE_ITEM* pritem = static_cast<TPROJECTILE_ITEM*>(p_gun->GetProjectileItem());

    TPOSITION_3D ipos;  //impact position
    TPOSITION_3D spos;  //start position
    float distance = 1.0;
    double impact_time;

    spos.SetPosition(proc_event->simple1, proc_event->simple2, proc_event->simple3);
    ipos.SetPosition(proc_event->simple4, proc_event->simple5, proc_event->simple6);

    if ((p_gun->GetRange().min != 1) || (p_gun->GetRange().max != 1)) {
      distance = static_cast<float>(sqrt(
      static_cast<float>(sqr(static_cast<int>(spos.x) - ipos.x)
      + sqr(static_cast<int>(spos.y) - ipos.y)
      + sqr(static_cast<int>(GetPosition().segment) - ipos.segment))
      ));
    }
    
    if (distance == 1)     //target is at the neighbourgh field in the map
      impact_time = 0;
    else
      impact_time = distance / pritem->GetSpeed();

    //allocate of the new TPROJECTILE
    shot = NEW TPROJECTILE_UNIT(GetPlayerID(), impact_time, spos, ipos, p_gun, proc_event->int2, true);

    #if DEBUG_EVENTS
      Debug(LogMsg("SET_SHOT_R: P:%d U:%d E:%s RQ:%d X:%d Y:%d R:%d I1:%d TS:%f COUNT:%d", proc_event->GetPlayerID(), proc_event->GetUnitID(), EventToString(proc_event->GetEvent()), proc_event->GetRequestID(), proc_event->simple1, proc_event->simple2, proc_event->simple4, proc_event->int2, proc_event->GetTimeStamp(), queue_events->GetQueueLength()));
    #endif

    if (!shot){
      Warning(LogMsg("Not enough memory for shot in the function FireOn().\n"));
      return;
    }

#if SOUND
    if (visible) {
      PlaySound(&itm->snd_fireoff, 2);
    }
#endif
  }
  
  // RQ_FIRE_OFF
  if (proc_event->TestEvent(RQ_FIRE_OFF))
  {
    //if shot is prepared fire off
    if ((shot != NULL) && (TestState(US_ATTACKING)))
    {
      //add unit to segment lists
      shot->AddToSegments();
      shot->Shot();

      if (player_array.IsRemote(proc_event->GetPlayerID())) {
        #if DEBUG_EVENTS
          Debug(LogMsg("NULLSHOT_R: P:%d U:%d E:%s RQ:%d X:%d Y:%d R:%d I1:%d TS:%f COUNT:%d", proc_event->GetPlayerID(), proc_event->GetUnitID(), EventToString(proc_event->GetEvent()), proc_event->GetRequestID(), proc_event->simple1, proc_event->simple2, proc_event->simple4, shot->GetUnitID(), proc_event->GetTimeStamp(), queue_events->GetQueueLength()));
        #endif

        shot = NULL; // null shot to remote players
      }
    }
  }

  // RQ_FEEDING
  if (proc_event->TestEvent(RQ_FEEDING))
  {
#if SOUND
    if ((visible) && (TestState(US_FEEDING))) {
      PlaySound(&itm->snd_fireon, 2);
    }
#endif
  }

  /****************** Compute next event (ONLY LOCAL UNITS) *******************************/
  
  if (!player_array.IsRemote(proc_event->GetPlayerID())) { // compute only not remote (local) units

    //US_START_ATTACK
    if (proc_event->TestEvent(US_START_ATTACK))
    {
      if (HasTarget()) 
      {
        SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_ATTACK, -1, pos.x, pos.y, pos.segment, 0, 0, target->GetPlayerID(), target->GetUnitID());
      }
      else 
      {
        ClearActions();
        SendEvent(false, proc_event->GetTimeStamp(), US_STAY, -1, pos.x, pos.y, pos.segment);
      }
    }

 
    // US_NEXT_ATTACK
    if (proc_event->TestEvent(US_NEXT_ATTACK))
    {
      //if target will be delete or unit cannot attack stop attacking
      if (target->TestState(US_DYING) || target->TestState(US_ZOMBIE) || target->TestState(US_DELETE) 
        || (static_cast<TMAP_ITEM*>(GetPointerToItem())->GetArmament()->GetOffensive() == NULL)) 
      {
        SendEvent(false, proc_event->GetTimeStamp(), US_END_ATTACK, -1, GetPosition().x, GetPosition().y, GetPosition().segment);
      }
      else
      {
        attack_info = itm->GetArmament()->IsPossibleAttack(this, target);   //tests possibility of attack and determine best impact position
        switch (attack_info.state)
        {
          case FIG_AIF_ATTACK_OK:       // Run of attack is O.K.
            //send event to start attack
            SendEvent(false, proc_event->GetTimeStamp(), US_ATTACKING, -1, GetPosition().x, GetPosition().y, GetPosition().segment, 0, 
                      attack_info.impact_position.x, attack_info.impact_position.y, attack_info.impact_position.segment);
          
            break;

          case FIG_AIF_TO_UPPER_SEG:    // Attack is possible only in the upper segment.
          case FIG_AIF_TO_LOWER_SEG:    // Attack is possible only in the lower segment.
            if ((itm) && (itm->GetArmament()->GetOffensive()->GetFlags() & FIG_GUN_SAME_SEGMENT) 
                && (!itm->GetExistSegments().IsMember(target->GetPosition().segment)))
            {   
              MessageText(false, const_cast<char*>("Target %s is in the unshotable segment."), target->GetPointerToItem()->name);

              //it is impossible shot to segment where is target -> stop all actions
              SendEvent(false, proc_event->GetTimeStamp(), US_END_ATTACK, -1, pos.x, pos.y, pos.segment);
              break;
            }
            
            //send event to start attack
            SendEvent(false, proc_event->GetTimeStamp(), US_ATTACKING, -1, GetPosition().x, GetPosition().y, GetPosition().segment, 0, 
                      attack_info.impact_position.x, attack_info.impact_position.y, attack_info.impact_position.segment);
            break;

          default:
            switch (attack_info.state) {
              case FIG_AIF_TOO_FAR_AWAY:    // Attacker is too far away from the target.
                MessageText(false, const_cast<char*>("Target %s is too far away."), target->GetPointerToItem()->name);
                break;
              case FIG_AIF_TOO_CLOSE_TO:    // Attacker is too close to the target.
                MessageText(false, const_cast<char*>("Target %s is too close."), target->GetPointerToItem()->name);
                break;

              case FIG_AIF_UNSHOTABLE_SEG:  // Defender is in the unshotable segment.
                MessageText(false, const_cast<char*>("Target %s is in the unshotable segment."), target->GetPointerToItem()->name);
                break;
              case FIG_AIF_ATTACT_NOT_EFFECIVE:  // Attacker's attack does not have effect.
                MessageText(false, const_cast<char*>("Attack to %s does not have effect."), target->GetPointerToItem()->name);
                break;
              case FIG_AIF_ATTACK_FAILED:   // Flag informates that attack failed.
              default:
                MessageText(false, const_cast<char*>("Attack to %s failed."), target->GetPointerToItem()->name);
                break;
            }
            
            SendEvent(false, proc_event->GetTimeStamp(), US_END_ATTACK, -1, pos.x, pos.y, pos.segment);
            break;
        }
      }
    }

    // US_ATTACKING
    if (proc_event->TestEvent(US_ATTACKING))
    {
      if (FireOn(proc_event->simple5, proc_event->simple6, proc_event->int1, proc_event->GetTimeStamp()))    //fire on
      {
        new_time_stamp = proc_event->GetTimeStamp();

        TEVENT * hlp;
        hlp = pool_events->GetFromPool();
        
        new_time_stamp += (itm->GetArmament()->GetOffensive()->GetShotTime() + TS_MIN_EVENTS_DIFF);
        SendRequest(false, new_time_stamp, RQ_FIRE_OFF, -1);
        // send info to not local players that new projectile is creating now
        hlp->SetEventProps(GetPlayerID(), GetUnitID(), false, new_time_stamp, RQ_FIRE_OFF, US_NONE, -1, GetPosition().x, GetPosition().y, GetPosition().segment, 0, 0, 0, 0);
        SendNetEvent(hlp, all_players);
              
        new_time_stamp += (itm->GetArmament()->GetOffensive()->GetWaitTime() + TS_MIN_EVENTS_DIFF);
        SendRequest(false, new_time_stamp, RQ_FEEDING, -1, 0, 0, 0, 0, 0, target->GetPlayerID(), target->GetUnitID()); 
        // send info to not local players that unit is feeding
        hlp->SetEventProps(GetPlayerID(), GetUnitID(), false, new_time_stamp, RQ_FEEDING, US_NONE, -1, 0, 0, 0, 0, 0, target->GetPlayerID(), target->GetUnitID()); 
        SendNetEvent(hlp, all_players);

        pool_events->PutToPool(hlp);

        // plan new US_NEXT_ATTACK (new attack cycle)
        new_time_stamp += (itm->GetArmament()->GetOffensive()->GetFeedTime() + TS_MIN_EVENTS_DIFF);
        SendEvent(false, new_time_stamp, US_NEXT_ATTACK, -1, 0, 0, 0, 0, 0, target->GetPlayerID(), target->GetUnitID());
      }
      else
      {
        MessageText(false, const_cast<char*>("Attack to %s failed."), target->GetPointerToItem()->name);
        SendEvent(false, proc_event->GetTimeStamp(), US_END_ATTACK, -1, pos.x, pos.y, pos.segment);
      }
    }

    if (proc_event->TestEvent(RQ_FIRE_OFF))
    // RQ_FIRE_OFF
    {
      //if shot is prepared fire off
      if ((shot != NULL) && (TestState(US_ATTACKING)))
      {
        // send info to not local players
        TEVENT * hlp;
        hlp = pool_events->GetFromPool();

        // send info about changing segment
        new_time_stamp = proc_event->GetTimeStamp() + TS_MIN_EVENTS_DIFF;
        for (int i = 0; i < (shot->GetPosition().segment - shot->GetImpactPos().segment); i++){
          shot->SendRequest(false, new_time_stamp + ((i+1)*shot->GetSegmentTime()), RQ_CHANGE_SEGMENT, -1);
          
          hlp->SetEventProps(shot->GetPlayerID(), shot->GetUnitID(), false, new_time_stamp + ((i+1)*shot->GetSegmentTime()), RQ_CHANGE_SEGMENT, US_NONE, -1, 0, 0, 0, 0, 0, 0, 0);
          SendNetEvent(hlp, all_players);
        }
        
        
        //send event about impact
        shot->SendRequest(false, new_time_stamp + shot->GetImpactTime(), RQ_IMPACT, -1, shot->GetImpactPos().x, shot->GetImpactPos().y, shot->GetImpactPos().segment);

        hlp->SetEventProps(shot->GetPlayerID(), shot->GetUnitID(), false, new_time_stamp + shot->GetImpactTime(), RQ_IMPACT, US_NONE, -1, shot->GetImpactPos().x, shot->GetImpactPos().y, shot->GetImpactPos().segment, 0, 0, 0, 0);
        SendNetEvent(hlp, all_players);
        
        
        pool_events->PutToPool(hlp);

        #if DEBUG_EVENTS
          Debug(LogMsg("NULLSHOT_L: P:%d U:%d E:%s RQ:%d X:%d Y:%d R:%d I1:%d TS:%f COUNT:%d", proc_event->GetPlayerID(), proc_event->GetUnitID(), EventToString(proc_event->GetEvent()), proc_event->GetRequestID(), proc_event->simple1, proc_event->simple2, proc_event->simple4, shot->GetUnitID(), proc_event->GetTimeStamp(), queue_events->GetQueueLength()));
        #endif

        shot = NULL;
      }
    }
  }
}


/**
 *  Method is called for periodically updating of look and features.
 *
 *  @param time_shift  Time shift from the last update.
 */
bool TMAP_UNIT::UpdateGraphics(double time_shift)
{
  TPLAYER_UNIT::UpdateGraphics(time_shift);

  if (TestState(US_GHOST)) return false;

  if (burn_animation && burn_animation->IsVisible()) burn_animation->Update(time_shift);

  if (!visible && selected) {
    selection->DeleteUnit(this);
  }

  return false;
}


/**
 *  Fires on position included in parameter. Creates new instance of TPROJECTILE class and fills its values.
 *  If the new instance of TPROJECTILE class is successful created returns true otherwise return false.
 *
 *  @param ipos_x First coordinate of the impact position.
 *  @param ipos_y Second coordinate of the impact position.
 *  @param ipos_segment Segment coordinate of the impact position.
 *  @param ts Time stamp of the impact.
 */
bool TMAP_UNIT::FireOn(const int ipos_x, const int ipos_y, const int ipos_segment, const double ts)
{
  double impact_time;
  TMAP_ITEM* mitem = static_cast<TMAP_ITEM*>(GetPointerToItem());
  float distance = 1.0;
  TGUN *p_gun = mitem->GetArmament()->GetOffensive();
  TPROJECTILE_ITEM* pitem = static_cast<TPROJECTILE_ITEM*>(p_gun->GetProjectileItem());
  TPOSITION_3D ipos;  //impact position
  TPOSITION_3D spos;  //start position
  
  if ((p_gun->GetRange().min != 1) || (p_gun->GetRange().max != 1))
  {
    //compute start position
    spos.x = GetPosition().x + GetUnitWidth()/2;
    spos.y = GetPosition().y + GetUnitHeight()/2;
    //compute start segment
    TITEM_TYPE item_type = mitem->GetItemType();
    if ((item_type == IT_FORCE) || (item_type == IT_WORKER)) 
    {
      spos.segment = GetPosition().segment;
    }
    else
    {
      if (mitem->GetExistSegments().IsMember(ipos_segment)) 
      {
        spos.segment = ipos_segment;
      }
      else if (mitem->GetExistSegments().max < ipos.segment)
      {
        spos.segment = mitem->GetExistSegments().max;
      }
      else
      {
        spos.segment = mitem->GetExistSegments().min;
      }
    }
    distance = static_cast<float>(sqrt(
      static_cast<float>(sqr(static_cast<int>(spos.x) - ipos_x)
      + sqr(static_cast<int>(spos.y) - ipos_y)
      + sqr(static_cast<int>(GetPosition().segment) - ipos_segment))
    ));
  } 
  else 
  {
    spos = GetPosition();
  }

  if (distance == 1)     //target is at the neighbourgh field in the map
    impact_time = 0;
  else
    impact_time = distance / pitem->GetSpeed();

  ipos.SetPosition(ipos_x, ipos_y, ipos_segment);

  //allocate of the new TPROJECTILE
  shot = NEW TPROJECTILE_UNIT(GetPlayerID(), impact_time, spos, ipos, p_gun, 0, true);

  #if DEBUG_EVENTS
    Debug(LogMsg("SET_SHOT_L: P:%d U:%d E:%s RQ:%d X:%d Y:%d R:%d I1:%d TS:%f COUNT:%d", GetPlayerID(), GetUnitID(), EventToString(0), 0, pos.x, pos.y, pos.segment, shot->GetUnitID(), ts, queue_events->GetQueueLength()));
  #endif

  if (!shot)    //test success of the allocate function
  {
    Warning(LogMsg("Not enough memory for shot in the function FireOn().\n"));
    return false;
  }

  // send info to not local players that new projectile is creating now
  TEVENT * hlp;

  hlp = pool_events->GetFromPool();
  hlp->SetEventProps(GetPlayerID(), GetUnitID(), false, ts, RQ_CREATE_PROJECTILE, US_NONE, -1, spos.x, spos.y, spos.segment, ipos.x, ipos.y, ipos.segment, 0, shot->GetUnitID());
  SendNetEvent(hlp, all_players);
  pool_events->PutToPool(hlp);


#if SOUND
  if (visible) {
    PlaySound(&mitem->snd_fireoff, 2);
  }
#endif

  return true;
}


/**
 *  Returns unit action - what is doing now.
 */
TUNIT_ACTION TMAP_UNIT::GetAction()
{
  if (target) return UA_ATTACK;
  else if (
    TestState(US_LANDING) || TestState(US_UNLANDING) ||
    TestState(US_MOVE) || TestState(US_NEXT_STEP) || TestState(US_TRY_TO_MOVE) || 
    TestState(US_LEFT_ROTATING) || TestState(US_RIGHT_ROTATING) || 
    TestState(US_WAIT_FOR_PATH) 
  ) return UA_MOVE;
  else return UA_STAY;
}

/**
 *  Clears all actions of unit.
 */
void TMAP_UNIT::ClearActions()
{
  ReleaseCountedPointer(target);
  waiting_request_id = 0;
}

/**
 *  Send event US_STAY for correct sop unit (land, stay).
 */
bool TMAP_UNIT::StartStaying()
{
  if (TestState(US_STAY)) return false;

  // send event to queue
  process_mutex->Lock();
  ClearActions();
  SendEvent(false, glfwGetTime(), US_STAY, -1, pos.x, pos.y, pos.segment);
  process_mutex->Unlock();

  return true;
}


/**
 *  Change unit animation according to its state.
 */
void TMAP_UNIT::ChangeAnimation()
{
  int tg;
  TMAP_ITEM *pit = static_cast<TMAP_ITEM *>(pitem);

  if (TestState(US_ATTACKING)  || TestState(US_FEEDING))
    tg = pit->tg_attack_id;
  else
    tg = pit->tg_stay_id;

  if (tg < 0) tg = pit->tg_stay_id;

  animation->SetTexItem(player->race->tex_table.GetTexture(tg, 0));
  animation->Play();
}


bool TMAP_UNIT::CanAcceptUnit(TFORCE_UNIT *unit)
{
  return ((hided_count + unit->GetUnitWidth() * unit->GetUnitHeight()) <= static_cast<TMAP_ITEM*>(pitem)->GetMaxHidedUnits());
}


/**
 *  The method adds unit into the list according to which_list parameter
 *  
 *  @param added  Pointer to added unit.
 *  @param which_list  Constant, which says, which list to add to. 
 *  Only two different lists supported for now.
 */
bool TMAP_UNIT::AddToListOfUnits(TFORCE_UNIT *unit, T_BYTE which_list)
{
  if (unit->IsHeld()) return false;

  //hide unit inside the source
  if (which_list == LIST_HIDING) {
    if (!CanAcceptUnit(unit)) return false;
    GetHidedUnits().AddNode(unit);
    hided_count += unit->GetUnitWidth() * unit->GetUnitHeight();
  }

  // add worker into workers list
  else if (unit->TestItemType(IT_WORKER))
    GetWorkingUnits().AddNode((TWORKER_UNIT *)unit);

  if (unit->GetPlayer() == myself) myself_units++;

  AcquirePointer();
  unit->SetHeld(true, this, which_list);

  return true;
}


/**
 *  The method removes unit from the list according to which_list parameter.
 *
 *  @param removed  Pointer to removed unit.
 *  @param which_list  Constant, which says, which list to add to. 
 *  Only two different lists supported for now.
 */
void TMAP_UNIT::RemoveFromListOfUnits(TFORCE_UNIT *unit, T_BYTE which_list)
{
  if (!unit->IsHeld()) return;

  if (which_list == LIST_HIDING) {
    GetHidedUnits().RemoveNode(unit);
    hided_count -= unit->GetUnitWidth() * unit->GetUnitHeight();
  }

  else if (unit->TestItemType(IT_WORKER)) {
    GetWorkingUnits().RemoveNode((TWORKER_UNIT *)unit);
  }

  else return;

  if (unit->GetPlayer() == myself) myself_units--;

  ReleasePointer();
  unit->SetHeld(false);
}


bool TMAP_UNIT::CanEject()
{
  return !hided_units.IsEmpty() && GetPlayer() == myself;
}


void TMAP_UNIT::EjectUnits()
{
  TLIST<TFORCE_UNIT> copy_hided_units;      // Copy of the list of force units which are hiding.
  TFORCE_UNIT *unit;  // Help variable.
  
  process_mutex->Lock();

  if (!CanEject()) {
    process_mutex->Unlock();
    return;
  }

  // copy hided units to help list 
  while (!hided_units.IsEmpty())
    copy_hided_units.AddNode(hided_units.TakeFirstOut());

  while (!copy_hided_units.IsEmpty()) {
    unit = copy_hided_units.TakeFirstOut();

    if (unit->LeaveHolderUnit(this)) {
      unit->SendEvent(false, glfwGetTime(), US_EJECTING, -1, unit->GetPosition().x, unit->GetPosition().y, unit->GetPosition().segment, unit->GetMoveDirection());
    }
    else {
      hided_units.AddNode(unit);
    }
  }

  process_mutex->Unlock();
}


/**
 *  Sets state to US_DYING
 */
void TMAP_UNIT::Dead(bool local)
{
  if (local) {
    if (group_id >= 0)
      selection->DeleteStoredUnit(group_id, this);

    SendEvent(false, glfwGetTime(), US_DYING, -1, pos.x, pos.y, pos.segment);
  }
  else
    SendRequestLocal(false, glfwGetTime(), US_DYING, -1, pos.x, pos.y, pos.segment);
}

/**
 *  Delete unit from all structures.
 */
void TMAP_UNIT::Disconnect()
{
  Dead(false);
}


/**
 *  Method place unit to map.
 *
 *  @param  to_segment  Add unit to segment list.
 */
bool TMAP_UNIT::AddToMap(bool to_segment, bool set_view)
{
  int i, j;

  // check if another unit is in the map
  for (i = 0; i < GetUnitWidth(); i++)
    for (j = 0; j < GetUnitHeight(); j++)                   
      if (map.segments[pos.segment].surface[pos.x + i][pos.y + j].unit) 
        return false;

  if (to_segment) AddToSegments();

  for (i = 0; i < GetUnitWidth(); i++)
    for (j = 0; j < GetUnitHeight(); j++)
    {
      map.segments[pos.segment].surface[pos.x + i][pos.y + j].unit = this;
      map.segments[pos.segment].surface[pos.x + i][pos.y + j].GetAimersList()->AttackEnemy(this, false);
      map.segments[pos.segment].surface[pos.x + i][pos.y + j].GetWatchersList()->AttackEnemy(this, true);
    }

  is_in_map = true;

  return true;
}


/**
 *  Method hides worker from map.
 */
void TMAP_UNIT::DeleteFromMap(bool from_segment)
{
  int i, j;

  if (from_segment) DeleteFromSegments();

  for (i = 0; i < GetUnitWidth(); i++)
    for (j = 0; j < GetUnitHeight(); j++)
      map.segments[pos.segment].surface[pos.x + i][pos.y + j].unit = NULL;

  if (selected) selection->DeleteUnit(this);

  is_in_map = false;
}


void TMAP_UNIT::MessageText(bool possitive, char *msg, ...)
{
  static char pom_text1[UNI_MAX_MESSAGE_LENGTH + 1];
  static char pom_text2[UNI_MAX_MESSAGE_LENGTH + 1];
  va_list arg;

  // message is written only if it is mine
  if (GetPlayer() == myself) {
    va_start(arg, msg);

    sprintf(pom_text1, "%s: %s", pitem->name, msg);

  #ifdef WINDOWS  // on Windows
    _vsnprintf(pom_text2, UNI_MAX_MESSAGE_LENGTH, pom_text1, arg);
  #else // on UNIX
    vsnprintf(pom_text2, UNI_MAX_MESSAGE_LENGTH, pom_text1, arg);
  #endif
  
    va_end(arg);

    if (possitive) ost->AddText(pom_text2, 0.5f, 0.8f, 0.5f);
    else ost->AddText(pom_text2, 1.0f, 0.5f, 0.5f);
  }
}

/** 
 *  The method start attack to the unit from parameter.
 *
 *  @param unit Pointer to attacked unit.
 *  @param automatic Value is true if function was called automaticly (watch, aim lists).
 *  @return The method return true if start was successful.
 */
bool TMAP_UNIT::StartAttacking(TMAP_UNIT *unit, bool automatic)
{
  process_mutex->Lock();

  SetAutoAttack(automatic);
  SendEvent(false, glfwGetTime(), US_START_ATTACK, -1, GetPosition().x, GetPosition().y, GetPosition().segment, 0, 0, unit->GetPlayerID(), unit->GetUnitID());

  process_mutex->Unlock();

  return true;
}


/** 
 *  Selects reaction of the unit with the dependence of the clicked unit/building.
 *  @param unit    Unit, that was clicked on. 
 *  @return The method returns true if selected action starts successfully.
 */
bool TMAP_UNIT::SelectReaction(TMAP_UNIT *unit, TUNIT_ACTION action) 
{ 
  bool ok = false;

  if (unit->TestState(US_GHOST)) unit = unit->GetGhostOwner();

  // attack mine
  switch (action) {
    case UA_ATTACK:
      ok = StartAttacking(unit, false);
      break;

    default: 
      break;
  }

  return ok;
}

/** 
 *  The method test whether unit is present in the segment interval of parameter.
 *  The unit is present in the interval if at least one part of the unit is in.
 *  The borders are included into the interval.
 *
 *  @param bottom The down border of the interval.
 *  @param top    The up border of the interval.
 *  @return The method returns true if unit is present in the interval.
 */
bool TMAP_UNIT::ExistInSegment(int bottom, int top) const
{
  if (static_cast<TMAP_ITEM*>(GetPointerToItem())->GetExistSegments().IsMember(bottom)
      || static_cast<TMAP_ITEM*>(GetPointerToItem())->GetExistSegments().IsMember(top))
    return true;
  else
    return false;
}


void TMAP_UNIT::TestVisibility()
{
  if (player == myself || show_all || myself_units) visible = true;
  else visible = myself->GetLocalMap()->GetAreaVisibility(pos, GetUnitWidth(), GetUnitHeight());
}



/**
 *  Counts distance of unit from the area given as parameters.Uses modified Pathfinder.
 *  @param area_position    position of down left corner of area
 *  @param area_width       width of area
 *  @param area_height      height of area
 *  @param max_cnt          maximal count of steps given from open to close set
 *  @return                 returns distance of the unit from the area
*/
int TMAP_UNIT::CountPathDistance(TPOSITION_3D area_pos, int area_width, int area_height,int max_cnt)
{ 
  TPOSITION_3D goal;

  for (int i = area_pos.x - this->GetPointerToItem()->GetWidth() +1 ; i <= area_pos.x + area_width -1; i++)
    for (int j= area_pos.y - this->GetPointerToItem()->GetHeight() +1 ; j <= area_pos.y + area_height -1 ;j++)
    {
          if (map.IsInMap(i,j,pos.segment))
          {
            if (area_pos == this->pos)
              return 0;
          }            
    }
  return -1;
}


/**
 *  The method sets units aggressivity mode.
 *  If given mode is not compatible, the highest possible mode is set.
 *  @param new_agg  The setted aggressivity.
 */
TAGGRESSIVITY_MODE TMAP_UNIT::SetAggressivity(TAGGRESSIVITY_MODE new_agg, bool test_is_being_build)
{
  TMAP_ITEM * pit = (TMAP_ITEM *)pitem;
  bool is_being_built = (test_is_being_build && ((TestItemType(IT_BUILDING) || TestItemType(IT_FACTORY)) && TestState(US_IS_BEING_BUILT)));

  switch (new_agg) {
    case AM_AGGRESSIVE:
    case AM_OFFENSIVE:
      if (pit->CanMove() && pit->CanAttack() && !is_being_built) {
        aggressivity = new_agg;
        break;
      }

    case AM_GUARDED:
      if (pit->CanAttack() && !is_being_built) {
        aggressivity = AM_GUARDED;
        break;
      }

    case AM_IGNORE:
      aggressivity = AM_IGNORE;
      break;
      
    default:
      aggressivity = AM_NONE;
      break;
  }

  return aggressivity;
}


//=========================================================================
// Class TPOOLED_LIST - method definitions
//=========================================================================


/** 
 *  Adds new node at the beginning of the list. 
 *
 *  @new_item Pointer to item which will be boxed into node and placed 
 *  at the beginning of the list.
 */
void TPOOLED_LIST::AddNode(TMAP_UNIT * const new_item)
{
  TNODE *aux_node;
  
  if (new_item == NULL)
    return;

  aux_node = pool->GetFromPool();
  aux_node->Clear(true);
  aux_node->SetNext(first);
  new_item->AcquirePointer();
  aux_node->SetUnit(new_item);
  first = aux_node;

  if (length == 0) 
    last = first;
  length++;
}


/** 
 *  Adds new node at the beginning of the list only if it isn't in list. 
 *
 *  @new_item Pointer to item which will be boxed into node and placed 
 *  at the beginning of the list if there is not yet.
 *
 *  @return The method returns true if unit wasn't in the list yet.
 */
bool TPOOLED_LIST::AddNonDupliciteNode(TMAP_UNIT * const new_item)
{
  TNODE *aux;
  bool exists = false;

  if (new_item == NULL)
    return false;

  for (aux = first; aux != NULL; aux = static_cast<TNODE*>(aux->GetNext()))
  {
    if (aux->IsSameUnit(new_item))
    {
      exists = true;
      break;
    }
  }
  
  if (!(exists)) 
    AddNode(new_item);

  return (!exists);
}

/** 
 *  Adds new node at the end of the list. 
 *
 *  @new_item Pointer to item which will be boxed into node and placed 
 *  at the end of the list.
 */
void TPOOLED_LIST::AddNodeToEnd(TMAP_UNIT * const new_item)
{
  TNODE *aux_node;
  
  if (new_item == NULL)
    return;

  aux_node = pool->GetFromPool();
  aux_node->Clear(true);
  aux_node->SetNext(NULL);
  new_item->AcquirePointer();
  aux_node->SetUnit(new_item);
  if (length == 0)
    first = last = aux_node;
  else
  {
    last->SetNext(aux_node);
    last = aux_node;
  }
  length++;
}


/** 
 *  Adds new node at the end of the list only if it isn't in list. 
 *
 *  @new_item Pointer to item which will be boxed into node and placed 
 *  at the end of the list if there is not yet.
 *
 *  @return The method returns true if unit wasn't in the list yet.
 */
bool TPOOLED_LIST::AddNonDupliciteNodeToEnd(TMAP_UNIT * const new_item)
{
  TNODE *aux;
  bool exists = false;

  if (new_item == NULL)
    return false;

  for (aux = first; aux != NULL; aux = static_cast<TNODE*>(aux->GetNext()))
  {
    if (aux->IsSameUnit(new_item))
    {
      exists = true;
      break;
    }
  }
  
  if (!(exists)) 
    AddNodeToEnd(new_item);

  return (!exists);
}


/** 
 *  The method removes node from the list. 
 *
 *  @delete_item Pointer to item which will is boxed into node and removed 
 *  from the list if there is.
 *
 *  @return @c True if deleted unit was in the list.
 */
bool TPOOLED_LIST::RemoveNode(TMAP_UNIT* delete_item)
{
  TNODE *node = first;
  TNODE *prev = NULL;

  while (node)
  {
    if (node->IsSameUnit(delete_item))
    {
      if (node == first) 
        first = static_cast<TNODE*>(node->GetNext());
      if (node == last)
        last = prev;
      if (prev)
        prev->SetNext(node->GetNext());

      node->GetUnit()->ReleasePointer();
      pool->PutToPool(node);
      length--;

      return true;
    }

    prev = node;
    node = static_cast<TNODE*>(node->GetNext());
  }

  return false;
}

/**
 *  Destructor clears the list.
 */
TPOOLED_LIST::~TPOOLED_LIST()
{
  TNODE *p_delete = first;

  while (p_delete)
  {
    first = static_cast<TNODE*>(p_delete->GetNext());
    p_delete->GetUnit()->ReleasePointer();
    pool->PutToPool(p_delete);
    p_delete = first;
  }
  first = NULL;
  last = NULL;
  length = 0;
  pool = NULL;
}


//=========================================================================
// Class TITERATOR_POOLED_LIST - method definitions
//=========================================================================


TITERATOR_POOLED_LIST* TPOOLED_LIST::GetIterator() const
{ 
  return NEW TITERATOR_POOLED_LIST(this);
}


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:
