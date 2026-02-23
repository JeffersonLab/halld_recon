// $Id$
//
//    File: JEventProcessor_npp_skim.cc
// Created: Tue Apr 16 10:14:04 EDT 2024
// Creator: gxproj2 (on Linux ifarm1802.jlab.org 3.10.0-1160.102.1.el7.x86_64 x86_64)
//

#include "JEventProcessor_npp_skim.h"
#include "DANA/DEvent.h"



// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactoryT.h>
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_npp_skim());
  }
} // "C"


//------------------
// JEventProcessor_npp_skim (Constructor)
//------------------
JEventProcessor_npp_skim::JEventProcessor_npp_skim()
{

}

//------------------
// ~JEventProcessor_npp_skim (Destructor)
//------------------
JEventProcessor_npp_skim::~JEventProcessor_npp_skim()
{

}

//------------------
// Init
//------------------
void JEventProcessor_npp_skim::Init(void)
{
  auto app = GetApplication();

  ECAL_E_CUT  = 0.5;
  app->SetDefaultParameter("NPP_SKIM:ECAL_E_CUT", ECAL_E_CUT, "min shower energy in ECAL");

  FCAL_E_CUT  = 0.3; 
  app->SetDefaultParameter("NPP_SKIM:FCAL_E_CUT", FCAL_E_CUT, "min shower energy in FCAL");

  BCAL_E_CUT  = 0.1;
  app->SetDefaultParameter("NPP_SKIM:BCAL_E_CUT", BCAL_E_CUT, "min shower energy in BCAL");

  EGG_CUT     = 3.3;
  app->SetDefaultParameter("NPP_SKIM:EGG_CUT",   EGG_CUT, "min two shower energy sum for gg skim");

  MGG_CUT     = 0.1;
  app->SetDefaultParameter("NPP_SKIM:MGG_CUT",   MGG_CUT, "min two shower inv. mass for gg skim");
 
  EPI0_CUT    = 0.5;
  app->SetDefaultParameter("NPP_SKIM:EPI0_CUT", EPI0_CUT, "min pi0 energy for 2pi0 skim");

  MPI0_CUT    = 0.1;
  app->SetDefaultParameter("NPP_SKIM:MPI0_CUT", MPI0_CUT, "min two shower inv. mass for 2pi0 skim");

  EPI0PI0_CUT = 3.5;
  app->SetDefaultParameter("NPP_SKIM:EPI0PI0_CUT", EPI0PI0_CUT, "min two pi0 energy sum for 2pi0 skim");

  return;
}

//------------------
// BeginRun
//------------------
void JEventProcessor_npp_skim::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  // This is called whenever the run number changes

  DGeometry *geom = DEvent::GetDGeometry(event);
  if(!geom) throw JException("DGeometry is nullptr");
	geom->GetTargetZ(targetZ);

  num_epics_events = 0;
  return;
}

//------------------
// Process
//------------------
void JEventProcessor_npp_skim::Process(const std::shared_ptr<const JEvent>& event){

  const DEventWriterEVIO* locEventWriterEVIO = nullptr;
  event->GetSingle(locEventWriterEVIO);

  if(locEventWriterEVIO == nullptr)
    throw JException("JEventProcessor_npp_skim: locEventWriterEVIO is not available");

  if(GetStatusBit(event, kSTATUS_BOR_EVENT)) { // Begin of Run event
    locEventWriterEVIO->Write_EVIOEvent(event,"npp_2g");
    locEventWriterEVIO->Write_EVIOEvent(event,"npp_2pi0");
    return;
  }

  if(GetStatusBit(event, kSTATUS_EPICS_EVENT)) { // Epics event
    if(num_epics_events<5) {
      locEventWriterEVIO->Write_EVIOEvent(event,"npp_2g");
      locEventWriterEVIO->Write_EVIOEvent(event,"npp_2pi0");
    }
    ++num_epics_events;
    return;
  }

  vector<const DFCALHit*> locFCALHits;
  event->Get(locFCALHits);
  if(locFCALHits.size()>200) return;

  vector< const DECALShower* > locECALShowers;
  event->Get(locECALShowers);

  vector< const DFCALShower* > locFCALShowers;
  event->Get(locFCALShowers);

  vector< const DBCALShower* > locBCALShowers;
  event->Get(locBCALShowers);

  bool write_2g = false, write_2pi0 = false;

  vector<Shower> showers;
  showers.clear();
  showers.reserve(locECALShowers.size()+locFCALShowers.size()+locBCALShowers.size());

  for(size_t i=0; i<locECALShowers.size(); ++i) {
  	const DECALShower *shecal = locECALShowers[i];
    double e = shecal->E;
    if(e<ECAL_E_CUT) continue;

    auto& sh = showers.emplace_back();
    sh.pos   = shecal->pos;
    sh.e     = e;
  }

  for(size_t i=0; i<locFCALShowers.size(); ++i) {
  	const DFCALShower *shfcal = locFCALShowers[i];
    double e = shfcal->getEnergy();
    if(e<FCAL_E_CUT) continue;

    auto& sh = showers.emplace_back();
    sh.pos   = shfcal->getPosition();
    sh.e     = e;
  }

  for(size_t i=0; i<locBCALShowers.size(); ++i) {
  	const DBCALShower *shbcal = locBCALShowers[i];
    double e = shbcal->E;
    if(e<BCAL_E_CUT) continue;

    auto& sh = showers.emplace_back();
    sh.pos.SetXYZ(shbcal->x,shbcal->y,shbcal->z);
    sh.e     = e;
  }

  for(size_t i=0; i+1<showers.size()&&!write_2g; ++i) {
    for(size_t j=i+1; j<showers.size(); ++j) {

      if(showers[i].e + showers[j].e < EGG_CUT) continue;
      double mraw= mgg(showers[i],showers[j]);
      if(mraw<MGG_CUT) continue;
      write_2g = true;
      break;

    }
  }

  for(size_t i=0; i+1<showers.size()&&!write_2pi0; ++i) {
    for(size_t j=i+1; j<showers.size()&&!write_2pi0; ++j) {

      if(showers[i].e + showers[j].e < EPI0_CUT) continue;
      double mraw12 = mgg(showers[i],showers[j]);
      if(mraw12<MPI0_CUT) continue;

      for(size_t i2=i+1; i2+1<showers.size()&&!write_2pi0; ++i2) {
        if(i2==j) continue;
        for(size_t j2=i2+1; j2<showers.size(); ++j2) {
          if(j2==j) continue;

          if(showers[i2].e + showers[j2].e < EPI0_CUT) continue;
          if(showers[i].e + showers[j].e + showers[i2].e + showers[j2].e < EPI0PI0_CUT) continue;
          double mraw34 = mgg(showers[i2],showers[j2]);
          if(mraw34<MPI0_CUT) continue;

          write_2pi0 = true;
          break;
        }
      }
    }
  }

  if(write_2g)    locEventWriterEVIO->Write_EVIOEvent(event,"npp_2g");
  if(write_2pi0)  locEventWriterEVIO->Write_EVIOEvent(event,"npp_2pi0");

  return;
}

//------------------
// EndRun
//------------------
void JEventProcessor_npp_skim::EndRun(void)
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return;
}

//------------------
// Finish
//------------------
void JEventProcessor_npp_skim::Finish(void)
{
  // Called before program exit after event processing is finished.
  return;
}

