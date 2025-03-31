// $Id$
//
//    File: DNeutralShower_factory_HadronPreSelect.cc
// Created: Tue Aug  9 14:29:24 EST 2011
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#include "DNeutralShower_factory_HadronPreSelect.h"
#include <JANA/JEvent.h>
#include <DANA/DGeometryManager.h>

//------------------
// init
//------------------
void DNeutralShower_factory_HadronPreSelect::Init()
{
	//Setting this flag makes it so that JANA does not delete the objects in _data.  This factory will manage this memory. 
	//This is because some/all of these pointers are just copied from earlier objects, and should not be deleted.  
	SetFactoryFlag(NOT_OBJECT_OWNER);

	dMinFCALE = 0.1;
	dMinBCALE = 0.05;
	dMinCCALE = 0.1;
	dMinBCALNcell = 1;   // reduced from 2 cells for low energy neutrons
	dMaxFCALR = 105.5;
	dMaxBCALZ = 393.0;
	
	dFCALInnerRingCut = true;
}

//------------------
// brun
//------------------
void DNeutralShower_factory_HadronPreSelect::BeginRun(const std::shared_ptr<const JEvent> &event)
{
	auto app = GetApplication();
	app->SetDefaultParameter("HADRONPRESELECT:MIN_FCAL_E", dMinFCALE);
	app->SetDefaultParameter("HADRONPRESELECT:MIN_BCAL_E", dMinBCALE);
	app->SetDefaultParameter("HADRONPRESELECT:MIN_CCAL_E", dMinCCALE);
	app->SetDefaultParameter("HADRONPRESELECT:MIN_BCAL_NCELL", dMinBCALNcell);
	app->SetDefaultParameter("HADRONPRESELECT:MIN_FCAL_R", dMaxFCALR);
	app->SetDefaultParameter("HADRONPRESELECT:MIN_BCAL_Z", dMaxBCALZ);
	app->SetDefaultParameter("HADRONPRESELECT:FCAL_INNER_CUT", dFCALInnerRingCut);

	auto geo_manager = app->GetService<DGeometryManager>();
	auto dgeom = geo_manager->GetDGeometry(event->GetRunNumber());
	haveInsert=dgeom->HaveInsert();

	// get FCAL Geometry object
	vector< const DFCALGeometry * > fcalGeomVec;
	event->Get( fcalGeomVec );

	if( fcalGeomVec.size() != 1 ){
		cerr << "Could not load FCAL Geometry!" << endl;
		return; // RESOURCE_UNAVAILABLE;
	}
  	dFCALGeometry = fcalGeomVec[0];		

	if (haveInsert){
	  const DECALGeometry *ecalGeometry=nullptr;
	  event->GetSingle(ecalGeometry);

	  cout << "Setting exclusion" << endl;
	  // build list of channels in the inner ring.  Pick all channels which
	  // touch the beam hole (including corners!)
	  dFCALInnerChannels.push_back( ecalGeometry->channel( 18, 18 ) );
	  dFCALInnerChannels.push_back( ecalGeometry->channel( 18, 19 ) );
	  dFCALInnerChannels.push_back( ecalGeometry->channel( 18, 20 ) );
	  dFCALInnerChannels.push_back( ecalGeometry->channel( 18, 21 ) );
	  dFCALInnerChannels.push_back( ecalGeometry->channel( 19, 18 ) );
	  dFCALInnerChannels.push_back( ecalGeometry->channel( 20, 18 ) );
	  dFCALInnerChannels.push_back( ecalGeometry->channel( 19, 21 ) );
	  dFCALInnerChannels.push_back( ecalGeometry->channel( 20, 21 ) );
	  dFCALInnerChannels.push_back( ecalGeometry->channel( 21, 18 ) );
	  dFCALInnerChannels.push_back( ecalGeometry->channel( 21, 19 ) );
	  dFCALInnerChannels.push_back( ecalGeometry->channel( 21, 20 ) );
	  dFCALInnerChannels.push_back( ecalGeometry->channel( 21, 21 ) );
	}
	else{
	  // build list of channels in the inner ring.  Pick all channels which
	  // touch the beam hole (including corners!)
	  dFCALInnerChannels.push_back( dFCALGeometry->channel( 27, 27 ) );
	  dFCALInnerChannels.push_back( dFCALGeometry->channel( 27, 28 ) );
	  dFCALInnerChannels.push_back( dFCALGeometry->channel( 27, 29 ) );
	  dFCALInnerChannels.push_back( dFCALGeometry->channel( 27, 30 ) );
	  dFCALInnerChannels.push_back( dFCALGeometry->channel( 27, 31 ) );
	  dFCALInnerChannels.push_back( dFCALGeometry->channel( 31, 27 ) );
	  dFCALInnerChannels.push_back( dFCALGeometry->channel( 31, 28 ) );
	  dFCALInnerChannels.push_back( dFCALGeometry->channel( 31, 29 ) );
	  dFCALInnerChannels.push_back( dFCALGeometry->channel( 31, 30 ) );
	  dFCALInnerChannels.push_back( dFCALGeometry->channel( 31, 31 ) );
	  dFCALInnerChannels.push_back( dFCALGeometry->channel( 28, 27 ) );
	  dFCALInnerChannels.push_back( dFCALGeometry->channel( 29, 27 ) );
	  dFCALInnerChannels.push_back( dFCALGeometry->channel( 30, 27 ) );
	  dFCALInnerChannels.push_back( dFCALGeometry->channel( 28, 31 ) );
	  dFCALInnerChannels.push_back( dFCALGeometry->channel( 29, 31 ) );
	  dFCALInnerChannels.push_back( dFCALGeometry->channel( 30, 31 ) );
	}
}

//------------------
// evnt
//------------------
void DNeutralShower_factory_HadronPreSelect::Process(const std::shared_ptr<const JEvent> &event)
{
	//Clear objects from last event
	mData.clear();

	//Get original objects
	vector<const DNeutralShower*> locNeutralShowers;
	event->Get(locNeutralShowers);

	//Cut on shower energy, BCAL cells, fiducial regions
	for(size_t loc_i = 0; loc_i < locNeutralShowers.size(); ++loc_i)
	{
		if(locNeutralShowers[loc_i]->dDetectorSystem == SYS_FCAL)
		{
			if(locNeutralShowers[loc_i]->dEnergy < dMinFCALE) 
				continue;

			// Fiducial cut: reject showers whose center is in the outer range of the FCAL,
			//   which is partially shadowed by the BCAL
			if(locNeutralShowers[loc_i]->dSpacetimeVertex.Vect().Perp() > dMaxFCALR)
				continue;
			// Fiducial cut: reject showers in the inner ring(s) which may leak into the beam hole 
			if(haveInsert==false && dFCALInnerRingCut) {
			  double locX=locNeutralShowers[loc_i]->dSpacetimeVertex.X();
			  double locY=locNeutralShowers[loc_i]->dSpacetimeVertex.Y();
			  // sanity check
			  if(dFCALGeometry == nullptr) 
			    jerr << "In DNeutralShower_factory_HadronPreSelect::evnt(), no FCAL Geometry???" << endl;
			  
			  int row=dFCALGeometry->y_to_row(locY);
			  int col=dFCALGeometry->x_to_column(locX);
			  int channel = dFCALGeometry->channel(row,col);
			  
			  // is the center of shower in one of the channels outside of our fiducial cut?
			  if( find(dFCALInnerChannels.begin(), dFCALInnerChannels.end(), channel) 
			      != dFCALInnerChannels.end() ) {
			    continue;
			  }
			}
		}
		else if(locNeutralShowers[loc_i]->dDetectorSystem == SYS_BCAL)
		  {
			if(locNeutralShowers[loc_i]->dEnergy < dMinBCALE)
				continue;
			// Fiducial cut: reject showers too close to the downstream face of the BCAL 
			if(locNeutralShowers[loc_i]->dSpacetimeVertex.Z() > dMaxBCALZ)
				continue;

			const DBCALShower* locBCALShower = NULL;
			locNeutralShowers[loc_i]->GetSingle(locBCALShower);
			if(locBCALShower->N_cell < dMinBCALNcell)
				continue;
		  }
		else if(locNeutralShowers[loc_i]->dDetectorSystem == SYS_CCAL)
		  {
			if(locNeutralShowers[loc_i]->dEnergy < dMinCCALE)
				continue;
			// Fiducial cut: reject showers too close to the downstream face of the BCAL 
			//if(locNeutralShowers[loc_i]->dSpacetimeVertex.Z() > dMaxBCALZ)
			//	continue;

			//const DCCALShower* locCCALShower = NULL;
			//locNeutralShowers[loc_i]->GetSingleT(locCCALShower);
			//if(locCCALShower->N_cell < dMinCCALNcell)
			//	continue;
		  }

		Insert(const_cast<DNeutralShower*>(locNeutralShowers[loc_i]));
	}
}

//------------------
// erun
//------------------
void DNeutralShower_factory_HadronPreSelect::EndRun()
{
}

//------------------
// fini
//------------------
void DNeutralShower_factory_HadronPreSelect::Finish()
{
}
