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
  // This is called once at program startup. 
  
  return; //NOERROR;
}

//------------------
// BeginRun
//------------------
void JEventProcessor_npp_skim::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  // This is called whenever the run number changes
  num_epics_events = 0;
  return; //NOERROR;
}

//------------------
// Process
//------------------
void JEventProcessor_npp_skim::Process(const std::shared_ptr<const JEvent>& event){
  // This is called for every event. Use of common resources like writing
  // to a file or filling a histogram should be mutex protected. Using
  // loop->Get(...) to get reconstructed objects (and thereby activating the
  // reconstruction algorithm) should be done outside of any mutex lock
  // since multiple threads may call this method at the same time.
  // Here's an example:
  //
  // vector<const MyDataClass*> mydataclasses;
  // loop->Get(mydataclasses);
  //
  // japp->RootFillLock(this);
  //  ... fill historgrams or trees ...
  // japp->RootFillUnLock(this);


  vector<const DFCALHit*> locFCALHits;
  event->Get(locFCALHits);
  if(locFCALHits.size()>200) return; //NOERROR;

  vector< const DFCALShower* > locFCALShowers;
  event->Get(locFCALShowers);

  vector< const DBCALShower* > locBCALShowers;
  event->Get(locBCALShowers);

  const DEventWriterEVIO* locEventWriterEVIO = NULL;
  event->GetSingle(locEventWriterEVIO);
  if(locEventWriterEVIO == NULL) {
    cerr << "from JEventProcessor_npp_skim: locEventWriterEVIO is not available" << endl;
    exit(1);
  }

  if(GetStatusBit(event, kSTATUS_BOR_EVENT)) { // Begin of Run event
    locEventWriterEVIO->Write_EVIOEvent(event,"npp_2g");
    locEventWriterEVIO->Write_EVIOEvent(event,"npp_2pi0");
    return; //NOERROR;
  }

  if(GetStatusBit(event, kSTATUS_EPICS_EVENT)) { // Epics event
    if(num_epics_events<5) {
      locEventWriterEVIO->Write_EVIOEvent(event,"npp_2g");
      locEventWriterEVIO->Write_EVIOEvent(event,"npp_2pi0");
    }
    ++num_epics_events;
    return; //NOERROR;
  }

  bool write_2g = false, write_2pi0 = false;

  for(unsigned int i=0; i<locFCALShowers.size()+locBCALShowers.size() && !write_2g; ++i) {
    double x1, y1, z1, e1;
    if(i<locFCALShowers.size()) {
      const DFCALShower *s1 = locFCALShowers[i];
      e1 = s1->getEnergy();
      x1 = s1->getPosition().X();
      y1 = s1->getPosition().Y();
      z1 = s1->getPosition().Z() - z_CPP_Target;
      if(e1<0.3) continue;
    } else {
      const DBCALShower *s1 = locBCALShowers[i-locFCALShowers.size()];
      e1 = s1->E;
      x1 = s1->x;
      y1 = s1->y;
      z1 = s1->z - z_CPP_Target;
      if(e1<0.1) continue;
    }

    for(unsigned int j=i+1; j<locFCALShowers.size()+locBCALShowers.size() && !write_2g; ++j) {
      double x2, y2, z2, e2;
      if(j<locFCALShowers.size()) {
    	const DFCALShower *s2 = locFCALShowers[j];
	e2 = s2->getEnergy();
	x2 = s2->getPosition().X();
	y2 = s2->getPosition().Y();
	z2 = s2->getPosition().Z() - z_CPP_Target;
	if(e2<0.3) continue;
      } else {
	const DBCALShower *s2 = locBCALShowers[j-locFCALShowers.size()];
	e2 = s2->E;
	x2 = s2->x;
	y2 = s2->y;
	z2 = s2->z - z_CPP_Target;
	if(e2<0.1) continue;
      }

      if(e1+e2<3.3) continue;

      double mraw= mgg(x1,y1,z1,e1,x2,y2,z2,e2);

      if(mraw<0.1) continue;

      write_2g = true;

    }
  }


  for(unsigned int i=0; i<locFCALShowers.size()+locBCALShowers.size() && !write_2pi0; ++i) {
    double x1, y1, z1, e1;
    if(i<locFCALShowers.size()) {
      const DFCALShower *s1 = locFCALShowers[i];
      e1 = s1->getEnergy();
      x1 = s1->getPosition().X();
      y1 = s1->getPosition().Y();
      z1 = s1->getPosition().Z() - z_CPP_Target;
      if(e1<0.3) continue;
    } else {
      const DBCALShower *s1 = locBCALShowers[i-locFCALShowers.size()];
      e1 = s1->E;
      x1 = s1->x;
      y1 = s1->y;
      z1 = s1->z - z_CPP_Target;
      if(e1<0.1) continue;
    }

    for(unsigned int j=i+1; j<locFCALShowers.size()+locBCALShowers.size() && !write_2pi0; ++j) {
      double x2, y2, z2, e2;
      if(j<locFCALShowers.size()) {
    	const DFCALShower *s2 = locFCALShowers[j];
	e2 = s2->getEnergy();
	x2 = s2->getPosition().X();
	y2 = s2->getPosition().Y();
	z2 = s2->getPosition().Z()  - z_CPP_Target;
	if(e2<0.3) continue;
      } else {
	const DBCALShower *s2 = locBCALShowers[j-locFCALShowers.size()];
	e2 = s2->E;
	x2 = s2->x;
	y2 = s2->y;
	z2 = s2->z-1.;
	if(e2<0.1) continue;
      }

      if(e1+e2<0.5) continue;
      double mraw12 = mgg(x1,y1,z1,e1,x2,y2,z2,e2);
      if(mraw12<0.1) continue;

      for(unsigned int i2=i+1; i2<locFCALShowers.size()+locBCALShowers.size() && !write_2pi0; ++i2) { // i2>i
	if(i2==j) continue;
	double x3, y3, z3, e3;
	if(i2<locFCALShowers.size()) {
	  const DFCALShower *s3 = locFCALShowers[i2];
	  e3 = s3->getEnergy();
	  x3 = s3->getPosition().X();
	  y3 = s3->getPosition().Y();
	  z3 = s3->getPosition().Z()  - z_CPP_Target;
	  if(e3<0.3) continue;
	} else {
	  const DBCALShower *s3 = locBCALShowers[i2-locFCALShowers.size()];
	  e3 = s3->E;
	  x3 = s3->x;
	  y3 = s3->y;
	  z3 = s3->z - z_CPP_Target;
	  if(e3<0.1) continue;
	}

	for(unsigned int j2=i2+1; j2<locFCALShowers.size()+locBCALShowers.size() && !write_2pi0; ++j2) { // j2>i2>i
	  if(j2==j) continue;
	  double x4, y4, z4, e4;
	  if(j2<locFCALShowers.size()) {
	    const DFCALShower *s4 = locFCALShowers[j2];
	    e4 = s4->getEnergy();
	    x4 = s4->getPosition().X();
	    y4 = s4->getPosition().Y();
	    z4 = s4->getPosition().Z() - z_CPP_Target;
	    if(e4<0.3) continue;
	  } else {
	    const DBCALShower *s4 = locBCALShowers[j2-locFCALShowers.size()];
	    e4 = s4->E;
	    x4 = s4->x;
	    y4 = s4->y;
	    z4 = s4->z - z_CPP_Target;
	    if(e4<0.1) continue;
	  }

	  if(e3+e4<0.5) continue;
	  if(e1+e2+e3+e4<3.5) continue;

	  double mraw34 = mgg(x3,y3,z3,e3,x4,y4,z4,e4);
	  if(mraw34<0.1) continue;

	  write_2pi0 = true;
	}
      }
    }
  }

  if(write_2g)   {
    locEventWriterEVIO->Write_EVIOEvent(event,"npp_2g");
  }

  if(write_2pi0) {
    locEventWriterEVIO->Write_EVIOEvent(event,"npp_2pi0");
  }

  return; //NOERROR;
}

//------------------
// EndRun
//------------------
void JEventProcessor_npp_skim::EndRun(void)
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return; //NOERROR;
}

//------------------
// Finish
//------------------
void JEventProcessor_npp_skim::Finish(void)
{
  // Called before program exit after event processing is finished.
  return; //NOERROR;
}

