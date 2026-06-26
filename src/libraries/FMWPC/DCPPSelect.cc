// DCPPSelect member functions

#include "DCPPSelect.h"

void DCPPSelect::RemoveFartherDuplicateHits(std::map<int,double>& plus_map,std::map<int,double>& minus_map){
    for (auto it_plus = plus_map.begin(); it_plus != plus_map.end(); ) {
        auto it_minus = minus_map.find(it_plus->first);
        if (it_minus == minus_map.end()) {
            ++it_plus;
            continue;
        }
        if (it_plus->second > it_minus->second)
            it_plus = plus_map.erase(it_plus);
        else {
            minus_map.erase(it_minus);
            ++it_plus;
        }
    }
}

double DCPPSelect::mwpc_sigma(int ic, double p) { // ic = chamber # from 0 to 5

	const double fmwpc_spar[6][4] = {	\
		3.8515, 1.6761, 0.98888, 0.041894, \
		3.9221, 1.7107, 1.18600, 0.057278, \
		4.0039, 1.6271, 1.53250, 0.088580, \
		13.070, 6.6978, 2.04570, 0.140260, \
		2.2558, .14294, -1., 0., \
		2.2548, .42019, 1.3483, 0.0022198 };
	
	if(p<0.2) p = 0.2;
	if(p>10.) p = 10.;
	double s  = exp( fmwpc_spar[ic][0] - fmwpc_spar[ic][1]*p );
	if(fmwpc_spar[ic][2]>0.)
			s += exp( fmwpc_spar[ic][2] - fmwpc_spar[ic][3]*p );
	return s;
}


bool DCPPSelect::CheckTrackinMWPCFiducial_CPP(DVector3 mwpc_proj_pos){
	int track_in_mwpc = 0;
	double x1fmwpc=0.0, y1fmwpc=0.0;
	
	x1fmwpc = mwpc_proj_pos.x();
	y1fmwpc = mwpc_proj_pos.y();

	if((x1fmwpc < 5.1 && x1fmwpc > -5.1) && (y1fmwpc < 5.1 && y1fmwpc > -5.1)){
		return false;
	}else if(x1fmwpc > 73.15 || x1fmwpc < -73.15 || y1fmwpc < -73.15 || y1fmwpc > 73.15){
		return false;
	}else{
		track_in_mwpc += 1;
	}
	return track_in_mwpc > 0;
}


ProjectionResults DCPPSelect::SwimTracksToAllDetectors(const DTrackTimeBased* locTimeTrack, Particle_t particle_id, const DMagneticFieldMap *bfield,double fcalfrontfaceZ,double m_TOFfront){

	const double mwpcz[6] = {811.62, 827.79, 849.04,890.65,921.84,941.12};

	ProjectionResults result;

	DReferenceTrajectory rt(bfield);
    rt.SetMass(ParticleMass(particle_id));
    rt.q = ParticleCharge(particle_id);
    rt.SetZmaxTrackingBoundary(1000.);
    rt.Swim(locTimeTrack->position(),locTimeTrack->momentum(),rt.q);

	DVector3 norm(0.0,0.0,1.0);
	DVector3 tof_face_pos(0.0,0.0,m_TOFfront);
	DVector3 fcal_face_pos(0.0,0.0,fcalfrontfaceZ);
	DVector3 fmwpc1_zpos(0.0,0.0,mwpcz[0]);
	DVector3 fmwpc2_zpos(0.0,0.0,mwpcz[1]);
	DVector3 fmwpc3_zpos(0.0,0.0,mwpcz[2]);
	DVector3 fmwpc4_zpos(0.0,0.0,mwpcz[3]);
	DVector3 fmwpc5_zpos(0.0,0.0,mwpcz[4]);
	DVector3 fmwpc6_zpos(0.0,0.0,mwpcz[5]);

	DVector3 tof_proj_pos, tof_proj_mom;
	if(rt.GetIntersectionWithPlane(tof_face_pos,norm,tof_proj_pos,tof_proj_mom) != NOERROR) return result;

	DVector3 fcal_proj_pos, fcal_proj_mom;
	if(rt.GetIntersectionWithPlane(fcal_face_pos,norm,fcal_proj_pos,fcal_proj_mom) != NOERROR) return result;

	result.mwpc_flags[0] = false, result.mwpc_flags[1] = false, result.mwpc_flags[2] = false, result.mwpc_flags[3] = false, result.mwpc_flags[4] = false, result.mwpc_flags[5] = false;

	DVector3 mwpc_proj_pos_chamber1, mwpc_proj_mom_chamber1;
	//if(rt.GetIntersectionWithPlane(fmwpc1_zpos,norm,mwpc_proj_pos_chamber1,mwpc_proj_mom_chamber1) != NOERROR) return result;
	if(rt.GetIntersectionWithPlane(fmwpc1_zpos,norm,mwpc_proj_pos_chamber1,mwpc_proj_mom_chamber1) != NOERROR) result.mwpc_flags[0] = true;

	DVector3 mwpc_proj_pos_chamber2, mwpc_proj_mom_chamber2;
	//if(rt.GetIntersectionWithPlane(fmwpc2_zpos,norm,mwpc_proj_pos_chamber2,mwpc_proj_mom_chamber2) != NOERROR) return result;
	if(rt.GetIntersectionWithPlane(fmwpc2_zpos,norm,mwpc_proj_pos_chamber2,mwpc_proj_mom_chamber2) != NOERROR) result.mwpc_flags[1] = true;

	DVector3 mwpc_proj_pos_chamber3, mwpc_proj_mom_chamber3;
	//if(rt.GetIntersectionWithPlane(fmwpc3_zpos,norm,mwpc_proj_pos_chamber3,mwpc_proj_mom_chamber3) != NOERROR) return result;
	if(rt.GetIntersectionWithPlane(fmwpc3_zpos,norm,mwpc_proj_pos_chamber3,mwpc_proj_mom_chamber3) != NOERROR) result.mwpc_flags[2] = true;

	DVector3 mwpc_proj_pos_chamber4, mwpc_proj_mom_chamber4;
	//if(rt.GetIntersectionWithPlane(fmwpc4_zpos,norm,mwpc_proj_pos_chamber4,mwpc_proj_mom_chamber4) != NOERROR) return result;
	if(rt.GetIntersectionWithPlane(fmwpc4_zpos,norm,mwpc_proj_pos_chamber4,mwpc_proj_mom_chamber4) != NOERROR) result.mwpc_flags[3] = true;

	DVector3 mwpc_proj_pos_chamber5, mwpc_proj_mom_chamber5;
	//if(rt.GetIntersectionWithPlane(fmwpc5_zpos,norm,mwpc_proj_pos_chamber5,mwpc_proj_mom_chamber5) != NOERROR) return result;
	if(rt.GetIntersectionWithPlane(fmwpc5_zpos,norm,mwpc_proj_pos_chamber5,mwpc_proj_mom_chamber5) != NOERROR) result.mwpc_flags[4] = true;

	DVector3 mwpc_proj_pos_chamber6, mwpc_proj_mom_chamber6;
	//if(rt.GetIntersectionWithPlane(fmwpc6_zpos,norm,mwpc_proj_pos_chamber6,mwpc_proj_mom_chamber6) != NOERROR) return result;
	if(rt.GetIntersectionWithPlane(fmwpc6_zpos,norm,mwpc_proj_pos_chamber6,mwpc_proj_mom_chamber6) != NOERROR) result.mwpc_flags[5] = true;

	result.tof_projection = tof_proj_pos;
	result.fcal_projection = fcal_proj_pos;

	result.mwpc_projections[0] = mwpc_proj_pos_chamber1;
	result.mwpc_projections[1] = mwpc_proj_pos_chamber2;
	result.mwpc_projections[2] = mwpc_proj_pos_chamber3;
	result.mwpc_projections[3] = mwpc_proj_pos_chamber4;
	result.mwpc_projections[4] = mwpc_proj_pos_chamber5;
	result.mwpc_projections[5] = mwpc_proj_pos_chamber6;

	DVector3 ctof_proj_pos;
	//if(rt.GetIntersectionWithPlane())

	result.projection_success = true;

	return result;

}


bool DCPPSelect::MatchToTOF_CPP_GEOM(const vector<const DTOFPoint*>& tof_points, DVector3 tof_proj_pos){
  double x1tof, y1tof;
  x1tof = tof_proj_pos.x();
  y1tof = tof_proj_pos.y();

  int tofxF = 0, tofyF = 0;
  for(unsigned int itof = 0;itof < tof_points.size();itof++){
	  const DTOFPoint *tp = tof_points[itof];
    double tofx = tp->pos.x();
    double tofy = tp->pos.y();
	  bool xDef = tp->Is_XPositionWellDefined();
	  bool yDef = tp->Is_YPositionWellDefined();
    if(xDef != 1 || yDef != 1) continue;
    double xdif = tofx - x1tof;
    double ydif = tofy - y1tof;

    if(xdif < -6.03 || xdif > 8.409){
      continue;
    }else{
      tofxF += 1;
	  }
    if(ydif < -5.73 || ydif > 5.08){
      continue;
    }else{
      tofyF += 1;
    }
  } 
  if(tofxF == 0 || tofyF == 0) return false;
  return true;
}

bool DCPPSelect::MatchToFCALShower_CPP(const vector<const DFCALShower*>& fcal_showers, vector<const DFCALShower*>& fcal_matched_showers, DVector3 fcal_proj_pos, DVector3 fcal_proj_mom){
  double x1fcal = 0.0, y1fcal = 0.0;

  x1fcal = fcal_proj_pos.x();
  y1fcal = fcal_proj_pos.y();
  const DFCALShower* best_fcal_shower = nullptr;
  double best_doca = 1e9;
  for(unsigned int ifs = 0; ifs<fcal_showers.size();++ifs){
    const DFCALShower *fs1 = fcal_showers[ifs];
    double xfs1 = fs1->getPosition().X();
    double yfs1 = fs1->getPosition().Y();

    double calculated_doca = hypot((xfs1-x1fcal-0.5),(yfs1-y1fcal));
    if(calculated_doca < 5.0){
      if(calculated_doca < best_doca){
        best_doca = calculated_doca;
        best_fcal_shower = fs1;
      }
    }
  }
  if(!best_fcal_shower) return false;
  
  fcal_matched_showers.push_back(best_fcal_shower);
  return true;
  
}
bool DCPPSelect::MatchToFCALHit_CPP(const vector<const DFCALHit*>& fcal_hits, vector<const DFCALHit*>& fcal_matched_hits, double& e9e25, double& doca, double& e1e9,DVector3 fcal_proj_pos, DVector3 fcal_proj_mom,double& sumUSh, double& sumVSh){
  double x1fcal = 0.0, y1fcal = 0.0;
  //int fcal_hit_match_count = 0;
  //double E1cut = 2.9;
  double E9cut = 4.5, E25cut = 8.5;
  
  
  x1fcal = fcal_proj_pos.x();
  y1fcal = fcal_proj_pos.y();

  //Helper Vectors
  //vector<const DFCALHit*> fcal_hits_list;
  //vector<double> e1e9_list;
  //vector<double> e9e25_list;
  //vector<double> doca_list;
  //vector<double> eoverp_list;

  const DFCALHit* best_hit = nullptr;
  double best_doca = 1e9;
  double best_e1e9 = 0.0;
  double best_e9e25 = 0.0;
  double best_sumu = 0.0;
  double best_sumv = 0.0;
  
  for(unsigned int ifh = 0; ifh<fcal_hits.size(); ++ifh){
    const DFCALHit *fcal_hit = fcal_hits[ifh];
    double fcalx = fcal_hit->x;
    double fcaly = fcal_hit->y;
    double fcale = fcal_hit->E;
    double dx = fcalx - x1fcal;
    double dy = fcaly - y1fcal;
    double fcalr = sqrt(dx*dx+dy*dy);
    if(fcalr < 5.0){
      double E1 = fcale,E9 = 0, E25 = 0;
      DVector3 trackVec(x1fcal,y1fcal,0);
      DVector3 hitVec(fcalx,fcaly,0);
      DVector3 u =(hitVec - trackVec).Unit();
      DVector3 z(0,0,1);
      DVector3 v = u.Cross(z);
      DVector3 hitLoc(0,0,0);
      sumUSh = 0;
      sumVSh = 0;
      E1 = fcale;
      double sumE = 0;
      double sumu_cand =0.,sumv_cand=0.;
      for(unsigned int jfh = 0; jfh < fcal_hits.size(); jfh++){
        const DFCALHit *fcal_hit1 = fcal_hits[jfh];
        hitLoc.SetX(fcal_hit1->x-hitVec.X());
        hitLoc.SetY(fcal_hit1->y-hitVec.Y());
        sumu_cand += (fcal_hit1->E) * pow( u.Dot( hitLoc ), 2 );
        sumv_cand += (fcal_hit1->E) * pow( v.Dot( hitLoc ), 2 );
        sumE += fcal_hit1->E;
        //if( abs(fcal_hit->x - fcal_hit1->x)<=E1cut && abs(fcal_hit->y - fcal_hit1->y)<=E1cut ) E1 = fcal_hit1->E;
        if( fabs(fcal_hit->x - fcal_hit1->x)<=E9cut && fabs(fcal_hit->y - fcal_hit1->y)<=E9cut ) E9 += fcal_hit1->E;
        if( fabs(fcal_hit->x - fcal_hit1->x)<=E25cut && fabs(fcal_hit->y - fcal_hit1->y)<=E25cut ) E25 += fcal_hit1->E;
      }
      if(sumE == 0) continue;
      sumu_cand /= sumE;
      sumv_cand /= sumE;

      double e1e9_cand = E1/E9;
      double e9e25_cand = E9/E25;

      if(fcalr < best_doca){
        best_doca = fcalr;
        best_hit = fcal_hit;
        best_e1e9 = e1e9_cand;
        best_e9e25 = e9e25_cand;
        best_sumu = sumu_cand;
        best_sumv = sumv_cand;
      }
    }
  }

  if(!best_hit) return false;

  fcal_matched_hits.push_back(best_hit);
  e9e25 = best_e9e25;
  doca = best_doca;
  e1e9 = best_e1e9;
  sumUSh = best_sumu;
  sumVSh = best_sumv;

  return true;
}

bool DCPPSelect::ComputeMWPCWireResiduals(const vector<const DFMWPCHit*> locFMWPCHits, std::map<MWPCKey,DVector3> mwpc_projections,double track1_energy,double track2_energy,map<MWPCKey,int>& mwpc_multis,bool& piplus_track_chamber6,bool& piminus_track_chamber6){
	
	piplus_track_chamber6 = CheckTrackinMWPCFiducial_CPP(mwpc_projections.at(MWPCKey::fmwpc6_proj_plus));
	piminus_track_chamber6 = CheckTrackinMWPCFiducial_CPP(mwpc_projections.at(MWPCKey::fmwpc6_proj_minus));

	std::map<int,double> fmwpc1_plus_wire_diff, fmwpc2_plus_wire_diff, fmwpc3_plus_wire_diff,fmwpc4_plus_wire_diff, fmwpc5_plus_wire_diff,fmwpc6_plus_wire_diff;

	const double fmwpc_al[6] = {  0.25,  -0.22,   0.23,  -0.27,   0.17,  -0.06};

	int fmwpc1m8 = 0,fmwpc2m8 = 0,fmwpc3m8 = 0,fmwpc4m8 = 0,fmwpc5m8 = 0,fmwpc6m8 = 0;
	int pfinalChamber = -1;
	for(int jlayer = 0;jlayer<6;jlayer++){
		double s=-1;
		for(unsigned j = 0; j < locFMWPCHits.size();j++){
			const DFMWPCHit *hit1 = locFMWPCHits[j];
			int ilayer = hit1->layer-1;
			if(ilayer!=jlayer) continue;
			int iwire = hit1->wire;
			double coord = (iwire -72.5)*2.54*0.4;
			double track_coord = NAN;
			if(ilayer == 0) track_coord = mwpc_projections.at(MWPCKey::fmwpc1_proj_plus).X();
			if(ilayer == 1) track_coord = mwpc_projections.at(MWPCKey::fmwpc2_proj_plus).Y();
			if(ilayer == 2) track_coord = mwpc_projections.at(MWPCKey::fmwpc3_proj_plus).X();
			if(ilayer == 3) track_coord = mwpc_projections.at(MWPCKey::fmwpc4_proj_plus).Y();
			if(ilayer == 4) track_coord = mwpc_projections.at(MWPCKey::fmwpc5_proj_plus).X();
			if(ilayer == 5) track_coord = mwpc_projections.at(MWPCKey::fmwpc6_proj_plus).Y();
			if(std::isnan(track_coord)){
				continue;
			}
			s = mwpc_sigma(jlayer,track1_energy);
			double diff = coord - track_coord - fmwpc_al[ilayer];
			if(fabs(diff)>s*5.)continue;
			if(pfinalChamber < ilayer) pfinalChamber = ilayer;
			if(ilayer == 0){
				fmwpc1m8 += 1;
				fmwpc1_plus_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 1){
				fmwpc2m8 += 1;
				fmwpc2_plus_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 2){
				fmwpc3m8 += 1;
				fmwpc3_plus_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 3){
				fmwpc4m8 += 1;
				fmwpc4_plus_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 4){
				fmwpc5m8 += 1;
				fmwpc5_plus_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 5){
				fmwpc6m8 += 1;
				fmwpc6_plus_wire_diff.emplace(iwire,diff);
			}
		}
	}
	int pfinalChamber1to5 = -1;
	if(fmwpc1m8 > 0) pfinalChamber1to5 = 0;
	if(fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber1to5 = 1;
	if(fmwpc3m8 > 0 && fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber1to5 = 2;
	if(fmwpc4m8 > 0 && fmwpc3m8 > 0 && fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber1to5 = 3;
	if(fmwpc5m8 > 0 && fmwpc4m8 > 0 && fmwpc3m8 > 0 && fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber1to5 = 4;


	pfinalChamber = -1;
	if(fmwpc1m8 > 0) pfinalChamber = 0;
	if(fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber = 1;
	if(fmwpc3m8 > 0 && fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber = 2;
	if(fmwpc4m8 > 0 && fmwpc3m8 > 0 && fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber = 3;
	if(fmwpc5m8 > 0 && fmwpc4m8 > 0 && fmwpc3m8 > 0 && fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber = 4;

	std::map<int,double> fmwpc1_minus_wire_diff,fmwpc2_minus_wire_diff,fmwpc3_minus_wire_diff,fmwpc4_minus_wire_diff,
		fmwpc5_minus_wire_diff,fmwpc6_minus_wire_diff;
	int fmwpc1m9 = 0,fmwpc2m9 = 0,fmwpc3m9 = 0,fmwpc4m9 = 0,fmwpc5m9 = 0,fmwpc6m9 = 0;
	int nfinalChamber = -1;
	for(int jlayer=0;jlayer < 6;jlayer++){
		double s = -1.;
		for(unsigned j = 0; j < locFMWPCHits.size();j++){
			const DFMWPCHit *hit1 = locFMWPCHits[j];
			int ilayer = hit1->layer-1;
			if(ilayer!=jlayer) continue;
			int iwire = hit1->wire;
			double coord = (iwire -72.5)*2.54*0.4;
			double track_coord = NAN;
			if(ilayer == 0) track_coord = mwpc_projections.at(MWPCKey::fmwpc1_proj_minus).X();
			if(ilayer == 1) track_coord = mwpc_projections.at(MWPCKey::fmwpc2_proj_minus).Y();
			if(ilayer == 2) track_coord = mwpc_projections.at(MWPCKey::fmwpc3_proj_minus).X();
			if(ilayer == 3) track_coord = mwpc_projections.at(MWPCKey::fmwpc4_proj_minus).Y();
			if(ilayer == 4) track_coord = mwpc_projections.at(MWPCKey::fmwpc5_proj_minus).X();
			if(ilayer == 5) track_coord = mwpc_projections.at(MWPCKey::fmwpc6_proj_minus).Y();
			if(std::isnan(track_coord)){
				continue;
			}
			s = mwpc_sigma(jlayer,track2_energy);
			double diff = coord - track_coord - fmwpc_al[ilayer];
			if(fabs(diff)>s*5.)continue;
			if(nfinalChamber < ilayer) nfinalChamber = ilayer;
			if(ilayer == 0){ 
				fmwpc1m9 += 1;
				fmwpc1_minus_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 1){
				fmwpc2m9 += 1;
				fmwpc2_minus_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 2){
				fmwpc3m9 += 1;
				fmwpc3_minus_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 3){
				fmwpc4m9 += 1;
				fmwpc4_minus_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 4){
				fmwpc5m9 += 1;
				fmwpc5_minus_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 5){
				fmwpc6m9 += 1;
				fmwpc6_minus_wire_diff.emplace(iwire,diff);
			}
		}
	}
	int nfinalChamber1to5 = -1;
	if(fmwpc1m9 > 0) nfinalChamber1to5 = 0;
	if(fmwpc2m9 > 0 && fmwpc1m9 > 0) nfinalChamber1to5 = 1;
	if(fmwpc3m9 > 0 && fmwpc2m9 > 0 && fmwpc1m9 > 0) nfinalChamber1to5 = 2;
	if(fmwpc4m9 > 0 && fmwpc3m9 > 0 && fmwpc2m9 > 0 && fmwpc1m9 > 0) nfinalChamber1to5 = 3;
	if(fmwpc5m9 > 0 && fmwpc4m9 > 0 && fmwpc3m9 > 0 && fmwpc2m9 > 0 && fmwpc1m9 > 0) nfinalChamber1to5 = 4;

	nfinalChamber = -1;
	if(fmwpc1m9 > 0) nfinalChamber = 0;
	if(fmwpc2m9 > 0 && fmwpc1m9 > 0) nfinalChamber = 1;
	if(fmwpc3m9 > 0 && fmwpc2m9 > 0 && fmwpc1m9 > 0) nfinalChamber = 2;
	if(fmwpc4m9 > 0 && fmwpc3m9 > 0 && fmwpc2m9 > 0 && fmwpc1m9 > 0) nfinalChamber = 3;
	if(fmwpc5m9 > 0 && fmwpc4m9 > 0 && fmwpc3m9 > 0 && fmwpc2m9 > 0 && fmwpc1m9 > 0) nfinalChamber = 4;
	if(fmwpc6m9 > 0 && fmwpc5m9 > 0 && fmwpc4m9 > 0 && fmwpc3m9 > 0 && fmwpc2m9 > 0 && fmwpc1m9 > 0) nfinalChamber = 5;

	int tfinalChamber = (pfinalChamber >= nfinalChamber) ? pfinalChamber : nfinalChamber;
	int tfinalChamber1to5 = (pfinalChamber1to5 >= nfinalChamber1to5) ? pfinalChamber1to5 : nfinalChamber1to5;

	RemoveFartherDuplicateHits(fmwpc1_plus_wire_diff,fmwpc1_minus_wire_diff);
	RemoveFartherDuplicateHits(fmwpc2_plus_wire_diff,fmwpc2_minus_wire_diff);
	RemoveFartherDuplicateHits(fmwpc3_plus_wire_diff,fmwpc3_minus_wire_diff);
	RemoveFartherDuplicateHits(fmwpc4_plus_wire_diff,fmwpc4_minus_wire_diff);
	RemoveFartherDuplicateHits(fmwpc5_plus_wire_diff,fmwpc5_minus_wire_diff);
	RemoveFartherDuplicateHits(fmwpc6_plus_wire_diff,fmwpc6_minus_wire_diff);

	int fmwpc1_plus_count = static_cast<int>(fmwpc1_plus_wire_diff.size());
	int fmwpc1_minus_count = static_cast<int>(fmwpc1_minus_wire_diff.size());
	int fmwpc2_plus_count = static_cast<int>(fmwpc2_plus_wire_diff.size());
	int fmwpc2_minus_count = static_cast<int>(fmwpc2_minus_wire_diff.size());
	int fmwpc3_plus_count = static_cast<int>(fmwpc3_plus_wire_diff.size());
	int fmwpc3_minus_count = static_cast<int>(fmwpc3_minus_wire_diff.size());
	int fmwpc4_plus_count = static_cast<int>(fmwpc4_plus_wire_diff.size());
	int fmwpc4_minus_count = static_cast<int>(fmwpc4_minus_wire_diff.size());
	int fmwpc5_plus_count = static_cast<int>(fmwpc5_plus_wire_diff.size());
	int fmwpc5_minus_count = static_cast<int>(fmwpc5_minus_wire_diff.size());
	int fmwpc6_plus_count = static_cast<int>(fmwpc6_plus_wire_diff.size());
	int fmwpc6_minus_count = static_cast<int>(fmwpc6_minus_wire_diff.size());

	mwpc_multis.emplace(MWPCKey::fmwpc1_proj_plus,fmwpc1_plus_count);
	mwpc_multis.emplace(MWPCKey::fmwpc2_proj_plus,fmwpc2_plus_count);
	mwpc_multis.emplace(MWPCKey::fmwpc3_proj_plus,fmwpc3_plus_count);
	mwpc_multis.emplace(MWPCKey::fmwpc4_proj_plus,fmwpc4_plus_count);
	mwpc_multis.emplace(MWPCKey::fmwpc5_proj_plus,fmwpc5_plus_count);
	mwpc_multis.emplace(MWPCKey::fmwpc6_proj_plus,fmwpc6_plus_count);
	mwpc_multis.emplace(MWPCKey::fmwpc1_proj_minus,fmwpc1_minus_count);
	mwpc_multis.emplace(MWPCKey::fmwpc2_proj_minus,fmwpc2_minus_count);
	mwpc_multis.emplace(MWPCKey::fmwpc3_proj_minus,fmwpc3_minus_count);
	mwpc_multis.emplace(MWPCKey::fmwpc4_proj_minus,fmwpc4_minus_count);
	mwpc_multis.emplace(MWPCKey::fmwpc5_proj_minus,fmwpc5_minus_count);
	mwpc_multis.emplace(MWPCKey::fmwpc6_proj_minus,fmwpc6_minus_count);

	return true;
}

bool DCPPSelect::ComputeSingleTrackMWPCWireResiduals(const vector<const DFMWPCHit*> locFMWPCHits, std::map<MWPCKey,DVector3> mwpc_projections,double track_energy,bool& track_in_chamber6, double track_charge,std::map<int,double>& fmwpc1_wire_diff, std::map<int,double>& fmwpc2_wire_diff, std::map<int,double>& fmwpc3_wire_diff, std::map<int,double>& fmwpc4_wire_diff,std::map<int,double>& fmwpc5_wire_diff, std::map<int,double>& fmwpc6_wire_diff){


	if(track_charge > 0.0){
		track_in_chamber6 = CheckTrackinMWPCFiducial_CPP(mwpc_projections.at(MWPCKey::fmwpc6_proj_plus));
	}else{
		track_in_chamber6 = CheckTrackinMWPCFiducial_CPP(mwpc_projections.at(MWPCKey::fmwpc6_proj_minus));
	}

	const double fmwpc_al[6] = {0.25,-0.22,0.23,-0.27,0.17, -0.06};

	int fmwpc1m8 = 0,fmwpc2m8 = 0,fmwpc3m8 = 0,fmwpc4m8 = 0,fmwpc5m8 = 0,fmwpc6m8 = 0;
	int pfinalChamber = -1;
	for(int jlayer = 0;jlayer<6;jlayer++){
		double s=-1;
		for(unsigned j = 0; j < locFMWPCHits.size();j++){
			const DFMWPCHit *hit1 = locFMWPCHits[j];
			int ilayer = hit1->layer-1;
			if(ilayer!=jlayer) continue;
			int iwire = hit1->wire;
			double coord = (iwire - 72.5)*2.54*0.4;
			double track_coord = NAN;
			if(track_charge > 0.0){
				if(ilayer == 0) track_coord = mwpc_projections.at(MWPCKey::fmwpc1_proj_plus).X();
				if(ilayer == 1) track_coord = mwpc_projections.at(MWPCKey::fmwpc2_proj_plus).Y();
				if(ilayer == 2) track_coord = mwpc_projections.at(MWPCKey::fmwpc3_proj_plus).X();
				if(ilayer == 3) track_coord = mwpc_projections.at(MWPCKey::fmwpc4_proj_plus).Y();
				if(ilayer == 4) track_coord = mwpc_projections.at(MWPCKey::fmwpc5_proj_plus).X();
				if(ilayer == 5) track_coord = mwpc_projections.at(MWPCKey::fmwpc6_proj_plus).Y();
			}else{
				if(ilayer == 0) track_coord = mwpc_projections.at(MWPCKey::fmwpc1_proj_minus).X();
				if(ilayer == 1) track_coord = mwpc_projections.at(MWPCKey::fmwpc2_proj_minus).Y();
				if(ilayer == 2) track_coord = mwpc_projections.at(MWPCKey::fmwpc3_proj_minus).X();
				if(ilayer == 3) track_coord = mwpc_projections.at(MWPCKey::fmwpc4_proj_minus).Y();
				if(ilayer == 4) track_coord = mwpc_projections.at(MWPCKey::fmwpc5_proj_minus).X();
				if(ilayer == 5) track_coord = mwpc_projections.at(MWPCKey::fmwpc6_proj_minus).Y();
			}
			if(std::isnan(track_coord)){
				continue;
			}
			s = mwpc_sigma(jlayer,track_energy);
			double diff = coord - track_coord - fmwpc_al[ilayer];
			if(fabs(diff)>s*5.) continue;
			if(pfinalChamber < ilayer) pfinalChamber = ilayer;
			if(ilayer == 0){
				fmwpc1_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 1){
				fmwpc2_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 2){
				fmwpc3_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 3){
				fmwpc4_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 4){
				fmwpc5_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 5){
				fmwpc6_wire_diff.emplace(iwire,diff);
			}
		}
	}
	int pfinalChamber1to5 = -1;
	if(fmwpc1m8 > 0) pfinalChamber1to5 = 0;
	if(fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber1to5 = 1;
	if(fmwpc3m8 > 0 && fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber1to5 = 2;
	if(fmwpc4m8 > 0 && fmwpc3m8 > 0 && fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber1to5 = 3;
	if(fmwpc5m8 > 0 && fmwpc4m8 > 0 && fmwpc3m8 > 0 && fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber1to5 = 4;


	pfinalChamber = -1;
	if(fmwpc1m8 > 0) pfinalChamber = 0;
	if(fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber = 1;
	if(fmwpc3m8 > 0 && fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber = 2;
	if(fmwpc4m8 > 0 && fmwpc3m8 > 0 && fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber = 3;
	if(fmwpc5m8 > 0 && fmwpc4m8 > 0 && fmwpc3m8 > 0 && fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber = 4;

	return true;
}

bool DCPPSelect::CheckTrackToCTOF_CPP(const vector<const DCTOFPoint*>& ctof_hit, DVector3 ctof_proj_pos, vector<int>& bCTGH){

	double cwire_min_X[4] = {-52.324,-33.02,12.7,32.004};
	double cwire_max_X[4] = {-28.956,-7.62,35.052,55.372};

	double x1ctof = ctof_proj_pos.x();
	double y1ctof = ctof_proj_pos.y();

	int b1f = 0, b2f = 0, b3f = 0, b4f = 0;
	if(cwire_min_X[0] < x1ctof && cwire_max_X[0] > x1ctof) b1f = 1;
	if(cwire_min_X[1] < x1ctof && cwire_max_X[1] > x1ctof) b2f = 1;
	if(cwire_min_X[2] < x1ctof && cwire_max_X[2] > x1ctof) b3f = 1;
	if(cwire_min_X[3] < x1ctof && cwire_max_X[3] > x1ctof) b4f = 1;

	int match = b1f + b2f + b3f + b4f;

	if(b1f) bCTGH[0]=1;
	if(b2f) bCTGH[1]=1;
	if(b3f) bCTGH[2]=1;
	if(b4f) bCTGH[3]=1;
	return match > 0;
}

bool DCPPSelect::MatchToPaddle(double x_track_pos, const vector<int>& allowedPaddles,int hitPaddleID){
    if(std::find(allowedPaddles.begin(), allowedPaddles.end(), hitPaddleID) == allowedPaddles.end()) {
        return false; // reject this event
    }else{
        return true;
    }
}

bool DCPPSelect::MatchToCTOFHit_CPP(const vector<const DCTOFPoint*>& ctof_Hits, vector<const DCTOFPoint*>& ctof_matched_hits, DVector3 ctof_proj_pos,const vector<int>& allowedPaddles){
	double cwire_min_X[4] = {-52.324,-33.02,12.7,32.004};
	double cwire_max_X[4] = {-28.956,-7.62,35.052,55.372};

	double x1ctof = 0.0;
	int b1CTGH = 0, b2CTGH = 0, b3CTGH = 0, b4CTGH = 0;
	
	x1ctof = ctof_proj_pos.x();
	
	bool bar_flags[4] = {0,0,0,0};

	for(int i = 0; i<4; i++){
		if(cwire_min_X[i] < x1ctof && cwire_max_X[i] > x1ctof) bar_flags[i] += 1;
	}
	
	if(bar_flags[0] == 0 && bar_flags[1] == 0 && bar_flags[2] == 0 && bar_flags[3] == 0) return false;
	for(unsigned int ictof = 0; ictof< ctof_Hits.size(); ictof++){
		const DCTOFPoint *hit1 = ctof_Hits[ictof];

		if(!MatchToPaddle(x1ctof,allowedPaddles,hit1->bar)) continue;
		
		if(hit1->bar == 1 && bar_flags[0]) b1CTGH +=1;
		if(hit1->bar == 2 && bar_flags[1]) b2CTGH +=1;
		if(hit1->bar == 3 && bar_flags[2]) b3CTGH +=1;
		if(hit1->bar == 4 && bar_flags[3]) b4CTGH +=1; 
		if(b1CTGH + b2CTGH + b3CTGH + b4CTGH > 0) ctof_matched_hits.push_back(hit1);

	}
	
	if(b1CTGH > 0 || b2CTGH > 0 || b3CTGH > 0 || b4CTGH > 0){
		return true;
	}else{
		return false;
	}
}
