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
 *  @file dofile.h
 *
 *  Working with configuration files.
 *
 *  @author Peter Knut
 *  @author Martin Kosalko
 *
 *  @date 2003
 */


#ifndef __dofile_h__
#define __dofile_h__

//========================================================================
// Forward declarations
//========================================================================

class TFE_LINE;
class TFE_ITEM;
class TFE_SECTION;
class TCONF_FILE;


//========================================================================
// Definitions
//========================================================================

/** Maximum length of line in configuration file. */
#define FILE_MAX_LINE_LENGTH      1024
/** Maximum length of configuration file name. */
#define FILE_MAX_NAME_LENGTH      128


/**
 *  Default indent string.  */
#define FILE_DEF_INDENT           "  "


// types of file entries

/** Simple line (empty line or comment). */
#define FE_LINE     0
/** Line that cotains item information. */
#define FE_ITEM     1
/** Special entry of the file that contains list of more entries. */
#define FE_SECTION  2


//========================================================================
// Included files
//========================================================================

#include "cfg.h"
#include "doalloc.h"

#include "dodata.h"
#include "dologs.h"
#include "dosimpletypes.h"


//========================================================================
// Typedefs
//========================================================================

/**
 *  Type of the file name.
 */
typedef char TFILE_NAME[FILE_MAX_NAME_LENGTH];
/**
 *  Type of the file line.
 */
typedef char TFILE_LINE[FILE_MAX_LINE_LENGTH];


//========================================================================
// Classes
//========================================================================

/**
 *  Basic class of configuration file entry.
 *  It is used for empty lines and comments.
 *
 *  @par Example:
 *  @verbatim # This is a comment line.           <- comment @endverbatim
 */
class TFE_LINE {
public:
  int type;                   //!< Type of file entry [FL_...].
  int line_num;               //!< Line number in file.
  int indent;                 //!< Entry indent.
  
  char *values;               //!< String of all values.
  char *act_value;            //!< Pointer to actual value.

  TCONF_FILE *conf_file;      //!< Configuration file in which is this entry occured.

  TFE_LINE *next;             //!< Next file entry.
  TFE_LINE *prev;             //!< Previous file entry.

  TFE_SECTION *GetOwner(void);

  void WriteIndent(void);
  virtual void Write(void);
  void ResetValue() {act_value = values;};
  
  TFE_LINE(TFE_SECTION *powner, char *line);
  virtual ~TFE_LINE(void);

private:
  TFE_SECTION *owner;         //!< Reference to owner section.
};


/**
 *  File entry that contains item declaration.
 *
 *  @par Syntax of item:
 *  @c item_name @e param1 [@e param2, ...]
 *  
 *  @par Notes:
 *  @c item_name is only one word.\n
 *  @e param should be enclosed in double quotes, when containing strings.
 *
 *  @par Examples:
@verbatim
 radius 125                          <- integer value
 fullscreen true                     <- boolean value
 show_fps 1                          <- boolean value
 player_name "Random Player"         <- string value
 map_data "Trial map" 40 80 true     <- mixed parameters
@endverbatim
 */
class TFE_ITEM: public TFE_LINE {
public:
  char *name;                           //!< Item name.
  /**
   *  Specifies, if some value was writen into the item. When it is @c true,
   *  other values will be added when using some of the Write... functions
   *  family.
   */
  bool modified;

  virtual void Write(void);

  void WriteValue(char *value);
  void SetValue(char *value);
  void AddValue(char *value);
  void ClearValues();
  
  TFE_ITEM(TFE_SECTION *powner, char *iname, char *ivalue, bool modify);
  ~TFE_ITEM(void);
};


/**
 *  Special entry of the file that contains list of more file entries.
 *  @par Syntax of section:
 *  @c \<section_name> [...] @c \</section_name>
 *
 *  @note Open tag <...> and close tag </...> of section must be alone in one
 *        line.
 *  @note @c section_name can contain more than one word.
 *
 *  @par Example:
@verbatim
 <Player>                            <- begin of the section called Player
   name "Titan"                      <- player items
   size 23
 
   <Gun>                             <- begin of the section called Gun
     type 4                          <- gun items
     ammo 125
   </Gun>                            <- end of Gun section
 
 </Player>                           <- end of Player section
@endverbatim
 */
class TFE_SECTION: public TFE_LINE {
public:

  char *name;                           //!< Section name.

  virtual void Write(void);

  void Clear(void);
  TFE_SECTION *SelectSection(char *name, bool mandatory);

  void DeleteSection(char *section);
  void DeleteItem(char *item);

  void WriteValue(char *item, char *value);
  int  ReadValue(char *value, char *item, bool warning);
  bool SetValue(char *item, char *value);
  void ClearValues(char *item);
  int ResetValue(char *item);

  void AddLoadedValue(char *item, char *value);

  void AddLine(char *text);
  
  TFE_ITEM *GetItem(char *item, bool warn);
  TFE_SECTION *GetSection(char *section);

  TFE_SECTION(TFE_SECTION *powner, char *sname);
  ~TFE_SECTION(void);

private:
  TFE_LINE *fst_entry;          //!< First entry in section.
  TFE_LINE *last_entry;         //!< Last entry in section.

  void AddEntry(TFE_LINE *entry);
};


/**
 *  Configuration file class.
 *  This class contains methods used to reading and writing configuration to file.
 *  Configuration file should contain empty lines, comments, items with parameters
 *  and sections.
 *
 *  @sa TFE_LINE, TFE_ITEM, TFE_SECTION
 */
class TCONF_FILE {
public:
  FILE *fh;                 //!< Handler of associated file.

  TFILE_NAME name;          //!< Whole file name (path/name).
  bool file_exists;         //!< If file exists in filesystem.

  int lines_count;          //!< Count of all lines in file.
  char *indent_string;      //!< Indent string.

  void Clear(void);
  int Reload(void);
  void Save(void);

  bool SelectSection(char *section, bool mandatory);
  void UnselectSection(void);
  void DeleteSection(char *section);
  void DeleteItem(char *item);

  void WriteLine(char *line);

  void WriteStr(char *item, char *value);
  bool ReadStr(char *value, char *item, char *def_value, bool warning);
  
  void WriteBool(char *item, bool value);
  bool ReadBool(bool *value, char *item, bool def_value);

  void WriteInt(char *item, int value);
  bool ReadInt(int *value, char *item, int def_value);
  bool ReadIntGE(int *value, char *item, int min, int def_value);
  bool ReadIntRange(int *value, char *item, int min, int max, int def_value);

  void WriteFloat(char *item, float value);
  bool ReadFloat(float *value, char *item, float def_value);
  bool ReadFloatGE(float *value, char *item, float min, float def_value);
  bool ReadFloatRange(float *value, char *item, float min, float max, float def_value);

  void WriteDouble(char *item, double value);
  bool ReadDouble(double *value, char *item, double def_value);
  bool ReadDoubleGE(double *value, char *item, double min, double def_value);
  bool ReadDoubleRange(double *value, char *item, double min, double max, double def_value);

  void WriteSimple(char *item, T_SIMPLE value);
  bool ReadSimple(T_SIMPLE *value, char *item, T_SIMPLE def_value);
  bool ReadSimpleGE(T_SIMPLE *value, char *item, T_SIMPLE min, T_SIMPLE def_value);
  bool ReadSimpleRange(T_SIMPLE *value, char *item, T_SIMPLE min, T_SIMPLE max, T_SIMPLE def_value);

  void WriteByte(char *item, T_BYTE value);
  bool ReadByte(T_BYTE *value, char *item, T_BYTE def_value);
  bool ReadByteGE(T_BYTE *value, char *item, T_BYTE min, T_BYTE def_value);
  bool ReadByteRange(T_BYTE *value, char *item, T_BYTE min, T_BYTE max, T_BYTE def_value);

  bool ReadTextureGroup(int * value, char *item, TTEX_TABLE * tex_table, bool mandatory, char * section);

#if SOUND
  bool ReadSound(TSND_GROUP * value, char *item, TSND_TABLE * snd_table, char * section);
#endif
  
  void ClearValues(char *item);

  void SetIndentString(char *str);

  TFE_SECTION * GetActSection(void) {return act_section;};

  TCONF_FILE(const char *fname);
  ~TCONF_FILE(void);

private:
  bool modified;              //!< If any changes was made.  

  TFE_SECTION *base_section;  //!< Base section (it contains all entries in the file).
  TFE_SECTION *act_section;   //!< Actual section.

  bool Open(char *attr);
  void Close(void);
};

//========================================================================
// Functions
//========================================================================

TCONF_FILE *CreateConfFile(const char *name);
TCONF_FILE *OpenConfFile(const char *name);
void CloseConfFile(TCONF_FILE *&cf);


#endif // __dofile_h__

//========================================================================
// End
//========================================================================
// vim:ts=2:sw=2:et:

