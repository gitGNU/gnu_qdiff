/*GPL*START*
 *
 * tappconfig.h - console application framework header file
 * 
 * Copyright (C) 1998 by Johannes Overmann <overmann@iname.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * *GPL*END*/  

#ifndef _ngw_tappconfig_h_
#define _ngw_tappconfig_h_

#include "terror.h"
#include "tstring.h"
#include "tassocarray.h"

enum TACO_TYPE {TACO_TYPE_NONE, STRING, INT, DOUBLE, BOOL, SWITCH};
enum TACO_STRING_MODE {OVERRIDE, APPEND, ONCE};
enum TACO_SET_IN {NEVER, DEFAULT, COMMAND_LINE, RC_FILE, ENVIRONMENT, APPLICATION};

// global application pointer
class TAppConfig;
extern TAppConfig *tApp;

// should be a private subclass of TAppConfig
class TAppConfigItem {
 public:
   // cons
   TAppConfigItem();
   TAppConfigItem(const char *confitem, const char *context, bool privat);
   
   // interface
   void printItemToFile(FILE *f) const; // print default
   void printCurItemToFile(FILE *f, bool simple) const; // print current
   void printValue(const TString& env, const TString& rcfile) const;
   void printHelp(int maxoptlen, bool globalonlycl) const;
   int getOptLen() const;
   TString getTypeStr() const;
   void setValue(const TString& par, TACO_SET_IN setin, 
		 bool verbose=false, const TString& env="", 
		 const TString& rcfile="",
		 const TString& context="command line");
   
   // set from application methods:
   
   // returns true if value is valid, else false
   // sets value from string according to any type (switch == bool here)
   bool setValueFromAppFromStr(const TString& parameter);
   void setValueFromApp(const TString& str);
   // returns true if value is valid, else false
   bool setValueFromApp(double d);
   // returns true if value is valid, else false
   bool setValueFromApp(int i);
   void setValueFromApp(bool b);
   



 private: 
   // private methods
   void setComp(const TArray<TString>& a, bool privat);
   void validate(const char *context);
   void privateInit();
   TString getParamStr() const;
   TString getWasSetStr(const TString& env, const TString& rcfile) const;
   TString getWasSetStr(TACO_SET_IN setin, const TString& env, const TString& rcfile) const;
   TString getFlagsStr(const TString& optprefix, bool globalonlycl) const;
   

 public:   
   // data
   bool must_have;
   bool should_have;
   bool only_cl;
   bool configopt;
   bool only_app;
   bool save;
   bool optional_param;
   bool hide;
   
   TACO_TYPE type;
   TACO_SET_IN set_in;
   TACO_STRING_MODE string_mode;
   TString string_sep;
   double double_value, double_upper, double_lower, double_default;
   int int_value, int_upper, int_lower, int_default;
   bool bool_value, bool_default;
   
   // temp flag
   bool printed;

   TString name;
   TString context;
   TString help;
   TString headline;
   TString char_name;
   TString par;
   TArray<TString> alias;
   TString type_str;
   TString upper, lower, def;
   TString string_value, string_default;
};


// this is the main class of this h file
class TAppConfig {
 public:
   // ctor & dtor
   TAppConfig(const char *conflist[], const char *listname,
	      int argc, char *av[],
	      const char *envstrname,
	      const char *rcname,
	      const TString& version);
   ~TAppConfig();
      
   // main interface
   void printHelp(bool show_hidden = false) const;
   void printValues() const;
   bool save(TString& rc_name_out); // save items with item.save==true
   
   // typed options:
   const TString& getString(const TString& par) const;
   double getDouble(const TString& par) const;
   int getInt(const TString& par) const;
   bool getBool(const TString& par) const;        // bool + switch
   bool getSwitch(const TString& par) const;      // bool + switch
   bool operator() (const TString& par) const; // bool + switch
   
   // untyped parameters:
   TString param(int i) const {return _params(i);}
   int numParam() const {return _params.num();}
   const TArray<TString>& params() const {return _params;}
   
   // set values: 
   // returns true if value is valid, else false
   // sets value from string according to any type (switch == bool here)
   bool setValueFromStr(const TString &n, const TString& str);
   void setValue(const TString &n, const TString& str);
   bool setValue(const TString &n, double d);
   bool setValue(const TString &n, int i);
   void setValue(const TString &n, bool b);

   // return the upper and lower bounds and defaults for the type
   int intUpper(const TString &n) const;
   int intLower(const TString &n) const;
   int intDefault(const TString &n) const;
   double doubleUpper(const TString &n) const;
   double doubleLower(const TString &n) const;
   double doubleDefault(const TString &n) const;
   TString stringDefault(const TString &n) const;
   bool   boolDefault(const TString &n) const;

   // return location where parameter was set
   TACO_SET_IN wasSetIn(const TString& n) const;
   
 private:
   // readonly public data
   TArray<TString> _params;
   
   // private data
   TAssocArray<TString,int> name;  // get index of long name
   int char2index[256];           // get index of short name
   TArray<TAppConfigItem> opt;    // all options in the order of conflist
   TAssocArray<TString,int> alias; // aliases for options
   TString envname;    // name of env var
   TString rc_name;    // name of loaded rc file
   TString rc_str;     // namestr of rc file
   bool verbose_conf; // verbose configuration: warnings and values
   bool onlycl;       // true== dont use rcfile and envvar, only command line
   bool stopatdd;     // stop option scanning after --
   bool removedd;     // remove first -- param
   bool ignore_negnum;// something like -20 or -.57 is not trated as options
   TString usage;      // usage string: printed before help
   TString trailer;    // trailer string: printed after help
   TString commonhead; // headline for common options
   
   int getMaxOptLen(bool show_hidden) const;
   void doMetaChar(const TString& str, const TString& context);
   void setComp(const TArray<TString>& a, const TString& context);
   void addConfigItems(const char **list, const char *listname, bool privat);
   void doCommandLine(int ac, char *av[], const TString& version);
   void doEnvironVar(const char *envvar);
   void doRCFile(const TString& rcfile, const TString& clrcfile);
   void setFromStr(const TString& str, const TString& context, TACO_SET_IN setin);
   TString getName(const TString& str, const TString& context, const TString& optprefix="") const;
   void createRCFile(const TString& fname, const TString& rcname) const;
};


#endif
