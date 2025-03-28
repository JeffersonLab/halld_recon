//******************************************************************
// DTRDSegment_factory.h: class definition for a factory creating
// track segments from points - derived from DFDCSegment_factory.h
//******************************************************************
#ifndef DFACTORY_DTRDSEGMENT_H
#define DFACTORY_DTRDSEGMENT_H

#include "JANA/JFactoryT.h"
#include "JANA/JException.h"

#include "DTRDSegment.h"
#include "DTRDPoint.h"

#include <algorithm>
#include <map>
#include <cmath>

class DTRDSegment_factory : public JFactoryT<DTRDSegment> {
public:
  DTRDSegment_factory();
  ~DTRDSegment_factory();
  
protected:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  // 		void EndRun() override;
  // 		void Finish() override;
  
private:
  void FindSegments(const vector<const DTRDPoint *>&points,
		    vector<vector<const DTRDPoint *>>&segments) const;
  void FitLine(const vector<const DTRDPoint *>&points,
	       double &x0,double &y0,double &tx,double &ty,
	       double &var_x,double &var_y,double &var_tx,double &var_ty) const;
  
  double dTRDz;
  int DEBUG_LEVEL;
};

#endif // DFACTORY_DTRDSEGMENT_H

