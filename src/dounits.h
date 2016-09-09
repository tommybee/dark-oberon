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
 *  @file dounits.h
 *
 *  Working with units.
 *
 *  @author Martin Kosalko
 *
 *  @date 2002, 2003
 */

#ifndef __dounits_h__
#define __dounits_h__

//=========================================================================
// Forward declarations
//=========================================================================

class TDRAW_UNIT;
class TPLAYER_UNIT;
class TMAP_UNIT;
class TPROJECTILE_UNIT;
class TBASIC_UNIT;
class TFORCE_UNIT;
class TBUILDING_UNIT;
class TWORKER_UNIT;
struct TNEAREST_BUILDINGS;
class TSOURCE_UNIT;
class TFACTORY_UNIT;
class TPATH_INFO;
class TMAP_POOLED_LIST;


//=========================================================================
// Definitions
//=========================================================================

// unit states
#define US_NONE               0     //!< State "no state".
#define US_STAY               1     //!< State of unit whether is staying on a place.
#define US_ANCHORING          2     //!< State of anchoring unit.
#define US_LANDING            3     //!< State of landing unit.
#define US_UNLANDING          4     //!< State of unlanding unit.
#define US_MOVE               5     //!< State of moving unit between two steps of the path.
#define US_NEXT_STEP          6     //!< State of moving unit before next step of the path.
#define US_TRY_TO_MOVE        7     //!< State of unit when is temporaly blocked during moving.
#define US_ATTACKING          8     //!< State of unit whether is attacking other one.
#define US_NEXT_ATTACK        9     //!< State of unit when is charging a gun.
#define US_MINING             10    //!< State of worker unit, while he mines material in the source.
#define US_UNLOADING          11    //!< State of worker unit means that unit is unloading material in the building. 
#define US_CONSTRUCTING       12    //!< State of worker unit when constructs building.
#define US_NEXT_CONSTRUCTING  13    //!< State of worker unit when constructs building.
#define US_REPAIRING          14    //!< State of worker unit when repairs other unit.
#define US_NEXT_REPAIRING     15    //!< State of worker unit when repairs other unit.
#define US_REGENERATING       16    //!< State of source when regenerate.
#define US_IS_BEING_BUILT     18    //!< State of building when it is being built by worker.
#define US_LEFT_ROTATING      19    //!< State of the unit when is changing the direction. The unit is using left rotation.
#define US_RIGHT_ROTATING     20    //!< State of the unit when is changing the direction. The unit is using right rotation.
#define US_GHOST              21    //!< State of the ghost unit. (in farfog)
#define US_DYING              22    //!< State of the unit which is actually dying. (falling on the earth)
#define US_ZOMBIE             23    //!< State of the zombie unit. (dead unit laying on the earth)
#define US_DELETE             24    //!< State of the unit which deleted.
#define US_NEXT_MINE          25    //!< State of worker unit when next action will be testing if can mine in the source.
#define US_NEXT_UNLOADING     26    //!< State of worker unit, when next action will be US_UNLOADING.
#define US_HEALING            27    //!< State of the unit which is healing self.
#define US_NEXT_HIDING        28    //!< State of the unit which is tries to hide to another unit.
#define US_HIDING             29    //!< State of the unit which is hided in another unit.
#define US_START_MINE         30    //!< State of the unit which is staring mining.
#define US_START_REPAIR       31    //!< State of the unit which is staring repairing.
#define US_START_UNLOAD       32    //!< State of the unit which is staring unloading.
#define US_START_HIDING       33    //!< State of the unit which is staring hiding.
#define US_WAIT_FOR_PATH      34    //!< State of unit when waits for path.
#define US_START_ATTACK       35    //!< State of unit when starts attacking.
#define US_SEARCHING_NEAREST  36    //!< State of unit when searching nearest building.
#define US_EJECTING           37    //!< State of unit which is "unhiding" from another unit.
#define US_END_ATTACK         38    //!< State of unit which end attacking.
#define US_FEEDING            39    //!< State of unit which is reloading gun.

// events requests to units
#define RQ_FIRST              1000
#define RQ_CAN_MINE           1000  //!< Request to source if it is possible to mine.
#define RQ_MINE_SOUND         1001  //!< Request for playing mine sound.
#define RQ_PRODUCING          1002  //!< Request for produce unit.
#define RQ_TRY_TO_LEAVE       1003  //!< Request of factory to leave all ready units.
#define RQ_CHANGE_SEGMENT     1004  //!< Request asks shot to change segment.
#define RQ_FIRE_OFF           1005  //!< Request asks to fire off the shot.
#define RQ_PATH_FINDING       1006  //!< Request: path counted 
#define RQ_IMPACT             1007  //!< Request asks the projectile to impact.
#define RQ_NEAREST_SEARCHING  1008  //!< Request: nearest building found
#define RQ_GROUP_MOVING       1009  //!< Request: group of units moved.  
#define RQ_SYNC_LIFE          1010  //!< Request for synchronising life of unit.
#define RQ_SYNC_MAT_AMOUNT    1011  //!< Request for synchronising amout of material (in source).
#define RQ_SYNC_PROGRESS      1012  //!< Request for synchronising progress (in buildings).
#define RQ_END_MINE           1013  //!< Request for not local players that unit end mining. (it is necesary to remove worker from source list)
#define RQ_END_UNLOAD         1014  //!< Request for not local players that unit end unloading. (it is necesary to remove worker from building list)
#define RQ_CREATE_UNIT        1015  //!< Request for creation unit on remote computer.
#define RQ_UPGRADE_UNIT       1016  //!< Request for upgrade unit on remote computer.
#define RQ_CREATE_PROJECTILE  1017  //!< Request for creation projectile on remote computers.
#define RQ_DYING              1018  //!< State of the unit which is actually dying on remote computer. (falling on the earth)
#define RQ_ZOMBIE             1019  //!< State of the zombie unit on remote computer. (dead unit laying on the earth)
#define RQ_DELETE             1020  //!< State of the unit which is deleted on remote computer.
#define RQ_FEEDING            1021  //!< State of unit which is reloading gun.


// draw style
#define DS_NORMAL             0     //!< Normal draw style.
#define DS_UNDERGROUND        1     //!< Draw style for underground units.
#define DS_BUILDING           2     //!< Draw style for buildings before building process.


// units
#define UNI_TRY_TO_MOVE_LIMIT   4.0       //!< Time limit for trying to move without punish. [seconds]
#define UNI_TRY_TO_STAY_LIMIT   1.0       //!< Time limit for trying to stay (unit trys to leave another unit). [seconds]
#define UNI_TRY_TO_MOVE_SHIFT   0.2       //!< Timeshift between 2 tryings to move. [seconds]
#define UNI_TRY_TO_LEAVE_SHIFT  1.0       //!< Timeshift between 2 tryings to leave a unit and addings to map.
#define UNI_TRY_TO_PRODUCE_SHIFT  5.0     //!< Timeshift between 2 tryings to produce a unit.
#define UNI_CHECK_TARGET        2         //!< Count of steps in path when is checked if position of target has changed.
#define UNI_CHECK_HIDER         5         //!< Count of steps in path when is checked if position of hider has changed.
#define UNI_CHECK_BUILT_OR_REPAIRED_UNIT 5  //!< Count of steps in path when is checked if position of built_or_repaired_unit has changed.

#define UNI_MAX_MESSAGE_LENGTH  100       //<! Maximum length of unit mesaage text.
#define UNI_MAX_ORDER_LENGTH    8         //!< Maximum count of the ordered units in the factory.
#define UNI_PRODUCING_COUNT     100       //!< Count of RQ_PRODUCING requests needed for produce one unit.
#define UNI_LIFE_LIMIT1         0.7f      //!< Unit life limit for drawing destruction.
#define UNI_LIFE_LIMIT2         0.3f      //!< Unit life limit for drawing destruction.
#define UNI_LIFE_BAR_SIZE       6.0f      //!< Size of life bar.
#define UNI_LIFE_BAR_SIZE_2     (UNI_LIFE_BAR_SIZE / 2)   //!< Half size of life bar.

#define UNI_BURNING_COEF1       UNI_LIFE_LIMIT1
#define UNI_BURNING_COEF2       0.5f

#define UNI_ZOMBIE_TIME         5.0

//searching nearest building
#define WRK_MAX_PATH_TIME       99999999      //!< Maximal time that can take worker's path from point A to B   

//how many percents of left material after will be return to player after building was deleted
#define RET_MAT_PERCENTAGE      50


#define LIST_HIDING             1
#define LIST_WORKING            2

#define UNI_MAX_PF_STEPS_COUNT  50            //!< Maximal count of steps of pathfinder during fast path counting (AI)


//========================================================================
// Macros
//========================================================================

#define SetUnitColor(style) \
  do { \
    if ((style) == DS_UNDERGROUND) glColor4f(1, 1, 1, 0.5f); \
    else if ((style) == DS_BUILDING) glColor4f(1, 1, 1, 0.3f); \
    else glColor3f(1, 1, 1); \
  } while(0)


#define ReleaseCountedPointer(unit) \
if (unit) { \
  unit->ReleasePointer(); \
  unit = NULL; \
}


//========================================================================
// Enumerations
//========================================================================

/**
 *  Type, which represents unit action.
 */
enum TUNIT_ACTION {
  UA_STAY,    //!< Unit action is staying.
  UA_MOVE,    //!< Unit action is moveing.
  UA_HIDE,    //!< Unit action is hiding.
  UA_ATTACK,  //!< Unit action is attacking.
  UA_MINE,    //!< Unit action is mining.
  UA_REPAIR,  //!< Unit action is repairing.
  UA_BUILD,   //!< Action for building - used only for mouse action.
  UA_NONE     //!< No action - used only for mouse action.
};


//========================================================================
// Included Files
//========================================================================

#include "cfg.h"
#include "doalloc.h"

#include "doraces.h"
#include "dowalk.h"
#include "dodraw.h"
#include "doevents.h"
#include "doplayers.h"


//=========================================================================
// Typedefs
//=========================================================================

/**
 *  Basic class contains common informations and methods for all drawn objects.
 *  Unit position are coordinates from left-down field in the map.
 *  Unit is NOT assignet to player (map objects...)
 *
 *  @sa TDRAW_ITEM
 */
class TDRAW_UNIT {
  friend class TSEG_UNITS;

public:

  virtual void Draw() { Draw(DS_NORMAL); };
  virtual void Draw(T_BYTE style);
  virtual void DrawToRadar() {};
  virtual bool UpdateGraphics(double time_shift);
  virtual void Dead(bool local);                        // The method correctly kills the unit.

  virtual void AddToSegments();
  virtual void DeleteFromSegments();

  TDRAW_ITEM* GetPointerToItem() const        //!< Returns pointer to kind of the unit.
    { return pitem; }; 
  /** Sets pointer to kind of the unit.
   *  @param new_item Pointer to new instance of the unit kind.*/
  void SetPointerToItem(TMAP_ITEM *new_item)
    { pitem = new_item;};

  void SetVisible(bool vis) { visible = vis; };             //!< Sets visible attribute.
  bool IsVisible(void) { return visible; };                 //!< Returns visible attribute.
 
  float GetRealPositionX() const {return rpos_x;};    //!< Returns value of rpos_x.
  float GetRealPositionY() const {return rpos_y;};    //!< Returns value of rpos_y.
  void SetRealPositionX(const float new_x) {rpos_x = new_x;};   //!< Sets rpos_x to the value from param.
  void SetRealPositionY(const float new_y) {rpos_y = new_y;};   //!< Sets rpos_y to the value from param.
  
  void SetSynchronisedPosition(const TPOSITION_3D new_pos) {sync_pos = new_pos;};     //!< Sets synchronised position of the unit to position of param.
  TPOSITION_3D GetSynchronisedPosition() const {return sync_pos;};   //!< Returns synchronised position of unit in mapels.

  TDRAW_UNIT();           //! Constructor only zeroize values.
  TDRAW_UNIT(int p_x, int p_y, int p_z, TDRAW_ITEM* set_item, TTEX_TABLE *tex_table, bool random_tex = false); //!< Parametrized constructor.
 
  virtual ~TDRAW_UNIT();  //! Destructor.

  TGUI_ANIMATION* GetAnimation() const {return animation;};   //!< Returns pointer to animation structure of the unit.
  void SetAnimation(TGUI_ANIMATION *new_anim) {animation = new_anim;};  //!< Sets pointer to animation structure of unit to value from param.

  TPOSITION_3D GetPosition() const {return pos;};         //!< Returns position of unit in mapels. Unit's position are coordinates of southwest corner of unit.
  TPOSITION_3D GetCenterPosition();
  TPOSITION_3D TranslateToCentralize(const TPOSITION_3D position);
  void SetPosition(const TPOSITION_3D new_pos);           //!< Sets position of unit to position of param if it is in the map.  
  void SetPosition(const T_SIMPLE nx, const T_SIMPLE ny, const T_SIMPLE ns); //!< Sets position of unit to position of parameters if it is in the map.

  T_SIMPLE GetUnitWidth() const {return pitem->GetWidth();};            //!< Returns width of the unit in mapels.
  T_SIMPLE GetUnitHeight() const {return pitem->GetHeight();};          //!< Returns height of the unit in mapels.

  bool IsLieingDown() { return lieing_down; }
  bool IsFlyingUp() { return flying_up; }
  bool IsInActiveArea() { return in_active_area; }


#if SOUND
  TSND_GROUP *snd_played;                       //!< Last played sound.
  /** Playes sound snd and stores it into snd_played.
      @param force 0 - dont play, 1 - stop previous, 2 - double play
  */
  void PlaySound(TSND_GROUP *snd, T_BYTE force = 0) {
    if (force != 2 && snd_played && snd_played->IsPlaying()) {
      if (force == 1) snd_played->Stop();
      else return;
    }

    snd_played = snd;
    snd->Play();
  }
#endif


protected:
  /** Returns pointer to next unit in the segment list of map units.
   *  @param seg  Segment number. */
  TDRAW_UNIT* GetNextInSegment(T_BYTE seg) const 
    {return seg_next[seg];};
  /** Returns pointer to previous unit in the segment list of map units.
   *  @param seg  Segment number. */
  TDRAW_UNIT* GetPrevInSegment(T_BYTE seg) const 
    {return seg_prev[seg];};
  /** Sets pointer to next unit in the segment list of map units to value from param.
   *  @param seg  Segment number.
   *  @param new_next Pointer to a new next unit. */
  void SetNextInSegment(T_BYTE seg, TDRAW_UNIT *new_next)
    {seg_next[seg] = new_next;};
  /** Sets pointer to previous unit in the segment list of map units to value from param.
   *  @param seg  Segment number.
   *  @param new_next Pointer to a new previous unit. */
  void SetPrevInSegment(T_BYTE seg, TDRAW_UNIT *new_prev)
    {seg_prev[seg] = new_prev;};

  bool IsCloserThan(TDRAW_UNIT *unit);

  virtual void TestVisibility();

  /** The method sets unit to will be deleted and delete unit. */
  virtual void UnitToDelete(bool lock) { delete this; }

  TDRAW_ITEM      *pitem;     //!< Pointer to kind of the unit.
  TGUI_ANIMATION  *animation; //!< Animation for unit.

  TPOSITION_3D pos;           //!< Position of the unit.
  TPOSITION_3D sync_pos;      //!< Synchronised position of the unit.
  float rpos_x, rpos_y;       //!< Real actual position in map. Need for drawing and textures sorting. [mapels]

  bool in_active_area;        //!< If unit is in active map area.
  bool visible;               //!< Whether unit is seen and drawn.
  bool lieing_down;           //!< Whether unit is lieing down and other units can walk over it. Used in sorting method.
  bool flying_up;             //!< Whether unit is flying up over other units. Used in sorting method.

  TDRAW_UNIT *seg_next[DAT_SEGMENTS_COUNT + 1];    //!< Pointer to a next unit in segment list of drawn units.
  TDRAW_UNIT *seg_prev[DAT_SEGMENTS_COUNT + 1];    //!< Pointer to a previous unit in segment list of drawn units.
};


/**
 *  Class contains information about assignment to player
 *
 *  @sa TDRAW_ITEM
 */
class TPLAYER_UNIT : public TDRAW_UNIT {
  friend class TPLAYER;

public:
  virtual TEVENT* SendEvent(bool n_priority=false, double n_time_stamp=0, int n_event=0, int n_request_id=0, T_SIMPLE n_simple1=0, T_SIMPLE n_simple2=0, T_SIMPLE n_simple3=0, T_SIMPLE n_simple4=0, T_SIMPLE n_simple5=0, T_SIMPLE n_simple6=0, int n_int1=0,int n_int2=0) {return NULL;}; //TPLAYER_UNIT::SendEvent() should be never called.
  virtual void TestVisibility();
  virtual bool IsGhost() { return false; }
  virtual void CreateGhost() {}
  virtual void DestroyGhost() {}
  virtual void Disconnect() {delete this;}

  T_BYTE GetPlayerID() const ;
  TPLAYER* GetPlayer() const { return player; };        //!< Returns pointer to owner of the unit.
  virtual void ProcessEvent(TEVENT * proc_event) {};
  int SendRequest(bool n_priority=false, double n_time_stamp=0, int n_event=0, int n_request_id =0, T_SIMPLE n_simple1=0, T_SIMPLE n_simple2=0, T_SIMPLE n_simple3=0, T_SIMPLE n_simple4=0, T_SIMPLE n_simple5=0, T_SIMPLE n_simple6=0, int n_int1=0,int n_int2=0);
  TEVENT * SendRequestLocal(bool priority=false, double n_time_stamp=0, int n_event=0, int n_request_id =0, T_SIMPLE n_simple1=0, T_SIMPLE n_simple2=0, T_SIMPLE n_simple3=0, T_SIMPLE n_simple4=0, T_SIMPLE n_simple5=0, T_SIMPLE n_simple6=0, int n_int1=0,int n_int2=0);
  void SendNetEvent(TEVENT * event, int player_id);

  TEVENT * pevent; //!< Pointer to event.

  /** Sets pointer to owner of the unit. 
  * @param owner  Pointer to the new unit's owner. */
  void SetPlayer(TPLAYER *owner) { player = owner; }
  bool TestPlayer(TPLAYER *owner) { return player == owner; }

  TPLAYER_UNIT();           //! Constructor only zeroizes values.
  TPLAYER_UNIT(int set_player, int p_x, int p_y, int p_z, TDRAW_ITEM* set_item, int new_unit_id, bool global_unit); //! Parametrized constructor
 
  virtual ~TPLAYER_UNIT();  //! Destructor.

  int GetUnitID(void) {return unit_id;};  //!< Returns unique unit identificator.

  
  char GetItemType() { return static_cast<TMAP_ITEM *>(pitem)->GetItemType(); };
  bool TestItemType(TITEM_TYPE it) { return (static_cast<TMAP_ITEM *>(pitem)->GetItemType() == it); };
  
  /** The method returns whether the unit has order */
  bool HaveOrder() {return have_order;};
  /** Set have_order to true */
  void SetOrder() {have_order = true;};
  /** Set have_order to false */
  void ResetOrder() {have_order = false;};

  /** Sets state to the value in the parameter.
  * @param putted  New value of the unit state. */
  void PutState(const unsigned int putted) {state = putted;};
  /** Tests state of unit to parameter. If send to test more then one return true when units is in any of the sended states.
  * @param tested State to test. */
  unsigned int GetState() const {return state;};    //!< Returns actual state of the unit.
  bool TestState(const unsigned int tested) const {
    return (tested == state);
  };

  double last_event_time_stamp; //!< Timestamp of last processed event of unit.

  TPLAYER_UNIT *GetNext() const { return next; }    //!< Returns pointer to next unit in the list of units owned by same player.
  /** Sets pointer to next map unit in the list to value of parameter.
  * @param new_next Pointer to next unit.*/
  void SetNext(TPLAYER_UNIT *new_next) { next = new_next; }
  TPLAYER_UNIT *GetPrev() const { return prev; }    //!< Returns pointer to previous unit in the list of units owned by same player.
  /** Sets pointer to previous map unit in the list to value of parameter.
  * @param new_prev Pointer to previous unit.*/
  void SetPrev(TPLAYER_UNIT *new_prev) { prev = new_prev; }

protected:
  TPLAYER *player;    //!< Pointer to instance of the unit owner.

  TPLAYER_UNIT *next; //!< Pointer to a next unit in the player list of map units.
  TPLAYER_UNIT *prev; //!< Pointer to a previous unit in the player list of map units.

  int unit_id;        //!< Numeric unique identificator of unit.
  unsigned int state;     //!< Unit state [US_...].
  int waiting_request_id; //!< If unit is waiting for some request, request id is stored here
  int sound_request_id;   //!< If unit is sending request for sound to itself, sound is played only when it is expected and sill valid.

  bool have_order;    //!< If unit has order in this round of AI
};


/**
 *  Class with information and methods which are the same all objects on the map.
 *  Each player has its own list of its units.
 *
 *  @sa TMAP_ITEM, TARMAMENT, TPLAYER, TPOSITION_3D
 */
class TMAP_UNIT : public TPLAYER_UNIT{
public:
  virtual void Draw(T_BYTE style);
  virtual void DrawToRadar();
  inline  void DrawBGSelection(T_BYTE style);
  inline  void DrawFGSelection(T_BYTE style);
  virtual void ProcessEvent(TEVENT * proc_event);
  virtual bool UpdateGraphics(double time_shift);
  virtual void Dead(bool local);
  virtual void Disconnect();

  virtual void TestVisibility();

  /** Tests whether the attack by attacker with flags sended in the parameter take an effect.
  * @param flags  Flags of attacker stored in his TARMAMENT class. */
  virtual bool DoesAttackTakeEffect(const char flags) const {return true;};
  /** Fires on position included in parameter.*/
  bool FireOn(const int ipos_x, const int ipos_y, const int ipos_segment, const double ts);

  void SetSelected(bool sel) { selected = sel; };         //!< Sets selected parameter.
  bool GetSelected(void) { return selected; };            //!< Returns selected parameter.
  int GetWaitRequestId() { return waiting_request_id;};    //<! Returns waiting request id.
  void SetWaitRequestId(int wr_id) { waiting_request_id = wr_id;};  //<! Sets waiting request id.
  
  bool IsInMap(void) { return is_in_map; };               //!< Returns if unit is in map.
  virtual bool IsGhost() { return state == US_GHOST; }
    
  void SetGroupID(int gid) { group_id = gid; };           //!< Sets group id parameter.
  int  GetGroupID(void) { return group_id; };             //!< Returns group id parameter.

  /** Set life to value from parameter.
  * @param value  New value of the life. The value have to be between zero and maximum life of unit kind.*/
  void SetLife(const float value);
  float GetLife() const { return life; };           //!< Returns actual value of the life. 
  bool  HasMaxLife() { return life >= static_cast<TMAP_ITEM *>(pitem)->GetMaxLife(); }

  /* Injure unit for the value of the parameter */
  bool Injure(const float injury); 
  /* Heal unit for the value of the parameter but outside to maximum life of unit kind */
  float Heal(const float value); 

  TMAP_UNIT();                        //! Constructor.
  TMAP_UNIT(int uplayer, int ux, int uy, int uz, TMAP_ITEM *mi, int new_unit_id, bool global_unit);   //! Constructor that sets position of unit and its kind.
  virtual ~TMAP_UNIT();   //! Destructor.


  TMAP_UNIT* GetTarget() const {return target;};          //!< Returns target of the unit.
  void SetTarget(TMAP_UNIT *nt) {target = nt;};           //!< Sets target of the unit to pointer from parameter.
  bool HasTarget() const {if (target) return true; else return false;}; //!< Tests whether unit has target of the attack.

  virtual int CountPathDistance(TPOSITION_3D area_pos, int area_width, int area_height,int max_cnt); 

  /** Selects reaction of the unit with the dependence of the clicked unit/building.
   *  @param unit    Unit, that was clicked on. */
  virtual bool SelectReaction(TMAP_UNIT *unit, TUNIT_ACTION action);
  virtual bool StartStaying();

  virtual void ClearActions();
  virtual TUNIT_ACTION GetAction();
  virtual void ChangeAnimation();

  virtual bool AddToListOfUnits(TFORCE_UNIT *unit, T_BYTE which_list);            //!< The method adds unit into the list according to the which_list param.
  virtual void RemoveFromListOfUnits(TFORCE_UNIT *unit, T_BYTE which_list);     //!< The method removes unit from the list according to the which_list param.

  virtual bool AddToMap(bool to_segment, bool set_view);
  virtual void DeleteFromMap(bool from_segment);

  /** The method acquire pointer to the object and increase counter 
   *  of the pointers if is not dedicated to deletion.
   *  @return If will_be_deleted is false then the method returns this pointer
   *  otherwise returns NULL. */
  TMAP_UNIT* AcquirePointer() {
    if (will_be_deleted) return NULL; else { pointer_counter++; return this; }
  }

  /** The method release pointer to the object and decrease counter of the pointers.
   *  If counter is zero and flag will_be_deleted is set then unit is deleted
   *  from the memory. 
   *  @return The method returns true if unit hasn't flag will_be_deleted setted to true.*/
  bool ReleasePointer() {
    if (!pointer_counter) return true;
    bool result = !will_be_deleted; pointer_counter--; if (!pointer_counter && will_be_deleted) delete this; return result;
  }

  TGUI_ANIMATION* GetBurnAnimation() const { return burn_animation; };          //!< Returns pointer to animation structure of the unit.
  TGUI_ANIMATION* GetSignAnimation() const { return sign_animation; };          //!< Returns pointer to animation of insufficient material or aid.
  void ResetSignAnimation() { sign_animation = NULL; }

  bool IsSelected() { return selected; }
  void MessageText(bool possitive, char *text, ...);

  /** @return The method returns reference to the list of held units. */
  TLIST<TFORCE_UNIT>& GetHidedUnits()
    { return hided_units; }

  TLIST<TWORKER_UNIT>& GetWorkingUnits()
    { return working_units; }

  bool CanAcceptUnit(TFORCE_UNIT *unit);
  bool AcceptsHidedUnits() { return (static_cast<TMAP_ITEM*>(pitem)->GetMaxHidedUnits() > 0); }
  bool IsFull() { return (hided_count == static_cast<TMAP_ITEM*>(pitem)->GetMaxHidedUnits()); }
  void EjectUnits();
  bool CanEject();
  T_BYTE GetHidedCount() { return hided_count; }

  /** The method start attack to the unit from parameter.*/
  virtual bool StartAttacking(TMAP_UNIT *unit, bool automatic);

  /** The method test whether object is present in the segment interval of parameter.*/
  virtual bool ExistInSegment(int bottom, int top) const;

  /** The method returns the worth of armament of unit */
  virtual float GetArmamentWorth(){return ((TMAP_ITEM *)pitem)->GetArmament()->GetWorth();};

  TMAP_UNIT *GetGhostOwner() { return ghost_owner; }
  
  /** @return The method returns units mode of aggressivity. */
  TAGGRESSIVITY_MODE GetAggressivity() const
    { return aggressivity;}
  TAGGRESSIVITY_MODE SetAggressivity(TAGGRESSIVITY_MODE new_agg, bool test_is_being_built);

  /** @return The method returns if attack unit was called automaticly. */
  bool GetAutoAttack() const
    { return auto_attack;}
  /** The method sets type of attack (auto, manual).
   *  @param new_auto_attack  The setted type of attack. */
  void SetAutoAttack(bool new_auto_attack)
    { auto_attack = new_auto_attack; }

protected:
  /** The method sets variable to will be deleted and delete unit. */
  virtual void UnitToDelete(bool lock) {
  	
#ifdef NEW_GLFW3
	if (lock) mtx_lock(&delete_mutex);
#else
	if (lock) glfwLockMutex(delete_mutex);
#endif
    
    if (!pointer_counter) delete this; else will_be_deleted = true;
    	
#ifdef NEW_GLFW3
	if (lock) mtx_unlock(&delete_mutex);
#else
	if (lock) glfwUnlockMutex(delete_mutex);
#endif
  }

protected:
  bool selected;          //!< If unit is in selection.
  bool is_in_map;         //!< If unit is in map.
  int  group_id;          //!< ID of stored group of units.
  float life;             //!< Actual life of the unit.
  TMAP_UNIT *target;      //!< Pointer to the unit whether is under attack by myself.
  TMAP_UNIT *last_target; //!< Pointer to the unit whether was last time under attack by myself.
  int myself_units;       //!< Count of myself units helded in map unit.

  TPROJECTILE_UNIT *shot; //!< Shoted shot before leaves the barrel.
  TMAP_UNIT *ghost_owner; //!< Owner of this unit in ghost state.

  TLIST<TFORCE_UNIT> hided_units;       //!< The list of force units which are hiding.
  TLIST<TWORKER_UNIT> working_units;    //!< The list of force units which are mining or unloading inside the source/building.

  T_BYTE hided_count;                    //!< Count of engaged hided places.

  unsigned int pointer_counter;       //!< The counter of pointers assigned to the instance of class.
  TGUI_ANIMATION  *burn_animation;    //!< Animation for insufficient material or aid.
  TGUI_ANIMATION  *sign_animation;    //!< Animation for fire over unit.

  bool will_be_deleted;   //!< Whether unit will be deleted or not.

private:
  TAGGRESSIVITY_MODE aggressivity;        //!< Aggressivity of the unit. 
  bool auto_attack;                       //!< mark if attack was run automaticly (from watch and aim lists).
};


//forward declaration
class TITERATOR_POOLED_LIST;

/**
 *  List which doesn't allocate members but takes its from the pool.
 */
class TPOOLED_LIST {

public:
  class TNODE : public TPOOL_ELEMENT {
  public:

    /** @return The method returns pointer to stored unit. */
    TMAP_UNIT* GetUnit() const 
      { return unit;};

    /** The method sets pointer to unit to @p new_unit. 
      *  @param new_unit Pointer to setted unit.*/
    void SetUnit(TMAP_UNIT* new_unit) 
      { unit = new_unit;};

    /** @return The method returns <b>true</b> if stored unit is same as @p tested
      *  @param tested Pointer to unit which storing is tested.*/
    bool IsSameUnit(TMAP_UNIT* tested) const
      { return (unit == tested);};
    
    /** 
      * The method clears attributes of the node.
      * @param all If true then is cleaned pointer to next node too.
      */
    virtual void Clear(bool all) {if (all) pool_next = NULL; unit = NULL;};

  private:
    TMAP_UNIT* unit;    //!< Unit which aim or watch the field
  };

  TPOOLED_LIST(TPOOL<TNODE> *elements_source) 
    { first = NULL; last = NULL; length = 0; pool = elements_source;}
  ~TPOOLED_LIST();

  /** Adds new node at the beginning of the list. */
  void AddNode(TMAP_UNIT * const new_pitem);
  /** Adds new node at the beginning of the list only if it isn't in list. */
  bool AddNonDupliciteNode(TMAP_UNIT * const new_pitem);
  /** Adds new node at the end of the list. */
  void AddNodeToEnd(TMAP_UNIT * const new_pitem);
  /** Adds new node at the end of the list only if it isn't in list. */
  bool AddNonDupliciteNodeToEnd(TMAP_UNIT * const new_pitem);
  /** The method removes node from the list. */
  bool RemoveNode(TMAP_UNIT* delete_node);

  /** The method tests whether parameter is in the list.
   *  @param tested Tested pointer. 
   *  @return The method returns true if parameter is in the list otherwise 
   *  returns false.*/
  bool IsMember(TMAP_UNIT * const tested) {
    for (TNODE *aux = first; aux; aux = dynamic_cast<TNODE*>(aux->GetNext()))
      if (aux->IsSameUnit(tested)) return true;
    return false;
  };

  /** @return The methdo returns length of the list.*/
  unsigned int GetLength() const
    { return length;};

  /** The method tests if list is empty or not. 
   *  @return The method returns true if list is empty. */
  bool IsEmpty() const
    { return (length == 0); };

  /** The method creates iterator for walking through the list.
   *  @return The method returns pointer to iterator on success. Otherwise NULL*/
  TITERATOR_POOLED_LIST* GetIterator() const;

protected:

  TNODE *first;          //!< First node in the list.
  TNODE *last;           //!< Last node in the list.

private:

  unsigned int length;   //!< Length of the list.
  TPOOL<TNODE> *pool;    //!< Pool with preallocated nodes for the list.

  friend class TITERATOR_POOLED_LIST;
};


/**
 *  Simple iterator for walking through pooled list.
 *
 *  @sa TPOOLED_LIST
 */
class TITERATOR_POOLED_LIST {
public:

  TITERATOR_POOLED_LIST(const TPOOLED_LIST *const list)
    { actual = list->first;}

  /** @return The method returns true if in the list is next unit. */
  bool HasNextUnit() const
    { return ((actual != NULL) && (actual->GetNext() != NULL));}

  /** The method returns pointer to actual unit and moves to next node.
   *  @return The method returns pointer to next unit if exists otherwise NULL.*/
  TMAP_UNIT* NextUnit() 
  { 
    TMAP_UNIT *result = NULL; 
    if (actual != NULL) { result = actual->GetUnit(); actual = reinterpret_cast<TPOOLED_LIST::TNODE*>(actual->GetNext());} 
    return result;
  }

private:
  TPOOLED_LIST::TNODE *actual;  //!< Pointer to actual position in the list.
};


/**
 *  List which doesn't allocate members but takes its from the pool.
 */
class TMAP_POOLED_LIST : public TPOOLED_LIST {

public:

  TMAP_POOLED_LIST(TPOOL<TNODE> *elements_source) : TPOOLED_LIST(elements_source) {}

  /** The method starts attack for each member of the list.*/
  void AttackEnemy(TMAP_UNIT *enemy, bool watchers_list);
};



/**
 *  Class includes informations about shot projectile.
 *
 *  @sa TPROJECTILE_ITEM, TGUN, TPOSITION_3D
 */
class TPROJECTILE_UNIT : public TPLAYER_UNIT {

public:
  TPROJECTILE_UNIT(int playerID, double time, TPOSITION_3D spos, TPOSITION_3D ipos, TGUN* g, int new_unit_id, bool global_unit);   //!< Constructor
  /**  Called when player disconnects and delete all units */
  virtual void Disconnect();
  
  /** Returns time lefted to the impact. */
  double GetImpactTime() const
    {return impact_time;};

  /** Add parameter to x coordinate.
   *  @param ch Change of the x coordinate. */
  float ChangeX(const float ch) 
    {return rpos_x += ch;};
  /** Add parameter to y coordinate.
   *  @param ch Change of the y coordinate. */
  float ChangeY(const float ch) 
    {return rpos_y += ch;};
  /** Returns segment of the actual position. */
  T_SIMPLE GetSegment() const 
    {return pos.segment;};
  double GetSegmentTime() const     //!< Returns time lefted to leave the actual segment.
    {return seg_time;};
  /** Sets time lefted to leave the actual segment.
  * @param time Time lefted to leave the actual segment. */
  void SetSegmentTime(const double time) 
    {seg_time = time;};

  /** @return The method returns impact position. */
  TPOSITION_3D GetImpactPos() const
    {return impact_pos;};
  /** @return The method returns start position. */
  TPOSITION_3D GetStartPos() const
    { return start_pos;};

  /** The method calculates sinus and cosinus of the moving angle. */
  double CalculateAngles();

  /** Called when projectile is impacting. */
  void Impact();

  virtual bool UpdateGraphics(double time_shift);
  virtual void ProcessEvent(TEVENT * proc_event);

  /** @return The method gets moving direction.*/
  int GetDirection() const
    { return direction;};

  /** @return The method returns true when projectile is shotted. 
   *  Otherwise returns false.*/
  bool IsShotted() const
    { return is_shotted; }

  /** The method sets shot state to true. */
  void Shot() 
    { is_shotted = true; }


private:
  const double impact_time;         //!< Left time to the impact.
  const TPOSITION_3D impact_pos;    //!< Impact position
  const TGUN* gun;                  //!< Gun which shot off the projectile.
  double seg_time;                  //!< Left time to leave the actual segment.
  double move_shift;                 //!< How long the projectile flying yet.
  float msin;                       //!< Sinus of the moving angle.
  float mcos;                       //!< Cosinus of the moving angle.
  const TPOSITION_3D start_pos;     //!< Start position of the projectile.
  int direction;                    //!< Direction of the moving.
  bool is_shotted;                  //!< Flag whether projectile is shotted or not.
};


/**
 *  Basic class with information and methods which are the same for makeable units and buildings.
 *  Specialization of the TMAP_UNIT.
 *  
 *  @sa TMAP_UNIT, TBASIC_ITEM
 */
class TBASIC_UNIT : public TMAP_UNIT {
public:
  TBASIC_UNIT(int uplayer, int ux, int uy, int uz, TBASIC_ITEM *mi, int new_unit_id, bool global_unit);
  virtual ~TBASIC_UNIT();

  void SetView(bool set);         // Sets view to the unit.
  bool IsSeenByUnit(TPOSITION pos, int x_new, int y_new, int u_width, int u_height, int view);      //!< Test whether unit can see the field.
  bool IsAimableByUnit(TPOSITION pos, int x_new, int y_new, int u_width, int u_height, int range_min, int range_max);      //!< Test whether unit can aim the field.
  bool IsGoodDistance(int tx, int ty, int radius_min, int radius_max);

  void ShowNeedFood();
  void ShowNeedEnergy();
  void ShowNeedMaterial(T_BYTE mat);
  void ShowNeedElement(T_BYTE id);

  /** The method checks neighbourhood of the unit to start attack to enemy.*/
  TMAP_UNIT* CheckNeighbourhood(TMAP_UNIT *previous = NULL);

protected:
  bool has_view;
};

//! Pointer to object of type TBASIC_UNIT.
typedef TBASIC_UNIT* PBASIC_UNIT;


/**
 *  Specialization of TBASIC_UNIT for moveable units.
 *
 *  @sa TBASIC_UNIT, TFORCE_ITEM, TPATH_LIST
 */
class TFORCE_UNIT : public TBASIC_UNIT {
public:

  virtual void ProcessEvent(TEVENT * proc_event);
  virtual TEVENT* SendEvent(bool n_priority=false, double n_time_stamp=0, int n_event=0, int n_request_id=0, T_SIMPLE n_simple1=0, T_SIMPLE n_simple2=0, T_SIMPLE n_simple3=0, T_SIMPLE n_simple4=0, T_SIMPLE n_simple5=0, T_SIMPLE n_simple6=0, int n_int1=0,int n_int2=0);
  virtual bool UpdateGraphics(double time_shift);
  virtual void DrawLine();
  virtual bool StartStaying();
  virtual void ClearActions();
  virtual void Disconnect();

  virtual void ComputePath(TPOSITION_3D goal,int request_id,int event_type = US_NONE,T_SIMPLE e_simple1=0, T_SIMPLE e_simple2=0);
  virtual void SearchForNearestBuilding(TSOURCE_UNIT *source, int request_id, int event_type,T_SIMPLE e_simple1=0,T_SIMPLE e_simple2=0);

  bool CanHide(TMAP_UNIT *unit, bool write_msg, bool auto_call);
  bool StartHiding(TMAP_UNIT *unit, bool auto_call);

  virtual void Dead(bool local);

  TFORCE_UNIT(int uplayer, int ux, int uy, int uz, int udirection, TFORCE_ITEM *mi, int new_unit_id, bool global_unit);    //!< Constructor.
  virtual ~TFORCE_UNIT(); //!< Destructor.

  /** The method sets move and look direction of unit.
   *  @param new_dr  New direction of unit. */
  void SetDirections(int new_dr)
    { move_direction = new_dr; if (new_dr <= LAY_SOUTH_EAST) look_direction = new_dr; };

  //!< Returns move direction of the unit.
  int GetMoveDirection() const 
  { return move_direction;};

  //!< Returns look direction of the unit.
  int GetLookDirection() const 
  { return look_direction;};

  //!< Set and unset visibility of fields around unit, which moves.
  void SetViewDirection(int direction);
  virtual bool SelectReaction(TMAP_UNIT *unit, TUNIT_ACTION action);
  virtual void ChangeAnimation();

  //!< Tests whether unit is landing.
  bool IsLanding() const
    {return TestState(US_LANDING);};

  bool MoveInDirection(int direct);               //!< Move unit one field in the direction from parameter.
  bool LandUnit(TPOSITION_3D * position);         //!< The unit try to land.

  /** Sets actual speed of the unit but only if the new speed is greater then zero.
   *  @param new_sp New speed of the unit. */
  void SetSpeed(const float new_sp) {if (new_sp <= 0) return; else speed = new_sp;};
  float GetSpeed() const {return speed;};         //!< Returns actual speed of the moving unit.

  /** Sets actual rotation speed of the unit but only if the new speed is greater then zero.
   *  @param new_sp New speed of the unit. */
  void SetRotationSpeed(const float new_sp) {if (new_sp <= 0) return; else speed = new_sp;};
  float GetRotationSpeed() const {return speed;};           //!< Returns actual rotation speed of the unit.

  /** Sets move time but only if the new move time is greater or equal then zero.
   *  @param new_mt New move time. */
  void SetMoveTime(const double new_mt) {if (new_mt >= 0) move_time = new_mt;};
  /** Returns time of moving. */
  double GetMoveTime() const {return move_time;};

  /** Sets try to move timer to zero. */
  void ResetTryToMoveTimer() {try_move_shift = 0.0;};
  /** Sets ty to move timer to value from parameter. 
   *  @param nttms  New value of the try to move timer.*/
  void SetTryToMoveTimer(const double nttms) { try_move_shift = nttms;};
  /** Returns value of the try to move timer. */
  double GetTryToMoveTimer() const { return try_move_shift;};

  /** Sets try to land timer to zero. */
  void ResetTryToLandTimer() {try_land_shift = 0.0;};
  /** Sets ty to land timer to value from parameter. 
   *  @param nttls  New value of the try to land timer.*/
  void SetTryToLandTimer(const double nttls) { try_land_shift = nttls;};
  /** Returns value of the try to land timer. */
  double GetTryToLandTimer() const { return try_land_shift;};

  /** Sets goal of the way. Doesn't control whether value from parameter is correct.
   *  @param ng New goal of the unit.*/
  void SetGoal(const TPOSITION_3D ng) {goal = ng;}; 
  /* Returns the goal position.
  */
  TPOSITION_3D GetGoal(void) {return goal;};  

  TPATH_LIST *CreatePathCopy(TPATH_LIST *leader_path,int shift_x, int shift_y, int shift_z);
  virtual int CountPathDistance(TPOSITION_3D area_pos, int area_width, int area_height,int max_cnt); 

  /* Sets states to correctly moving start. */
  bool StartMoving(TPOSITION_3D pos, bool auto_call);

  /** Determines direction of the rotation. */
  int DetermineRotationDirection(int direction);

  /** The method finds place adjacent to holder unit.*/
  virtual bool LeaveHolderUnit(TMAP_UNIT *holder);

  /** The method tests whether unit from parameter is neighbour of the unitself.*/
  bool IsNeighbour(TMAP_UNIT *unit) const;

  virtual bool AddToMap(bool to_segment, bool set_view);
  virtual void DeleteFromMap(bool from_segment);

  bool IsHeld() { return held; }
  void SetHeld(bool held, TMAP_UNIT *held_where = NULL, T_BYTE which_list = 0)
  { this->held = held; this->held_where = held_where; this->held_list = which_list; }

  void SetHider(TMAP_UNIT *hid) { hider = hid; }
  TMAP_UNIT *GetHider() { return hider; }
  
  /** The method start attack to the unit from parameter.*/
  virtual bool StartAttacking(TMAP_UNIT *unit, bool auto_call);

  /** The method test whether object is present in the segment interval of parameter.*/
  virtual bool ExistInSegment(int bottom, int top) const;

protected:
  /** Tests whether is adjacent position in direction from parameter available to move. */
  unsigned int IsAdjacentPositionAvailable(const int direct);
  /** Tests whether is selected position available to move. */
  bool IsSelectedPositionAvailable(const TPOSITION_3D new_pos);

public:
  TPATH_LIST *path;       //!< Path of moving unit.

protected:

  float speed;            //!< Actual speed. [mapel / second]
  float rotation_speed;   //!< Actual speed of the rotation. [radians per second]
  int look_direction;     //!< Actual direction. [LAY_...]
  int move_direction;     //!< Actual move direction. [LAY_...]
  double move_shift;      //!< Shift time. [seconds]
  double move_time;       //!< Time of moving. [seconds]
  double heal_shift;      //!< Length of the time that the unit doing nothing. [seconds]
  double try_move_shift;  //!< How long is the unit in the state US_TRY_TO_MOVE. [seconds]
  double try_land_shift;  //!< How long can unit TRY_TO_LAND wihout be Injured.
  int check_target;       //!< Counter of steps in path from last check of target position.
  int check_hider;        //!< Counter of steps in path from last check of hider position.

  TMAP_UNIT *hider;       //!< It set if unit want to hide itself in another unit.
  bool held;              //!< If unit is held in some held list.
  TMAP_UNIT *held_where;  //!< Map unit where the unit is held.
  T_BYTE held_list;       //!< In which list is the unit held.

  TPOSITION_3D goal;      //!< Goal of the path during unit movement.
};


/**
 *  Specialization of TBASIC_UNIT for buildings.
 *
 *  @sa TBASIC_UNIT, TBUILDING_ITEM
 */

class TBUILDING_UNIT : public TBASIC_UNIT {
public:
  TBUILDING_UNIT(int uplayer, int ux, int uy, TBUILDING_ITEM *mi, int new_unit_id, bool global_unit);  // Constructor
  TBUILDING_UNIT(TBUILDING_UNIT *copy_unit, int new_unit_id, bool global_unit);                        // Constructor
  virtual ~TBUILDING_UNIT(void);                                    // Destructor

  virtual void ClearActions();
  virtual void ChangeAnimation();
  virtual TEVENT* SendEvent(bool n_priority=false, double n_time_stamp=0, int n_event=0, int n_request_id=0, T_SIMPLE n_simple1=0, T_SIMPLE n_simple2=0, T_SIMPLE n_simple3=0, T_SIMPLE n_simple4=0, T_SIMPLE n_simple5=0, T_SIMPLE n_simple6=0, int n_int1=0,int n_int2=0);
  virtual void ProcessEvent(TEVENT * proc_event);
  virtual bool UpdateGraphics(double time_shift);

  virtual void TestVisibility();
  virtual bool StartAttacking(TMAP_UNIT *unit, bool auto_call);
  virtual void Disconnect();

  void RecreateBurnAnimation();

  /** Tests whether the attack by attacker with flags sended in the parameter
   *  take an effect.
   *  @param flags  Flags of attacker stored in his TARMAMENT class. */
  virtual bool DoesAttackTakeEffect(const char flags) const
    {if (flags & static_cast<char>(FIG_GUN_DAMAGE_BUILDINGS)) return true; else return false;};

  /** Sets private variable prepayed.
   *  @param value  The value which is setted as prepayd.*/
  void SetPrepayed(const float value)
  { 
    prepayed = max_prepayed = value; 
    SetProgress(0);
  };
  /** Gets private variable prepayed.*/
  float GetPrepayed() const 
    { return prepayed; };
  /** Decreases private variable prepayed about value.
   *  @param value  The value to decrease.
   *  @return The method returns really decreased value. Which is minimum
   *  of the prepayed amount and @p value.*/
  float DecPrepayed(const float value) 
    { float result = MIN(value, prepayed); prepayed -= result; SetProgress(T_BYTE(100 - (prepayed * 100) / max_prepayed)); return result;};
  
  /** Sets private variable progress and send messagethrough network to remote players. */
  void SetProgress(T_BYTE pr);
  /** Gets private variable progress. */
  T_BYTE GetProgress(void) { return progress; }

  bool GetAllowedMaterial(T_BYTE material) const { return static_cast<TBUILDING_ITEM*>(pitem)->GetAllowedMaterial(material); };
  bool AllowAnyMaterial() { return ((TBUILDING_ITEM *)pitem)->AllowAnyMaterial(); }

  //** Method creates ghost unit which is drawn under warfog. */
  virtual void CreateGhost();
  virtual void DestroyGhost() {
    if (!TestState(US_GHOST)) return;
    DeleteFromMap(true);
    UnitToDelete(true);
  }

  virtual void AddToSegments();
  virtual void DeleteFromSegments();
  virtual bool AddToMap(bool to_segment, bool set_view);
  virtual void DeleteFromMap(bool from_segment);

  void AddToPlayerArray();   //<! adds building into the player's material_array
  void DeleteFromPlayerArray();   //<! deletes building from the player's material_array
  virtual void Dead(bool local);

private:
  float prepayed;
  float max_prepayed;
  T_BYTE progress;    //!< Progress of building or any action. [%]
};


/**
 *  Specialization of TFORCE_UNIT, worker - special kind of unit, which is able
 *  to mine material.
 *
 *  @sa TFORCE_UNIT, TWORKER_ITEM
 */
class TWORKER_UNIT : public TFORCE_UNIT {
public:
  //! Gets the material type which the unit has mined.
  char GetMaterial() { return mined_material; };
  //! Sets the material type which the unit has mined.
  void SetMaterial(char material_type) { mined_material = material_type; };
  //! Gets the amount of material the unit extracted and is carrying.
  float GetMaterialAmount() { return material_amount; };
  //! Sets the amount of material the unit is carrying.
  void SetMaterialAmount(float mat_amount) { material_amount = mat_amount ;};
  //! Finds a new source (when the old one has collapsed).
  
  // help functions to finding new source (FindNewSource() function)
  TSOURCE_UNIT * TestSourceNorth(TPOSITION_3D position, T_SIMPLE w, T_SIMPLE h, T_SIMPLE v1, T_SIMPLE v2);
  TSOURCE_UNIT * TestSourceSouth(TPOSITION_3D position, T_SIMPLE w, T_SIMPLE h, T_SIMPLE v1, T_SIMPLE v2);
  TSOURCE_UNIT * TestSourceEast(TPOSITION_3D position, T_SIMPLE w, T_SIMPLE h, T_SIMPLE v1, T_SIMPLE v2);
  TSOURCE_UNIT * TestSourceWest(TPOSITION_3D position, T_SIMPLE w, T_SIMPLE h, T_SIMPLE v1, T_SIMPLE v2);

  TSOURCE_UNIT * FindNewSource(TPOSITION_3D position);
  //bool FindNewSource(TPOSITION_3D position);
#ifdef __GNUC__
	TSOURCE_UNIT * IsSourceOnPosition(int pos_x, int pos_y);
#else
  	TSOURCE_UNIT * TWORKER_UNIT::IsSourceOnPosition(int pos_x, int pos_y);
#endif
  

  //! Constructor.
  TWORKER_UNIT(int uplayer, int ux, int uy, int uz, int udirection, TFORCE_ITEM *mi, int new_unit_id, bool global_unit)
    :TFORCE_UNIT(uplayer, ux, uy, uz, udirection, mi, new_unit_id, global_unit)
  {
    mined_material = (T_BYTE) -1; //no material
    material_amount = 0;
    change_mined_material = false;
    built_or_repaired_unit = NULL; source = NULL; acceptor = NULL;
    check_built_or_repaired_unit = 0;
  };


  virtual bool SelectReaction(TMAP_UNIT *unit, TUNIT_ACTION action);
  virtual void ProcessEvent(TEVENT * proc_event);

  virtual void ClearActions();
  virtual TUNIT_ACTION GetAction();
  virtual void ChangeAnimation();  

  bool CanBuildOrRepair(TBASIC_UNIT *unit, bool write_msg, bool auto_call);
  //bool CanBuild(TBUILDING_UNIT *building, TPOSITION pos, bool **build_map, bool write_msg);  
  bool CanBuild(TBUILDING_ITEM *building, TPOSITION pos, bool **build_map, bool write_msg, bool auto_call);  

  TBUILDING_UNIT *StartBuild(TBUILDING_ITEM *building, TPOSITION build_here, bool auto_call);
  void Build();                         // Adds 'life' to building, the goal is to build new building.

  bool StartRepair(TBASIC_UNIT *unit, bool auto_call);  // Finds out, whether the building given as the first parameter can be repaired or not.
  void Repair();                        // Adds 'life' to building, the goal is to repair given building.

  bool CanMine(TSOURCE_UNIT *unit, bool write_msg, bool auto_call);
  bool StartMine(TSOURCE_UNIT *unit, bool auto_call);

  bool CanUnload(TBUILDING_UNIT *unit, bool write_msg, bool auto_call);
  bool StartUnload(TBUILDING_UNIT *unit, bool auto_call);

  virtual void Dead(bool local);              // The method correctly kill the unit.

  //!< The method gets pointer to nearest building for the worker from the source src.
  TBUILDING_UNIT* GetNearestBuilding(TSOURCE_UNIT * src, TA_STAR_ALG *path_tool); 
 
  void FindNearestBuilding(TLIST<TBUILDING_UNIT>& acceptable_buildings, TNEAREST_BUILDINGS &nearest, TA_STAR_ALG *path_tool);    //<! The method finds nearest building which accepts the same material as the source.

private:
  T_BYTE mined_material;      //!< Material, which worker extracts from the material source (source is typically some kind of building or mine). 
  float material_amount;      //!< Amount of the extracted material.
  bool change_mined_material; //!< Flag if worker change mined material type during mining.
  
  int check_built_or_repaired_unit;   //!< Counter of steps in path from last check of built_or_repair_unit position.

  TSOURCE_UNIT *source;       //!< Source inside which unt is mining right now.    
  TBASIC_UNIT *built_or_repaired_unit;  //!<Unit, which is actually built or repaired by worker.
  TBUILDING_UNIT *acceptor;   //!< Building that the accepts material from the worker
};


/**
 *  Help class of TSOURCE_UNIT, contains the nearest building which accepts
 *  the same material as the source offers.
 */
struct TNEAREST_BUILDINGS {
  int new_buildings_count;        //!< Count of newly built buildings that accept same material type.
  /** Pointer to nearest building which accepts the same material as the source offers. */
  TBUILDING_UNIT *nearest_building;
  double path_time;               //!< Lowest known time necessary for take away the material.
  TNEAREST_BUILDINGS() { new_buildings_count = 0; nearest_building = NULL; path_time = WRK_MAX_PATH_TIME;}
};

typedef  TNEAREST_BUILDINGS*  PTNEAREST_BUILDINGS;


/**
 *  Specialization of the TMAP_UNIT, source - special object that is source 
 *  of the material in the map. The material can be mined by workers unit.
 *  
 *  @sa TSOURCE_ITEM, TNEAREST_BUILDINGS
 */
class TSOURCE_UNIT : public TMAP_UNIT {
public:
  TSOURCE_UNIT(int uplayer, int uid, int ux, int uy, TSOURCE_ITEM *pit, int new_unit_id, bool global_unit);  //!< Constructor
  TSOURCE_UNIT(TSOURCE_UNIT *copy_unit, int new_unit_id, bool global_unit);                                  //!< Copy constructor
  ~TSOURCE_UNIT();        //!< Destructor

  virtual bool UpdateGraphics(double time_shift);
  virtual void ProcessEvent(TEVENT * proc_event);
  virtual void TestVisibility();
  virtual TEVENT* SendEvent(bool n_priority=false, double n_time_stamp=0, int n_event=0, int n_request_id=0, T_SIMPLE n_simple1=0, T_SIMPLE n_simple2=0, T_SIMPLE n_simple3=0, T_SIMPLE n_simple4=0, T_SIMPLE n_simple5=0, T_SIMPLE n_simple6=0, int n_int1=0,int n_int2=0);
  virtual void Disconnect();
  
  T_BYTE GetOfferMaterial() const { return static_cast<TSOURCE_ITEM*>(GetPointerToItem())->GetOfferMaterial(); };
  
  /** @def IncMaterialBalance
   *  Increase the source material balance by parameter.
   *  @return True if increasing was successful, false otherwise. */
  /** @def DecMaterialBalance
   *  Decrease the source material balance by parameter.
   *  @return True if decreasing was successful, false otherwise. */
  /** @def GetMaterialBalance
   *  @return Returns actual source material balance. */
  bool IncMaterialBalance() { return SetMaterialBalance(material_balance + 1); }
  bool DecMaterialBalance() { return SetMaterialBalance(material_balance - 1); }
  int  GetMaterialBalance() const { return material_balance; }
  bool SetMaterialBalance(int new_value);

  /** @def IsEmpty
   *  @return True if actual material balance is 0, false otherwise. */
  /** @def IsInsideMining
   *  @return True if workers mine inside this source. */
  bool IsEmpty() { return (material_balance == 0); }
  bool IsInsideMining() { return ((TSOURCE_ITEM *)pitem)->IsInsideMining(); }

  /** @return The method returns pointer to array nearest buildings for workers
   *  items available for the player.
   *  @param  Player ID.*/
  TNEAREST_BUILDINGS* GetPositionInPlayerArray(const int pid) { return my_player_array[pid];}; 

  

  TNEAREST_BUILDINGS **GetPlayerArray() { return my_player_array; };

  /** Tests whether the attack by attacker with flags sended in the parameter take an effect.
  * @param flags  Flags of attacker stored in his TARMAMENT class. */
  virtual bool DoesAttackTakeEffect(char const flags) const
    { return ((flags & static_cast<char>(FIG_GUN_DAMAGE_SOURCES))?true:false);};
//  void BuildingDestroyed(void *);         //!< Fills flags into player_array when building from param is destroyed.

  /** Method creates ghost unit which is drawn under warfog. */
  virtual void CreateGhost();
  virtual void DestroyGhost() {
    if (!TestState(US_GHOST)) return;
    DeleteFromMap(true);
    UnitToDelete(true);
  }

  virtual bool AddToListOfUnits(TFORCE_UNIT *unit,T_BYTE which_list);           //!< The method adds worker into the list according to the which_list param.
  virtual void RemoveFromListOfUnits(TFORCE_UNIT *worker,T_BYTE which_list);    //!< The method removes worker from the list according to the which_list param.

  virtual void ChangeAnimation();
  
  virtual void AddToSegments();
  virtual void DeleteFromSegments();
  virtual bool AddToMap(bool to_segment, bool set_view);
  virtual void DeleteFromMap(bool from_segment);

private:
  double renew_count;       //!< How much time it is since the last regeneration.

  int material_balance;   //!< Amount of the material, that is available in the source.
  /** The double dimension array with information about nearest buildings. 
   *  The first index determines player and second index determines the workers
   *  order between the workers items available for the player's race. */
  TNEAREST_BUILDINGS **my_player_array;
};


/**
 *  Specialization of the buildings contains necessary attributes and methods
 *  for producing force units and that descendants.
 *  The array of the orders is implemented as cyclic. The actual begin of the
 *  array is now producing unit. The begin is stored in attribut producing.
 *
 *  @sa TBUILDING_UNIT, TFORCE_UNIT, TFACTORY_ITEM
 */
class TFACTORY_UNIT : public TBUILDING_UNIT {
public:
  /** Constructor.*/
  TFACTORY_UNIT(int uplayer, int ux, int uy, TBUILDING_ITEM *mi, int new_unit_id, bool global_unit);

  /** Destructor.*/
  ~TFACTORY_UNIT(){
    ready_units.DestroyList();
  };

  /** Returns count of the ordered units.*/
  int GetOrderSize() const
    { return order_size;};
  bool IsPaused()
    { return paused; }

  virtual void ProcessEvent(TEVENT * proc_event);
  virtual bool UpdateGraphics(double time_shift);

  /** Returns time lefted to complete production of the unit.*/
  double GetLeftedTime() const
    { return production_time;};
  
  bool CanAddUnitToOrder(TFORCE_ITEM *unit_item = NULL);
  bool AddUnitToOrder(TFORCE_ITEM *unit_item);
  void TakeOffUnitFromOrder();
  void TogglePausedProducing();
  void CancelProducing(int orderid);

  TPRODUCEABLE_NODE* GetOrderedUnit(int index) { return order[(producing + index) % UNI_MAX_ORDER_LENGTH]; }
  TPOSITION_3D FindPlaceForProduct(TFORCE_ITEM *fitem);         //!< Find empty and available place for product.

  /** @return -1 if factory can produce next part of unit, 0-food needed, 1-energy needed, >1-material with id i-2 needed */
  signed char GetNeedID() {return need_id;};

private:
  TPRODUCEABLE_NODE* order[UNI_MAX_ORDER_LENGTH];     //!< Order of the units to production.
  int order_size;                   //!< Count of the ordered units.
  int producing;                    //!< Begin of the order. Index to the array of orders.
  double production_time;           //!< Time of unit production.
  int production_count;             //!< Count of left requests of production.
  signed char need_id;              //!< Identifier of missing material / food.
  bool paused;                      //!< If pruducing is paused.
  
  TLIST<TFORCE_UNIT> ready_units;   //!< The list of force units which are ready, but can not get out from factory.
};


//=========================================================================
// Variables
//=========================================================================

extern const double dir_angle[LAY_DIRECTIONS_COUNT][3];


//=========================================================================
// Global functions
//=========================================================================

#if DEBUG
  const char *EventToString(const int event);
#endif


#endif  // __dounits_h__

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

