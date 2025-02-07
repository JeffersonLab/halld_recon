
#include "DTRDStripCluster_factory_RAW.h"

#include <DAQ/Df125PulseIntegral.h>
#include <DAQ/Df125WindowRawData.h>
#include <DAQ/Df125FDCPulse.h>
#include <TRD/DTRDDigiHit.h>

#include "TH2F.h"

static bool DTRDHit_cmp(const DTRDHit* a, const DTRDHit* b) 
{
  if (a->plane==b->plane){
    return a->t < b->t;
  }
  return a->plane < b->plane;
}


///
/// DTRDHit_strip_cmp():
///	a non-member function passed to std::sort() for sorting DTRDHit pointers by
/// their strip (wire or strip) numbers. Typically only used for a single layer
/// of hits.
///
static bool DTRDHit_strip_cmp(const DTRDHit* a, const DTRDHit* b) 
{
	if(a->strip != b->strip) return a->strip < b->strip;
	if(a->t       != b->t      ) return a->t < b->t;
	return a->pulse_height < b->pulse_height;
}

///
/// DTRDHit_time_cmp()
///    a non-member function passed to std::stable_sort() for sorting DTRDHit 
/// pointers in order of increasing time, provided that the time difference is
/// significant.
///

static bool DTRDHit_time_cmp(const DTRDHit* a, const DTRDHit* b) 
{
  if (fabs(a->t-b->t)>HIT_TIME_DIFF_MIN && (a->t < b->t))
    return true;
  return false;
}

///
/// DTRDStripCluster_gPlane_cmp():
/// a non-member function passed to std::sort() for sorting DTRDStripCluster pointers
/// by their gPlane (plane number over all modules, 1-74) attributes.
///
static bool DTRDStripCluster_gPlane_cmp(	const DTRDStripCluster* a, 
					const DTRDStripCluster* b) 
{
	return a->plane < b->plane;
}

///
/// Initialization
///
jerror_t DTRDStripCluster_factory_RAW::init(void)
{
//   TIME_SLICE=200.0; //ns
//   gPARMS->SetDefaultParameter("TRD:CLUSTER_TIME_SLICE",TIME_SLICE);

	// hit detection algorithm parameters, in ADC units
	TimeWindowStart = 45;
	TimeMin = 0;
	TimeMax = 100;
	THRESHOLD=600;
	//THRESHOLD=50; 

    gPARMS->SetDefaultParameter("TRDCLUSTERRAW:THRESHOLD",THRESHOLD);
    gPARMS->SetDefaultParameter("TRDCLUSTERRAW:TIME_WINDOW_START",TimeWindowStart);
    gPARMS->SetDefaultParameter("TRDCLUSTERRAW:TIME_MIN",TimeMin);
    gPARMS->SetDefaultParameter("TRDCLUSTERRAW:TIME_MAX",TimeMax);
	
	MINIMUM_HITS_FOR_CLUSTERING = 100;
    gPARMS->SetDefaultParameter("TRDCLUSTERRAW:MINIMUM_HITS_FOR_CLUSTERING",MINIMUM_HITS_FOR_CLUSTERING);

	CLUSTERING_THRESHOLD = 1.2;
    gPARMS->SetDefaultParameter("TRDCLUSTERRAW:CLUSTERING_THRESHOLD",CLUSTERING_THRESHOLD);

	MinClustSize=10;
	MinClustWidth=0.001;
	MinClustLength=0.01;
	zStart =  0.; // mm
	zEnd   = 40.; // mm

    gPARMS->SetDefaultParameter("TRDCLUSTERRAW:MIN_CLUST_SIZE",MinClustSize);
    gPARMS->SetDefaultParameter("TRDCLUSTERRAW:MIN_CLUST_WIDTH",MinClustWidth);
    gPARMS->SetDefaultParameter("TRDCLUSTERRAW:MIN_CLUST_LENGTH",MinClustLength);
    gPARMS->SetDefaultParameter("TRDCLUSTERRAW:ZSTART",zStart);
    gPARMS->SetDefaultParameter("TRDCLUSTERRAW:ZEND",zEnd);


    return NOERROR;	
}

///
/// DTRDStripCluster_factory_RAW::evnt():
/// is the place strip hits are associated into strip clusters.  
///
jerror_t DTRDStripCluster_factory_RAW::evnt(JEventLoop *eventLoop, uint64_t eventNo) 
{
	vector<const DTRDHit*> allHits;
	vector<const DTRDHit*> planeHits[2];
	vector<vector<const DTRDHit*> >thisLayer;
	
	eventLoop->Get(allHits);
	
// 	jerr << "In DTRDStripCluster_factory_RAW::evnt() ..." << endl;
// 	jerr << "  Total hits = " << allHits.size() << endl;
	
	if (allHits.size() == 0) 
		return NOERROR;

	// initialize pulse data vector
	vector< vector<double> > emptyPlaneVec;
	vector<double> blankChannelVec(MAX_ADC_SAMPLES, 0);
	
	// assume two planes
	pulse_data.push_back(emptyPlaneVec);
	pulse_data.push_back(emptyPlaneVec);

	// fill X plane data
	for(int i=0; i<NUM_X_PLANES; i++)
		pulse_data[0].push_back(blankChannelVec);
	// fill Y plane data
	for(int i=0; i<NUM_Y_PLANES; i++)
		pulse_data[1].push_back(blankChannelVec);
	
	// Sort hits by layer number and by time
	sort(allHits.begin(),allHits.end(),DTRDHit_cmp);
	
	int num_good_hits = 0;  // keep track of the number of good hits
	
	// Sift through all hits and select out X and Y hits.
	// Also apply the raw hit analysis
	for (vector<const DTRDHit*>::iterator i = allHits.begin(); i != allHits.end(); ++i) {
		// sort hits
		int stripPlane = (*i)->plane-1;
		int strip = (*i)->strip-1;
		if( (stripPlane<0) || (stripPlane>=2) ){ // only two planes
			static int Nwarn = 0;
			if( Nwarn<10 ){
				jerr << " stripPlane is outside of array bounds!! stripPlane="<< stripPlane << std::endl;
				if( ++Nwarn==10 )jerr << " LAST WARNING!" << std::endl;
			}
			continue;
		}
		planeHits[stripPlane].push_back(*i);
		
		//if(stripPlane != 0) continue;  // only X plane
		
		// get raw data
		// the access path is DTRDDigiHit -> Df125FDCPulse -> Df125WindowRawData
		const DTRDDigiHit *digihit;
		(*i)->GetSingle(digihit);
		if(digihit == nullptr) {
			jerr << "DTRDStripCluster_factory_RAW::evnt(): Could not find DTRDDigiHit objects!" << endl;
			continue; 
		}
		
		const Df125FDCPulse *pulsedata;
		digihit->GetSingle(pulsedata);
		if(pulsedata == nullptr) {
			jerr << "DTRDStripCluster_factory_RAW::evnt(): Could not find Df125FDCPulse objects!" << endl;
			continue; 
		}
		
		const Df125WindowRawData *rawdata;
		pulsedata->GetSingle(rawdata);
		if(rawdata == nullptr) {
			jerr << "DTRDStripCluster_factory_RAW::evnt(): Could not find Df125WindowRawData objects!" << endl;
			continue; 
		}

		// just get the fADC pedestal rather than redoing the calculation
		// this is different than the standalone code, but seems like it should be the same?
		int ped = (*i)->pedestal;
		
        //if (0. > ped || ped > 200 ) ped = 100;  // should we renormalize the pedestals?  I think this is based on beam test data...
        
	    for (int si=0; si<rawdata->samples.size(); si++) {
      	 	int time=si;
      	  	int adc = rawdata->samples.at(si);
      	  	adc = adc-ped;
//         	jerr << " time = " << time << "  adc = " << adc << endl;

      	  	if (adc<THRESHOLD) continue;  // reimplement the threshold cut since we're looking at the raw samples
      	    time -= TimeWindowStart;    
      	    
      	    // --- drop early and late hits from clustering --- 
      	    if ( TimeMin > time || time > TimeMax ) continue; 
      	    
      	    pulse_data.at(stripPlane).at(strip).at(time) = adc/100.;   // for debugging
      	    //pulse_data[stripPlane][time][strip] = adc/100.;
      	    num_good_hits++;
      	}
	}

// 	jerr << "good hits = " << num_good_hits << endl;
// 	jerr << " hits for planes 1, 2 = " << planeHits[0].size() << ", " << planeHits[1].size() << endl;

	// require a minimum number of hits
    if (num_good_hits < MINIMUM_HITS_FOR_CLUSTERING)   
    	return NOERROR; //-- skip event !!!!!

	// do the clustering
	vector<DTRDStripCluster *> clusters; 
 	for(uint iplane = 0; iplane < 2; iplane++) {
 
 		//if(iplane != 0) continue;  // only X plane

 		if(planeHits[iplane].size()>0){

			const float CL_DIST=2.7; // mm
						
			double nx = MAX_ADC_SAMPLES;
			double xmi = 0.;     double xma = nx;
			double ny = 0;
			if(iplane == 0) {
				ny = NUM_X_PLANES;
			} else if(iplane == 1) {
				ny = NUM_Y_PLANES;
			} 
			double ymi = 0.;     double yma = ny;
 			double binx = (xma-xmi)/nx;      double biny = (yma-ymi)/ny;
			
			// scan BinContent of hp = hevt
			for (int ix=0; ix<nx; ix++) {  //-------------------- clustering loop ------------------------------------
			  for (int iy=0; iy<ny; iy++) {
				//double c1 = hpc->GetBinContent(ix,iy);                    // energy
				double c1 = pulse_data[iplane][iy][ix];          // energy
				double x1=double(ix)/double(nx)*(xma-xmi)+xmi-binx/2.;    // drift time
				double y1=double(iy)/double(ny)*(yma-ymi)+ymi-biny/2.;    // X strip
				
				if (c1 < CLUSTERING_THRESHOLD) continue;
	
				// first iteration if c1 > threshold, set clust_Xpos and clust_Zpos
				if (clusters.size() == 0) {
					DTRDStripCluster *new_cluster = new DTRDStripCluster;
					new_cluster->plane = iplane+1;
					new_cluster->q_tot = c1;
					new_cluster->num_hits = 1;
					new_cluster->pos.SetXYZ(y1, 0, x1);
					new_cluster->width.SetXYZ(y1, y1, 0);
					new_cluster->length.SetXYZ(x1, x1, 0);
					
// 					jerr << "NEW cluster" << endl;
// 					jerr << " q_tot = " << new_cluster->q_tot << endl;
// 					jerr << " ";  new_cluster->pos.Print();
					
					clusters.push_back(new_cluster);
					continue;
				}
				
				  int added=0;
				  for (int k=0; k<clusters.size(); k++) {
					double dist=sqrt(pow((y1-clusters[k]->pos.x()),2.)+pow((x1-clusters[k]->pos.z()),2.)); //--- dist hit to clusters
					// check the distance from the x1,y1 to the center of the cluster based on the radius (2.7 mm)
					if (dist<CL_DIST) {
					  // if it's within the radius set this as a new position using weighted average to approximate the new central position
					  clusters[k]->pos.SetX( (y1*c1+clusters[k]->pos.x()*clusters[k]->q_tot)/(c1+clusters[k]->q_tot) );  //--  new X pos
					  clusters[k]->pos.SetZ( (x1*c1+clusters[k]->pos.z()*clusters[k]->q_tot)/(c1+clusters[k]->q_tot) );  //--  new Z pos
					  // new dedx is the sum of the two weighted averaged amplitude
					  clusters[k]->q_tot += c1;  // new dEdx
					  clusters[k]->num_hits++;
	
					  // update cluster width in x and y
					  if (y1<clusters[k]->width.x()) clusters[k]->width.SetX(y1); 
					  if (y1>clusters[k]->width.y()) clusters[k]->width.SetY(y1); 
					  clusters[k]->width.SetZ( clusters[k]->width.y() - clusters[k]->width.x() );
					  if (x1<clusters[k]->length.x()) clusters[k]->length.SetX(x1); 
					  if (x1>clusters[k]->length.y()) clusters[k]->length.SetY(x1); 
					  clusters[k]->length.SetZ( clusters[k]->length.y() - clusters[k]->length.x() );

// 					  jerr << "UPDATE cluster" << endl;
// 					  jerr << " q_tot = " << clusters[k]->q_tot << endl;
// 					  jerr << " ";  clusters[k]->pos.Print();
					
					  added=1; break;
					}
				  }
	
				// if it's outside the radius, set this as a new center
				  if (added==0) {					
					DTRDStripCluster *new_cluster = new DTRDStripCluster;
					new_cluster->plane = iplane+1;
					new_cluster->q_tot = c1;
					new_cluster->num_hits = 1;
					new_cluster->pos.SetXYZ(y1, 0, x1);
					new_cluster->width.SetXYZ(y1, y1, 0);
					new_cluster->length.SetXYZ(x1, x1, 0);

// 					jerr << "NEW cluster" << endl;
// 					jerr << " q_tot = " << new_cluster->q_tot << endl;
// 					jerr << " ";  new_cluster->pos.Print();
				  }
				}
			} //----------- end  clustering loop ---------------
	
		}
	}


	// Apply quality cuts before we save the clusters
	for (int k=0; k<clusters.size(); k++) {
	  //-------------  Cluster Filter -----------------
	  //if (clusters[k]->num_hits>= MinClustSize && zStart < clusters[k]->pos.z() && clusters[k]->pos.z() < zEnd 
	  //		&& clusters[k]->width.z()>MinClustWidth )
		_data.push_back(clusters[k]);
// 			  else 
// 			     delete clusters[k];

	}

	// Plane by plane, create clusters of strips
// 	for(uint iplane = 0; iplane < 2; iplane++) {
// 		if (planeHits[iplane].size()>0){
// 			thisLayer.clear();
// 			vector<const DTRDHit*>::iterator i = planeHits[iplane].begin();	      
// 			vector<const DTRDHit*> hits;	
// 			float old_time=(*i)->t;
// 			while(i!=planeHits[iplane].end()){ 
// 				// Look for hits falling within a time slice
// 				if (fabs((*i)->t-old_time)>TIME_SLICE){
// 					// Sort hits by strip number
// 					sort(hits.begin(),hits.end(),DTRDHit_strip_cmp);
// 					// put into the vector
// 					thisLayer.push_back(hits);
// 					hits.clear();
// 					old_time=(*i)->t;
// 				}
// 				hits.push_back(*i);
// 				
// 				i++;
// 			}
// 			
// 			// Sort hits by strip number
// 			sort(hits.begin(),hits.end(),DTRDHit_strip_cmp);
// 			// add the last vector of hits
// 			thisLayer.push_back(hits);
// 			
// 			// Create clusters from these lists of hits
// 			for (unsigned int k=0;k<thisLayer.size();k++) pique(thisLayer[k]);
// 			
// 			// Clear the hits and layer vectors for the next ones
// 			thisLayer.clear();	
// 			hits.clear();
// 		}
// 	}

	// Ensure that the data are still in order of planes.
	std::sort(_data.begin(), _data.end(), DTRDStripCluster_gPlane_cmp);
	
// 	delete hevt;
// 	delete hevtc;
	//delete hevtf;
	
	return NOERROR;	
}			

// //-----------------------------
// // pique
// //-----------------------------
// void DTRDStripCluster_factory_RAW::pique(vector<const DTRDHit*>& H)
// {
// 	/// Find clusters within GEM plane.
// 	///
// 	/// Upon entry, the vector "H" should already be sorted
// 	/// by strip number and should only contains hits from
// 	/// the same plane that are in time with each other.
// 	/// This will form clusters from all contiguous strips.
// 
// 	// Loop over hits
// 	for(uint32_t istart=0; istart<H.size(); istart++){
// 		const DTRDHit *first_hit = H[istart];
// 		
// 		// Find end of contiguous section
// 		uint32_t iend=istart+1;
// 		for(; iend<H.size(); iend++){
// 			if(iend>=H.size()) break;
// 			if( (H[iend]->strip - H[iend-1]->strip) > 1 ) break;
// 		}
// 		if( (iend-istart)<2 ) continue; // don't allow single strip clusters
// 		
// 		// istart should now point to beginning of cluster 
// 		// and iend to one past end of cluster
// 		DTRDStripCluster* newCluster = new DTRDStripCluster();
// 		newCluster->q_tot   = 0.0;
// 		newCluster->plane   = first_hit->plane;
// 		for(uint32_t i=istart; i<iend; i++){
// 			newCluster->q_tot += H[i]->pulse_height;
// 			newCluster->members.push_back(H[i]);
// 		}
// 		_data.push_back(newCluster);
// 		
// 		istart = iend-1;
// 	}
// }

