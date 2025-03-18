// $Id$
//
//    File: DReaction_factory_PhiSkim.h
// Created: Wed Mar 11 20:34:22 EDT 2015
// Creator: jrsteven (on Linux halldw1.jlab.org 2.6.32-504.8.1.el6.x86_64 x86_64)
//

#ifndef _DReaction_factory_PhiSkim_
#define _DReaction_factory_PhiSkim_

#include <iostream>
#include <iomanip>

#include <JANA/JFactoryT.h>
#include <ANALYSIS/DReaction.h>
#include <ANALYSIS/DHistogramActions.h>
#include <ANALYSIS/DCutActions.h>

using namespace std;

class DReaction_factory_PhiSkim : public JFactoryT<DReaction>
{
 public:
  DReaction_factory_PhiSkim()
    {
      // This is so that the created DReaction objects persist throughout the life of the program instead of being cleared each event. 
      SetFactoryFlag(PERSISTENT);
    }
  const char* Tag(void){return "PhiSkim";}

 private:
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void Finish() override;

  double dBeamBunchPeriod;
  deque<DReactionStep*> dReactionStepPool; //to prevent memory leaks

  void PIDCuts(DReaction* locReaction);
};

#endif // _DReaction_factory_PhiSkim_

