// $Id: DFCALShower_factory.cc 2001 2006-11-28 11:09:47Z mikornic $
//
//    File: DFCALShower_factory.cc
// Created: Tue Nov 28 11:57:50 EST 2006
// Creator: remitche (on Linux mantrid00 2.4.20-18.8smp i686)
//

#include <iostream>
#include <fstream>
#include <math.h>
#include <DVector3.h>
using namespace std;

#include <JANA/JEvent.h>
#include "DANA/DEvent.h"

#include "FCAL/DFCALCluster.h"
#include "FCAL/DFCALCluster_factory.h"
#include "FCAL/DFCALHit.h"
#include "FCAL/DFCALGeometry.h"
#include <HDGEOMETRY/DGeometry.h>

// Used to sort hits by Energy
int FCALHitsSort_C(const void *a,const void *b){
  const DFCALCluster::userhit_t hit1=*(const DFCALCluster::userhit_t *)a;
  const DFCALCluster::userhit_t hit2=*(const DFCALCluster::userhit_t *)b;
  if (hit1.E<hit2.E) return 1;
  return -1;
}

const DFCALHit *GetDFCALHitFromClusterHit(const DFCALCluster::DFCALClusterHit_t &theClusterHit, const vector<const DFCALHit*> &fcalhits) {
    for( vector<const DFCALHit*>::const_iterator fcalhits_itr = fcalhits.begin();
         fcalhits_itr != fcalhits.end(); fcalhits_itr++) {
        if(theClusterHit.id == (*fcalhits_itr)->id)
            return *fcalhits_itr;
    }

    return NULL;
}

//----------------
// Constructor
//----------------
void DFCALCluster_factory::Init()
{
	// Set defaults
	MIN_CLUSTER_BLOCK_COUNT = 2;
	MIN_CLUSTER_SEED_ENERGY = 0.035; // GeV
	TIME_CUT = 15.0 ; //ns
	MAX_HITS_FOR_CLUSTERING = 250;
	REMOVE_BAD_BLOCK = 0;
	
	auto app = GetApplication();
	app->SetDefaultParameter("FCAL:MIN_CLUSTER_BLOCK_COUNT", MIN_CLUSTER_BLOCK_COUNT);
	app->SetDefaultParameter("FCAL:MIN_CLUSTER_SEED_ENERGY", MIN_CLUSTER_SEED_ENERGY);
	app->SetDefaultParameter("FCAL:MAX_HITS_FOR_CLUSTERING", MAX_HITS_FOR_CLUSTERING);
	app->SetDefaultParameter("FCAL:TIME_CUT",TIME_CUT,"time cut for associating FCAL hits together into a cluster");
	app->SetDefaultParameter("FCAL:REMOVE_BAD_BLOCK",REMOVE_BAD_BLOCK,"remove bad block");
}

//------------------
// BeginRun
//------------------
void DFCALCluster_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	double targetZ;
	double fcalFrontFaceZ;

	// Get geometry
	auto runnumber = event->GetRunNumber();
	auto app = event->GetJApplication();
	auto geo_manager = app->GetService<DGeometryManager>();
	auto dgeom = geo_manager->GetDGeometry(runnumber);

	if( dgeom ){
	  dgeom->GetTargetZ( targetZ );
	  dgeom->GetFCALZ( fcalFrontFaceZ );
	}
	else{

	  cerr << "No geometry accessible." << endl;
	  return; // RESOURCE_UNAVAILABLE;
	}

	fcalFaceZ_TargetIsZeq0 = fcalFrontFaceZ - targetZ;

	bad_blocks_list.clear();
	if (DEvent::GetCalib(event, "/FCAL/bad_block", bad_blocks_list))
	  jout << "Error loading /FCAL/bad_block !" << endl;
	
}

//------------------
// Process
//    Implementation of UConn LGD clusterizer (M. Kornicer)
//------------------
void DFCALCluster_factory::Process(const std::shared_ptr<const JEvent>& event)
{

	vector<const DFCALHit*> fcalhits;
	event->Get(fcalhits);
	if (fcalhits.size()==0) return;
	
	// LED events will have hits in nearly every channel. Do NOT
	// try clusterizing if more than 250 hits in FCAL
	if(fcalhits.size() > MAX_HITS_FOR_CLUSTERING) return; //NOERROR;
	
	const DFCALGeometry* fcalGeom=NULL;
	event->GetSingle(fcalGeom);

	// fill user's hit list
        int nhits = 0;
	//oid_t id=1;
        DFCALCluster::userhits_t* hits = 
	  (DFCALCluster::userhits_t*) malloc(sizeof(DFCALCluster::userhits_t)*FCAL_USER_HITS_MAX);

	// Fill the structure that used to be used by clusterizers in Radphi 
	for (vector<const DFCALHit*>::const_iterator hit  = fcalhits.begin(); 
                                                     hit != fcalhits.end(); hit++ ) {

	   if ( (**hit).E <  1e-6 ) continue;
	   if (REMOVE_BAD_BLOCK == 1 &&
	       bad_blocks_list.size() > 0 &&
	       0 <= fcalGeom->channel( (**hit).row, (**hit).column ) && fcalGeom->channel( (**hit).row, (**hit).column ) <= 2799 &&
	       bad_blocks_list[fcalGeom->channel( (**hit).row, (**hit).column )] == 1) continue;
           //hits->hit[nhits].id = id++;
           hits->hit[nhits].id = (**hit).id;
	   hits->hit[nhits].ch = fcalGeom->channel( (**hit).row, (**hit).column );
           hits->hit[nhits].x = (**hit).x;
           hits->hit[nhits].y = (**hit).y;
           hits->hit[nhits].E = (**hit).E; 
           hits->hit[nhits].t = (**hit).t;
	   hits->hit[nhits].intOverPeak = (**hit).intOverPeak;
           nhits++;
      
           if (nhits >= (int) FCAL_USER_HITS_MAX)  { 
              cout << "ERROR: FCALCluster_factory: number of hits " 
		   << nhits << " larger than " << FCAL_USER_HITS_MAX << endl;
              break;
           }
        }
        hits->nhits = nhits;

        int hitUsed[nhits]; 
        for ( int i = 0; i < nhits; i++ ) {
	  hitUsed[i] = 0; 
	}

	// Sort the hit array by energy
	qsort(&hits->hit[0],nhits,sizeof(DFCALCluster::userhit_t),FCALHitsSort_C);
	
        const unsigned int max = 999;
	DFCALCluster* clusterList[max];
	unsigned int clusterCount = 0;
	int iter;
	for ( iter=0; iter < 99; iter++ ) {

          // 1. At beginning of iteration, recompute info for all clusters.
          //    If something changed, return all hits to the pool and repeat.

	   bool something_changed = false;
	   for ( unsigned int c = 0; c < clusterCount; c++ ) {
              //cout << " --------- Update iteration " << iter << endl;
	     something_changed |= clusterList[c]->update( hits, fcalFaceZ_TargetIsZeq0, fcalGeom );
           }
      	   if (something_changed) {
              for ( unsigned int c = 0; c < clusterCount; c++ ) {
                  clusterList[c]->resetClusterHits();
              }
              // reset hits in factory also:
              for ( int h = 0; h < nhits; h++ ) hitUsed[h] = 0;
           }
           else if (iter > 0) {
              break;
           }
           
	   // 2. Look for blocks with energy large enough to require formation
	   //    of a new cluster, and assign them as cluster seeds.

	   for ( int ih = 0; ih < hits->nhits; ih++ ) {
	      double energy = hits->hit[ih].E;
              //cout << "hit: " << ih <<  " E: " << energy << endl;
	      if (energy < MIN_CLUSTER_SEED_ENERGY)
		 break;
	      double totalAllowed = 0;
	      for ( unsigned int c = 0; c < clusterCount; c++ ) {
		 totalAllowed += clusterList[c]->getEallowed(ih);
                 //cout << " totalAlowed from clust " << c <<  " is " << totalAllowed << endl;
                 
	      }
	      if (energy > totalAllowed) {
		 clusterList[clusterCount] = new DFCALCluster( hits->nhits );
                 hitUsed[ih] = -1;
		 clusterList[clusterCount]->addHit(ih,1.);
		 clusterList[clusterCount]->update( hits, fcalFaceZ_TargetIsZeq0, fcalGeom );
		 ++clusterCount;
	      }
	      else if (iter > 0) {
		 for ( unsigned int c = 0; c < clusterCount; c++ ) {
                    int nh_clust = clusterList[c]->getHits();
                    //cout << " Nhits " << nh_clust << " from clust " << c << " ? " << endl;
		    if ( nh_clust )
		       continue;
		    totalAllowed -= clusterList[c]->getEallowed(ih);
                    //cout << " else totalAlowed from " << c <<  " E: " << totalAllowed << endl;
		    if (energy > totalAllowed) {
                       if ( clusterList[c]->getHits() == 0  ) {
                          hitUsed[ih] = -1;
                       }
                       else {
                          ++hitUsed[ih];
                       }
		       clusterList[c]->addHit(ih,1.);
		       break;
		    }
	         }
	      }
	   }


	   // 3. Share all non-seed blocks among seeded clusters, where
	   //    any cluster shares a block if it expects at least 1 KeV in it.

	   for ( int ih = 0; ih < hits->nhits; ih++ ) {
              if ( hitUsed[ih]  < 0) // cannot share seed 
		 continue;
	      double totalExpected = 0;
              //cout << " Share hit: " << ih <<  " E: " << hits->hit[ih].E 
	      //   << " ch: " << hits->hit[ih].ch << " t: " << hits->hit[ih].t
	      //	   << endl;
	      for ( unsigned int c = 0; c < clusterCount; c++ ) {
		 if (clusterList[c]->getHits() > 0) {
		    totalExpected += clusterList[c]->getEexpected(ih);
		 }
	      }
              //cout << " totExpected " << totalExpected ;
	      for ( unsigned int c = 0; c < clusterCount; c++ ) {
		 if (clusterList[c]->getHits() > 0) {
		    double expected = clusterList[c]->getEexpected(ih);
                    //cout << " expected " << expected ;
		    if (expected > 1e-6 
			&& fabs(clusterList[c]->getTimeMaxE()
				-hits->hit[ih].t)<TIME_CUT	) {
		       clusterList[c]->addHit(ih,expected/totalExpected);
                       ++hitUsed[ih];
		    }
		 }
	      }
	   }
        }

        for ( unsigned int c = 0; c < clusterCount; c++) {
           unsigned int blockCount = clusterList[c]->getHits();
           //cout << " Blocks " << blockCount << endl;
	   if (blockCount < MIN_CLUSTER_BLOCK_COUNT) {
              delete clusterList[c];
	      continue;
	   }
	   else {

              clusterList[c]->saveHits( hits );

              // save associated FCAL hit information
              const vector<DFCALCluster::DFCALClusterHit_t> &clusterHits = clusterList[c]->GetHits();
              for(size_t loc_i = 0; loc_i < clusterHits.size(); loc_i++) {
                  const DFCALHit *clusterHit = GetDFCALHitFromClusterHit(clusterHits[loc_i], fcalhits);
                  if( clusterHit != NULL ) {
                      clusterList[c]->AddAssociatedObject( clusterHit );
                    }
              }

              Insert( clusterList[c] );
	   }
        }
  

        if (hits) {
           free(hits);
           hits=0;
        }
}

