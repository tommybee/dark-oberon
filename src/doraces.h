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
 *  @file doraces.h
 *
 *  Races declarations and methods.
 *
 *  @author Peter Knut
 *  @author Martin Kosalko
 *  @author Valeria Sventova
 *
 *  @date 2003
 */

#ifndef __doraces_h__
#define __doraces_h__

//=========================================================================
// Forward declarations
//=========================================================================

class TDEPENDENCES_TABLE;
class TDRAW_ITEM;
class TSURFACE_ITEM;
class TPROJECTILE_ITEM;
class TMAP_ITEM;
class TBASIC_ITEM;
class TFORCE_ITEM;
class TBUILDING_ITEM;
class TWORKER_ITEM;
class TSOURCE_ITEM;
class TPRODUCEABLE_NODE;
class TLIST_OF_PRODUCTS;
class TFACTORY_ITEM;
struct TRACE;


//=========================================================================
// Definitions
//=========================================================================

#define RAC_PATH  (app_path + DATA_DIR "races/").c_str()       //!< Path to data race directory.
  
#define RAC_MAX_UNIT_SIZE           15    //!< Max. unit width or height.
#define RAC_MAX_NAME_LENGTH         30    //!< Max. available race name length.

#define RAC_NO_FEATURES             0     //!< Unit hasn't any special features.
#define RAC_HAVE_TO_LAND            1     //!< Feature of the moveable units. If isn't moving have to anchor.
#define RAC_HEAL_WHEN_STAY          2     //!< Feature of the moveable units. Heals/repairs themself if stayes at the place and is injured.
#define RAC_HEAL_WHEN_ANCHOR        4     //!< Feature of the moveable units. Heals/repairs themself if anchors at the place and is injured.

#define RAC_UNABLE_POSITION         0     //!< Result of a position test. The position is landable nor moveable for the unit.
#define RAC_MOVEABLE_POSITION       1     //!< Result of a position test. The position is moveable for the unit.
#define RAC_LANDABLE_POSITION       2     //!< Result of a position test. The position is landable for the unit.
#define RAC_BOTHABLE_POSITION       3     //!< Result of a position test. The position is landable and moveable for the unit.

#define RAC_MAX_CONDITION_RETURNED_UNITS  10  //!< Maximum count of units returned by FindUnits and FindBuildings functions.
#define RAC_MAX_CONDITION_VALUE           10  //!< Max. value (necessary) of condition in FindUnits and FindBuildings functions.


//=========================================================================
// Included files
//=========================================================================

#include "cfg.h"
#include "doalloc.h"

#include "dofight.h"
#include "doschemes.h"
typedef float TNEURON_VALUE;


//=========================================================================
// Typedefs
//=========================================================================

typedef char TRAC_NAME[RAC_MAX_NAME_LENGTH];
typedef char TRAC_FILENAME[DAT_MAX_FILENAME_LENGTH];      //!< Used for maximal length fo rac filename.

enum TITEM_TYPE {
  IT_DRAW       = 'd',
  IT_PROJECTILE = 'p',
  IT_MAP        = 'm',
  IT_FORCE      = 'f',
  IT_WORKER     = 'w',
  IT_BUILDING   = 'b',
  IT_SOURCE     = 's',
  IT_FACTORY    = 'a',
};


/** The type expresses aggresivity of the unit when detecs enemy. */
enum TAGGRESSIVITY_MODE 
{
  AM_IGNORE,        //!< Ignores enemy - doesn't take any action
  AM_GUARDED,       //!< Attack enemy when it is possible to aim him but doesn't follow him
  AM_OFFENSIVE,     //!< Attack enemy when it is possible to aim him and follow him if run away
  AM_AGGRESSIVE,    //!< Attack and follow enemy when sees him
  AM_NONE = 255     //!< Empty mode.
};

#define RAC_AGGRESIVITY_MODE_COUNT        4


//=========================================================================
// Classes
//=========================================================================

/**
 *  Class contains common informations for all drawn units.
 */
class TDRAW_ITEM {
public:
  int index;                //!< Identificator of item in its table.
  char * text_id;           //!< User text identifikator used in conf. files
  char * name;              //!< Text name of item.
  int tg_stay_id;           //!< Texture group identifier.
  int tg_dying_id;          //!< Texture group identifier for dying.
  int tg_zombie_id;         //!< Texture group identifier for zombie.

  TDRAW_ITEM() {            //!< Constructor
    text_id = NULL; name = NULL; tg_stay_id = tg_dying_id = tg_zombie_id = -1;
    width = height = 0;
    index = -1;
    item_type = IT_DRAW;
  };

  virtual ~TDRAW_ITEM()     //!< Destructor.
  {
    if (text_id) delete[] text_id;
    if (name) delete[] name;
  };

  //! Returns width of unit kind in mapels.
  T_SIMPLE GetWidth() const { return width; };
  //! Returns height of unit kind in mapels.
  T_SIMPLE GetHeight() const { return height; };

  //! Sets width in mapels of unit kind to value in parameter.
  void SetWidth(T_SIMPLE w) { width = w; };
  //! Sets height in mapels of unit kind to value in parameter.
  void SetHeight(T_SIMPLE h) { height = h; };

  /** @return The method returns item type.*/
  TITEM_TYPE GetItemType()
    { return item_type;};

protected:
  TITEM_TYPE item_type;     //!< Type op item.

  T_SIMPLE width;           //!< Width the unit kind in mapels.
  T_SIMPLE height;          //!< Height of the unit kind in mapels.
};


/**
 *  Class contains projectile properties.
 */
class TPROJECTILE_ITEM : public TDRAW_ITEM {
public:
#if SOUND
  TSND_GROUP snd_hit;                       //!< Hit sound.
#endif

  float GetSpeed() const            //!< Returns speed of the shot case.
    {return speed;};
  /** Sets speed of the shot case if it is greater or equal to zero. Zero speed has gun with range <1,1>.
  * @param s  New speed of the shot case.*/
  void SetSpeed(const float s) 
    {if (s >= 0) speed = s;};

  T_SIMPLE GetScope() const         //!< Returns scope
    {return scope;};

  void SetScope(T_SIMPLE new_scope) //!< Sets scope
  {
      scope = new_scope;

      // calculate count of mapels attacked by projectile
      attacked_mapels_count = 0;
      for (int i = -scope ; i <= scope; i++)
        for (int j = -scope; j <= scope; j++)
          if ((Sqr(i) + Sqr(j)) <= Sqr(scope)) attacked_mapels_count++;
  };

  int GetAttackedMapels() const     //!< Returns count of attacked mapels
    {return attacked_mapels_count;};

  TPROJECTILE_ITEM()                //!< Constructor
    { item_type = IT_PROJECTILE; speed = 0.0f; scope = 0; attacked_mapels_count = 0; width = 1; height = 1; };

private:

  float speed;                //!< Speed of shot case.
  T_SIMPLE scope;             //!< Mine radius. [mapels]
  int attacked_mapels_count;  //!< Count of mapels attacked by projectile.
};


/**
 *  Class contains terrain ids.
 */
class TSURFACE_ITEM : public TDRAW_ITEM {
public:
  TTERRAIN_ID **terrain_field;   //!< Terrain id.

  TSURFACE_ITEM():TDRAW_ITEM()  { terrain_field = NULL; }  //! Constructor

  ~TSURFACE_ITEM()  { DeleteTerrainField(terrain_field, width); }   //! Destructor
};


/**
 *  Class contains common information for all map items.
 */
class TMAP_ITEM : public TDRAW_ITEM {
public:
  int tg_picture_id;        //!< Texture group identifier for picture.
  int tg_attack_id;         //!< Texture group identifier for attacking.
  int tg_burning_id;        //!< Texture group identifier for burning.

  T_BYTE selection_height;  //!< Height of selection box. [pixels]
  float burning_x;          //!< X position of burning animation. [pixels]
  float burning_y;          //!< Y position of burning animation. [pixels]

#if SOUND
  TSND_GROUP snd_selected;  //!< Selected sounds.
  TSND_GROUP snd_dead;      //!< Sound of unit dead.
  TSND_GROUP snd_burning;   //!< Sound of unit in burn.
  TSND_GROUP snd_fireon;
  TSND_GROUP snd_fireoff;
#endif

  //! Constructor
  TMAP_ITEM():TDRAW_ITEM() {
    item_type = IT_MAP; max_life = 0; armament = NULL;
    can_move = can_mine = can_repair = can_build = false;
    tg_attack_id = tg_picture_id = tg_burning_id = -1;
    selection_height = 0;
    burning_x = burning_y = 0.0f;
    max_hided_units = 0;
    aggressivity = AM_IGNORE;
    available_positions = 0.0f;
  };
  virtual ~TMAP_ITEM()          //!< Destructor.
  { 
    if (armament) delete armament;
    hide_list.DestroyList();
  };

  
  TLIST<TFORCE_ITEM> hide_list;  //!< List of units (force units) that can be hidden by unit.
    
  //! Returns armament of the unit kind.
  TARMAMENT* GetArmament() const {return armament;};
  /** Sets pointer to the armament of the map item. Dealocate old instance of the armament.
  * @param arm  Pointer to the new armament of the map item.*/
  void SetArmament(TARMAMENT *arm) {if (armament) delete armament; armament = arm;}

  //! Returns value of TMAP_ITEM::max_life.
  int GetMaxLife() const {return max_life;};
  //! Sets value of TMAP_ITEM::max_life if parameter is greater than zero.
  void SetMaxLife(const int m_life) { if (m_life > 0) max_life = m_life;};
  //! Sets can move flag of the unit.
  void SetCanMove(bool cm) {can_move = cm;};

  //! Returns @c true if user can move with unit.
  bool CanMove(void) { return can_move; };
  //! Returns @c true if unit can mine resources.
  bool CanMine(void) { return can_mine; };
  //! Returns @c true if unit can repair units.
  bool CanRepair(void) { return can_repair; };
  //! Returns @c true if unit can nuild buildings.
  bool CanBuild(void) { return can_build; };
  //! Returns true if the unit can attack.
  bool CanAttack() { return ((armament->GetOffensive() == NULL)?false:true);};

  /** The method gets interval of segments in which exists.
   *  @return The method returns reference to exist segments. */
  TINTERVAL<T_SIMPLE>& GetExistSegments() { return exist_segments;};
  /** The methos checks, whether segment given as parameter is in interval of segments,in which unit exists */
  bool IsExistSegment(int segment) 
  { 
    for (int i = exist_segments.min ; i <= exist_segments.max ; i++)
      if (segment == i) return true;    
    return false;
  }

  /** @return The method return count of force units which the unit can held.*/
  T_BYTE GetMaxHidedUnits() const
    { return max_hided_units; }
  /** The method sets count of force units which the unit can held.
   *  @param count  New count of force units which the unit can held */
  void SetMaxHidedUnits(const T_BYTE count)
  { max_hided_units = count; }

  /** @return The method returns true if the unit is moveable, otherwise false.*/
  virtual bool IsMoveable() const
    { return false;};

  /** @return The method returns true if the position is possible available. */
  virtual bool IsPossibleAvailablePosition(const TPOSITION_3D t_pos);
  /** @return The method returns true if the position is possible available. */
  virtual bool IsPossibleAvailablePosition(const T_SIMPLE t_pos_x, const T_SIMPLE t_pos_y, const T_SIMPLE t_pos_seg);

  /** @return The method returns units mode of aggressivity. */
  TAGGRESSIVITY_MODE GetAggressivity() const
    { return aggressivity;}
  /** The method sets units aggressivity mode.
   *  @param new_agg  The setted aggressivity. */
  void SetAggressivity(TAGGRESSIVITY_MODE new_agg)
    { aggressivity = new_agg; }

  /** @return The method returns percantage of available positions in the map.*/
  float GetAvailablePositions() const
    { return available_positions;}
  /** The method counts percentage of available positions in the map. */
  void CountAvailablePositions();

protected:
  bool can_move;            //!< Player can move with this unit.
  bool can_mine;            //!< Unit can mine materials.
  bool can_repair;          //!< Unit can repair units.
  bool can_build;           //!< Unit can repair buildings.

  int max_life;           //!< Maximum value of life of the unit kind.
  
  TARMAMENT *armament;      //!< Armament of the unit kind.

  TINTERVAL<T_SIMPLE> exist_segments;    //!< In which segments the unit exists.

  /** Maximal count of held units. Use follow values:
   *  RAC_NO_HELD_UNITS     The unit can't held any unit.
   *  other                 The unit can held 'count' units.*/
  T_BYTE max_hided_units;

private:
  TAGGRESSIVITY_MODE aggressivity;        //!< Aggressivity of the item (defined in conf file).
  float available_positions;              //!< Percentage of available positions.
};


/**
 *  Class contains common information for buildings and force items.
 */
class TBASIC_ITEM : public TMAP_ITEM {
public:

  TBASIC_ITEM();           //!< Constructor.
  
  virtual ~TBASIC_ITEM() { //!< Destructor.
    is_builded_by_list.DestroyList();
    is_repaired_by_list.DestroyList();
  };

  //! Returns count of instances of the unit kind.
  int GetCountOfActiveInstances() { return count_of_active_instances; };
  /** Sets count of instances of the unit kind if parameter is equal or greater
   *  then zero.
   *  @param new_count  New count of instances of the unit kind. */
  void SetCountOfActiveInstances(int new_count) 
    {if (new_count >= 0) count_of_active_instances = 0;};
  //! Increases count of instances of the unit kind and returns old count.
  int IncreaseActiveUnitCount()
  {return count_of_active_instances++;};
  /** Decreases count of instances of the unit kind if is greater or equal then
   *  zero and returns old count. */
  int DecreaseActiveUnitCount() 
    {if (count_of_active_instances > 0) return count_of_active_instances--; return count_of_active_instances;};

  float materials[SCH_MAX_MATERIALS_COUNT];  //!< How much of which material is needed to build the unit.
  float mat_per_pt[SCH_MAX_MATERIALS_COUNT]; //! <How much material is needed for one unit of life (jednotku zivota) of unit. 
    

  TINTERVAL<T_SIMPLE> visible_segments[DAT_SEGMENTS_COUNT];  //!< Segments into which the unit sees when it is in x-th segment.

  T_SIMPLE view;                  //!< Specifies how far the unit sees. [mapels]

  int energy;       //!< How much energy item needs ( <0 ) or supply ( >0 )
  int min_energy;   //!< How much percents (%) of energy item need for work (minimal amount)
  int food;         //!< How much food item needs ( <0 ) or supply ( >0 )

  TLIST<TBASIC_ITEM> is_builded_by_list;       //!< List of units (buildings) that are "builders" of unit.
  TLIST<TBASIC_ITEM> is_repaired_by_list;      //!< List of units (buildings) that are "repairers" of unit.

  //!< Function returns if it is possible to create unit with this type of item according to dependencies.
  virtual bool IsPossibleBuildable(float * price, TBASIC_ITEM ** next_itm);
  bool buildable_counted; //!< mark if item was counted to finding product (if function IsPossibleBuildable was run)
  float buildable_value [SCH_MAX_MATERIALS_COUNT + 2];  //!< value of item couted by IsPossibleBuildable function)

protected:
  int count_of_active_instances;         //!< Count of instances of the unit kind.  
};


/**
 *  Class containing information about unit kinds.
 *  In program it is used in dynamic allocated array (table).
 */
class TFORCE_ITEM : public TBASIC_ITEM {
public:

  int tg_move_id;           //!< Texture group identifier for moving.
  int tg_land_id;           //!< Texture group identifier for landing and unlanding.
  int tg_anchor_id;         //!< Texture group identifier for anchoring.
  int tg_rotate_id;         //!< Texture group identifier for rotating.

  TFORCE_ITEM();
  virtual ~TFORCE_ITEM(){};

  /** Sets special features of the unit kind.
   *  @param new_feats Features to set. */
  void SetFeatures(const char new_feats) { features = new_feats;};
  /** Adds special features of the unit kind.
   *  @param new_feats Features to set. */
  void AddFeatures(const char new_feats) { features |= new_feats;};
  char GetFeatures() const { return features;};         //!< Returns flags array of the special features of the unit.
  /** Tests whether unit has all features sended in the parameter. If hasn't return false.
   *  @param tested Flags array of the tested features. */
  bool TestAllFeatures(const char tested) const {if ((features & tested) == features) return true; else return false;};
  /** Test whether unit has some of the features sended in the parameter. If hasn't anyone returns false.
   *  @param tested Flags array of the tested features. */
  bool TestSomeFeatures(const char tested) const 
    { if (features & tested) return true; else return false;};

  /** Tests whether position is available for the kind of the unit.*/
  bool IsPositionAvailable(int pos_x, int pos_y, int pos_seg);
  /** Tests whether any position around holder unit  is available */
  bool IsPosAroundHolderAvailable(TFORCE_UNIT *unit, TMAP_UNIT* holder, T_BYTE seg, TPOSITION_3D *free_position);
  /** The method tests whether a position is landable, moveable or both. */
  unsigned char TestPositionFeatures(T_SIMPLE tx, T_SIMPLE ty, T_SIMPLE ts);

  /** Returns value that units needs to heal/repair one life point. If unit hasn't special feature 
   *  RAC_HEAL_WHEN_STAY or RAC_HEAL_WHEN_ANCHOR returns -1. */
  double GetHealTime() const {return (TestSomeFeatures(RAC_HEAL_WHEN_STAY | RAC_HEAL_WHEN_ANCHOR))?heal_time:-1;};
  /** Sets value that units needs to heal/repair one life point.
   *  @param nht  Value of the new heal time. */
  void SetHealTime(const double nht) {heal_time = nht;};

  /** Returns maximum rotation speed of the unit in the segment from parameter. If the parameter is invalid return zero.
   *  @param seg  Number of the segment. */
  float GetMaximumRotation(const int seg)
    { return ((seg >= 0) && (seg < DAT_SEGMENTS_COUNT))? max_rotation[seg] : 0.0f; };
  /** The method sets maximum speed of the rotation in the segment. Segment number is checked.
   *  @param nmr  New maximum speed of the rotation. Has to be greater or equal than zero.
   *  @param seg  Number of the setted segment. */
  void SetMaximumRotation(const float nmr, const int seg)
    { if ((seg >= 0) && (seg < DAT_SEGMENTS_COUNT) && (nmr >= 0.0)) max_rotation[seg] = nmr;};

  /** @return The method returns true if the unit is moveable, otherwise false.*/
  virtual bool IsMoveable() const
    { return true;};

public:
  float max_speed[DAT_SEGMENTS_COUNT];        //!< Maximum speed in every segment.
  float max_rotation[DAT_SEGMENTS_COUNT];     //!< Maximum rotation speed in radians/second for every segment.

  TINTERVAL<TTERRAIN_ID> moveable[DAT_SEGMENTS_COUNT];  //!< Specifies on which terrains the unit can move.
  TINTERVAL<TTERRAIN_ID> landable[DAT_SEGMENTS_COUNT];  //!< Specifies on which terrains the unit can land.
  TTERRAIN_ID land_segment;                   //!< Where is the unit landed.

#if SOUND
  TSND_GROUP snd_ready;                       //!< Ready sound.
  TSND_GROUP snd_command;                     //!< Command sound.
#endif

  /** @return The method returns true if the position is possible available. */
  virtual bool IsPossibleAvailablePosition(const T_SIMPLE t_pos_x, const T_SIMPLE t_pos_y, const T_SIMPLE t_pos_seg);

private:
  unsigned char features;                 //!< Flags array of special features of the unit kind.

  /** Time that units needs to heal/repair one life point. If unit anchors needs half of the time.
   *  If unit hasn't set special feature RAC_HEAL_WHEN_STAY or RAC_HEAL_WHEN_ANCHOR is value ignored. */
  double heal_time;                       // Time that units needs to heal/repair one life point.
};


/**
 *  Class containing information about building kinds.
 *  In program is used in dynamic allocated array (table).
 */
class TBUILDING_ITEM : public TBASIC_ITEM  {
public:

  int tg_build_id;                      //!< Texture group identifier for building.
  TBUILDING_ITEM *ancestor;             //!< Building is upgrade of its ancestor.

  TBUILDING_ITEM();
  virtual ~TBUILDING_ITEM();

  //! Returns if @p material is accepted by building.
  bool GetAllowedMaterial(T_BYTE material) {
    if (material < SCH_MAX_MATERIALS_COUNT)
      return allowed_materials[material];
    else return false;
  };

  /** Adds allowed material from parameter.
   *  @param added Added materials. */
  void AddAllowedMaterial(const T_BYTE added) {
    if (added < SCH_MAX_MATERIALS_COUNT)
      allowed_materials[added] = true;

    allow_any_material = true;
  };

  /** Returns true if any allowed material is true.
   *  Returns false if no allowed material is true. 
   */
  bool AllowAnyMaterial() { return allow_any_material; }

  //! Specifies on which terrains it is able to build the item.
  TINTERVAL<TTERRAIN_ID> buildable[DAT_SEGMENTS_COUNT];

  /** Tests whether position is available for the kind of the building.*/
  bool IsPositionAvailable(int pos_x, int pos_y, bool test_ancestor);

  //!< Function returns if it is possible to create unit with this type of item according to dependencies.
  virtual bool IsPossibleBuildable(float * price, TBASIC_ITEM ** next_itm);

  /** @return The method returns true if the position is possible available. */
  virtual bool IsPossibleAvailablePosition(const T_SIMPLE t_pos_x, const T_SIMPLE t_pos_y, const T_SIMPLE t_pos_seg);

private:
  bool allowed_materials[SCH_MAX_MATERIALS_COUNT];    //!< Materials, which the building accepts.
  bool allow_any_material;
};


/**
 *  Class contains common information about worker - special type of unit,
 *  which is able to mine material from a source (typically natural or built).
 */
class TWORKER_ITEM : public TFORCE_ITEM {
public:

  /** Gets the order of the unit among the types of workers. */
  int GetOrder(int material) { return order[material]; };  

  /** Sets order of worker among type for mateerial @param material */
  void SetOrder(int material, int ord) {order[material] = ord; };

  /** Constructor.
   *  @param o  Worker order between other workers of the race. */
  TWORKER_ITEM():TFORCE_ITEM()
    {
      for (register int i = 0; i < SCH_MAX_MATERIALS_COUNT ; i++)  
      {
        allowed_materials[i] = false;
        max_amount[i] = 0;
        unloading_time[i] = 0.0;
        mining_time[i] = 0;
        mining_sound_time[i] = 0;
        mining_sound_shift[i] = 0;
        order[i] = -1;
      }
      add_life = 0;
      tg_mine_id = tg_repair_id = -1;
    
      can_mine = can_repair = can_build = true;
      item_type = IT_WORKER;
      repairing_time = 0;

#if SOUND
      snd_workcomplete.repeat_limit = 0.3;
#endif
    };

  //! Destructor
  virtual ~TWORKER_ITEM();

  /** Returns maximum of given material, that the worker can mine during one extraction or -1 if index is out of array. 
   *  @param index Index to the max_amount array.
   */
  int GetMaxMaterialAmount(T_BYTE index) const {
    if (index < SCH_MAX_MATERIALS_COUNT)
      return max_amount[index];
    else return -1;
  };

  /** Sets maximum of given material, that the worker can mine during one
   *  extraction. 
   *  @param material   Number of material in which is interested.
   *  @param new_amount Maximum amount that can by mined at once.
   */
  bool SetMaxMaterialAmount(T_BYTE material, int new_amount)
    { 
      if (material < SCH_MAX_MATERIALS_COUNT) {
        if (new_amount >=0) max_amount[material] = new_amount;
        else max_amount[material] = 0;
        return true;
      }
      else return false;
    }; 

  /** Test whether the material is allowed.
   *  @return @c true if material is allowed, @c false otherwise.
   */
  bool GetAllowedMaterial(const T_BYTE material) {
    if (material < SCH_MAX_MATERIALS_COUNT)
      return allowed_materials[material];
    else return false;
  };
    
  /** Sets allowed material to value from parameter.
   *  @param new_materials  New value of materials.
   */
  void SetAllowedMaterial(const T_BYTE material, bool value) {
    if (material < SCH_MAX_MATERIALS_COUNT)
      allowed_materials[material] = value;
  };

  /** Adds allowed material from parameter. Its possible add more then one allowed material in the one time.
   *  @param added Added materials. 
   */ 
  void AddAllowedMaterial(const T_BYTE added) {
    if (added < SCH_MAX_MATERIALS_COUNT)
      allowed_materials[added] = true;
  };

  /** Returns true if any allowed material is true.
   *  Returns false if no allowed material is true. 
   */
  bool AllowAnyMaterial() {
    bool is = false;
    for (int i = 0; i < SCH_MAX_MATERIALS_COUNT; i++)
      is = (is) || (allowed_materials[i]);
    return is;
  };

  /** @return The method returns time necessary for unloading of the material
   *  with the index from the parameter.
   *  @param index  Material index. Use constants such as #RAC_FIRST_MATERIAL.*/
  double GetUnloadingTime(const int index) const
    { return unloading_time[index];};

  /** The method sets time necessary for unloading of the material with index
   *  from the parameter @p index.
   *  @param index  Material index. Use constants such as #RAC_FIRST_MATERIAL
   *  @param time Time necessary for unloading. 
   *  @return The method returns old value of the necessary time.*/
  double SetUnloadingTime(const int index, const double time)
    { double old_time = unloading_time[index]; unloading_time[index] = time; return old_time;};

  /** @return The method return time which workers needs to do one loop of mining material.*/
  double GetMiningTime(T_BYTE material) const
    { return mining_time[material]; };

  double GetMiningSoundTime(T_BYTE material) const
    { return mining_sound_time[material]; };

  double GetMiningSoundShift(T_BYTE material) const
    { return mining_sound_shift[material]; };
  
  /** @return The method return time which workers needs to do one loop of job (repiring, building).*/
  double GetRepairingTime() const
    { return repairing_time;};
  

  /** The method sets time which workers needs to do one loop of his mining material.
   *  @param  time  The time which is setted. 
   *  @param  material  Index of material which is setted. */
  void SetMiningTime(T_BYTE material, double time)
    { mining_time[material] = time;};
  
  /** The method sets time which workers needs to do one loop of his job (repaiting, building).
   *  @param  time  The time which is setted. */
  void SetRepairingTime(double time)
    { repairing_time = time;};

  void SetMiningSoundTime(T_BYTE material, double time)
    { mining_sound_time[material] = time;};

  void SetMiningSoundShift(T_BYTE material, double time)
    { mining_sound_shift[material] = time;};


  TLIST<TBUILDING_ITEM> build_list;     //!< List of buildings that can be built by worker.
  TLIST<TBASIC_ITEM> repair_list;       //!< List  of units (buildings) that can be repaired by worker.

  int tg_mine_id;           //!< Texture group identifier for mining.
  int tg_repair_id;         //!< Texture group identifier for repairing (building).

#if SOUND
  TSND_GROUP snd_workcomplete;                              //!< Sounds for finished work.
  TSND_GROUP snd_mine_material[SCH_MAX_MATERIALS_COUNT];    //!< Sounds for every material.
#endif


protected:
  bool allowed_materials[SCH_MAX_MATERIALS_COUNT];    //!< Max. 4 types of allowed material, that can be mined by worker.
  int max_amount[SCH_MAX_MATERIALS_COUNT];            //!< How much material can worker mine during one extraction.
  double mining_time[SCH_MAX_MATERIALS_COUNT];        //!< How long worker has to mine material;
  double unloading_time[SCH_MAX_MATERIALS_COUNT];     //!< How long worker has to unload material;
  double mining_sound_shift[SCH_MAX_MATERIALS_COUNT]; //!< Time shift before playing a sound for mining.
  double mining_sound_time[SCH_MAX_MATERIALS_COUNT];  //!< How often is played a sound for mining.
  
  int order[SCH_MAX_MATERIALS_COUNT];           //!< Order of the unit among the types of workers for every material.
  int add_life;                        //!< Amount of 'life' which can worker add to repaired or built unit at once. 

private:
  double repairing_time;   //!< How long worker has to work to do one loop of his job (repairing, building).
};


/**
 *  Class containing sources owned by player with ID number 0 (hyperplayer).
 */
class TSOURCE_ITEM: public TMAP_ITEM
{
public:
  /** The method sets capacity to the value from the parameter.
   *  @cap  The new value of the capacity. */
  void SetCapacity(int const cap) { capacity = cap; }
  int GetCapacity() const { return capacity; }

  //! Gets the material, which the source offers for mining.
  T_BYTE GetOfferMaterial() const { return offer_material; };
  //! Returns @c true, if the source is renewable.
  bool IsRenewable() const { return renewable;};
  //! Returns @c true, if the source is hideable.
  bool IsHideable() const { return hideable;};
  //! Returns the time neccesary to increment one unit of the material of source during refgeneration.
  double GetRegenerationTime() const { return time_of_regeneration;};  
  //! Returns the time neccesary to add first unit of the material of source during refgeneration.
  double GetFirstRegenerationTime() const { return time_of_first_regeneration;};  
  //!< Returns way of mining (extern=false, intern=true).
  bool IsInsideMining() const { return inside_mining;};

  //!<Sets the material which the source offers.
  void SetOfferMaterial(T_BYTE mat) {offer_material = mat;};
  //!<Sets the renewability of the source.
  void SetRenewability(bool renew) { renewable = renew;};
  //!<Sets the time of regenerating of one unit of the material.
  void SetRegenerationTime(double const time) { time_of_regeneration = time;};
  //!<Sets the time, which is necessary to add first unit of the material.
  void SetFirstRegenerationTime(double const time) { time_of_first_regeneration = time;};
  //!<Sets way of mining of source (extern=false, intern=true).
  void SetInsideMining(bool ins) {inside_mining = ins;};
  //!<Sets if unit can walk on source or building can be built on source when is empty.
  void SetHideable(bool hide) {hideable = hide;};
  //!<Tests whether position is available for this kind of the building.
#ifdef __GNUC__
	bool IsPositionAvailable(int pos_x, int pos_y);
#else
  bool TSOURCE_ITEM::IsPositionAvailable(int pos_x, int pos_y);
#endif

  //! Specifies on which terrains it is able to build the item.
  TINTERVAL<TTERRAIN_ID> buildable[DAT_SEGMENTS_COUNT];

  /** @return The method returns true if the position is possible available. */
  virtual bool IsPossibleAvailablePosition(const T_SIMPLE t_pos_x, const T_SIMPLE t_pos_y, const T_SIMPLE t_pos_seg);
  

private:
  int capacity;                       //!< Amount of material, that can be mined from this source.
  T_BYTE offer_material;              //!< Material, which the source offers for mining.

  double time_of_regeneration;        //!< How long does it take to renew one unit of material of the source.
  double time_of_first_regeneration;  //!< How long does it take to renew first unit of material of the source.

  bool renewable;                     //!< Is the source able to renew itself?.

  bool inside_mining;                 //!< Worker mine source from outside = true (trees) or from inside=false (mine).
  bool hideable;                      //!< Units can walk on source or byuilding can be built on source when source is empty.
  
public:
  //! Constructor.
  TSOURCE_ITEM():TMAP_ITEM() {
    int i;
    
    capacity = 0;
    item_type = IT_SOURCE; 
    time_of_regeneration = time_of_first_regeneration =  0;
    exist_segments.min = exist_segments.max = 0;
    offer_material = (T_BYTE) -1; // max. T_BYTE
    renewable = false;
    inside_mining = false;
    hideable = false;

    for (i = 0; i < DAT_SEGMENTS_COUNT; i++) {
      buildable[i].min = buildable[i].max = 0;
    }
  };
};


/**
 *  Class contains information about produceable kinds of units and about time requested to produce.
 */
class TPRODUCEABLE_NODE {
private:
  TFORCE_ITEM *pitem;       //!< Pointer to produceable item.
  double produce_time;      //!< Time needence for product the item.
  TPRODUCEABLE_NODE *next;  //!< Pointer to next node in the list.
  TPRODUCEABLE_NODE *prev;  //!< Pointer to prev node in the list.

public:
  //!< Constructor. If initialize values from parameters aren't correct sets values to NULL and zero.
  TPRODUCEABLE_NODE(TFORCE_ITEM *item, double time, TPRODUCEABLE_NODE *node);
  /** Returns pointer to produceable item.*/
  TFORCE_ITEM* GetProduceableItem() const
    { return pitem;};
  bool SetProduceableItem(TFORCE_ITEM* item);       //!< Sets pointer to produceable item.

  /** Returns time necessary to build the unit. */
  double GetProduceTime() const 
    { return produce_time;};
  /** Sets time necessary to produce unit but only if new value is positive. If isn't doesn't change
   *  old value. Returns true if the new value is positive otherwise returns false.
   *  @param time New value of the time necessary to produce unit. */
  bool SetProduceTime(const double time)
    { if (time > 0.0) {produce_time = time; return true;} else return false;};

  /** Sets pointer to next node.
   *  @param node New next node. */
  void SetNextNode(TPRODUCEABLE_NODE* const node)
    { next = node;};
  /** Sets pointer to previous node.
   *  @param node New previous node. */
  void SetPrevNode(TPRODUCEABLE_NODE* const node)
    { prev = node;};
  /** Retruns pointer to next node. */
  TPRODUCEABLE_NODE* GetNextNode() const
    { return next;};
  /** Retruns pointer to previous node. */
  TPRODUCEABLE_NODE* GetPrevNode() const
    { return prev;};
};


/**
 *  Class is interface for the list of the produceable items.
 */
class TLIST_OF_PRODUCTS {
public:
  /** Constructor. Only zeroize attributs.*/
  TLIST_OF_PRODUCTS()
    { first = last = NULL; count = 0;};
  /** Constructor. Sets first node to node from parameter and counts lenght of the list.
   *  @param node Future first node in the list. */
  TLIST_OF_PRODUCTS(TPRODUCEABLE_NODE *node) : first(node), last(node)
    { CountNodes();};
  /** Destructor. Destroies the list and frees memory allocated for the nodes.*/
  ~TLIST_OF_PRODUCTS()
    { DestroyList();};

  /** Destroies the list and frees memory allocated for the nodes.*/
  void DestroyList()
    { for (TPRODUCEABLE_NODE *aux = first; aux; aux = first) {first = first->GetNextNode(); delete aux;} count = 0;};

  /** Adds new node to the end of the list. 
   *  @param new_node Added node. */
  void AddProduceableItem(TPRODUCEABLE_NODE *new_node)
    { if (new_node) {new_node->SetPrevNode(last); if (!first) first = new_node; last = new_node; count++;}};
  /** Creates new node at the end of the list from parameters.
   *  Returns true if is adding successful. Otherwise returns false.
   *  @param item Pointer to produceable item. 
   *  @param time Produce time.*/
  bool AddProduceableItem(TFORCE_ITEM *item, double time)
    { TPRODUCEABLE_NODE *aux = NEW TPRODUCEABLE_NODE(item, time, last);
      if (aux) {if (!first) first = aux; last = aux; count++; return true;} else return false;};

  /** Returns first node of the list. */
  TPRODUCEABLE_NODE *GetFirstNode() const
    { return first;};
  /** Sets first node of the list. Destroies old list before.
   *  @param node New first node of the list.*/
  void SetFirstNode(TPRODUCEABLE_NODE *node)
    { if (first) DestroyList(); first = last = node; CountNodes();};

  /** Returns count of the nodes in the list.*/
  int GetCount() const
    { return count;};

  /** Returns node that is n-th in the list. First node of the list is first.
   *  Returns pointer to node if number is in the list. Otherwise return NULL.
   *  @param n  Ordinal number of the node. */
  TPRODUCEABLE_NODE* GetNthNode(int n) const
    { if ((n > 0) && (n <= count)) {TPRODUCEABLE_NODE *aux = first; for (n--; n; n--) aux = aux->GetNextNode(); return aux;}
      else return NULL;};

  /** Returns pointer to node of the list with pointer to item same as is in the parameter.
   *  If this node doesn't exist return NULL.
   *  @param pfitem Pointer to the seeking force item.*/
  TPRODUCEABLE_NODE* GetNodeWithItem(TFORCE_ITEM *pfitem) const
    { for (TPRODUCEABLE_NODE* aux = first; aux; aux = aux->GetNextNode()) 
        if (aux->GetProduceableItem() == pfitem) return aux;
      return NULL;};

private:
  /** Counts nodes in the list. Returns counts. */
  int CountNodes()
    { count = 0; for (TPRODUCEABLE_NODE *aux = first; aux; aux = aux->GetNextNode()) count++; return count;};

  TPRODUCEABLE_NODE *first;     //!< The first node of the list.
  TPRODUCEABLE_NODE *last;      //!< The last node of the list.
  int count;                    //!< Count of the nodes in the list.
};


/** 
 *  Class contains attributs and methods of the buildings that can produce force units.
 */
class TFACTORY_ITEM : public TBUILDING_ITEM {
public:
  /** Returns reference to products list.*/
  TLIST_OF_PRODUCTS& GetProductsList()
    {return products;};
  
  //! Constructor.
  TFACTORY_ITEM():TBUILDING_ITEM()
    { item_type = IT_FACTORY; can_build = true; };

  //! Destructor.
  virtual ~TFACTORY_ITEM();

private:
  TLIST_OF_PRODUCTS products;   //!< The list of the force units that are productable in the building;
};


/**
 *  Structure containing information about races. It is implemented as a linked
 *  list.
 */
struct TRACE {
  TRAC_NAME name;               //!< Race name.
  TRAC_NAME id_name;            //!< Name of race file (uniform ID).
  TAUTHOR author;               //!< Author of race.
  int tg_food_id;               //!< Textures for food.
  int tg_energy_id;             //!< Textures for energy.
  int tg_burning_id;            //!< Textures for burn over unit.

  TFORCE_ITEM **units;          //!< Units table.
  TBUILDING_ITEM **buildings;   //!< Buildings table.
  TSOURCE_ITEM **sources;       //!< Sources table.

  int units_count;              //!< Count of units.
  int buildings_count;          //!< Count of buildings.
  int sources_count;            //!< Count of sources.

  TRACE *next;                  //!< Pointer to next race.
  int workers_item_count[SCH_MAX_MATERIALS_COUNT];  //!Number of types of workers

  TTEX_TABLE tex_table;         //!< Table of textures.

#if SOUND
  TSND_TABLE snd_table;         //!< Table of sounds.

  TSND_GROUP snd_error;         //!< Sound of error action.
  TSND_GROUP snd_placement;     //!< Sound of building placement before construction.
  TSND_GROUP snd_construction;  //!< Sound of building construction.
  TSND_GROUP snd_burning;       //!< Default sound of unit / building in burn.
  TSND_GROUP snd_dead;          //!< Default sound of unit dead.
  TSND_GROUP snd_explosion;     //!< default sound of building explosion.
  TSND_GROUP snd_building_selected;   //<! Default sound for building selection.
#endif

  /** Constructor. Initializes all variables. */
  TRACE() {
    *name = *id_name = *author = 0;
    units = NULL; buildings = NULL; sources = NULL;
    units_count = buildings_count = sources_count = 0;
    next = NULL;
    tg_food_id = tg_energy_id = tg_burning_id = -1;
    for (int i = 0; i < SCH_MAX_MATERIALS_COUNT; i++) {
      workers_item_count[i] = 0;
    }
  }
  ~TRACE();

  //!< Function returns TBASIC_ITEM which is necessary to build target.
  TBASIC_ITEM * FindProduct(TBASIC_ITEM * target);
  //!< Function returns list of workers which can mine material given as parameter.
  TLIST<TWORKER_ITEM> * FindWorker(T_BYTE material);
  //!< Function returns list of units which can build unit given as parameter.
  TLIST<TBASIC_ITEM> * FindBuilders(TBASIC_ITEM * unit);
  //!< Function returns list of units which can repair unit given as parameter.
  TLIST<TBASIC_ITEM> * FindRepairers(TBASIC_ITEM * unit);
  //!< Function returns array and count of buildings according to input conditions.
  int FindBuilding(int * result, int * mat, int * shot_seg, int need_food, int need_energy, TPOSITION start_pos, TPOSITION end_pos);
  //!< Function returns array and count of units according to input conditions.
  int FindUnit(int * result, int * mat, int * shot_seg, int need_food, int need_energy, TPOSITION start_pos, TPOSITION end_pos);
};


//=========================================================================
// Macros
//=========================================================================

#define RacCriticalTable(race, table)    Critical(LogMsg("Can not allocate memory for '%s' %s table", race->name, table))


//=========================================================================
// Variables
//=========================================================================

extern TRACE  *races;             // races


//=========================================================================
// Functions
//=========================================================================

bool LoadRace(char *file_name, bool hyper_player);
bool LoadRaces(void);
void DeleteRaces(void);
int GetItemPrgID(char * usr_id, TMAP_ITEM **table, int count);
char GetSegmentID(int user_terrID, int to_where);
bool IsValidForceItem(TFORCE_ITEM* item);           //!< Tests whether it is valid pointer to force item of any race.


#endif  // __doraces_h__

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

