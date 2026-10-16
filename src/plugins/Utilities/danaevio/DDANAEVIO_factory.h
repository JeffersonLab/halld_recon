// $Id$

//    File: DDANAEVIO_factory.h
// Created: Mon Mar 15 09:08:37 EDT 2010
// Creator: wolin (on Linux stan.jlab.org 2.6.18-164.el5 x86_64)



#ifndef _DDANAEVIO_factory_
#define _DDANAEVIO_factory_


#include <string>
#include <map>
#include <set>
using namespace std;


#include <JANA/JFactoryT.h>


#include <danaevio/DDANAEVIODOMTree.h>



//------------------------------------------------------------------------------------


class DDANAEVIO_factory : public JFactoryT<DDANAEVIODOMTree> {
  
 public:
  DDANAEVIO_factory();
  ~DDANAEVIO_factory() {};

  static const map< string, pair<uint16_t,uint8_t> > *getTagMapPointer();


 private:
  void Process(const std::shared_ptr<const JEvent>& event, uint64_t eventnumber);
  void get_tagNum_dictionary(void);
  static void startElement(void *userData, const char *xmlname, const char **atts);

  template<typename T> evioDOMNodeP createLeafNode(string nameId);
  evioDOMNodeP createContainerNode(string nameId);


  void addObjIdBank(evioDOMTree &tree);

  void addDMCTrackHit(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDBeamPhoton(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDMCThrown(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDFCALTruthShower(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDBCALTruthShower(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDTOFTruth(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDSCTruthHit(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDMCTrajectoryPoint(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDCDCHit(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDFDCHit(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDFCALHit(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDHDDMBCALHit(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDHDDMTOFHit(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDSCHit(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDCDCTrackHit(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDFDCPseudo(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDTrackWireBased(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDTrackTimeBased(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDChargedTrack(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDPhoton(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDVertex(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDTrackCandidate(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDBCALPhoton(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDFCALPhoton(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDChargedTruthMatch(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDTOFRawHitMC(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDTOFRawHit(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDTOFHit(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDTOFPoint(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDTOFMCResponse(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDBCALHit(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDBCALMCResponse(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDBCALShower(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDFCALCluster(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDFDCCathodeCluster(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDFDCSegment(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDTwoGammaFit(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDParticle(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDTAGMHit(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);
  void addDTAGHHit(const std::shared_ptr<const JEvent>& event, evioDOMTree &tree);


  // map of which factory/tags to convert, can be set on-the-fly via setEVIOMap()
  map< string, set<string> > evioMap;


  // event-specific global object id map
  map<int,string> objIdMap;


public:
  void setEVIOMap(string danaevio);

};

#endif // _DDANAEVIO_factory_


//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
