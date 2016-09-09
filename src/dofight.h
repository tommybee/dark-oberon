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
 *  @file dofight.h        
 *
 *  Armaments classes, fight and combat functions.
 *
 *  @author Jiri Krejsa
 *
 *  @date 2003
 */

#ifndef __dofight_h__
#define __dofight_h__


//=========================================================================
// Forward declarations
//=========================================================================

struct TGUN_POWER;
class TGUN;
class TDEFENSE;
struct TATTACK_INFO;
class TARMAMENT;
class TPROJECTILE_ITEM;
class TMAP_UNIT;
class TFORCE_UNIT;

//=========================================================================
// Definitions
//=========================================================================

// TGUN flags
#define FIG_GUN_NOTHING           0       //!< No special properties of the attack.
#define FIG_GUN_SAME_SEGMENT      1       //!< Flag informates that attack can be done only in the segment where is attacker.
#define FIG_GUN_DAMAGE_BUILDINGS  2       //!< Flag informates that attack damaged buildings. Without it attacking building doesn't take any efect.
#define FIG_GUN_DAMAGE_SOURCES    4       //!< Flag informates that attack damaged sources of materials. Without it attacking sources of materials doesn't take any efect.
#define FIG_GUN_POSITION_ATTACK   8       //!< Flag informates that is possible to attack the position in the map without occupancy by map unit.
#define FIG_GUN_NOTLANDING        16      //!< Flag informates that is possible to attack only if the unit isn't landing.

// TATTACK_INFO states
#define FIG_AIF_ATTACK_OK           0       //!< Run of attack is O.K.
#define FIG_AIF_ATTACK_FAILED       1       //!< Flag informates that attack failed.
#define FIG_AIF_TO_UPPER_SEG        2       //!< Attack is possible only in the upper segment.
#define FIG_AIF_TO_LOWER_SEG        3       //!< Attack is possible only in the lower segment.
#define FIG_AIF_TOO_FAR_AWAY        4       //!< Attacker is too far away from the target.
#define FIG_AIF_TOO_CLOSE_TO        5       //!< Attacker is too close to the target.
#define FIG_AIF_UNSHOTABLE_SEG      6       //!< Target is in the unshotable segment.
#define FIG_AIF_START_FIRST         7       //!< Attacker have to stop landing first.
#define FIG_AIF_TURN_LEFT           8       //!< Attacker have to turn left first.
#define FIG_AIF_TURN_RIGHT          9       //!< Attacker have to turn right first.
#define FIG_AIF_ATTACT_NOT_EFFECIVE 10      //!< Attack does not have effect.

//=========================================================================
// Included files
//=========================================================================

#include <cstdio>

#include "cfg.h"
#include "doalloc.h"

#include "dolayout.h"
typedef float TNEURON_VALUE;


//=========================================================================
// Type definitions
//=========================================================================

/**
 *  Structure includes information about gun power.
 */
struct TGUN_POWER {
  int min;        //!< Minimum gun power.
  int max;        //!< Maximum gun potential power.
  TGUN_POWER()    //!< Constructor.
    {min = max = 0;};
};


/**
 *  Class with information and methods required during offensive phase of the
 *  fight.
 *  Accuracy of gun is number from interval (0;1)
 *
 *  @sa TGUN_POWER
 */
class TGUN {
public:
  float GetAccuracy() const         //!< Returns accuracy of gun.
    {return accuracy;};
  /** Sets accuracy to value of the parameter if it is between zero and one.
  * If it is greater one sets to one. If it is lower then zero sets to zero.
  * @param new_acc  Value of the new accuracy.*/
  void SetAccuracy(const float new_acc) 
    {if (new_acc >= 1.0f) accuracy = 1.0f; else if (new_acc <= 0) accuracy = 0; else accuracy = new_acc;};
  /** Increase/decrease accuracy according to the parameter. Returns new value of accuracy.
  * @param change Value of change.*/
  float ChangeAccuracy(const float change) 
    {accuracy += change; if (accuracy < 0) accuracy = 0; else if (accuracy > 1.0f) accuracy = 1.0f; return accuracy;};
 
  unsigned char GetFlags() const    //!< Returns flags of attack.
    {return flags;};
  /** Sets flags of attack to value from parameter.
  * @param new_flags  New value of flags.*/
  void SetFlags(const unsigned char new_flags) 
    {flags = new_flags;};
  /** Adds flag of attack from parameter. Its possible add more then one flag in the one time.
   *  @param added Added flags. */
  void AddFlag(const unsigned char added) 
    {flags |= added;};
  /** The method tests whether specified flags are settedd.
   *  @return The method returns true if flags are setted otherwise returns false.*/
  bool TestFlags(const unsigned char tested)
    { return ((flags & tested) != 0);}

  TGUN_POWER GetPower() const       //!< Returns power of the gun.
    {return power;};

  TINTERVAL<int>& GetRange()        //!< Returns range of the gun.
    {return range;};

  TINTERVAL<T_SIMPLE>& GetShotableSegments()       //!< Returns reference to shotable segments of the gun.
    {return shotableSeg;};
  /** Sets upper limit of shotable segments.
  * @param u  Upper limit of shotable segments.*/
  void SetUpperShotableLimit(const T_SIMPLE u) 
    {shotableSeg.max = u;};
  /** Sets bottom limit of shotable segments.
  * @param b  Bottom limit of shotable segments.*/
  void SetBottomShotableLimit(const T_SIMPLE b) 
    {shotableSeg.min = b;};

  void SetPowerMin( int new_min)    //!< Sets gun minimal power
    {power.min = new_min;};

  void SetPowerMax( int new_max)    //!< Sets gun maximal potential
    {power.max = new_max;};

  double GetFeedTime() const        //!< Returns time of feed.
    {return feed_time;};

  void SetFeedTime(double new_feed) //!< Sets time of feed.
    {feed_time = new_feed;};

  double GetShotTime() const        //!< Returns shot time
    {return shot_time;};

  void SetShotTime(double new_st)   //!< Sets shot time
    {shot_time = new_st;};

  double GetWaitTime() const        //!< Returns wait time between shot and feef
    {return wait_time;};

  void SetWaitTime(double new_wt)   //!< Sets wait time between shot and feed
    {wait_time = new_wt;};

  void SetRange_min (int min)       //!< Sets gun min range
    {range.min = min;};

  void SetRange_max (int max)       //!< Sets gun max range
    {range.max = max;};

  void ExecuteAttack(TMAP_UNIT *) const;      //!< Execute attack to unit sended in parameter.

  /** @return The method returns pointer to item of the gun projectile.*/
  TPROJECTILE_ITEM* GetProjectileItem() const
    { return item;};
  /** The methods sets pointer to item of the gun projectile.
   *  @param nitem New pointer to item of the gun projectile.*/
  void SetItem(TPROJECTILE_ITEM *nitem);

  TGUN();
  ~TGUN();

private:
  float accuracy;         //!< Accuracy of gun.
  unsigned char flags;    //!< Flags informing about special properties of attack.
  TGUN_POWER power;       //!< Gun power.
  TINTERVAL<int> range;   //!< Gun-range. [mapels]
  double feed_time;       //!< Time needed for feed [seconds].
  double shot_time;       //!< Time needed for fire on shot [seconds].
  double wait_time;       //!< Time between shot and feed [seconds].
  TINTERVAL<T_SIMPLE> shotableSeg;  //!< Interval of segments available for shoting (attacking) [segment].
  TPROJECTILE_ITEM *item; //!< Pointer to item of the projectile.
};


/**
 *  Class with information and methods required during defensive phase of the
 *  fight.
 */
class TDEFENSE {
public:
  //! Returns value of armour.
  int GetArmour()
    {return armour;};
  /** Sets armour to value from parameter. If the parameter is lower then zero
   *  sets armour to zero.
   *  @param new_armour Value of new armour.*/
  void SetArmour(const int new_armour) {if (new_armour >= 0) armour = new_armour; else armour = 0;};
  /** Changes the armour by value of @p change. If resulting value is
   *  lower then zero sets armour to zero.
   *  @param change Value of change.
   *  @return Value of armour after change. */
  int ChangeArmour(const int change) {armour += change; if (armour < 0) armour = 0; return armour;};

  //! Returns value of protection.
  float GetProtection()
    {return protection;};
  /** Sets protection to value of the parameter if it is between zero and one.
   *  If it is greater one sets to one. If it is lower then zero sets to zero.
   *  @param new_value  New value of protection. */
  void SetProtection(const float new_value)
    {if (new_value > 1.0f) protection = 1.0f; else if (new_value < 0) protection = 0; else protection = new_value;};

#ifdef __GNUC__
	TDEFENSE() { armour = 0; protection = 0; }
#else
  TDEFENSE::TDEFENSE() { armour = 0; protection = 0; }
#endif

private:
  int armour;             //!< Armour strength.
  float protection;       //!< Ability to dodge.
};


/**
 *  Structure carries run information of the attack.
 */
struct TATTACK_INFO {
  TPOSITION_3D impact_position;       //!< Position of the impact.
  unsigned char state;                //!< State of the attack. For example: attack failed.
  signed char direction;              //!< Direction of attack.

  TATTACK_INFO(): state(FIG_AIF_ATTACK_FAILED), direction(LAY_UNDEFINED) {};               //!< Constructor.
};


/**
 *  Class contains information and methods required for all fight phases.
 *
 *  @sa TDEFENSE, TGUN, TPROJECTILE
 */
class TARMAMENT {
public:
  //!< Tests whether is possible attack unit.
  TATTACK_INFO IsPossibleAttack(const TMAP_UNIT* attacker, const TMAP_UNIT* defender, const bool only_test = false) const;
  TGUN* GetOffensive() const {return offensive;};         //!< Returns offensive.
  /** Sets pointer to the gun. Dealocate old gun.
  * @param gun  Pointer to the new gun.*/
  void SetOffensive(TGUN *gun) {if (offensive) delete offensive; offensive = gun;};
  TDEFENSE* GetDefense() const {return defense;};         //!< Returns defense.
  /** Sets pointer to the defense. Dealocate old instance of the TDEFENSE class.
  * @param def  Pointer to the new defense instance of the TDEFENSE class.*/
  void SetDefense(TDEFENSE *def) {if (defense) delete defense; defense = def;};

  TNEURON_VALUE GetWorth() {return worth;};   //!< Returns value
  void SetWorth(TNEURON_VALUE val) {worth = val;};  //!< Sets value

  //!< Basic constructor.
  TARMAMENT();
  /** Constructor that sets pointers to gun and defense of the armament.
  * @param gun  Pointer to gun of the armament.*
  * @param def  Pointer to defense of the armament. */
  TARMAMENT(TGUN* gun, TDEFENSE* def) {offensive = gun; defense = def; worth = 0;};
  ~TARMAMENT()            //!< Destructor.
    {if (offensive) delete offensive; if (defense) delete defense;};
 
private:  
  /** Calculate best impact positon and involve impact deviation.*/
  void CalculateImpactPosition(const TMAP_UNIT* attacker, const TMAP_UNIT* defender, TATTACK_INFO &info, const int segment_distance) const;
  /** Calculate real impact position.*/
  void CalculateRealImpactPosition(TATTACK_INFO &attack_info, const float dist) const;

  /** The method tests equality of the attacker look direction and shot direction.*/
  void TestDirection(const TFORCE_UNIT *attacker, TATTACK_INFO &info) const;

  TGUN *offensive;        //!< Offensive phase.
  TDEFENSE *defense;      //!< Defensive phase.
  TNEURON_VALUE worth;    //!< Value get from neuron network
};


#endif  //__dofight_h__

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

