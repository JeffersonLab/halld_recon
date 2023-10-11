// $Id$
//
//    File: DTrackCandidate_factory_CDC_or_FDCpseudo.cc
// Created: Thu Apr 16 09:14:49 EDT 2009
// Creator: davidl (on Darwin harriet.jlab.org 9.6.0 i386)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include "DTrackCandidate_factory_CDC_or_FDCpseudo.h"


//------------------
// Init
//------------------
void DTrackCandidate_factory_CDC_or_FDCpseudo::Init()
{
	DEBUG_LEVEL = 0;

	auto app = GetApplication();
	app->SetDefaultParameter("TRKFIND:DEBUG_LEVEL", DEBUG_LEVEL);
}

//------------------
// BeginRun
//------------------
void DTrackCandidate_factory_CDC_or_FDCpseudo::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void DTrackCandidate_factory_CDC_or_FDCpseudo::Process(const std::shared_ptr<const JEvent>& event)
{

	/// This factory simply combines the list of candidates from the 
	/// DTrackCandidate:CDC and DTrackCandidate:FDCpseudo factories
	/// into a single lit. It simply copies the pointers and flags
	/// itself as not being the owner of any of these objects.
	vector<const DTrackCandidate*> cdc;
	vector<const DTrackCandidate*> fdc;

	event->Get(cdc, "CDC");
	event->Get(fdc, "FDCpseudo");
	
	// Add FDC candidates
	if(DEBUG_LEVEL>2)_DBG_<<"Copying "<<fdc.size()<<" FDC generated candidates to final list"<<endl;
	for(unsigned int i=0; i<fdc.size(); i++)mData.push_back((DTrackCandidate*)fdc[i]);

	// Add CDC candidates providing they aren't clones of ones already added
	if(DEBUG_LEVEL>2)_DBG_<<"Checking "<<cdc.size()<<" CDC generated candidates for inclusion in final list ..."<<endl;
	for(unsigned int i=0; i<cdc.size(); i++){
		// Look to see that we don't already have a similar candidate in the list
		DVector3 mom1 = cdc[i]->momentum();
		DVector3 pos1 = cdc[i]->position();
		double q1 = cdc[i]->charge();
		bool is_clone = false;
		for(unsigned int j=0; j<mData.size(); j++){
			DVector3 mom2 = mData[j]->momentum();
			DVector3 pos2 = mData[j]->position();
			double q2 = mData[j]->charge();
			double curvature_diff = fabs(1.0/mom1.Mag() - 1.0/mom2.Mag())/((1.0/mom1.Mag() + 1.0/mom2.Mag())/2.0);
			double relative_mom_diff = fabs(mom1.Mag()-mom2.Mag())/((mom1.Mag()+mom2.Mag())/2.0);
			if(DEBUG_LEVEL>4)_DBG_<<"q1="<<q1<<" q2="<<q2<<" mom1,mom2 angle:"<<mom1.Angle(mom2)*57.3<<" curvature diff:"<<curvature_diff<<" relative_mom_diff:"<<relative_mom_diff<<" pos diff:"<<(pos1-pos2).Mag()<<endl;
			if(q1!=q2)continue; 
			if(mom1.Angle(mom2)*57.3<5.0){	// Angle within 5 degrees
				if(curvature_diff<0.15 || relative_mom_diff<0.15){ // curvature or momentum within 15 % (should use Perp, but this avoids division by zero)
					if((pos1-pos2).Mag()<30.0){ // vertex position within  30cm
						is_clone = true;
						if(DEBUG_LEVEL>1)_DBG_<<" CDC candidate "<<i<<" is a clone of candidate "<<j<<" dropping."<<endl;
						break;
					}
				}
			}
		}
		if(!is_clone){
			mData.push_back((DTrackCandidate*)cdc[i]);
		}
	}
	
	SetFactoryFlag(NOT_OBJECT_OWNER); // make sure these aren't deleted twice
}

//------------------
// EndRun
//------------------
void DTrackCandidate_factory_CDC_or_FDCpseudo::EndRun()
{
}

//------------------
// Finish
//------------------
void DTrackCandidate_factory_CDC_or_FDCpseudo::Finish()
{
}

