// $Id: DBCALShower_factory_JLAB.h 
//
//    File: DBCALShower_factory_JLAB.h
// Created: Mon Mar 18 09:42:29 EDT 2013
// Creator: Benedikt Zihlmann version 0.1
//


#ifndef _DBCALShower_factory_JLAB_
#define _DBCALShower_factory_JLAB_

#include <JANA/JFactoryT.h>

#include <BCAL/DBCALShower.h>


/// The showers produced here are based on the output of the DBCALClump_factory

class DBCALShower_factory_JLAB: public JFactoryT<DBCALShower>{
  
 public:
  
  DBCALShower_factory_JLAB();
  ~DBCALShower_factory_JLAB() override = default;
  

 private:
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  
};

#endif // _DBCALShower_factory_JLAB_

