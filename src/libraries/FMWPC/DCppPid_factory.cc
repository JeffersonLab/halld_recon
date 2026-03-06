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
    int fmwpc1m_count = 0,fmwpc2m_count = 0,fmwpc3m_count = 0,fmwpc4m_count = 0,fmwpc5m_count = 0,fmwpc6m_count = 0; 
    auto hyp=(*track).Get_Hypothesis(PiPlus);
    if (hyp!=nullptr){
      auto matches=hyp->Get_FMWPCMatchParams();
      if (matches!=nullptr){
        for (size_t i=0;i<matches->dLayers.size();i++){
          for (auto &hit:fmwpchits){
            if ((*hit).layer!=matches->dLayers[i]) continue;
            if ((*hit).wire!=matches->dClosestWires[i]) continue;

            if((*hit).layer==1)fmwpc1m_count = matches->dNhits[i];
            if((*hit).layer==1)fmwpc2m_count = matches->dNhits[i];
            if((*hit).layer==1)fmwpc3m_count = matches->dNhits[i];
            if((*hit).layer==1)fmwpc4m_count = matches->dNhits[i];
            if((*hit).layer==1)fmwpc5m_count = matches->dNhits[i];
            if((*hit).layer==1)fmwpc6m_count = matches->dNhits[i];

            //if((*hit).layer==1) fmwpc1m_count += 1;
            //if((*hit).layer==2) fmwpc2m_count += 1;
            //if((*hit).layer==3) fmwpc3m_count += 1;
            //if((*hit).layer==4) fmwpc4m_count += 1;
            //if((*hit).layer==5) fmwpc5m_count += 1;
            //if((*hit).layer==6) fmwpc6m_count += 1;

            //cout << "Matched Wire:  " << (*hit).wire << endl;
          }
        }
      }
    }
    DCppPid *mycppPid = new DCppPid;
    mycppPid->fmwpc1m = fmwpc1m_count;
    mycppPid->fmwpc2m = fmwpc2m_count;
    mycppPid->fmwpc3m = fmwpc3m_count;
    mycppPid->fmwpc4m = fmwpc4m_count;
    mycppPid->fmwpc5m = fmwpc5m_count;
    mycppPid->fmwpc6m = fmwpc6m_count;

    Insert(mycppPid);

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

