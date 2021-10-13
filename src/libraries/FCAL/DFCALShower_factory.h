// $Id: DFCALShower_factory.h 1899 2006-07-13 16:29:56Z davidl $
//
//    File: DFCALShower_factory.h
// Created: Tue May 17 11:57:50 EST 2005
// Creator: remitche (on Linux mantrid00 2.4.20-18.8smp i686)
// Edited: B. Schaefer 3/23/2012 removed radiation hard insert functionality

#ifndef _DFCALShower_factory_
#define _DFCALShower_factory_

#include <JANA/JFactory.h>
#include <JANA/JEventLoop.h>
#include <FCAL/DFCALShower.h>
#include <FCAL/DFCALCluster.h>
#include <DANA/DApplication.h>

#include <DMatrixDSym.h>

#include <TH2F.h>

class DFCALHit;
class DTrackWireBased;

class DFCALShower_factory:public JFactory<DFCALShower>{
 public:
  DFCALShower_factory();
  ~DFCALShower_factory(){};
  jerror_t LoadCovarianceLookupTables(JEventLoop *eventLoop);
  jerror_t FillCovarianceMatrix(DFCALShower* shower);
	
 private:

  jerror_t evnt(JEventLoop *eventLoop, uint64_t eventnumber);	///< Invoked via JEventProcessor virtual method
  jerror_t brun(JEventLoop *loop, int32_t runnumber);
  jerror_t erun(void);

  void GetCorrectedEnergyAndPosition(const DFCALCluster* cluster, int ring_nb,
				     double &Ecorrected,
				     DVector3 &pos_corrected, double &errZ,
				     const DVector3 *aVertex,
				     bool in_insert=false);
  
  void GetLogWeightedPosition( const DFCALCluster* cluster, DVector3 &pos_log, 
  				     double Egamma, const DVector3 *aVertex  );

  unsigned int getMaxHit( const vector< const DFCALHit* >& hitVec ) const;
  unsigned int getMaxHit(int chan_Emax, const vector< const DFCALHit* >& hitVec ) const;

  void getUVFromHits( double& sumUSh, double& sumVSh, 
		      const vector< const DFCALHit* >& hits,
		      const DVector3& showerVec,
		      const DVector3& trackVec ) const;

  void getE1925FromHits( double& e1e9Sh, double& e9e25Sh, 
			 const vector< const DFCALHit* >& hits,
			 unsigned int maxIndex ) const;

  vector< const DTrackWireBased* >
    filterWireBasedTracks( vector< const DTrackWireBased* >& wbTracks ) const;
  
  double m_zTarget, m_FCALfront;
  double m_FCALdX,m_FCALdY;
  double m_beamSpotX;
  double m_beamSpotY;  
  bool LOAD_NONLIN_CCDB;
  bool LOAD_TIMING_CCDB; 
  bool LOAD_CCDB_CONSTANTS;
  bool USE_RING_E_CORRECTION;
  double SHOWER_ENERGY_THRESHOLD;
  double cutoff_energy;
  double linfit_slope;
  double linfit_intercept;
  double expfit_param1;
  double expfit_param2;
  double expfit_param3;
  
  vector < vector <double> > energy_dependence_correction_vs_ring;
  
  double timeConst0;
  double timeConst1;
  double timeConst2;
  double timeConst3;
  double timeConst4;

  double FCAL_RADIATION_LENGTH;
  double FCAL_CRITICAL_ENERGY;
  double FCAL_SHOWER_OFFSET;
  double FCAL_C_EFFECTIVE;

  // parameters for insert
  double INSERT_PAR1,INSERT_PAR2;
  double INSERT_RADIATION_LENGTH;
  double INSERT_CRITICAL_ENERGY;
  double INSERT_SHOWER_OFFSET;
  double INSERT_C_EFFECTIVE;
  double m_insertFront;
  double INSERT_POS_RES1,INSERT_POS_RES2;
  double INSERT_E_VAR1,INSERT_E_VAR2,INSERT_E_VAR3;

  const DFCALGeometry *fcalGeom=NULL;

  int VERBOSE;
  string COVARIANCEFILENAME;
  TH2F *CovarianceLookupTable[5][5];
  
  double log_position_const;
};


#endif // _DFCALShower_factory_

