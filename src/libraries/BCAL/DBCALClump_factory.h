/*
 *  DBCALClump_factory.h
 *
 *  Created by Beni Zihlmann Tue Mar 12 2013       version: 0.1
 *
 */

#ifndef _DBCALClump_factory_
#define _DBCALClump_factory_

#include <JANA/JFactoryT.h>


#include "BCAL/DBCALClump.h"

class DBCALClump_factory : public JFactoryT<DBCALClump> {

  /// This factory creats Clumps based on all DBCALHits. A Clump is an object of
  /// two related clusters between up stream and down stream BCAL Hits. A seed
  /// is found by searching for the highest energy deposited in the hit list that
  /// has both and up stream and down stream value. A list of hits is formed 
  /// around this seed hit independently for up stream and downstream. The hits used
  /// are removed from the search list. From the remaining hit list again the highest
  /// energy deposit is searched that has both an up stream and down stream value and
  /// the procedure is repeated till no hits with both upstream and down stream values
  /// are found. Each seed results in two lists of hits for up and down stream shower
  /// cluster that form a Clump.
  /// Note that at this point a Clump could contain easily two particle showers close 
  /// together. A disentanglement of the Clump will require a successive step.
  /// From a Clump (see DBCALClump.cc) a shower is constructed: verstion 0.1
  /// BTW. Clump is a perfectly good english word ;-)

 public:

  DBCALClump_factory() {}
  ~DBCALClump_factory() {}
  
  double VELOCITY;

 private:
  
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  
};

#endif //_DBCALClump_factory_
