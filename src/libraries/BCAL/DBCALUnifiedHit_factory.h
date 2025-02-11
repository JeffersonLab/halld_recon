#ifndef _DBCALUnifiedHit_factory_
#define _DBCALUnifiedHit_factory_

#include <JANA/JFactoryT.h>

#include "BCAL/DBCALUnifiedHit.h"
#include "BCAL/DBCALTDCHit.h"
#include "BCAL/DBCALHit.h"
#include "BCAL/DBCALGeometry.h"

#include <TTree.h>

class DBCALUnifiedHit_factory : public JFactoryT<DBCALUnifiedHit> {

 public:

  int VERBOSE;
  DBCALUnifiedHit_factory() = default;
  ~DBCALUnifiedHit_factory() override = default;

  TTree *bcal_points_tree;

 private:
 
  class cellHits{
   public:
    vector<const DBCALHit*> hits;
    vector<const DBCALTDCHit*> tdc_hits;
  };
 
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& aEvent) override;

  // Use TDC Times"
  bool USE_TDC;

  float E_tree;
  float t_tdc_tree;
  float t_adc_tree;
  float t_tdc_corrected_tree;
  float t_adc_corrected_tree;
  int layer_tree;
  bool end_tree;

  //Used as a key for maps
  class readout_channel {
   public:
    readout_channel(int cellId, DBCALGeometry::End end) :
      cellId(cellId), end(end) {}

    int cellId;
    DBCALGeometry::End end;

    bool operator<(const readout_channel &c) const {
      if (cellId<c.cellId) return true;
      if (cellId>c.cellId) return false;
      if (end==DBCALGeometry::kUpstream && c.end==DBCALGeometry::kDownstream) return true;
      return false;
    }
  };

  //For now timewalk corrections are of the form f(ADC) = c0 + c1/(ADC-c3)^c2
  //Store all coefficients in one structure
  /* class timewalk_coefficients { */
  /*  public: */
  /*   timewalk_coefficients() : */
  /*     a_thresh(0), c0(0), c1(0), c2(0) {} */
  /*   timewalk_coefficients(float c0, float c1, float c2, float a_thresh) : */
  /*     a_thresh(a_thresh), c0(c0), c1(c1), c2(c2) {} */
  /*   float a_thresh,c0,c1,c2; */
  /* }; */

  /* map<readout_channel,timewalk_coefficients> tdc_timewalk_map; */

  // structure to store a 4 parameter fit with a threshold
  class timewalk_coefficients_c4 {
  public:
  timewalk_coefficients_c4() :
      thresh(0), c0(0), c1(0), c2(0), c3(0) {}
  timewalk_coefficients_c4(float c0, float c1, float c2, float c3, float thresh) :
      thresh(thresh), c0(c0), c1(c1), c2(c2), c3(c3) {}
	  float thresh,c0,c1,c2,c3;
  };

  map<readout_channel,timewalk_coefficients_c4> tdc_timewalk_map_c4;

  const DBCALGeometry *dBCALGeom;

  //write out tree with hit info?
  static const int enable_debug_output = 0;
};

#endif //_DBCALUnifiedHit_factory_
