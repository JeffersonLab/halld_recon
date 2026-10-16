// $Id$
//
//    File: DFactoryGenerator_OmegaSkim.h
// Created: Wed Mar 11 20:34:22 EDT 2015
// Creator: jrsteven (on Linux halldw1.jlab.org 2.6.32-504.8.1.el6.x86_64 x86_64)
//

#ifndef _DFactoryGenerator_OmegaSkim_
#define _DFactoryGenerator_OmegaSkim_

#include <JANA/JFactoryGenerator.h>

#include "DReaction_factory_OmegaSkim.h"

class DFactoryGenerator_OmegaSkim : public JFactoryGenerator
{
 public:
  virtual const char* className(void){return static_className();}
  static const char* static_className(void){return "DFactoryGenerator_OmegaSkim";}
		
  void GenerateFactories(JFactorySet* fs)
  {
    fs->Add(new DReaction_factory_OmegaSkim());
  }
};

#endif // _DFactoryGenerator_OmegaSkim_

