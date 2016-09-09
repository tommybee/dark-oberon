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
 *  @file dofile.cpp
 *
 *  Common methods.
 *
 *  @author Peter Knut
 *  @author Martin Kosalko
 *
 *  @date 2003, 2004, 2005
 */

#include <stdlib.h>
#include <string.h>
#include <string>

#include "dofile.h"

using std::string;


//=========================================================================
// Chop, GetWord
//=========================================================================

/**
 *  Converts string to integer value.
 *
 *  @param value Pointer to integer variable, result of covertation will be stored here.
 *  @param str   String that will be converted.
 *
 *  @note If conversion fails, parameter @p value will not be modified.
 *
 *  @return @c true on success, otherwise @c false.
 */
bool StringToInt(int *value, char *str)
{
  int v;
  bool ok = true;

  v = atoi(str);  // simple convertation
  if (!v) {       // if v = 0, check if str is zero indeed
    ok = (str[0] == '0' || str[0] == '+' || str[0] == '-');
    for (int i = 1; str[i] && ok; i++) ok = (str[i] == '0');
  }

  if (ok) *value = v;

  return ok;
}

/**
 *  Converts string to float value.
 *
 *  @param value Pointer to float variable, result of covertation will be stored here.
 *  @param str   String that will be converted.
 *
 *  @note If conversion fails, parameter @p value will not be modified.
 *
 *  @return @c true on success, otherwise @c false.
 */
bool StringToFloat(float *value, char *str)
{
  float v;
  bool ok = true;

  v = (float)atof(str);  // simple convertation
  if (!v) {       // if v = 0, check if str is zero indeed
    ok = (str[0] == '0' || str[0] == '+' || str[0] == '-');
    for (int i = 1; str[i] && ok; i++) ok = (str[i] == '0');
  }

  if (ok) *value = v;

  return ok;
}


/**
 *  Converts string to double value.
 *
 *  @param value Pointer to double variable, result of covertation will be stored here.
 *  @param str   String that will be converted.
 *
 *  @note If conversion fails, parameter @p value will not be modified.
 *
 *  @return @c true on success, otherwise @c false.
 */
bool StringToDouble(double *value, char *str)
{
  double v;
  bool ok = true;

  v = atof(str);  // simple convertation
  if (!v) {       // if v = 0, check if str is zero indeed
    ok = (str[0] == '0' || str[0] == '+' || str[0] == '-');
    for (int i = 1; str[i] && ok; i++) ok = (str[i] == '0');
  }

  if (ok) *value = v;

  return ok;
}


/**
 *  Removes empty chars (with ASCII code lower or equal to 32) from string
 *  begin and end.
 *
 *  @param line String that will be chopped.
 */
void Chop(char *line)
{
  char *p;

  // remove empty chars from line end
  if (*line) {
    // move to line end
    for (p = line + strlen(line); p != line && *p <= 32; p--);
    // remove empty chars 
    *(p + 1) = 0;
  }

  // move to line begin
  for (p = line; *p && *p <= 32; p++);

  // move line content
  if (p != line) {
    string tmp(p);
    strcpy(line, tmp.c_str ());
  }
}


/**
 *  Separates first word from line.
 *
 *  @param word First word will be stored here.
 *  @param line Pointer to string of words. Will be moved to the beginnig of the next word.
 *
 *  @note If separation failed, parameters will not be modified.
 *
 *  @return @c true if non-empty word was separated, otherwise @c false.
 */
bool GetWord(char *word, char **line)
{
  if (!*line) return false;

  char *p, *q;
  char c;

  bool in_dq = false;     // in "..."
  bool out_dq = false;    // out "..."

  bool in_tag = false;    // in <...>
  bool out_tag = false;   // out <...>

  // move to word begin
  for (p = *line; *p && *p <= 32; p++);

  // first double quote occured
  if (*p == '"' && !in_tag) {
    p++;
    in_dq = true;
  }

  // first char of tag occured
  else if (*p == '<' && !in_dq) {
    p++;
    in_tag = true;
  }
  
  if (*p) {
    // move to str end
    for (q = p; *q && (*q > 32 || in_dq || in_tag); q++) {
      if (*q == '"' && in_dq) {   // second double quote occured
        out_dq = true;
        break;   
      }
      if (*q == '>' && in_tag) {  // end char of tag occured
        out_tag = true;
        break;   
      }
    }

    // separate and copy found word to word
    c = *q;
    *q = 0;
    strcpy(word, p);
    *q = c;

    if (out_dq || out_tag) q++;   // move behind double quote or '>'
    for (; *q && *q <= 32; q++);  // move to next word begin
    *line = q;                    // set line to the begin of next word

    return true;
  } // if
  else return false;
}


//=========================================================================
// Classe TFE_LINE
//=========================================================================

/**
 *  Class constructor.
 *
 *  @param powner pointer to owner section, to which this entry belongs.
 *  @param line   line content.
 */
TFE_LINE::TFE_LINE(TFE_SECTION *powner, char *line)
{
  // initialize variables
  owner = powner;
  if (owner) {
    conf_file = owner->conf_file;
    indent = owner->indent + 1;
    line_num = conf_file->lines_count;
  }
  else {
    conf_file = NULL;
    indent = -1;
    line_num = 0;
  }

  type = FE_LINE;
  
  next = NULL;
  prev = NULL;

  // initialize values
  if (line) {
    values = NEW char[strlen(line) + 1];
    strcpy(values, line);
    act_value = values;
  }
  else values = act_value = NULL;
}


/**
 *  Class Destructor.
 */
TFE_LINE::~TFE_LINE(void)
{
  if (values) delete []values;
  values = NULL;
}


/**
 *  Gets handler to owner section.
 *
 *  @return Handler to owner section.
 */
TFE_SECTION *TFE_LINE::GetOwner(void)
{
  return owner;
}


/**
 *  Writes line content into file.
 */
void TFE_LINE::WriteIndent(void)
{
  if (indent) for (int i = 0; i < indent; i++)
    fprintf(conf_file->fh, conf_file->indent_string);
}


/**
 *  Writes line content into file.
 */
void TFE_LINE::Write(void)
{
  WriteIndent();

  if (values) fprintf(conf_file->fh, values);
  fprintf(conf_file->fh, "\n");
}


//=========================================================================
// Classe TFE_ITEM
//=========================================================================

/**
 *  Class constructor.
 *
 *  @param powner Pointer to owner section, to which this entry belongs.
 *  @param iname  Item name.
 *  @param ivalue Item value stored as a string.
 *  @param modify Whether it should be initialized as modified.
 */
TFE_ITEM::TFE_ITEM(TFE_SECTION *powner, char *iname, char *ivalue, bool modify)
: TFE_LINE(powner, NULL)
{
  // initialize name
  name = NEW char[strlen(iname) + 1];
  strcpy(name, iname);

  // initialize values
  values = NEW char[strlen(ivalue) + 1];
  strcpy(values, ivalue);
  act_value = values;
  modified = modify;

  type = FE_ITEM;
}


/**
 *  Class Destructor.
 */
TFE_ITEM::~TFE_ITEM(void)
{
  if (name) delete []name;
}


/**
 *  Set item value.
 *
 *  @param value new item value.
 *
 *  @note All previous values will be deleted.
 */
void TFE_ITEM::SetValue(char *value)
{
  // delete old values
  if (values) delete values;

  // initialize new values
  values = NEW char[strlen(value) + 1];
  strcpy(values, value);

  // reset actual value
  act_value = values;

  modified = true;
}


/**
 *  Add item value.
 *
 *  @param value new item value.
 *
 *  @note Previous values will not be deleted.
 */
void TFE_ITEM::AddValue(char *value)
{
  TFILE_LINE newval;
  int dv;

  if (values) {
    // copy all old values and new value to newval
    sprintf(newval, "%s %s", values, value);
    dv = act_value - values;

    // delete old values
    delete values;
  }
  else {
    // copy new value to newval
    strcpy(newval, value);
    dv = 0;
  }

  // initialize all values together
  values = NEW char[strlen(newval) + 1];
  sprintf(values, "%s", newval);

  // set actual value to previous actual value
  act_value = values + dv;

  modified = true;
}


/**
 *  Write item value.
 *
 *  @param value new item value.
 *
 *  @note First value vill be set any other will be added.
 */
void TFE_ITEM::WriteValue(char *value)
{
  if (modified) AddValue(value);
  else SetValue(value);
}


/**
 *  Clears all item values.
 */
void TFE_ITEM::ClearValues(void)
{
  // delete values
  if (values) delete values;
  values = NULL;

  modified = true;
}


/**
 *  Writes item into file.
 */
void TFE_ITEM::Write(void)
{
  WriteIndent();

  fprintf(conf_file->fh, "%s ", name);
  if (values) fprintf(conf_file->fh, values);
  fprintf(conf_file->fh, "\n");

  modified = false;
}


//=========================================================================
// Class TFE_SECTION
//=========================================================================

/**
 *  Class constructor.
 *
 *  @param powner pointer to owner section, to which this entry belongs.
 *  @param sname  section name.
 */
TFE_SECTION::TFE_SECTION(TFE_SECTION *powner, char *sname)
: TFE_LINE(powner, NULL)
{
  type = FE_SECTION;

  // initialize name
  if (!sname) name = NULL;
  else {
    name = NEW char[strlen(sname) + 1];
    strcpy(name, sname);
  }

  fst_entry = last_entry = NULL;
}


/**
 *  Class destructor.
 */
TFE_SECTION::~TFE_SECTION(void)
{
  Clear();    // clear all entries

  if (name) delete []name;
}


/**
 *  Writes whole section into file inluded with all entries in it.
 */
void TFE_SECTION::Write(void)
{
  if (name) {
    WriteIndent();
    fprintf(conf_file->fh, "<%s>\n", name);   // begin tag of section
  }

  // write all section entries
  for (TFE_LINE *line = fst_entry; line; line = line->next) line->Write();

  if (name) {
    WriteIndent();
    fprintf(conf_file->fh, "</%s>\n", name);  // end tag of section
  }
}


/**
 *  Deletes all section entries.
 */
void TFE_SECTION::Clear(void)
{
  TFE_LINE *entry = fst_entry;

  // while is there any entry, delete it
  while (entry) {
    fst_entry = fst_entry->next;
    delete entry;
    entry = fst_entry;
  }
}


/**
 *  Finds and returns section.
 *
 *  @param section section name.
 *
 *  @return Handler to section on success, otherwise @c NULL.
 */
TFE_SECTION *TFE_SECTION::GetSection(char *section)
{
  TFE_LINE *bl;

  for (bl = fst_entry; bl; bl = bl->next) {
    if (bl->type == FE_SECTION && !strcmp(((TFE_SECTION *)bl)->name, section)) break;
  }

  return (TFE_SECTION *)bl;
}


/**
 *  Finds and returns item.
 *
 *  @param item item name.
 *
 *  @return Handler to item on success, otherwise @c NULL.
 */
TFE_ITEM *TFE_SECTION::GetItem(char *item, bool warn)
{
  TFE_LINE *bl;

  for (bl = fst_entry; bl; bl = bl->next) {
    if (bl->type == FE_ITEM && !strcmp(((TFE_ITEM *)bl)->name, item)) break;
  }

  // item is not found
  if (!bl && warn) {
    if (name) Warning(LogMsg("Can not find item '%s' in section <%s> on line %d", item, name, line_num));
    else Warning(LogMsg("Can not find item '%s'", item));
  }

  return (TFE_ITEM *)bl;
}


/**
 *  Gets section handler. If section is not found, new one will be created.
 *
 *  @param name section name.
 *
 *  @return Handler to actual section.
 */
TFE_SECTION *TFE_SECTION::SelectSection(char *s_name, bool mandatory)
{
  TFE_SECTION *sect;

  if (!(sect = GetSection(s_name))) {   // if no section is find
    // write error message
    if (mandatory) {
      if (name) Error(LogMsg("Can not find section <%s> in <%s>", s_name, name));
      else Error(LogMsg("Can not find section <%s>", s_name));
    }

    // create new section
    else {
      sect = NEW TFE_SECTION(this, s_name);
      AddEntry(sect);                   // add new section to list
    }
  }

  return sect;
}


/**
 *  Deletes section with all entries.
 *
 *  @param section section name.
 */
void TFE_SECTION::DeleteSection(char *section)
{
  TFE_SECTION *sect = GetSection(section);

  if (sect) {
    // remove section from list of entries
    if (sect->next) sect->next->prev = sect->prev;
    if (sect->prev) sect->prev->next = sect->next;

    delete sect;
  }
}


/**
 *  Deletes item.
 *
 *  @param item item name.
 */
void TFE_SECTION::DeleteItem(char *item)
{
  TFE_ITEM *it = GetItem(item, false);

  if (it) {
    
    if (it == this->fst_entry) fst_entry = it->next;
    if (it == this->last_entry) last_entry = it->prev;
    
    // remove entry from list of entries
    if (it->next) it->next->prev = it->prev;
    if (it->prev) it->prev->next = it->next;

    delete it;
  }
}


/**
 *  Adds new entry to the end of entry list.
 *
 *  @param entry new entry.
 */
void TFE_SECTION::AddEntry(TFE_LINE *entry)
{
  if (!entry) return;

  // if list is empty
  if (!last_entry) {
    fst_entry = last_entry = entry;
    entry->line_num = 1;
  }

  // if there are some entries in the list
  else {
    last_entry->next = entry;
    entry->prev = last_entry;
    last_entry = entry;

    entry->line_num = entry->prev->line_num + 1;
  }
}


/**
 *  Writes item value.
 *
 *  @param item   item name.
 *  @param value  new item value.
 */
void TFE_SECTION::WriteValue(char *item, char *value)
{
  TFE_ITEM *fi;

  if ((fi = GetItem(item, false))) fi->WriteValue(value);
  else {
    // if no item is find, create new item
    fi = NEW TFE_ITEM(this, item, value, true);
    AddEntry(fi);
  }
}


/**
 *  Sets item value.
 *
 *  @param item   item name.
 *  @param value  new item value.
 */
bool TFE_SECTION::SetValue(char *item, char *value)
{
  TFE_ITEM *fi;

  if ((fi = GetItem(item, false))){
    fi->SetValue(value);
    return true;
  }
  else
    return false;
}


/**
 *  Adds new loaded item with its value.
 *
 *  @param item   item name.
 *  @param value  new item value.
 */
void TFE_SECTION::AddLoadedValue(char *item, char *value)
{
  TFE_ITEM *fi;

  // if no item is find, create new item
  fi = NEW TFE_ITEM(this, item, value, false);
  AddEntry(fi);
}


/**
 *  Reads item's value.
 *
 *  @param value    item value.
 *  @param item     item name.
 *  @param warning  if false, no warning about missing value is written to log.
 *
 *  @return Line number of item on success, 0 otherwise.
 */
int TFE_SECTION::ReadValue(char *value, char *item, bool warning)
{
  TFE_ITEM *fi;
  TFILE_LINE val = "";

  *value = 0;

  if (!(fi = GetItem(item, true))) return 0;

  if (!GetWord(val, &fi->act_value)) {
    // there is no next value to read
    if (warning){
      Warning(LogMsg("Can not find value of item '%s' on line %d", item, fi->line_num));
      return 0;
    }
  }

  strcpy(value, val);

  return fi->line_num;
}


/**
 *  Resetss item's value.
 *
 *  @param item     item name.
 *
 *  @return Line number of item on success, 0 otherwise.
 */
int TFE_SECTION::ResetValue(char *item)
{
  TFE_ITEM *fi;

  if (!(fi = GetItem(item, false))) return 0;

  fi->ResetValue();

  return fi->line_num;
}


/**
 *  Clears the item's value.
 *
 *  @param item Name of the item.
 */
void TFE_SECTION::ClearValues(char *item)
{
  TFE_ITEM *fi;

  if ((fi = GetItem(item, false))) fi->ClearValues();
}


/**
 *  Add line entry.
 *
 *  @param text line content.
 */
void TFE_SECTION::AddLine(char *text)
{
  // create new basic entry
  TFE_LINE *bl = NEW TFE_LINE(this, text);

  if (bl) AddEntry(bl);
}


//=========================================================================
// Class TCONF_FILE
//=========================================================================

/**
 *  Class constructor.
 *
 *  @param fname name of configuration file.
 */
TCONF_FILE::TCONF_FILE(const char *fname)
{
  // initialize variables
  strcpy(name, fname);
  modified = false;
  file_exists = false;

  fh = NULL;
  lines_count = 0;
  
  // create base section
  act_section = base_section = NEW TFE_SECTION(NULL, NULL);
  base_section->conf_file = this;

  indent_string = NULL;
  SetIndentString(const_cast<char*>(FILE_DEF_INDENT));
}


/**
 *  Class destructor.
 */
TCONF_FILE::~TCONF_FILE(void)
{
  Close();  // close (and save) asociated file
  Clear();  // delete all entries
  
  if (indent_string) delete[] indent_string;
  delete base_section;
}


/**
 *  Opens associated file.
 *
 *  @param attr  attributes for fopen function.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool TCONF_FILE::Open(char *attr)
{
  if (!(fh = fopen(name, attr))) {
    // can not open file (probably file does not exist)
    Error(LogMsg("Can not open configuration file '%s' with attributes '%s'", name, attr));
    file_exists = false;

    return false;
  }

  // file is opened, it exists
  file_exists = true;
  return true;
}


/**
 *  Closes associated file. All changes will be saved.
 */
void TCONF_FILE::Close(void)
{
  // if there are any changes, save file
  if (modified) Save();

  // free file handler
  if (fh) {
    fclose(fh);
    fh = NULL;
  }

}


/**
 *  Clears all entries in the file.
 */
void TCONF_FILE::Clear(void)
{
  base_section->Clear();

  act_section = base_section;
}


/**
 *  Reloads entries from file. All previous entries and changes will be lost.
 */
int TCONF_FILE::Reload(void)
{
  // open file for reading
  if (!Open(const_cast<char*>("rt"))) return 0;

  TFILE_LINE line, item;
  char buffer [10*FILE_MAX_LINE_LENGTH];
  char *values, *buff;
  bool done = false;
  bool long_line = false;

  // if there are any entries, clear all
  Clear();

  buff = buffer;
  buff[0] = '\0';
  
  do {
    line[0] = '\0';   // empty string

    // read line from file
    fgets(line, FILE_MAX_LINE_LENGTH, fh);
    Chop(line);
    values = line;  // set pointer to line begin

    if (!*line && feof(fh)){
      done = true;  // end of file is occured
      if (long_line){
        GetWord(item, &buff);
        act_section->AddLoadedValue(item, buff);
      
        long_line = false;
        buff[0] = '\0';
      }
    }
    else {
      // increase line count
      lines_count++;

      if (!*line) WriteLine(const_cast<char*>(""));                      // empty line
      else if (line[0] == '#'){
        WriteLine(line);       // comment
      }
      else if (line[0] == '<' && line[1] != '/') {    // begin of the section
        if (long_line){
          GetWord(item, &buff);
          act_section->AddLoadedValue(item, buff);
        
          long_line = false;
          buff[0] = '\0';
        }

        GetWord(item, &values);
        SelectSection(item, false);
      }
      else if (line[0] == '<') {                      // end of the section
        if (long_line){
          GetWord(item, &buff);
          act_section->AddLoadedValue(item, buff);
        
          long_line = false;
          buff[0] = '\0';
        }
        UnselectSection();
      }
      else {                                          // item with parameters
        if (*values && values[strlen(values)-1] == '_'){
          values[strlen(values)-1] = ' ';
          buff = strcat(buff, values);
          long_line = true;
        }
        else{
          if (long_line){
            buff = strcat(buff, values);
            GetWord(item, &buff);
            act_section->AddLoadedValue(item, buff);
          
            long_line = false;
            buff[0] = '\0';
          }
          else{
            GetWord(item, &values);
            act_section->AddLoadedValue(item, values);
          }
        }
      }
    }

  } while (!done);

  modified = false;  // we have no changes, we only read a file
  Close();          // close file

  return 1;
}


/**
 *  Saves all changes in configuration file.
 */
void TCONF_FILE::Save(void)
{
  if (!modified) return;   // nothing to save

  // open file for writing
  if (!Open(const_cast<char*>("wt"))) {
    Warning (LogMsg ("Could not save file '%s'.", name));
    return;
  }

  // write all entries
  base_section->Write();
  modified = false;

  Close();                // close file
}


/**
 *  Selects section as an actual. If section is not found, new one will be created.
 *
 *  @param section section name.
 */
bool TCONF_FILE::SelectSection(char *section, bool mandatory)
{
  // if section is NULL, select base section
  if (!section) {
    act_section = base_section;
    return true;
  }

  TFE_SECTION *sec = act_section->SelectSection(section, mandatory);

  if (sec) {
    act_section = sec;
    return true;
  }
  else return false;
}


/**
 *  Unselects actual section. As actual section will be set its owner.
 */
void TCONF_FILE::UnselectSection(void)
{
  if (!act_section->GetOwner()) return;

  act_section = act_section->GetOwner();
}


/**
 *  Deletes section with all entries from actual section.
 *
 *  @param section section name.
 */
void TCONF_FILE::DeleteSection(char *section)
{
  act_section->DeleteSection(section);

  modified = true;
}


/**
 *  Deletes item from actual section.
 *
 *  @param item item name.
 */
void TCONF_FILE::DeleteItem(char *item)
{
  act_section->DeleteItem(item);

  modified = true;
}


/**
 *  Writes new line into actual section.
 *
 *  @param line Line content.
 */
void TCONF_FILE::WriteLine(char *line)
{
  act_section->AddLine(line);

  modified = true;
}


/**
 *  Writes new string value into item.
 *
 *  @param item   item name.
 *  @param value  new item value.
 *
 *  @note Previous values will not be deleted.
 */
void TCONF_FILE::WriteStr(char *item, char *value)
{
  TFILE_LINE str;

  sprintf(str, "\"%s\"", value);      // string will be written in double quotes ("...")
  act_section->WriteValue(item, str);

  modified = true;
}


/**
 *  Writes new integer value into item.
 *
 *  @param item   item name.
 *  @param value  new item value.
 *
 *  @note Previous values will not be deleted.
 */
void TCONF_FILE::WriteInt(char *item, int value)
{
  TFILE_LINE str;

  sprintf(str, "%d", value);

  act_section->WriteValue(item, str);

  modified = true;
}


/**
 *  Writes new float value into item.
 *
 *  @param item   item name.
 *  @param value  new item value.
 *
 *  @note Previous values will not be deleted.
 */
void TCONF_FILE::WriteFloat(char *item, float value)
{
  TFILE_LINE str;

  sprintf(str, "%f", value);

  act_section->WriteValue(item, str);

  modified = true;
}


/**
 *  Writes new double value into item.
 *
 *  @param item   item name.
 *  @param value  new item value.
 *
 *  @note Previous values will not be deleted.
 */
void TCONF_FILE::WriteDouble(char *item, double value)
{
  TFILE_LINE str;

  sprintf(str, "%f", value);

  act_section->WriteValue(item, str);

  modified = true;
}


/**
 *  Writes new simple value into item.
 *
 *  @param item   item name.
 *  @param value  new item value.
 *
 *  @note Previous values will not be deleted.
 */
void TCONF_FILE::WriteSimple(char *item, T_SIMPLE value)
{
  TFILE_LINE str;

  sprintf(str, "%d", value);

  act_section->WriteValue(item, str);

  modified = true;
}


/**
 *  Writes new byte value into item.
 *
 *  @param item   item name.
 *  @param value  new item value.
 *
 *  @note Previous values will not be deleted.
 */
void TCONF_FILE::WriteByte(char *item, T_BYTE value)
{
  TFILE_LINE str;

  sprintf(str, "%d", value);

  act_section->WriteValue(item, str);

  modified = true;
}


/**
 *  Writes new boolean value into item.
 *
 *  @param item   item name.
 *  @param value  new item value.
 *
 *  @note Previous values will not be deleted.
 */
void TCONF_FILE::WriteBool(char *item, bool value)
{
  if (value) act_section->WriteValue(item, const_cast<char*>("true"));    // true
  else act_section->WriteValue(item, const_cast<char*>("false"));         // false

  modified = true;
}


/**
 *  Reads string value from item. On failure @p def_value will be stored into
 *  @p value.
 *
 *  @param value      Read value will be stored here.
 *  @param item       Item name.
 *  @param def_value  Default value.
 *  @param warning    If @c false, no warning about missing value is written to log.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool TCONF_FILE::ReadStr(char *value, char *item, char *def_value, bool warning)
{
  if (!act_section->ReadValue(value, item, warning)) {
    strcpy(value, def_value);   // value was not found, return default value
    Warning(LogMsg("Value of item '%s' was set to default value '%s'", item, def_value));
    return false;
  }

  return true;
}



/**
 *  Reads float value from item. On failure @p def_value will be returned.
 *
 *  @param value      read value will be stored here.
 *  @param item       item name.
 *  @param def_value  default value.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool TCONF_FILE::ReadFloat(float *value, char *item, float def_value)
{
  TFILE_LINE str;
  int line;
  float v;

  bool ok = true;

  ok = ((line = act_section->ReadValue(str, item, true)) > 0);

  if (ok && !StringToFloat(&v, str)) {  // in str is not valid float number
    Warning(LogMsg("Invalid float value '%s' on line %d", str, line));
    ok = false;
  }
  
  if (ok) *value = v;       // return result
  else {
    *value = def_value;  // return default value
    Warning(LogMsg("Value of item '%s' was set to default value %f", item, def_value));
  }

  return ok;
}



/**
 *  Reads float value from item with condition "greater or equal then". On failure @p def_value will be returned.
 *
 *  @param value      read value will be stored here.
 *  @param item       item name.
 *  @param min        min value.
 *  @param def_value  default value.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool TCONF_FILE::ReadFloatGE(float *value, char *item, float min, float def_value)
{
  TFILE_LINE str;
  int line;
  float v;

  bool ok = true;

  ok = ((line = act_section->ReadValue(str, item, true)) > 0);

  if (ok && !StringToFloat(&v, str)) {  // in str is not valid float number
    Warning(LogMsg("Invalid float value '%s' on line %d", str, line));
    ok = false;
  }

  if (ok && v < min) {  // result does not fulfil condition
    Warning(LogMsg("Invalid value range on line %d, value '%f' is less then min '%f'", line, v, min));
    ok = false;
  }
  
  if (ok) *value = v;       // return result
  else {
    *value = def_value;  // return default value
    Warning(LogMsg("Value of item '%s' was set to default value %f", item, def_value));
  }

  return ok;
}



/**
 *  Reads float value from item. This value must be in asked range.
 *  On failure @p def_value will be returned.
 *
 *  @param value      read value will be stored here.
 *  @param item       item name.
 *  @param min        minimum range value.
 *  @param max        maximum range value.
 *  @param def_value  default value.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool TCONF_FILE::ReadFloatRange(float *value, char *item, float min, float max, float def_value)
{
  TFILE_LINE str;
  int line;
  float v;

  bool ok = true;

  ok = ((line = act_section->ReadValue(str, item, true)) > 0);

  if (ok && !StringToFloat(&v, str)) {  // in str is not valid float number
    Warning(LogMsg("Invalid float value '%s' on line %d", str, line));
    ok = false;
  }

  if (ok && (v < min || v > max)) {   // result does not fulfil conditions
    Warning(LogMsg("Invalid value range on line %d, value '%f' is not in range <%f, %f>", line, v, min, max));
    ok = false;
  }
  
  if (ok) *value = v;       // return result
  else {
    *value = def_value;  // return default value
    Warning(LogMsg("Value of item '%s' was set to default value %f", item, def_value));
  }

  return ok;
}


/**
 *  Reads double value from item. On failure @p def_value will be returned.
 *
 *  @param value      read value will be stored here.
 *  @param item       item name.
 *  @param def_value  default value.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool TCONF_FILE::ReadDouble(double *value, char *item, double def_value)
{
  TFILE_LINE str;
  int line;
  double v;

  bool ok = true;

  ok = ((line = act_section->ReadValue(str, item, true)) > 0);

  if (ok && !StringToDouble(&v, str)) {  // in str is not valid double number
    Warning(LogMsg("Invalid double value '%s' on line %d", str, line));
    ok = false;
  }
  
  if (ok) *value = v;       // return result
  else{
    *value = def_value;  // return default value
    Warning(LogMsg("Value of item '%s' was set to default value %f", item, def_value));
  }
  
  return ok;
}


/**
 *  Reads double value from item with condition "greater or equal then". On failure @p def_value will be returned.
 *
 *  @param value      read value will be stored here.
 *  @param item       item name.
 *  @param min        min value.
 *  @param def_value  default value.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool TCONF_FILE::ReadDoubleGE(double *value, char *item, double min, double def_value)
{
  TFILE_LINE str;
  int line;
  double v;

  bool ok = true;

  ok = ((line = act_section->ReadValue(str, item, true)) > 0);

  if (ok && !StringToDouble(&v, str)) {  // in str is not valid float number
    Warning(LogMsg("Invalid double value '%s' on line %d", str, line));
    ok = false;
  }

  if (ok && v < min) {  // result does not fulfil condition
    Warning(LogMsg("Invalid value range on line %d, value '%f' is less then min '%f'", line, v, min));
    ok = false;
  }
  
  if (ok) *value = v;       // return result
  else {
    *value = def_value;  // return default value
    Warning(LogMsg("Value of item '%s' was set to default value %f", item, def_value));
  }

  return ok;
}



/**
 *  Reads double value from item. This value must be in asked range.
 *  On failure @p def_value will be returned.
 *
 *  @param value      read value will be stored here.
 *  @param item       item name.
 *  @param min        minimum range value.
 *  @param max        maximum range value.
 *  @param def_value  default value.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool TCONF_FILE::ReadDoubleRange(double *value, char *item, double min, double max, double def_value)
{
  TFILE_LINE str;
  int line;
  double v;

  bool ok = true;

  ok = ((line = act_section->ReadValue(str, item, true)) > 0);

  if (ok && !StringToDouble(&v, str)) {  // in str is not valid float number
    Warning(LogMsg("Invalid double value '%s' on line %d", str, line));
    ok = false;
  }

  if (ok && (v < min || v > max)) {   // result does not fulfil conditions
    Warning(LogMsg("Invalid value range on line %d, value '%f' is not in range <%f, %f>", line, v, min, max));
    ok = false;
  }
  
  if (ok) *value = v;       // return result
  else {
    *value = def_value;  // return default value
    Warning(LogMsg("Value of item '%s' was set to default value %f", item, def_value));
  }

  return ok;
}


/**
 *  Reads integer value from item. On failure @p def_value will be returned.
 *
 *  @param value      read value will be stored here.
 *  @param item       item name.
 *  @param def_value  default value.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool TCONF_FILE::ReadInt(int *value, char *item, int def_value)
{
  TFILE_LINE str;
  int line;
  int v;

  bool ok = true;

  ok = ((line = act_section->ReadValue(str, item, true)) > 0);

  if (ok && !StringToInt(&v, str)) {  // in str is not valid integer number
    Warning(LogMsg("Invalid integer value '%s' on line %d", str, line));
    ok = false;
  }
  
  if (ok) *value = v;       // return result
  else {
    *value = def_value;  // return default value
    Warning(LogMsg("Value of item '%s' was set to default value %d", item, def_value));
  }

  return ok;
}


/**
 *  Reads integer value with condition "greater or equal than" from item.
 *  On failure @p def_value will be returned.
 *
 *  @param value      read value will be stored here.
 *  @param item       item name.
 *  @param min        minimum value.
 *  @param def_value  default value.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool TCONF_FILE::ReadIntGE(int *value, char *item, int min, int def_value)
{
  
  TFILE_LINE str;
  int line;
  int v;

  bool ok = true;

  ok = ((line = act_section->ReadValue(str, item, true)) > 0);

  if (ok && !StringToInt(&v, str)) {  // in str is not valid integer number
    Warning(LogMsg("Invalid integer value '%s' on line %d", str, line));
    ok = false;
  }
    
  if (ok && v < min) {  // result does not fulfil condition
    Warning(LogMsg("Invalid value range on line %d, value '%d' is less then min '%d'", line, v, min));
    ok = false;
  }
  
  if (ok) *value = v;       // return result
  else{
    *value = def_value;  // return default value
    Warning(LogMsg("Value of item '%s' was set to default value %d", item, def_value));
  }

  return ok;
}


/**
 *  Reads integer value from item. This value must be in asked range.
 *  On failure @p def_value will be returned.
 *
 *  @param value      read value will be stored here.
 *  @param item       item name.
 *  @param min        minimum range value.
 *  @param max        maximum range value.
 *  @param def_value  default value.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool TCONF_FILE::ReadIntRange(int *value, char *item, int min, int max, int def_value)
{
  TFILE_LINE str;
  int line;
  int v;

  bool ok = true;

  ok = ((line = act_section->ReadValue(str, item, true)) > 0);

  if (ok && !StringToInt(&v, str)) {  // in str is not valid integer number
    Warning(LogMsg("Invalid integer value '%s' on line %d", str, line));
    ok = false;
  }

  if (ok && (v < min || v > max)) {   // result does not fulfil conditions
    Warning(LogMsg("Invalid value range on line %d, value '%d' is not in range <%d, %d>", line, v, min, max));
    ok = false;
  }
  
  if (ok) *value = v;       // return result
  else {
    *value = def_value;  // return default value
    Warning(LogMsg("Value of item '%s' was set to default value %d", item, def_value));
  }

  return ok;
}

/**
 *  Reads T_SIMPLE value from item. On failure @p def_value will be returned.
 *
 *  @param value      read value will be stored here.
 *  @param item       item name.
 *  @param def_value  default value.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool TCONF_FILE::ReadSimple(T_SIMPLE *value, char *item, T_SIMPLE def_value)
{
  TFILE_LINE str;
  int line;
  int v;

  bool ok = true;

  ok = ((line = act_section->ReadValue(str, item, true)) > 0);

  if (ok && !StringToInt(&v, str)) {  // in str is not valid integer number
    Warning(LogMsg("Invalid numerical value '%s' on line %d", str, line));
    ok = false;
  }

  // check for TSIMPLE type range
  if (ok){
    if ((v < 0) && (v > MAX_T_SIMPLE)){
      Warning(LogMsg("Value of item '%s' is not in T_SIMPLE range", item));
      ok = false;
    }
  }
  
  if (ok) *value = (T_SIMPLE)v;       // return result
  else {
    *value = def_value;            // return default value
    Warning(LogMsg("Value of item '%s' was set to default value %d", item, def_value));
  }

  return ok;
}


/**
 *  Reads T_SIMPLE value with condition "greater or equal than" from item.
 *  On failure @p def_value will be returned.
 *
 *  @param value      read value will be stored here.
 *  @param item       item name.
 *  @param min        minimum value.
 *  @param def_value  default value.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool TCONF_FILE::ReadSimpleGE(T_SIMPLE *value, char *item, T_SIMPLE min, T_SIMPLE def_value)
{
  TFILE_LINE str;
  int line;
  int v;

  bool ok = true;

  ok = ((line = act_section->ReadValue(str, item, true)) > 0);

  if (ok && !StringToInt(&v, str)) {  // in str is not valid integer number
    Warning(LogMsg("Invalid numerical value '%s' on line %d", str, line));
    ok = false;
  }

  // check for TSIMPLE type range
  if (ok){
    if ((v < 0) && (v > MAX_T_SIMPLE)){
      Warning(LogMsg("Value of item '%s' is not in T_SIMPLE range", item));
      ok = false;
    }
  }


  if (ok && v < min) {  // result does not fulfil condition
    Warning(LogMsg("Invalid value range on line %d, value '%d' is less then min '%d'", line, v, min));
    ok = false;
  }
  
  if (ok) *value = (T_SIMPLE)v;   // return result
  else {
    *value = def_value;        // return default value
    Warning(LogMsg("Value of item '%s' was set to default value %d", item, def_value));
  }

  return ok;
}


/**
 *  Reads T_SIMPLE value from item. This value must be in asked range.
 *  On failure @p def_value will be returned.
 *
 *  @param value      read value will be stored here.
 *  @param item       item name.
 *  @param min        minimum range value.
 *  @param max        maximum range value.
 *  @param def_value  default value.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool TCONF_FILE::ReadSimpleRange(T_SIMPLE *value, char *item, T_SIMPLE min, T_SIMPLE max, T_SIMPLE def_value)
{
  TFILE_LINE str;
  int line;
  int v;

  bool ok = true;

  ok = ((line = act_section->ReadValue(str, item, true)) > 0);

  if (ok && !StringToInt(&v, str)) {  // in str is not valid integer number
    Warning(LogMsg("Invalid numerical value '%s' on line %d", str, line));
    ok = false;
  }
  
  // check for TSIMPLE type range
  if (ok){
    if ((v < 0) && (v > MAX_T_SIMPLE)){
      Warning(LogMsg("Value of item '%s' is not in T_SIMPLE range", item));
      ok = false;
    }
  }
    
  if (ok && (v < min || v > max)) {   // result does not fulfil conditions
    Warning(LogMsg("Invalid value range on line %d, value '%d' is not in range <%d, %d>", line, v, min, max));
    ok = false;
  }
  
  if (ok) *value = (T_SIMPLE)v;       // return result
  else {
    *value = def_value;            // return default value
    Warning(LogMsg("Value of item '%s' was set to default value %d", item, def_value));
  }

  return ok;
}


/**
 *  Reads T_BYTE value from item. On failure @p def_value will be returned.
 *
 *  @param value      read value will be stored here.
 *  @param item       item name.
 *  @param def_value  default value.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool TCONF_FILE::ReadByte(T_BYTE *value, char *item, T_BYTE def_value)
{
  TFILE_LINE str;
  int line;
  int v;

  bool ok = true;

  ok = ((line = act_section->ReadValue(str, item, true)) > 0);

  if (ok && !StringToInt(&v, str)) {  // in str is not valid integer number
    Warning(LogMsg("Invalid numerical value '%s' on line %d", str, line));
    ok = false;
  }
  
  // check for T_BYTE type range
  if (ok){
    if ((v < 0) && (v > MAX_T_BYTE)){
      Warning(LogMsg("Value of item '%s' is not in T_BYTE range", item));
      ok = false;
    }
  }
  
  if (ok) *value = (T_BYTE)v;          // return result
  else {
    *value = def_value;            // return default value
    Warning(LogMsg("Value of item '%s' was set to default value %d", item, def_value));
  }

  return ok;
}


/**
 *  Reads T_BYTE value with condition "greater or equal than" from item.
 *  On failure @p def_value will be returned.
 *
 *  @param value      read value will be stored here.
 *  @param item       item name.
 *  @param min        minimum value.
 *  @param def_value  default value.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool TCONF_FILE::ReadByteGE(T_BYTE *value, char *item, T_BYTE min, T_BYTE def_value)
{
  TFILE_LINE str;
  int line;
  int v;

  bool ok = true;

  ok = ((line = act_section->ReadValue(str, item, true)) > 0);

  if (ok && !StringToInt(&v, str)) {  // in str is not valid integer number
    Warning(LogMsg("Invalid numerical value '%s' on line %d", str, line));
    ok = false;
  }

  // check for T_BYTE type range
  if (ok){
    if ((v < 0) && (v > MAX_T_BYTE)){
      Warning(LogMsg("Value of item '%s' is not in T_BYTE range", item));
      ok = false;
    }
  }
  
  if (ok && v < min) {  // result does not fulfil condition
    Warning(LogMsg("Invalid value range on line %d, value '%d' is less then min '%d'", line, v, min));
    ok = false;
  }
  
  if (ok) *value = (T_BYTE)v;   // return result
  else {
    *value = def_value;        // return default value
    Warning(LogMsg("Value of item '%s' was set to default value %d", item, def_value));
  }

  return ok;
}


/**
 *  Reads T_BYTE value from item. This value must be in asked range.
 *  On failure @p def_value will be returned.
 *
 *  @param value      read value will be stored here.
 *  @param item       item name.
 *  @param min        minimum range value.
 *  @param max        maximum range value.
 *  @param def_value  default value.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool TCONF_FILE::ReadByteRange(T_BYTE *value, char *item, T_BYTE min, T_BYTE max, T_BYTE def_value)
{
  TFILE_LINE str;
  int line;
  int v;

  bool ok = true;

  ok = ((line = act_section->ReadValue(str, item, true)) > 0);

  if (ok && !StringToInt(&v, str)) {  // in str is not valid integer number
    Warning(LogMsg("Invalid numerical value '%s' on line %d", str, line));
    ok = false;
  }

  // check for T_BYTE type range
  if (ok){
    if ((v < 0) && (v > MAX_T_BYTE)){
      Warning(LogMsg("Value of item '%s' is not in T_BYTE range", item));
      ok = false;
    }
  }
  
  if (ok && (v < min || v > max)) {   // result does not fulfil conditions
    Warning(LogMsg("Invalid value range on line %d, value '%d' is not in range <%d, %d>", line, v, min, max));
    ok = false;
  }
  
  if (ok) *value = (T_BYTE)v;       // return result
  else {
    *value = def_value;            // return default value
    Warning(LogMsg("Value of item '%s' was set to default value %d", item, def_value));
  }

  return ok;
}


/**
 *  Reads string value from item. This value must be in tex_table names. 
 *  If value is in tex_table, to @param value will be stored -1, else texture group is stored.
 *  On failure false will be returned.
 *
 *  @param value      read value will be stored here.
 *  @param item       item name.
 *  @param tex_table  table of texture groups.
 *  @param mandatory  if texture must be filled.
 *  @param section    actual section.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool TCONF_FILE::ReadTextureGroup(int * value, char *item, TTEX_TABLE * tex_table, bool mandatory, char * section)
{
  int i;
  TFILE_LINE pom;
  bool ok = true;

  if (ok) ok = ReadStr(pom, item, const_cast<char*>(""), true);
  *value = -1; // default value
  
  if (strcmp(pom, "none")){ // if 'none', sets default value
    for (i = 0; (ok) && (i < tex_table->count); i++){
      if (!(strcmp(tex_table->groups[i].name, pom))){
        *value = i;
        break;
      }
    }
  
    // string was readed from conf. file, but not exists in dat file
    if ((ok) && (*value == -1)){
      ok = false;
      Warning(LogMsg("Can not find value '%s' of item '%s' (section %s) in DAT file.", pom, item, section));
    }
  }
  else{ 
    if (mandatory){ //this texture must be filled
      Warning(LogMsg("Item '%s' (section %s) is mandatory and have to be filled with texture.", item, section));
      ok = false;
    }
  }

  return ok;
}


#if SOUND
/**
 *  Reads string value from item. This value must be in snd_table names. 
 *  If value is in snd_table, to @param value will be stored -1, else sound identificator is stored.
 *  On failure false will be returned.
 *
 *  @param value      read value will be stored here.
 *  @param item       item name.
 *  @param snd_table  table of sound groups.
 *  @param mandatory  if sound must be filled.
 *  @param section    actual section.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool TCONF_FILE::ReadSound(TSND_GROUP * value, char *item, TSND_TABLE * snd_table, char * section)
{
  int i, j;
  
  TFILE_LINE pom;
  TSOUND * sounds[(T_BYTE) -1]; //array of pointers to sounds
  TSOUND * act_sound;
  
  bool ok = true;

  
  // clear help array of pointers to sounds
  for (i = 0; i < (T_BYTE) -1; i++) sounds[i] = NULL;
  
  // read first word from item
  if (ok) ok = ReadStr(pom, item, const_cast<char*>(""), false);
  
  // set default values
  value->count = 0;
  value->sounds = NULL;

  if (ok) {
    if ((*pom != 0) && (strcmp(pom, "none"))){  // first word exists and it is not "none" 

      // read from item tag while exists any value
      i = 0;
      GetActSection()->ResetValue(item);

      do{
        if (ok) ok = ReadStr(pom, item, const_cast<char*>(""), false);

        if ((ok) && (*pom != 0)){
          act_sound = snd_table->GetSound(pom);
          if (act_sound){
            sounds[i++] = act_sound;
          }
          else Warning(LogMsg("Can not find value '%s' of item '%s' (section %s) in DAT file.", pom, item, section));
        }
      } while (*pom != 0);

      ok = true;

      if ((ok) && (i > 0)){ // else default values
        value->count = (T_BYTE)i;
        value->sounds = NEW TSOUND *[i];
        for (j = 0; j < i; j++)
        value->sounds[j] = sounds[j];
      }
      
    }
    else{ // first word is "none"
      if (*pom == 0) Warning(LogMsg("Item '%s' (section %s) has no value. Please fill value with 'none' value.", item, section));
    }
  }

  return ok;
}
#endif


/**
 *  Reads boolean value from item. Allowed values are: true, false, yes, no, 0,
 *  non zero number.
 *  On failure @p def_value will be returned.
 *
 *  @param value      read value will be stored here.
 *  @param item       item name.
 *  @param def_value  default value.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool TCONF_FILE::ReadBool(bool *value, char *item, bool def_value)
{
  TFILE_LINE str;
  int line;
  int pom;
  bool v;

  bool ok = true;

  ok = ((line = act_section->ReadValue(str, item, true)) > 0);
  if (!ok) {
    *value = def_value;
    Warning(LogMsg("Value of item '%s' was set to default value '%s'", item, (def_value?"true" : "false")));
    return false;
  }

  if (!strcmp(str, "true")) v = true;           // true
  else if (!strcmp(str, "false")) v = false;    // false
  else if (!strcmp(str, "yes")) v = true;       // yes
  else if (!strcmp(str, "no")) v = false;       // no
  else {
    // some integer number
    ok = StringToInt(&pom, str);
    if (ok) v = (pom != 0);
  }
  
  if (!ok) {
    // in str is not a valid boolean value
    Warning(LogMsg("Invalid boolean value '%s' on line %d", str, line));
    Warning(LogMsg("Value of item '%s' was set to default value '%s'", item, (def_value?"true" : "false")));
    *value = def_value;
    return false;
  }

  *value = v;
  return true;
}


/**
 *  Clears the item's value.
 *
 *  @param item Name of the item.
 */
void TCONF_FILE::ClearValues(char *item)
{
  act_section->ClearValues(item);
}


/**
 *  Set indent string used in file;
 *
 *  @param str      this string will be used as indent.
 */
void TCONF_FILE::SetIndentString(char *str)
{
  if (indent_string) delete[] indent_string;

  indent_string = NEW char[strlen(str) + 1];
  strcpy(indent_string, str);
}


//========================================================================
// Create, Delete
//========================================================================

/**
 *  Creates new configuration file structure.
 *
 *  @param name File name (with path).
 *
 *  @return Pointer to new structure on success, otherwise @c NULL.
 */
TCONF_FILE *CreateConfFile(const char *name)
{
  // create new structure for configuration file
  return NEW TCONF_FILE(name);
}


/**
 *  Creates new configuration file structure and reloads the file.
 *
 *  @param name File name (with path).
 *  @return Pointer to new structure on success, otherwise @c NULL.
 *
 *  @see TCONF_FILE::Reload()
 */
TCONF_FILE *OpenConfFile(const char *name)
{
  TCONF_FILE *cf;

  // create new structure for configuration file
  if (!(cf = CreateConfFile(name)))
    return NULL;

  // reload data from file to structure
  if (cf->Reload())
    return cf;
  else {
    CloseConfFile(cf);
    return NULL;
  }
}


/**
 *  Closes configuration file and deletes its structure. All changes will be saved.
 *
 *  @param cf pointer to opened configuration file.
 */
void CloseConfFile(TCONF_FILE *&cf)
{
  if (cf) delete cf;    // save and delete structure of configuratin file
  cf = NULL;
}


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

