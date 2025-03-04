// $Id$
//
//    File: DPSPair_factory.h
// Created: Fri Mar 20 07:51:31 EDT 2015
// Creator: nsparks (on Linux cua2.jlab.org 2.6.32-431.5.1.el6.x86_64 x86_64)
//

#ifndef _DPSPair_factory_
#define _DPSPair_factory_

#include <JANA/JFactoryT.h>
#include "DPSPair.h"

class DPSPair_factory:public JFactoryT<DPSPair>{
 public:
  DPSPair_factory(){};
  ~DPSPair_factory(){};

  double DELTA_T_CLUST_MAX;
  double DELTA_T_PAIR_MAX;


  typedef struct {
    int     column;
    double  energy;

    double  integral;
    double  pulse_peak;
    double  time;
    int     used;       
  } tile;

  typedef struct {

    int ntiles;

    vector<int> hit_index;

    double  energy;
    double  time;

    int     column;
    double  integral;
    double  pulse_peak;
    double  time_tile;

  } clust;


  vector<tile> tiles_left;
  vector<tile> tiles_right;

  vector<clust> clust_left;
  vector<clust> clust_right;


  static bool SortByTile(const tile &tile1, const tile &tile2);

 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;
};

#endif // _DPSPair_factory_

