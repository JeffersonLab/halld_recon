// $Id$
//
//    File: DTrackFinder.cc
// Created: Fri Aug 15 09:43:08 EDT 2014
// Creator: staylor (on Linux ifarm1102 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#include "DTrackFinder.h"

bool DTrackFinder_cdc_hit_cosmics_cmp(const DCDCTrackHit *a,const DCDCTrackHit *b){
   return(a->wire->origin.Y()<b->wire->origin.Y());
}

bool DTrackFinder_cdc_hit_cmp(const DCDCTrackHit *a,const DCDCTrackHit *b){
   return (a->wire->ring<b->wire->ring);
}

bool DTrackFinder_fdc_hit_cmp(const DTrackFinder::fdc_hit_t &a,
      const DTrackFinder::fdc_hit_t &b){
   return (a.hit->wire->origin.z()>b.hit->wire->origin.z());
}


//---------------------------------
// DTrackFinder    (Constructor)
//---------------------------------
DTrackFinder::DTrackFinder(JApplication* app)
{

   COSMICS=false;
   app->SetDefaultParameter("TRKFIND:COSMICS",COSMICS);

   DEBUG_HISTS=false;
   app->SetDefaultParameter("TRKFIND:DEBUG_HISTS", DEBUG_HISTS);

   VERBOSE=0;
   app->SetDefaultParameter("TRKFIND:VERBOSE", VERBOSE);

   FDC_MATCH_RADIUS=2.;
   if (COSMICS) FDC_MATCH_RADIUS=10.;
   app->SetDefaultParameter("FDC:MATCH_RADIUS", FDC_MATCH_RADIUS);

   CDC_MATCH_RADIUS=2.5;
   app->SetDefaultParameter("TRKFIND:CDC_MATCH_RADIUS", CDC_MATCH_RADIUS);

   CDC_MATCH_PHI=0.2;
   app->SetDefaultParameter("TRKFIND:CDC_MATCH_PHI", CDC_MATCH_PHI);
   
   CDC_COSMIC_MATCH_PHI=0.55;
   app->SetDefaultParameter("TRKFIND:CDC_COSMIC_MATCH_PHI", CDC_COSMIC_MATCH_PHI);

   if (DEBUG_HISTS){
      hCDCMatch_PairD = new TH1F("CDC Pair distance", "CDC Pair distance", 100, 0.0, 20.0);
      hCDCMatch_Axial = new TH1F("CDC Match Axial", "CDC Match Axial", 360, -M_PI_2, M_PI_2);
   
      hFDCLayerRaw = new TH1I("FDC Layer Raw", "Layer of FDC hit before track finding", 24, 0.5 , 24.5);
      hFDCLayer = new TH1I("FDC Layer", "Layer of FDC hit after segment finding", 24, 0.5 , 24.5);
      hFDCLayerFirst = new TH1I("First FDC Layer", "First Layer of FDC segment finding", 24, 0.5 , 24.5);
   }
}

//---------------------------------
// ~DTrackFinder    (Destructor)
//---------------------------------
DTrackFinder::~DTrackFinder()
{

}

void DTrackFinder::Reset(void){

   axial_hits.clear();
   stereo_hits.clear();

   for (unsigned int i=0;i<axial_segments.size();i++){
      axial_segments[i].hits.clear();
   }    
   axial_segments.clear();

   for (unsigned int i=0;i<cdc_tracks.size();i++){
      cdc_tracks[i].axial_hits.clear();
      cdc_tracks[i].stereo_hits.clear();
   }
   cdc_tracks.clear();

   fdc_hits.clear();
   for (unsigned int i=0;i<4;i++){
      for (unsigned int j=0;j<fdc_segments[i].size();j++){
         fdc_segments[i][j].hits.clear();
      }
      fdc_segments[i].clear();
   }
   fdc_tracks.clear();

}

void DTrackFinder::AddHit(const DFDCPseudo *hit){
   fdc_hits.push_back(fdc_hit_t(hit));
}


void DTrackFinder::AddHit(const DCDCTrackHit *hit){  
   int ring=hit->wire->ring;
   if (ring<=4) axial_hits.push_back(cdc_hit_t(hit));
   else if (ring<=12) stereo_hits.push_back(cdc_hit_t(hit));
   else if (ring<=16) axial_hits.push_back(cdc_hit_t(hit));
   else if (ring<=24) stereo_hits.push_back(cdc_hit_t(hit));
   else axial_hits.push_back(cdc_hit_t(hit)); 
}

// Find segments in cdc axial layers
bool DTrackFinder::FindAxialSegments(void){
   if (axial_hits.size()==0) return false;
   //jout << " Entering track finding with " << axial_hits.size() << " axial CDC hits" << endl;

   // Group adjacent axial_hits into pairs
   vector<pair<unsigned int,unsigned int> > pairs;
   for (unsigned int i=0;i<axial_hits.size()-1;i++){
      for (unsigned int j=i+1;j<axial_hits.size();j++){ 
         const DCDCWire *first_wire=axial_hits[i].hit->wire;
         const DCDCWire *second_wire=axial_hits[j].hit->wire;
         int r1=first_wire->ring;
         int r2=second_wire->ring;
         int s1=first_wire->straw;
         int s2=second_wire->straw;
         double d=(first_wire->origin-second_wire->origin).Perp();
         if (DEBUG_HISTS) hCDCMatch_PairD->Fill(d);
         if ((abs(r1-r2)<=2 && abs(r1-r2)>0 && d<CDC_MATCH_RADIUS) 
               || (r1 == r2 && abs(s1-s2)==1)){
            pair <unsigned int,unsigned int> mypair(i,j);
            if(VERBOSE) jout<< " Pair formed R" << r1 << " S" << s1 << " ,R" << r2 << " S" << s2 << jendl;
            pairs.push_back(mypair);
         }
      }
   }
   //jout << " Made " << pairs.size() << " pairs" << endl;
   // Link pairs of axial_hits together into segments
   for (unsigned int i=0;i<pairs.size();i++){
      if (axial_hits[pairs[i].first].used==false 
            && axial_hits[pairs[i].second].used==false){
         vector<const DCDCTrackHit *>neighbors;
         set<unsigned int> used;
         axial_hits[pairs[i].first].used=true;
         axial_hits[pairs[i].second].used=true;
         neighbors.push_back(axial_hits[pairs[i].first].hit);
         neighbors.push_back(axial_hits[pairs[i].second].hit);
         used.insert(pairs[i].first);
         used.insert(pairs[i].second);
         for (unsigned int j=i+1;j<pairs.size();j++){
            // If any of the hits match in the pairs, add them together
            unsigned int first=pairs[j].first;
            unsigned int second=pairs[j].second;
            set<unsigned int>::iterator it1, it2;
            it1 = used.find(first);
            it2 = used.find(second);

            if (it1 != used.end() && it2 == used.end()){
               axial_hits[second].used=true;
               neighbors.push_back(axial_hits[second].hit);
               used.insert(second);
            }
            else if (it2 != used.end() && it1 == used.end()){
               axial_hits[first].used=true;
               neighbors.push_back(axial_hits[first].hit);
               used.insert(first);
            }

         }
         if (COSMICS){
            sort(neighbors.begin(),neighbors.end(),DTrackFinder_cdc_hit_cosmics_cmp);
         } else{
            sort(neighbors.begin(),neighbors.end(),DTrackFinder_cdc_hit_cmp);
         }

         // Take Average direction from hits
         DVector3 dir(0.,0.,0.);
         DVector3 origin;
         if(COSMICS){
            unsigned int iHit = 1;
            origin=neighbors[0]->wire->origin;
            for (iHit = 1; iHit < neighbors.size(); iHit++){
               dir+=neighbors[iHit]->wire->origin-origin;
            }
	 
            if(dir.Mag() != 0.) dir.SetMag(1.);
         }
         else{
            for (unsigned int iHit = 0; iHit < neighbors.size(); iHit++) dir += neighbors[iHit]->wire->origin;
            if(dir.Mag() != 0.) dir.SetMag(1.);
         }
	 
         if (VERBOSE){
            jout << " Axial Segment Formed: " << jendl;
            for(unsigned int jj = 0; jj<neighbors.size(); jj++){
               jout << "  R" << neighbors[jj]->wire->ring << " S" << neighbors[jj]->wire->straw << jendl;
            }
            jout << "Dir" << jendl;
            dir.Print();
         }
         axial_segments.push_back(cdc_segment_t(neighbors,dir));
      }
   }

   //jout << " into " << axial_segments.size() << " axial segments" << endl;
   return true;
}



// Link axial segments together to form track candidates and match to stereo 
// hits
bool DTrackFinder::LinkCDCSegments(void){
   unsigned int num_axial=axial_segments.size();
   if (num_axial<1) return false;
   for (unsigned int i=0;i<num_axial;i++){
      if (axial_segments[i].matched==false){
         DTrackFinder::cdc_track_t mytrack(axial_segments[i].hits);

	 const DCDCWire *wire1=axial_segments[i].hits[0]->wire;
         DVector3 vhat=axial_segments[i].dir;
	 int superlayer1=(wire1->ring-1)/4;
         for (unsigned int j=i+1;j<num_axial;j++){
            if (axial_segments[j].matched==false){	      
	      const DCDCWire *wire2=axial_segments[j].hits[0]->wire;
	      int superlayer2=(wire2->ring-1)/4;
	      if (superlayer1==superlayer2){
		// Don't try to link segments in the same ring for events with 
		// the beam 
		if (COSMICS==false) continue;
	      }

               double dphi = axial_segments[j].dir.Phi() - axial_segments[i].dir.Phi();
               while (dphi>M_PI) dphi-=2*M_PI;
               while (dphi<-M_PI) dphi+=2*M_PI;
               double matchphi = CDC_MATCH_PHI;
	       if (DEBUG_HISTS){
		 hCDCMatch_Axial->Fill(dphi);
	       }
               if (COSMICS) matchphi = CDC_COSMIC_MATCH_PHI;
               if ( fabs(dphi) < matchphi){
                  axial_segments[j].matched=true;
                  mytrack.axial_hits.insert(mytrack.axial_hits.end(),
                        axial_segments[j].hits.begin(),
                        axial_segments[j].hits.end());
                  if (COSMICS) sort(mytrack.axial_hits.begin(),mytrack.axial_hits.end(),DTrackFinder_cdc_hit_cosmics_cmp);
                  else sort(mytrack.axial_hits.begin(),mytrack.axial_hits.end(),DTrackFinder_cdc_hit_cmp);

		  unsigned int last_index=mytrack.axial_hits.size()-1;
		  vhat=mytrack.axial_hits[last_index]->wire->origin
		    -mytrack.axial_hits[0]->wire->origin;
		  vhat.SetMag(1.);
               }
            }
         }
         //  Position of the first axial wire in the track  
         DVector3 pos0=mytrack.axial_hits[0]->wire->origin;
         if (VERBOSE){
            jout << " Axial track Formed: pos vhat" << jendl;
            pos0.Print(); vhat.Print();
            for(unsigned int jj = 0; jj<mytrack.axial_hits.size(); jj++){
               jout << "  R" << mytrack.axial_hits[jj]->wire->ring << " S" << mytrack.axial_hits[jj]->wire->straw << jendl;
            }
         }

         // Now try to associate stereo hits with this track
         for (unsigned int j=0;j<stereo_hits.size();j++){
            if (stereo_hits[j].used==false){
               if (MatchCDCStereoHit(vhat,pos0,stereo_hits[j].hit)){
                  //stereo_hits[j].used=true;
                  mytrack.stereo_hits.push_back(stereo_hits[j].hit);
                  if (VERBOSE) jout << "Added stereo hit R" << stereo_hits[j].hit->wire->ring << " S" << stereo_hits[j].hit->wire->straw << jendl;
               }
            }
         }
         size_t num_stereo=mytrack.stereo_hits.size();
         size_t num_axial=mytrack.axial_hits.size();
         if (VERBOSE) jout << " num_axial num_stereo " << num_axial << " " << num_stereo << jendl;
         if (num_stereo>0 && num_stereo+num_axial>4){
            mytrack.dir=vhat;
            if (mytrack.FindStateVector(COSMICS)==NOERROR){
               cdc_tracks.push_back(mytrack);
            }
         }
      }
   }
   return true;
}


// Match a CDC hit with a line starting at pos0 going in the vhat direction
bool DTrackFinder::MatchCDCHit(const DVector3 &vhat,const DVector3 &pos0,
      const DCDCTrackHit *hit, double cut){  
   DVector3 pos1=hit->wire->origin;
   DVector3 uhat=hit->wire->udir;
   DVector3 diff=pos1-pos0;
   double vhat_dot_uhat=vhat.Dot(uhat);
   double scale=1./(1.-vhat_dot_uhat*vhat_dot_uhat);
   double s=scale*(vhat_dot_uhat*diff.Dot(vhat)-diff.Dot(uhat));
   double t=scale*(diff.Dot(vhat)-vhat_dot_uhat*diff.Dot(uhat));
   if (COSMICS==false && t<0) return false;
   double d=(diff+s*uhat-t*vhat).Mag();

   if (d<cut) return true;

   return false;
}

// Match a CDC hit with a line starting at pos0 going in the vhat direction
bool DTrackFinder::MatchCDCStereoHit(const DVector3 &tdir,const DVector3 &t0,
      const DCDCTrackHit *hit){
   DVector3 w0=hit->wire->origin;
   DVector3 wdir=hit->wire->udir;
   DVector3 diff = t0-w0;
   double diffCrosstdir=diff.X()*tdir.Y()-diff.Y()*tdir.X();
   double diffCrosswdir=diff.X()*wdir.Y()-diff.Y()*wdir.X();
   double wdirCrosstdir=wdir.X()*tdir.Y()-wdir.Y()*tdir.X();
   double w=diffCrosstdir/wdirCrosstdir;
   double t=diffCrosswdir/wdirCrosstdir;
   if (COSMICS==false && t<0.) return false;
   if(fabs(w)<hit->wire->L/2.) return true;

   return false;
}

// Compute initial guess for state vector (x,y,tx,ty) for a track in the CDC
// by fitting a line to the intersections between the line in the xy plane and 
// the stereo wires.
jerror_t DTrackFinder::cdc_track_t::FindStateVector(bool IsCosmics){  
   // Parameters for line in x-y plane
   double vx=this->dir.x();
   double vy=this->dir.y();
   DVector3 pos0=this->axial_hits[0]->wire->origin;
   double xa=pos0.x();
   double ya=pos0.y();
 
   double sumv=0,sumz=0,sumrz=0,sumr=0,sumrr=0.;
   for (unsigned int i=0;i<this->stereo_hits.size();i++){
      // Intersection of line in xy-plane with this stereo straw
      DVector3 origin_s=this->stereo_hits[i]->wire->origin;
      DVector3 dir_s=this->stereo_hits[i]->wire->udir;
      double ux_s=dir_s.x();
      double uy_s=dir_s.y();
      double dx=xa-origin_s.x();
      double dy=ya-origin_s.y();
      double s=(dx*vy-dy*vx)/(ux_s*vy-uy_s*vx);
      DVector3 pos1=origin_s+s*dir_s;
      double z=pos1.z();
      double r=(pos1-pos0).Perp();

      if (z>17.0 && z<167.0)
      { // Check for CDC dimensions
         sumv+=1.;
         sumz+=z;
	 sumr+=r;
	 sumrz+=r*z;
	 sumrr+=r*r;
      }
   }

   // Find the z position of the start of the track
   double denom=sumv*sumrr-sumr*sumr;
   if (fabs(denom)<1e-4) return VALUE_OUT_OF_RANGE;
   double z0=(sumz*sumrr-sumr*sumrz)/denom;

   // Find the direction of the track
   double tanl=(sumv*sumrz-sumz*sumr)/denom;
   double dydx=vy/vx;
   double temp=1./tanl/sqrt(1.+dydx*dydx);
  
   this->z=z0;
   this->S(state_x)=xa;
   this->S(state_y)=ya;
   this->S(state_tx)=temp;
   this->S(state_ty)=dydx*temp;

   return NOERROR;

}

// Given two straight tracks, find the doca between them
double DTrackFinder::FindDoca(const DVector3 &pos1,const DVector3 &mom1,
      const DVector3 &pos2,const DVector3 &mom2,
      DVector3 *poca) const{
   DVector3 diff=pos1-pos2;
   DVector3 uhat=mom1;
   uhat.SetMag(1.);
   DVector3 vhat=mom2;
   vhat.SetMag(1.);

   double vhat_dot_diff=diff.Dot(vhat);
   double uhat_dot_diff=diff.Dot(uhat);
   double uhat_dot_vhat=uhat.Dot(vhat);
   double D=1.-uhat_dot_vhat*uhat_dot_vhat;
   double N=uhat_dot_vhat*vhat_dot_diff-uhat_dot_diff;
   double N1=vhat_dot_diff-uhat_dot_vhat*uhat_dot_diff;
   double scale=1./D;
   double s=scale*N;
   double t=scale*N1;

   if (poca!=NULL) *poca=pos1+s*uhat;

   diff+=s*uhat-t*vhat;
   return diff.Mag();
}


// Given state vector S, find doca to wire given by origin and wdir
double DTrackFinder::FindDoca(double z,const DMatrix4x1 &S,const DVector3 &wdir,
      const DVector3 &origin,DVector3 *poca) const{
   DVector3 pos(S(state_x),S(state_y),z);
   DVector3 diff=pos-origin;

   DVector3 uhat(S(state_tx),S(state_ty),1.);
   uhat.SetMag(1.); 
   DVector3 vhat=wdir;
   vhat.SetMag(1.);

   double vhat_dot_diff=diff.Dot(vhat);
   double uhat_dot_diff=diff.Dot(uhat);
   double uhat_dot_vhat=uhat.Dot(vhat);
   double D=1.-uhat_dot_vhat*uhat_dot_vhat;
   double N=uhat_dot_vhat*vhat_dot_diff-uhat_dot_diff;
   double N1=vhat_dot_diff-uhat_dot_vhat*uhat_dot_diff;
   double scale=1./D;
   double s=scale*N;
   double t=scale*N1;

   if (poca!=NULL) *poca=pos+s*uhat;

   diff+=s*uhat-t*vhat;
   return diff.Mag();
}


// Find segments by associating adjacent hits within a package together.
bool DTrackFinder::FindFDCSegments(void){
   if (fdc_hits.size()==0) return false;
   unsigned int num_hits=fdc_hits.size();
   const double ADJACENT_MATCH_RADIUS=1.0;

   // Order points by z
   sort(fdc_hits.begin(),fdc_hits.end(),DTrackFinder_fdc_hit_cmp);

   // Put indices for the first point in each plane before the most downstream
   // plane in the vector x_list.
   int old_layer=fdc_hits[0].hit->wire->layer;
   vector<unsigned int>x_list;
   x_list.push_back(0);
   for (unsigned int i=0;i<num_hits;i++){
      if(DEBUG_HISTS){
         hFDCLayerRaw->Fill(fdc_hits[i].hit->wire->layer);
      }
      if (fdc_hits[i].hit->wire->layer!=old_layer){
         x_list.push_back(i);
      }
      old_layer=fdc_hits[i].hit->wire->layer;
   }
   x_list.push_back(num_hits); 

   unsigned int start=0;
   // loop over the start indices, starting with the first plane
   while (start<x_list.size()-1){
      // Now loop over the list of track segment start fdc_hits
      for (unsigned int i=x_list[start];i<x_list[start+1];i++){
         if (fdc_hits[i].used==false){
            fdc_hits[i].used=true;
            if(DEBUG_HISTS){
               hFDCLayerFirst->Fill(fdc_hits[i].hit->wire->layer);
            }

            // Point in the current plane in the package 
            DVector2 XY=fdc_hits[i].hit->xy;
            double z=fdc_hits[i].hit->wire->origin.z();

            // Create list of nearest neighbors
            vector<const DFDCPseudo*>neighbors;
            neighbors.push_back(fdc_hits[i].hit);
            unsigned int match=0;
            double delta,delta_min=1000.;
            for (unsigned int k=0;k<x_list.size()-1;k++){
               delta_min=1000.;
               match=0;
               bool hasMatch=false;
               for (unsigned int m=x_list[k];m<x_list[k+1];m++){
                  if(fdc_hits[m].used==false){
                     delta=(XY-fdc_hits[m].hit->xy).Mod();
                     double delta_z=fabs(z-fdc_hits[m].hit->wire->origin.z());
                     if (delta<delta_min){
                        delta_min=delta;
                        if (delta<FDC_MATCH_RADIUS && delta_z<11.0) {
                           match=m;
                           hasMatch = true;
                        }
                     }
                  }
               }
               if (hasMatch){
                  XY=fdc_hits[match].hit->xy;
                  fdc_hits[match].used=true;
                  neighbors.push_back(fdc_hits[match].hit);	  
               }
            }
            unsigned int num_neighbors=neighbors.size();

            // Look for hits adjacent to the ones we have in our segment candidate
            for (unsigned int k=0;k<num_hits;k++){
               if (fdc_hits[k].used==false){
                  for (unsigned int j=0;j<num_neighbors;j++){
                     delta=(fdc_hits[k].hit->xy-neighbors[j]->xy).Mod();

                     if (delta<ADJACENT_MATCH_RADIUS && 
                           abs(neighbors[j]->wire->wire-fdc_hits[k].hit->wire->wire)<=1
                           && neighbors[j]->wire->origin.z()==fdc_hits[k].hit->wire->origin.z()){
                        fdc_hits[k].used=true;
                        neighbors.push_back(fdc_hits[k].hit);
                     }      
                  }
               }
            } // loop looking for hits adjacent to hits on segment

            if (neighbors.size()>3){
               unsigned int packNum=(neighbors[0]->wire->layer-1)/6;
               if (DEBUG_HISTS){
                  for (size_t iN = 0; iN < neighbors.size(); iN++){
                     hFDCLayer->Fill(neighbors[iN]->wire->layer);
                  }
               }
               fdc_segments[packNum].push_back(fdc_segment_t(neighbors));
            }
         }
      }// loop over start points within a plane

      // Look for a new plane to start looking for a segment
      while (start<x_list.size()-1){
         if (fdc_hits[x_list[start]].used==false) break;
         start++;
      }

   }

   return true;
}

// Link segments from package to package by doing straight-line projections
bool DTrackFinder::LinkFDCSegments(void){
   // matching cut
   const double LINK_MATCH_RADIUS=7.0;

   // Vector to store hits for the linked segments
   vector<const DFDCPseudo *>myhits;

   // loop over packages
   for (unsigned int i=0;i<4;i++){
      for (unsigned int j=0;j<fdc_segments[i].size();j++){
         if (fdc_segments[i][j].matched==false){
            unsigned i_plus_1=i+1; 
            if (i_plus_1<4){
               double tx=fdc_segments[i][j].S(state_tx);
               double ty=fdc_segments[i][j].S(state_ty);
               double x0=fdc_segments[i][j].S(state_x);
               double y0=fdc_segments[i][j].S(state_y);

               for (unsigned int k=0;k<fdc_segments[i_plus_1].size();k++){
                  if (fdc_segments[i_plus_1][k].matched==false){
                     double z=fdc_segments[i_plus_1][k].hits[0]->wire->origin.z();
                     DVector2 proj(x0+tx*z,y0+ty*z);

                     if ((proj-fdc_segments[i_plus_1][k].hits[0]->xy).Mod()<LINK_MATCH_RADIUS){
                        fdc_segments[i_plus_1][k].matched=true;
                        myhits.insert(myhits.end(),fdc_segments[i_plus_1][k].hits.begin(),
                              fdc_segments[i_plus_1][k].hits.end());

                        unsigned int i_plus_2=i_plus_1+1;
                        if (i_plus_2<4){
                           tx=fdc_segments[i_plus_1][k].S(state_tx);
                           ty=fdc_segments[i_plus_1][k].S(state_ty);
                           x0=fdc_segments[i_plus_1][k].S(state_x);
                           y0=fdc_segments[i_plus_1][k].S(state_y);

                           for (unsigned int m=0;m<fdc_segments[i_plus_2].size();m++){
                              if (fdc_segments[i_plus_2][m].matched==false){
                                 z=fdc_segments[i_plus_2][m].hits[0]->wire->origin.z();
                                 proj.Set(x0+tx*z,y0+ty*z);

                                 if ((proj-fdc_segments[i_plus_2][m].hits[0]->xy).Mod()<LINK_MATCH_RADIUS){
                                    fdc_segments[i_plus_2][m].matched=true;
                                    myhits.insert(myhits.end(),fdc_segments[i_plus_2][m].hits.begin(),
                                          fdc_segments[i_plus_2][m].hits.end());

                                    unsigned int i_plus_3=i_plus_2+1;
                                    if (i_plus_3<4){
                                       tx=fdc_segments[i_plus_2][m].S(state_tx);
                                       ty=fdc_segments[i_plus_2][m].S(state_ty);
                                       x0=fdc_segments[i_plus_2][m].S(state_x);
                                       y0=fdc_segments[i_plus_2][m].S(state_y);

                                       for (unsigned int n=0;n<fdc_segments[i_plus_3].size();n++){
                                          if (fdc_segments[i_plus_3][n].matched==false){
                                             z=fdc_segments[i_plus_3][n].hits[0]->wire->origin.z();
                                             proj.Set(x0+tx*z,y0+ty*z);

                                             if ((proj-fdc_segments[i_plus_3][n].hits[0]->xy).Mod()<LINK_MATCH_RADIUS){
                                                fdc_segments[i_plus_3][n].matched=true;
                                                myhits.insert(myhits.end(),fdc_segments[i_plus_3][n].hits.begin(),
                                                      fdc_segments[i_plus_3][n].hits.end());

                                                break;
                                             } // matched a segment
                                          }
                                       }  // loop over last set of segments
                                    } // if we have another package to loop over
                                    break;
                                 } // matched a segment
                              }
                           } // loop over second-to-last set of segments
                        }
                        break;
                     } // matched a segment
                  }
               } // loop over third-to-last set of segments
            }	
            if (myhits.size()>0){ 
               myhits.insert(myhits.begin(),fdc_segments[i][j].hits.begin(),fdc_segments[i][j].hits.end());	
               fdc_tracks.push_back(fdc_segment_t(myhits));
            }	  
            myhits.clear();
         } // check if we have already used this segment
      } // loop over first set of segments
   } // loop over packages

   // Try to link tracks together
   if (fdc_tracks.size()>1){
      for (unsigned int i=0;i<fdc_tracks.size()-1;i++){
         DMatrix4x1 S=fdc_tracks[i].S;
         size_t last_index_1=fdc_tracks[i].hits.size()-1;
         int first_pack_1=(fdc_tracks[i].hits[0]->wire->layer-1)/6;
         int last_pack_1=(fdc_tracks[i].hits[last_index_1]->wire->layer-1)/6;
         for (unsigned int j=i+1;j<fdc_tracks.size();j++){
            size_t last_index_2=fdc_tracks[j].hits.size()-1;
            int first_pack_2=(fdc_tracks[j].hits[0]->wire->layer-1)/6;
            int last_pack_2=(fdc_tracks[j].hits[last_index_2]->wire->layer-1)/6;

            if (last_pack_1<first_pack_2 || first_pack_1 > last_pack_2){
               double z=fdc_tracks[j].hits[0]->wire->origin.z();
               DVector2 proj(S(state_x)+z*S(state_tx),S(state_y)+z*S(state_ty));
               double diff=(fdc_tracks[j].hits[0]->xy-proj).Mod();

               if (diff<FDC_MATCH_RADIUS){
                  // Combine the hits from the two tracks and recompute the state 
                  // vector S
                  if (last_pack_1<first_pack_2){
                     fdc_tracks[i].hits.insert(fdc_tracks[i].hits.end(),
                           fdc_tracks[j].hits.begin(),
                           fdc_tracks[j].hits.end());
                  }
                  else{
                     fdc_tracks[i].hits.insert(fdc_tracks[i].hits.begin(),
                           fdc_tracks[j].hits.begin(),
                           fdc_tracks[j].hits.end());
                  }
                  fdc_tracks[i].FindStateVector();

                  // Drop the second track from the list 
                  fdc_tracks.erase(fdc_tracks.begin()+j);
                  break;
               }
            }
         }
      } // loop over tracks 
   } // check if we have more than one track

   // Try to attach unmatched segments to tracks
   for (unsigned int i=0;i<fdc_tracks.size();i++){
      DMatrix4x1 S=fdc_tracks[i].S;
      size_t last_index_1=fdc_tracks[i].hits.size()-1;
      int first_pack_1=(fdc_tracks[i].hits[0]->wire->layer-1)/6;
      int last_pack_1=(fdc_tracks[i].hits[last_index_1]->wire->layer-1)/6; 
      for (unsigned int j=0;j<4;j++){
         for (unsigned int k=0;k<fdc_segments[j].size();k++){
            if (fdc_segments[j][k].matched==false){
               int pack_2=(fdc_segments[j][k].hits[0]->wire->layer-1)/6;
               if (pack_2<first_pack_1 || pack_2>last_pack_1){
                  double z=fdc_segments[j][k].hits[0]->wire->origin.z();
                  DVector2 proj(S(state_x)+z*S(state_tx),S(state_y)+z*S(state_ty));
                  double diff=(fdc_segments[j][k].hits[0]->xy-proj).Mod();

                  if (diff<FDC_MATCH_RADIUS){
                     fdc_segments[j][k].matched=true;

                     // Add hits and recompute S
                     if (pack_2<first_pack_1){
                        fdc_tracks[i].hits.insert(fdc_tracks[i].hits.begin(),
                              fdc_segments[j][k].hits.begin(),
                              fdc_segments[j][k].hits.end());
                     }
                     else {
                        fdc_tracks[i].hits.insert(fdc_tracks[i].hits.end(),
                              fdc_segments[j][k].hits.begin(),
                              fdc_segments[j][k].hits.end());

                     }
                     fdc_tracks[i].FindStateVector();
                  }
               }
            } // check if already matched to other segments
         } // loop over segments in package
      } // loop over packages
   } //loop over existing tracks

   // Make track candidates out of single segments not connected to other tracks
   for (unsigned int j=0;j<4;j++){
     for (unsigned int k=0;k<fdc_segments[j].size();k++){
       if (fdc_segments[j][k].matched==false){
	 fdc_tracks.push_back(fdc_segments[j][k]);
       }
     }
   }

   return true;
}



// Use linear regression on the hits to obtain a first guess for the state
// vector.  Method taken from Numerical Recipes in C.
DMatrix4x1 
DTrackFinder::fdc_segment_t::FindStateVector(void) const {
   double S1=0.;
   double S1z=0.;
   double S1y=0.;
   double S1zz=0.;
   double S1zy=0.;  
   double S2=0.;
   double S2z=0.;
   double S2x=0.;
   double S2zz=0.;
   double S2zx=0.;

   double sig2v=0.25; // rough guess;

   for (unsigned int i=0;i<hits.size();i++){
      double cosa=hits[i]->wire->udir.y();
      double sina=hits[i]->wire->udir.x();
      double x=hits[i]->xy.X();
      double y=hits[i]->xy.Y();
      double z=hits[i]->wire->origin.z();
      double sig2x=cosa*cosa/12+sina*sina*sig2v;
      double sig2y=sina*sina/12+cosa*cosa*sig2v;
      double one_over_var1=1/sig2y;
      double one_over_var2=1/sig2x;

      S1+=one_over_var1;
      S1z+=z*one_over_var1;
      S1y+=y*one_over_var1;
      S1zz+=z*z*one_over_var1;
      S1zy+=z*y*one_over_var1;    

      S2+=one_over_var2;
      S2z+=z*one_over_var2;
      S2x+=x*one_over_var2;
      S2zz+=z*z*one_over_var2;
      S2zx+=z*x*one_over_var2;
   }
   double D1=S1*S1zz-S1z*S1z;
   double y_intercept=(S1zz*S1y-S1z*S1zy)/D1;
   double y_slope=(S1*S1zy-S1z*S1y)/D1;
   double D2=S2*S2zz-S2z*S2z;
   double x_intercept=(S2zz*S2x-S2z*S2zx)/D2;
   double x_slope=(S2*S2zx-S2z*S2x)/D2;

   return DMatrix4x1(x_intercept,y_intercept,x_slope,y_slope);
}

// Find intersection between a straight line and a plane
bool DTrackFinder::FindIntersectionWithPlane(const DVector3 &origin,
      const DVector3 &norm,
      const DVector3 &pos,
      const DVector3 &dir,
      DVector3 &outpos) const{
   DVector3 mydir(dir);
   mydir.SetMag(1.);
   double dot=mydir.Dot(norm);
   if (fabs(dot)<1e-16) return false; // parallel lines
   double s=(origin-pos).Dot(norm)/dot;
   outpos=pos+s*mydir;

   return true;
}


// Find the intersections between a straight line and a cylinder of radius R
bool DTrackFinder::FindIntersectionsWithCylinder(double R,
      const DVector3 &dir,
      const DVector3 &pos,
      DVector3 &out1,
      DVector3 &out2) const{
   double denom=dir.Mag();
   double ux=dir.x()/denom;
   double uy=dir.y()/denom;
   double uz=dir.z()/denom;
   double ux2=ux*ux;
   double uy2=uy*uy;
   double ux2_plus_uy2=ux2+uy2;
   double x0=pos.x();
   double y0=pos.y();
   double z0=pos.z();
   double A=ux2_plus_uy2*R*R-uy2*x0*x0-ux2*y0*y0+2.*ux*uy*x0*y0;
   if (A<0) return false;

   double t0=-(x0*ux+y0*uy)/ux2_plus_uy2;
   double dt=sqrt(A)/ux2_plus_uy2;
   double tplus=t0+dt;
   out1.SetXYZ(x0+ux*tplus,y0+uy*tplus,z0+uz*tplus);
   double tminus=t0-dt;
   out2.SetXYZ(x0+ux*tminus,y0+uy*tminus,z0+uz*tminus);

   return true;
}
