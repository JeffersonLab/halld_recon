// $Id$
//
//    File: DCppPid_factory.cc
// Created: Thu Mar  5 02:05:32 PM EST 2026
// Creator: staylor (on Linux ifarm2401.jlab.org 5.14.0-611.30.1.el9_7.x86_64 x86_64)
//

#include <iostream>
#include <iomanip>

#include "DCppPid_factory.h"
using namespace std;
#include <PID/DChargedTrack.h>
#include <FMWPC/DFMWPCHit.h>

//------------------
// Init
//------------------
void DCppPid_factory::Init()
{
  auto app = GetApplication();
	
}

//------------------
// BeginRun
//------------------
void DCppPid_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void DCppPid_factory::Process(const std::shared_ptr<const JEvent>& event)
{
  auto fmwpchits=event->Get<DFMWPCHit>();
  if (fmwpchits.size()==0) return;

  auto tracks=event->Get<DChargedTrack>();
  if (tracks.size()==0) return;

  for (auto &track:tracks){
    auto hyp=(*track).Get_Hypothesis(PiPlus);
    if (hyp!=nullptr){
      auto matches=hyp->Get_FMWPCMatchParams();
      if (matches!=nullptr){
	for (size_t i=0;i<matches->dLayers.size();i++){
	  for (auto &hit:fmwpchits){
	    if ((*hit).layer!=matches->dLayers[i]) continue;
	    if ((*hit).wire!=matches->dClosestWires[i]) continue;

	    cout << "Matched Wire:  " << (*hit).wire << endl;
	  }
	}
      }
    } 
  }
}

//------------------
// EndRun
//------------------
void DCppPid_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DCppPid_factory::Finish()
{
}

