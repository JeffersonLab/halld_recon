// $Id$
//
//    File: DTrackCandidate_factory_StraightLine.h
// Created: Fri Aug 15 09:14:04 EDT 2014
// Creator: staylor (on Linux ifarm1102 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#ifndef _DTrackCandidate_factory_StraightLine_
#define _DTrackCandidate_factory_StraightLine_

#include <JANA/JFactoryT.h>
#include "DTrackCandidate.h"
#include "TRACKING/DTrackFinder.h"
#include <TRACKING/DTrackFitter.h>
#include "CDC/DCDCTrackHit.h"
#include "FDC/DFDCPseudo.h"
#include "DMatrixSIMD.h"
#include <deque>
#include <TROOT.h>
#include <TH2F.h>
#include <TH1I.h>
#include "PID/DParticleID.h"

class DParticleID;

class DTrackCandidate_factory_StraightLine:public JFactoryT<DTrackCandidate>{
   public:
      DTrackCandidate_factory_StraightLine(){
      	SetTag("StraightLine");
      };
      ~DTrackCandidate_factory_StraightLine(){};

      enum state_vector{
         state_x,
         state_y,
         state_tx,
         state_ty,
      };
      enum state_cartesian{
	state_Px,
	state_Py,
	state_Pz,
	state_X,
	state_Y,
	state_Z,
	state_T
      };


      class trajectory_t{
         public:
            trajectory_t(double z,double t,DMatrix4x1 S,DMatrix4x4 J,DMatrix4x1 Skk,
                  DMatrix4x4 Ckk,unsigned int id=0,unsigned int numhits=0)
               :z(z),t(t),S(S),J(J),Skk(Skk),Ckk(Ckk),id(id),numhits(numhits){}
            double z,t; 
            DMatrix4x1 S;
            DMatrix4x4 J;
            DMatrix4x1 Skk;
            DMatrix4x4 Ckk;
            unsigned int id,numhits;

      };

      typedef struct{
         double resi,err,d,delta,tdrift,ddrift,s,V;
         DMatrix4x1 S;
         DMatrix4x4 C;
      }cdc_update_t;

      typedef struct{
         double d,tdrift,s;
         DMatrix4x1 S;
         DMatrix4x4 C;
         DMatrix2x2 V;
      }fdc_update_t;


   private:
      void Init() override;
      void BeginRun(const std::shared_ptr<const JEvent>& event) override;
      void Process(const std::shared_ptr<const JEvent>& event) override;
      void EndRun() override;
      void Finish() override;

      bool COSMICS,DEBUG_HISTS,USE_FDC_DRIFT_TIMES,SKIP_CDC,SKIP_FDC;
      float CHI2CUT;
      int DO_PRUNING;
      int PLANE_TO_SKIP;

      DTrackFinder *finder;
      const DParticleID* dParticleID;

      // Diagnostic histograms
      TH2F *Hvres;
      TH1I *hFDCOccTrkFind, *hFDCOccTrkFit, *hFDCOccTrkSmooth;

      // pool of TMatrix's
      shared_ptr<DResourcePool<TMatrixFSym>> dResourcePool_TMatrixFSym;

};

#endif // _DTrackCandidate_factory_StraightLine_

