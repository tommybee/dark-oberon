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
 *  @file doselection.cpp
 *
 *  Selection of units.
 *
 *  @author Peter Knut
 *
 *  @date 2003, 2004
 */

#include <stdlib.h>

#include "doselection.h"
#include "domouse.h"
#include "doengine.h"


//=========================================================================
// Global variables
//=========================================================================

TSELECTION *selection;       //!< Selection of units or one building.


//=========================================================================
// class TSTORED_SELECTION
//=========================================================================

/**
 *  Cleares all stored units.
 */
void TSTORED_SELECTION::Reset()
{
  if (!units) return;

  for (int i = 0; i < units_count; i++) if (units[i]) units[i]->SetGroupID(-1);

  delete[] units;
  units = NULL;

  units_count = active_units = 0;
}


void TSTORED_SELECTION::PrepareField(int count)
{
  Reset();

  // creates field
  units_count = active_units = count;
  units = NEW TMAP_UNIT *[units_count];
}


void TSTORED_SELECTION::DeleteUnit(TMAP_UNIT *unit)
{
  if (!active_units) return;

  for (int i = 0; i < units_count; i++) 
    if (units[i] == unit) units[i] = NULL;

  active_units--;
  if (!active_units) Reset();
}


//=========================================================================
// class TSELECTION
//=========================================================================


/**
 * Constructor.
 */
TSELECTION::TSELECTION(void)
{
  units = NULL; units_count = 0;
  is_my = false;
  can_move = can_attack = can_mine = can_build = can_repair = false;  
  builder_item = NULL;
  units_action = UA_NONE;
  aggressivity_mode = AM_NONE;
  timer = 0.0;

  // create mutex
  if ((mutex = glfwCreateMutex ()) == NULL) {
    Critical ("Could not create units mutex");
  }
}


/**
 * Destructor.
 */
TSELECTION::~TSELECTION()
{
  if (IsEmpty()) return;

  glfwLockMutex(mutex);

  TNODE_OF_UNITS_LIST *u = units;
  TNODE_OF_UNITS_LIST *u2;

  // delete all items of units list
  while (u) {
    u2 = u->next;
    delete u;
    u = u2;
  }

  glfwUnlockMutex(mutex);

  glfwDestroyMutex(mutex);
}


/**
 *  Selects one unit. All previous selected units will be deselected.
 *
 *  @param punit  Unit that will be selected.
 */
void TSELECTION::SelectUnit(TMAP_UNIT *punit)
{
  if (!punit) return;

  glfwLockMutex(mutex);

  _UnselectAll(false);
  _AddUnit(punit, true, false);

  glfwUnlockMutex(mutex);
}


/**
 *  If unit is not in selection adds unit into selection else deletes unit from selection.
 *
 *  @param punit  Unit that will be added / deleted.
 */
void TSELECTION::AddDeleteUnit(TMAP_UNIT *punit)
{
  if (!punit) return;

  if (punit->GetSelected()) DeleteUnit(punit);
  else AddUnit(punit, true);
}


/**
 *  Adds unit into selection.
 *
 *  @param punit  Unit that will be added / deleted.
 */
void TSELECTION::_AddUnit(TMAP_UNIT *punit, bool sound, bool lock)
{
  // unit is in selection
  if (punit->IsSelected() || !punit->IsVisible() || 
    punit->TestState(US_DYING) || punit->TestState(US_ZOMBIE) || punit->TestState(US_DELETE)
  ) return;
  
  if (lock) glfwLockMutex(mutex);

  Debug(LogMsg("X:%d, Y:%d, Z:%d, UID:%d, STATE:%d", punit->GetPosition().x, punit->GetPosition().y, punit->GetPosition().segment, punit->GetUnitID(), punit->GetState()));
  StartTimer();

  TMAP_ITEM* item = static_cast<TMAP_ITEM*>(punit->GetPointerToItem()); // pointer to unit item
  bool is_building = (punit->TestItemType(IT_BUILDING) || punit->TestItemType(IT_FACTORY));

  bool new_is_my = (punit->GetPlayer() == myself);

#if SOUND
  // play sound
  if (sound && config.snd_unit_speech && (new_is_my || punit->GetPlayer() == hyper_player)) {
    if ((is_building || punit->TestItemType(IT_SOURCE)) && item->tg_burning_id > -1) {
      if (punit->TestState(US_IS_BEING_BUILT))
        punit->PlaySound(&punit->GetPlayer()->race->snd_construction);

      else if (punit->GetLife() < item->GetMaxLife() * UNI_BURNING_COEF1) {
        if (item->snd_burning.IsEmpty())
          punit->PlaySound(&punit->GetPlayer()->race->snd_burning);
        else
          punit->PlaySound(&item->snd_burning);
      }

      else if (item->snd_selected.IsEmpty())
        punit->PlaySound(&punit->GetPlayer()->race->snd_building_selected);

      else
        punit->PlaySound(&item->snd_selected);
    }
    else {
      if (punit->GetLife() < item->GetMaxLife() * UNI_BURNING_COEF1 && !item->snd_burning.IsEmpty()) {
        punit->PlaySound(&item->snd_burning);
      }
      else
        punit->PlaySound(&item->snd_selected);
    }
  }
#endif

  // only one enemy unit could be selected;
  if (!new_is_my)  _UnselectAll(false);

  else {
    // if enemy unit is selected and I am trying to add my unit
    if (!IsEmpty() && !is_my) _UnselectAll(false);

    // only one not moveable unit could be selected
    else if (!IsEmpty() && !item->CanMove()) _UnselectAll(false);

    // not moveable unit will be always unselected
    else if (!IsEmpty() && !can_move) _UnselectAll(false);
  }

  // set can_move
  if (IsEmpty()) can_move = item->CanMove();
  else can_move = can_move && item->CanMove();

  // set possible actions
  can_attack = can_attack || item->CanAttack();
  can_mine = can_mine || item->CanMine();
  can_repair = can_repair || item->CanRepair();
  can_build = can_build || item->CanBuild();

  if (can_build) {
    builder_item = item;
  }
  else
    builder_item = NULL;

  // disable all actions when selected unit is building in proggress or it is an enemy unit
  if ((is_building && punit->TestState(US_IS_BEING_BUILT)) || !new_is_my) {
    can_move = can_attack = can_mine = can_repair = can_build = false;
  }

  // set units action and aggressivity
  if (!new_is_my) {
    units_action = UA_NONE;
    aggressivity_mode = AM_NONE;
  }
  else {
    if (IsEmpty()) {
      units_action = punit->GetAction();
      aggressivity_mode = punit->GetAggressivity();
    }
    else {
      if (units_action != punit->GetAction()) units_action = UA_NONE;
      if (aggressivity_mode != punit->GetAggressivity()) aggressivity_mode = AM_NONE;
    }
  }

  // add unit into list
  TNODE_OF_UNITS_LIST *u = NEW TNODE_OF_UNITS_LIST();

  u->unit = punit;
  punit->SetSelected(true);

  if (units) {
    u->next = units;
    units->prev = u;
  }

  units = u;
  units_count++;
  is_my = new_is_my;

  // update
  UpdateInfo(true, false);

  if (state == ST_GAME) {
    for (int i = 0; i < MNU_PANELS_COUNT; i++)
      panel_info.action_panel[i]->ResetSliders();
  }

  if (lock) glfwUnlockMutex(mutex);

  //Debug(LogMsg("unit position: %d, %d", punit->GetPosition().x, punit->GetPosition().y));
}


/**
 *  Deletes unit from selection.
 *
 *  @param punit  Unit that will be deleted.
 */
bool TSELECTION::DeleteUnit(TMAP_UNIT *punit)
{
  if (!punit->IsSelected()) return false;
  
  glfwLockMutex(mutex);

  TNODE_OF_UNITS_LIST *u;

  punit->SetSelected(false);

  // tries to find unit in selection
  for (u = units; u; u = u->next)
    if (u->unit == punit) break;

  // if unit is not found, return
  if (!u) {
    glfwUnlockMutex(mutex);
    return false;
  }

  // remove unit from list
  if (u->prev) u->prev->next = u->next;
  if (u->next) u->next->prev = u->prev;

  if (u == units) units = u->next;

  delete u;
  units_count--;

  // set moveability and is_my
  if (IsEmpty()) {
    can_move = false;
    is_my = false;

    myself->build_item = NULL;
    mouse.action = UA_NONE;
  }

  can_attack = can_mine = can_build = can_repair = false;
  builder_item = NULL;
  units_action = UA_NONE;

  // set actions and aggressivity mode
  if (!IsEmpty()) {
    TMAP_ITEM *item;

    units_action = units->unit->GetAction();
    aggressivity_mode = units->unit->GetAggressivity();

    for (u = units; u; u = u->next) {
      item = static_cast<TMAP_ITEM*>(u->unit->GetPointerToItem());

      can_attack = can_attack || item->CanAttack();
      can_mine = can_mine || item->CanMine();
      can_repair = can_repair || item->CanRepair();
      can_build = can_build || item->CanBuild();

      if (can_build) {
        if (!builder_item)
          builder_item = item;
      }
      else
        builder_item = NULL;

      if (units_action != u->unit->GetAction()) units_action = UA_NONE;
      if (aggressivity_mode != u->unit->GetAggressivity()) aggressivity_mode = AM_NONE;
    }
  }

  glfwUnlockMutex(mutex);

  UpdateInfo(true);
  return true;
}


/**
 *  Unselects all units or building.
 */
void TSELECTION::_UnselectAll(bool lock)
{
  if (IsEmpty()) return;

  if (lock) glfwLockMutex(mutex);

  TNODE_OF_UNITS_LIST *u = units;
  TNODE_OF_UNITS_LIST *u2;

  // delete all items of units list
  while (u) {
    u2 = u->next;
    u->unit->SetSelected(false);
    delete u;
    u = u2;
  }

  // reset variables
  units = NULL;
  units_count = 0;
  can_move = can_attack = can_mine = can_build = can_repair = false;
  builder_item = NULL;
  units_action = UA_NONE;

  if (lock) glfwUnlockMutex(mutex);

  UpdateInfo(true, lock);
}


/**
 *  Updates panel information.
 */
void TSELECTION::Update(double time_shift)
{
  char txt[100];
  TUNIT_ACTION last_action;

  if (timer) {
    timer -= time_shift;
    if (timer < 0) timer = 0.0;
  }

  glfwLockMutex(mutex);

  // dynamic unit info
  if (state == ST_GAME && !IsEmpty() && !units->next) {
    TMAP_ITEM *item = static_cast<TMAP_ITEM *>(units->unit->GetPointerToItem());

    // life
    sprintf(txt, "%d/%d", (int)units->unit->GetLife(), item->GetMaxLife());
    panel_info.life_label->SetCaption(txt);

    // hided units
    if (IsMy() && GetFirstUnit()->AcceptsHidedUnits()) {
      TMAP_ITEM *it = static_cast<TMAP_ITEM *>(GetFirstUnit()->GetPointerToItem()); 

      sprintf(txt, "%d/%d", GetFirstUnit()->GetHidedCount(), it->GetMaxHidedUnits());
      panel_info.hided_label->SetCaption(txt);
    }
    else panel_info.hided_label->SetCaption(NULL);

    // progress
    switch (GetFirstUnit()->GetItemType()) {
    case IT_SOURCE:
      {
        TSOURCE_UNIT *u = static_cast<TSOURCE_UNIT *>(units->unit);

        sprintf(txt, "Material Left: %d", u->GetMaterialBalance());      
        panel_info.progress_label->SetCaption(txt);
      }
      break;

    case IT_FACTORY:
      if (IsMy()) {
        TFACTORY_UNIT *b = static_cast<TFACTORY_UNIT*>(units->unit);

        if (b->GetProgress() || b->IsPaused()) {
          if (b->IsPaused())
            sprintf(txt, "Progress: %d%% Paused", b->GetProgress());
          else
            sprintf(txt, "Progress: %d%%", b->GetProgress());

          panel_info.progress_label->SetCaption(txt);
        }
        else panel_info.progress_label->SetCaption(NULL);
      }
      break;

    case IT_BUILDING:
      if (IsMy()) {
        TBUILDING_UNIT *b = static_cast<TBUILDING_UNIT*>(units->unit);

        if (b->GetProgress()) {
          sprintf(txt, "Progress: %d%%", b->GetProgress());
          panel_info.progress_label->SetCaption(txt);
        }
        else panel_info.progress_label->SetCaption(NULL);
      }
      break;

    case IT_WORKER:
      if (IsMy()) {
        TWORKER_UNIT *w = static_cast<TWORKER_UNIT*>(units->unit);

        if (w->GetMaterialAmount() > 0) {
          sprintf(txt, "Carrying: %.0f", w->GetMaterialAmount());
          panel_info.progress_label->SetCaption(txt);
        }
        else panel_info.progress_label->SetCaption(NULL);
      }
      break;

    default:
      break;
    }
  }

  // upadte units action
  last_action = units_action;
  if (IsEmpty() || !IsMy()) units_action = UA_NONE;
  else {
    TNODE_OF_UNITS_LIST *u;
    units_action = units->unit->GetAction();

    for (u = units; u; u = u->next)
      if (units_action != u->unit->GetAction()) {
        units_action = UA_NONE;
        break;
      }
  }

  glfwUnlockMutex(mutex);

  if (last_action != units_action && mouse.action == UA_NONE && !panel_info.build_button->IsChecked()) UpdateInfo(true);
}


/**
 *  Updates panel information depends on selected units.
 */
void TSELECTION::UpdateInfo(bool update_action, bool lock)
{
  if (state != ST_GAME) return;

  char txt[1024];
  *txt = 0;
  int i;
  GLfloat y = GLfloat(config.scr_height - 315);
  GLfloat lh = panel_info.info_label->GetLineHeight();

  if (lock) glfwLockMutex(mutex);

  for (i = 0; i < scheme.materials_count; i++) {
    panel_info.material_image[i]->Hide();
  }
  SetOrderVisibility(false);
  ChangeActionPanel(MNU_PANEL_STAY);
  
  if (IsEmpty()) {
    panel_info.name_label->SetCaption(NULL);
    panel_info.life_label->SetCaption(NULL);
    panel_info.hided_label->SetCaption(NULL);
    panel_info.progress_label->SetCaption(NULL);
    
  }
  else {
    if (units->next) {
      panel_info.name_label->SetCaption("Multi select");
      panel_info.life_label->SetCaption(NULL);
      panel_info.hided_label->SetCaption(NULL);
      panel_info.progress_label->SetCaption(NULL);

      if (can_build) {
        CreateBuildButtons();
        ChangeActionPanel(MNU_PANEL_BUILD);
      }
    }
    else {
      TMAP_ITEM *item = static_cast<TMAP_ITEM *>(GetFirstUnit()->GetPointerToItem());
      panel_info.name_label->SetCaption(item->name);
      panel_info.progress_label->SetCaption(NULL);

      if (IsMy() && GetFirstUnit()->AcceptsHidedUnits()) {
        TMAP_ITEM *it = static_cast<TMAP_ITEM *>(GetFirstUnit()->GetPointerToItem()); 

        sprintf(txt, "%d/%d", GetFirstUnit()->GetHidedCount(), it->GetMaxHidedUnits());
        panel_info.hided_label->SetCaption(txt);
      }
      else panel_info.hided_label->SetCaption(NULL);

      // static unit info
      switch (GetFirstUnit()->GetItemType()) {
      case IT_FORCE: {
          TFORCE_ITEM *it = static_cast<TFORCE_ITEM *>(units->unit->GetPointerToItem()); 
          char speed_txt[DAT_SEGMENTS_COUNT][10];
          char offensive_txt[50] = "";

          if (it->GetArmament()->GetOffensive()) {
            sprintf(offensive_txt, "    Damage: %d-%d/%d\n     Range: %d-%d:%d%%\n", 
              it->GetArmament()->GetOffensive()->GetPower().min,
              it->GetArmament()->GetOffensive()->GetPower().max,
              it->GetArmament()->GetOffensive()->GetProjectileItem()->GetScope(),
              it->GetArmament()->GetOffensive()->GetRange().min,
              it->GetArmament()->GetOffensive()->GetRange().max,
              (int)(it->GetArmament()->GetOffensive()->GetAccuracy()*100)
            );
          }
          else y += lh;

          for (i = 0; i < DAT_SEGMENTS_COUNT; i++) sprintf(speed_txt[i], "%.0f", it->max_speed[i]);

          sprintf(txt, "     Armor: %d:%d%%\n%s      View: %d\nMax speeds: %s/%s/%s\n Food/Ener: %d/%d",
            it->GetArmament()->GetDefense()->GetArmour(),
            int(it->GetArmament()->GetDefense()->GetProtection() * 100),
            offensive_txt,
            it->view,
            it->GetExistSegments().IsMember(0) ? speed_txt[0] : "-",
            it->GetExistSegments().IsMember(1) ? speed_txt[1] : "-",
            it->GetExistSegments().IsMember(3) ? speed_txt[2] : "-",
            it->food, it->energy
          );

          panel_info.info_label->SetPosY(y);
        }
        break;

      case IT_WORKER: {
          TWORKER_ITEM *it = static_cast<TWORKER_ITEM *>(units->unit->GetPointerToItem());
          char speed_txt[DAT_SEGMENTS_COUNT][10];
          char offensive_txt[50] = "";
          y -= lh;

          if (IsMy()) {
            CreateBuildButtons();
            ChangeActionPanel(MNU_PANEL_BUILD);
          }

          if (it->GetArmament()->GetOffensive()) {
            sprintf(offensive_txt, "    Damage: %d-%d/%d\n     Range: %d-%d:%d%%\n", 
              it->GetArmament()->GetOffensive()->GetPower().min,
              it->GetArmament()->GetOffensive()->GetPower().max,
              it->GetArmament()->GetOffensive()->GetProjectileItem()->GetScope(),
              it->GetArmament()->GetOffensive()->GetRange().min,
              it->GetArmament()->GetOffensive()->GetRange().max,
              (int)(it->GetArmament()->GetOffensive()->GetAccuracy()*100)
            );
          }
          else y += lh;

          for (i = 0; i < DAT_SEGMENTS_COUNT; i++) sprintf(speed_txt[i], "%.0f", it->max_speed[i]);

          sprintf(txt, "     Armor: %d:%d%%\n%s      View: %d\nMax speeds: %s/%s/%s\n Food/Ener: %d/%d\n Materials: ",
            it->GetArmament()->GetDefense()->GetArmour(),
            int(it->GetArmament()->GetDefense()->GetProtection() * 100),
            offensive_txt,
            it->view,
            it->GetExistSegments().IsMember(0) ? speed_txt[0] : "-",
            it->GetExistSegments().IsMember(1) ? speed_txt[1] : "-",
            it->GetExistSegments().IsMember(3) ? speed_txt[2] : "-",
            it->food, it->energy
          );

          for (i = 0; i < scheme.materials_count; i++) {
            panel_info.material_image[i]->SetVisible(it->GetAllowedMaterial(i));

            if (panel_info.material_image[i]) panel_info.material_image[i]->SetPos(GLfloat(99 + i * 20), y + 1);
          }
          
          if (!it->AllowAnyMaterial()) strcat(txt, "---");
          
          panel_info.info_label->SetPosY(y);
        }
        break;

      case IT_FACTORY:
        if (IsMy()) {
          SetOrderVisibility(true);
          CreateOrderButtons();

          if (!GetFirstUnit()->TestState(US_IS_BEING_BUILT)) {
            CreateBuildButtons();
            ChangeActionPanel(MNU_PANEL_BUILD);
          }
        }
        // break; tu nema by

      case IT_BUILDING: {
          TBUILDING_ITEM *it = static_cast<TBUILDING_ITEM *>(units->unit->GetPointerToItem());
          char offensive_txt[50] = "";

          if (it->GetArmament()->GetOffensive()) {
            sprintf(offensive_txt, "    Damage: %d-%d/%d\n     Range: %d-%d:%d%%\n", 
              it->GetArmament()->GetOffensive()->GetPower().min,
              it->GetArmament()->GetOffensive()->GetPower().max,
              it->GetArmament()->GetOffensive()->GetProjectileItem()->GetScope(),
              it->GetArmament()->GetOffensive()->GetRange().min,
              it->GetArmament()->GetOffensive()->GetRange().max,
              (int)(it->GetArmament()->GetOffensive()->GetAccuracy()*100)
            );
          }
          else y += lh;

          sprintf(txt, "     Armor: %d:%d%%\n%s      View: %d\n Food/Ener: %d/%d", 
            it->GetArmament()->GetDefense()->GetArmour(),
            int(it->GetArmament()->GetDefense()->GetProtection() * 100),
            offensive_txt,
            it->view,
            it->food, it->energy
          );

          if (it->AllowAnyMaterial()) {
            strcat(txt, "\n Materials: ");

            for (i = 0; i < scheme.materials_count; i++) {
              panel_info.material_image[i]->SetVisible(it->GetAllowedMaterial(i));
              if (panel_info.material_image[i]) panel_info.material_image[i]->SetPos(GLfloat(99 + i * 20), y + 1);
            }
          }

          panel_info.info_label->SetPosY(y);
        }
        break;

      case IT_SOURCE: {
          TSOURCE_ITEM *it = static_cast<TSOURCE_ITEM *>(units->unit->GetPointerToItem());
          y += lh;

          sprintf(txt, "     Armor: %d:%d%%\n  Material:\n  Capacity: %d",
            it->GetArmament()->GetDefense()->GetArmour(),
            int(it->GetArmament()->GetDefense()->GetProtection() * 100),
            it->GetCapacity()
          );

          panel_info.material_image[it->GetOfferMaterial()]->Show();
          if (panel_info.material_image[it->GetOfferMaterial()]) panel_info.material_image[it->GetOfferMaterial()]->SetPos(99, y + lh + 1);
          
          panel_info.info_label->SetPosY(y);
        }

        break;

      default:
        break;
      }
    }
  }

  panel_info.info_label->SetCaption(txt);

  // picture image
  if (!IsEmpty()) {
    int tg = static_cast<TMAP_ITEM *>(units->unit->GetPointerToItem())->tg_picture_id;

    if (tg >= 0) {
      panel_info.picture_image->SetTexture(units->unit->GetPlayer()->race->tex_table.GetTexture(tg, 0));
      panel_info.picture_image->Show();
    }
    else panel_info.picture_image->Hide();
  }
  else panel_info.picture_image->Hide();

  // action buttons
  panel_info.stay_button->SetEnabled(!IsEmpty() && IsMy());
  panel_info.move_button->SetEnabled(can_move);
  panel_info.attack_button->SetEnabled(can_attack);
  panel_info.mine_button->SetEnabled(can_mine);
  panel_info.repair_button->SetEnabled(can_repair);
  panel_info.build_button->SetEnabled(can_build);

  if (lock) glfwUnlockMutex(mutex);

  if (update_action) UpdateAction(lock);
}


/**
 *  Updates action information depends on selected units.
 */
void TSELECTION::UpdateAction(bool lock)
{
  if (state != ST_GAME) return;

  if (lock) glfwLockMutex(mutex);

  switch (units_action) {
  case UA_NONE:
    panel_info.stay_button->SetChecked(true);
    panel_info.stay_button->SetChecked(false);

    if (panel_info.stay_button->IsEnabled()) {
      UpdateGuardButtons();
      CheckGuardButton(aggressivity_mode);
    }
    else
      ChangeActionPanel(MNU_PANEL_EMPTY);
    break;

  case UA_STAY:
    panel_info.stay_button->SetChecked(true);
    UpdateGuardButtons();
    CheckGuardButton(aggressivity_mode);
    break;

  case UA_MOVE:   panel_info.move_button->SetChecked(true); break;
  case UA_ATTACK: panel_info.attack_button->SetChecked(true); break;
  case UA_MINE:   panel_info.mine_button->SetChecked(true); break;
  case UA_REPAIR: panel_info.repair_button->SetChecked(true); break;
  default:
    break;
  }

  if (lock) glfwUnlockMutex(mutex);
}


void TSELECTION::DrawUnitsLines()
{
  // move lines
  if (!CanDrawLines() || !can_move) return;

  glfwLockMutex(mutex);

  TNODE_OF_UNITS_LIST *n;

  for (n = units; n; n = n->next) {
    static_cast<TFORCE_UNIT *>(n->unit)->DrawLine();
  }

  glfwUnlockMutex(mutex);
}


bool TSELECTION::TestCanHide(TMAP_UNIT *over_unit)
{
  bool ok = true;

  glfwLockMutex(mutex);

  if (
    !over_unit || !can_move || 
    (OnlyOne() && GetFirstUnit() == over_unit) ||
    over_unit->GetPlayer() != myself ||
    !over_unit->AcceptsHidedUnits() || over_unit->IsFull()
  ) ok = false;

  if (ok) {
    TNODE_OF_UNITS_LIST *n;
    ok = false;

    for (n = units; n && !ok; n = n->next) {
      if (static_cast<TFORCE_UNIT *>(n->unit)->CanHide(over_unit, false, false)) ok = true;
    }
  }

  glfwUnlockMutex(mutex);

  return ok;
}


bool TSELECTION::TestCanAttack(TMAP_UNIT *over_unit)
{
  bool ok = true;

  glfwLockMutex(mutex);

  if (
    !over_unit || !can_attack || over_unit->IsGhost() ||
    (OnlyOne() && GetFirstUnit() == over_unit)
  ) ok = false;

  glfwUnlockMutex(mutex);

  return ok;
}


bool TSELECTION::TestCanMine(TSOURCE_UNIT *over_unit)
{
  bool ok = true;

  glfwLockMutex(mutex);

  if (
    !over_unit || !can_mine || 
    (OnlyOne() && GetFirstUnit() == over_unit) ||
    over_unit->IsEmpty()
  ) ok = false;

  if (ok) {
    TNODE_OF_UNITS_LIST *n;
    ok = false;

    for (n = units; n && !ok; n = n->next) {
      if (
        n->unit->TestItemType(IT_WORKER) &&
        static_cast<TWORKER_UNIT *>(n->unit)->CanMine(over_unit, false, false)
      ) ok = true;
    }
  }

  glfwUnlockMutex(mutex);

  return ok;
}


bool TSELECTION::TestCanUnload(TBUILDING_UNIT *over_unit)
{
  bool ok = true;

  glfwLockMutex(mutex);

  if (
    !over_unit || !can_mine ||
    over_unit->GetPlayer() != myself ||
    (OnlyOne() && GetFirstUnit() == over_unit) ||
    !over_unit->AllowAnyMaterial()
  ) ok = false;

  if (ok) {
    TNODE_OF_UNITS_LIST *n;
    ok = false;

    for (n = units; n && !ok; n = n->next) {
      if (
        n->unit->TestItemType(IT_WORKER) &&
        static_cast<TWORKER_UNIT *>(n->unit)->CanUnload(over_unit, false, false)
      ) ok = true;
    }
  }

  glfwUnlockMutex(mutex);

  return ok;
}


bool TSELECTION::TestCanBuildOrRepair(TBASIC_UNIT *over_unit)
{
  bool ok = true;

  glfwLockMutex(mutex);

  if (
    !over_unit || !can_repair ||
    over_unit->GetPlayer() != myself ||
    (OnlyOne() && GetFirstUnit() == over_unit)
  ) ok = false;

  if (ok) {
    TNODE_OF_UNITS_LIST *n;
    ok = false;

    for (n = units; n && !ok; n = n->next) {
      if (
        n->unit->TestItemType(IT_WORKER) &&
        static_cast<TWORKER_UNIT *>(n->unit)->CanBuildOrRepair(over_unit, false, false)
      ) ok = true;
    }
  }

  glfwUnlockMutex(mutex);

  return ok;
}


bool TSELECTION::TestCanBuild(TBUILDING_ITEM *building, TPOSITION pos, bool **build_map)
{
  bool ok = true;

  glfwLockMutex(mutex);

  if (
    !can_build || !building
  ) ok = false;

  if (ok) ok = static_cast<TWORKER_UNIT *>(units->unit)->CanBuild(building, pos, build_map, false, false);

  glfwUnlockMutex(mutex);

  return ok;
}


/**
 *  Stop all units in selection.
 */
void TSELECTION::StopUnits()
{
  glfwLockMutex(mutex);

  TNODE_OF_UNITS_LIST *ul;      // actual item in units list

  bool stopped = false;

  for (ul = units; ul; ul = ul->next)
    if (ul->unit->StartStaying())
      stopped = true;

#if SOUND
  TMAP_UNIT *u = units->unit;

  // play command sound
  if (stopped && config.snd_unit_speech && (u->TestItemType(IT_FORCE) || u->TestItemType(IT_WORKER)))
  {
    u->PlaySound(&static_cast<TFORCE_ITEM *>(u->GetPointerToItem())->snd_command);
  }
#endif


  glfwUnlockMutex(mutex);
}


/**
 *  React with selected units.
 */
bool TSELECTION::ReactUnits()
{
  TNODE_OF_UNITS_LIST *ul;      // actual item in units list
  TUNIT_ACTION action;

  bool ok = false;

  switch (mouse.cursor_id) 
  {
    case MC_CAN_HIDE:   action = UA_HIDE;   break;
    case MC_CAN_ATTACK: action = UA_ATTACK; break;
    case MC_CAN_MINE:   action = UA_MINE;   break;
    case MC_CAN_REPAIR: action = UA_REPAIR; break;
    default: return false;                  break;
  }

  glfwLockMutex(mutex);

  for (ul = units; ul; ul = ul->next)
    if (ul->unit != mouse.over_unit && ul->unit->SelectReaction(mouse.over_unit, action)) ok = true;

  if (ok) {
    UpdateInfo(true, false);
    StartTimer();

#if SOUND
    // play command sound
    if (config.snd_unit_speech &&
      (GetFirstUnit()->TestItemType(IT_FORCE) || GetFirstUnit()->TestItemType(IT_WORKER))
    ) {
      GetFirstUnit()->PlaySound(&static_cast<TFORCE_ITEM *>(GetFirstUnit()->GetPointerToItem())->snd_command);
    }
#endif
  }

  glfwUnlockMutex(mutex);

  return ok;
}


/*
  Method MoveUnits with all of its submethods implements moving of groups of units. 
  It runs: 1.one thread, which devides selected units to smaller groups.
           2.threads, which for every group find the leader and the path for every one unit of the group.
  Method uses threads and nodes of lists from pool, it doesnt allocate dynamicaly any memmory.
*/
bool TSELECTION::MoveUnits(TPOSITION goal)
{
  process_mutex->Lock();
  glfwLockMutex(mutex);

  if (!TestCanMove()) 
  {
    glfwUnlockMutex(mutex);
    process_mutex->Unlock();
    return false;
  }

  if (!map.IsInMap(goal.x, goal.y)) 
  {
    GetFirstUnit()->MessageText(false, const_cast<char*>("Can not move outside of map."));
    glfwUnlockMutex(mutex);
    process_mutex->Unlock();
    return false;
  }

#if SOUND
  // play command sound
  if (config.snd_unit_speech) 
  {
    GetFirstUnit()->PlaySound(&static_cast<TFORCE_ITEM *>(GetFirstUnit()->GetPointerToItem())->snd_command);
  }
#endif

  UpdateInfo(true, false);
  StartTimer();
  
  TPATH_INFO * path_info = NULL;       //global information for whole selected groups of units
  TFORCE_UNIT * fu = NULL;
  TSEL_NODE *new_node = NULL;
  TNODE_OF_UNITS_LIST *node = NULL;
  double time_stamp = glfwGetTime();

  //fill all the information needed for whole group
  path_info = pool_path_info->GetFromPool(); 

  if (!path_info)
  {
    glfwUnlockMutex(mutex);
    process_mutex->Unlock();
    return false;
  }

  path_info->goal.x  = goal.x;  
  path_info->goal.y  = goal.y;
  path_info->real_goal.x = goal.x;
  path_info->real_goal.y = goal.y;
  path_info->unit_list = NULL;

  if (units && units->unit->GetPlayer() == myself)
    path_info->loc_map = units->unit->GetPlayer()->GetLocalMap();  
  else
  {
    pool_path_info->PutToPool(path_info);    
    glfwUnlockMutex(mutex);
    process_mutex->Unlock();
    return false;
  }

  //copy the list of selected units to path_info
  for (node = units; node; node = node->next)
  {
    new_node = pool_sel_node->GetFromPool();

    if (!new_node)
    {
      //dealocate memmory and returns node to pool
      TSEL_NODE *n = path_info->unit_list;
      TSEL_NODE *del = NULL;
      while (n)
      {
        del = n->next;
        pool_sel_node->PutToPool(n);
        n = del; 
      }
      path_info->unit_list = NULL;
      pool_path_info->PutToPool(path_info);

      glfwUnlockMutex(mutex);
      process_mutex->Unlock();
      return false;
    }

    new_node->next = NULL;
    new_node->prev = NULL;

    glfwLockMutex(delete_mutex);
    new_node->unit = (TFORCE_UNIT *)node->unit->AcquirePointer();
    glfwUnlockMutex(delete_mutex);

    if (!new_node->unit) {
      pool_path_info->PutToPool(path_info);
      continue;
    }

    fu = new_node->unit;
    
    //insert to list
    if (!path_info->unit_list)
    {
      path_info->unit_list = new_node;
      //jednotke, na ktoru ukazuje krabicka posli event, zapamataj si request id pre ostatne jednotky
      
      fu->SendEvent(false, time_stamp, US_WAIT_FOR_PATH, 0);

      path_info->request_id  = fu->pevent->GetRequestID();
      path_info->event_type = ET_GROUP_MOVING;
    }
    else
    {
      path_info->unit_list->prev = new_node;   //zoznam ma jednotky v obratenom poradi ako zoznam TFORCE_UNIT
      new_node->next  = path_info->unit_list;
      path_info->unit_list = new_node;

      fu->SendEvent(false, time_stamp, US_WAIT_FOR_PATH, path_info->request_id);
    }

    //posli jednotke event, ze caka na najdenie cesty
    node->unit->SetWaitRequestId(path_info->request_id);    //waited request id
  }
  
  //v path_info je nastaveny requestId, takze sa message moze poslat

  threadpool_astar->AddRequest(path_info, &TA_STAR_ALG::DevideToGroups);

  //fcia group management rozdeli skupinu do mensich skupin a pre kazdu skupinu najde leadra.
  glfwUnlockMutex(mutex);
  process_mutex->Unlock();

  return false;
}


void TSELECTION::SetAggressivity(TAGGRESSIVITY_MODE mode)
{
  if (IsEmpty()) return;

  TNODE_OF_UNITS_LIST *node;

  // set aggressivity to all units in selection
  for (node = GetUnitsList(); node; node = node->next)
    node->unit->SetAggressivity(mode, true);
  
  // compute new agressivity of selection
  aggressivity_mode = units->unit->GetAggressivity();

  for (node = GetUnitsList(); node; node = node->next) {
    if (node->unit->GetAggressivity() != aggressivity_mode) {
      aggressivity_mode = AM_NONE;
      break;
    }
  }
}


void TSELECTION::StoreSelection(int gid)
{
  TNODE_OF_UNITS_LIST *u;
  int i;

  glfwLockMutex(mutex);

  if (!IsMy()) {
    glfwUnlockMutex(mutex);
    return;
  }

  groups[gid].PrepareField(units_count);

  for (u = units, i = 0; u; u = u->next, i++) {
    groups[gid].units[i] = u->unit;
    if (u->unit->GetGroupID() >= 0) groups[u->unit->GetGroupID()].DeleteUnit(u->unit);
    u->unit->SetGroupID(gid);
  }

  glfwUnlockMutex(mutex);
}


void TSELECTION::RestoreSelection(int gid, bool center, bool sound)
{
  if (gid < 0 || gid > PL_MAX_SELECTIONS || !groups[gid].active_units) {
    char txt[50];

    sprintf(txt, "Undefined selection #%d", gid + 1);
    ost->AddText(txt);
    return;
  }

  int i;
  TMAP_UNIT *u = NULL;

  glfwLockMutex(mutex);

  _UnselectAll(false);

  for (i = 0; i < groups[gid].units_count; i++) 
    if (groups[gid].units[i] && groups[gid].units[i]->IsInMap()) {
      _AddUnit(groups[gid].units[i], sound, false);
      sound = false;
      if (!u) u = groups[gid].units[i];
    }

  // move map to position of first unit
  if (center && u)
    map.CenterMapel(u->GetPosition().x, u->GetPosition().y);

  // reset drawing selection rectangle
  mouse.draw_selection = false;

  glfwUnlockMutex(mutex);
}


void TSELECTION::DeleteStoredUnit(int gid, TMAP_UNIT *unit)
{
  glfwLockMutex(mutex);

  groups[gid].DeleteUnit(unit);

  glfwUnlockMutex(mutex);
}


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

