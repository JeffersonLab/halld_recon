//
// File: DTAGHGeometry.cc
// Created: Sat Jul 5 10:18:56 EST 2014
// Creator: jonesrt on gluey.phys.uconn.edu
//
//  10/19/2019 A.S 
//
//  Modify calculation of the photon beam energy to account 
//  for the fact that the energy of bremsstrahlung electrons detected
//  by each tagger counter does not depend on the electron beam energy.
//  The photon beam energy E_gamma has to be computed as
//  
//     E_gamma = R * E_endpoint_calib  +  DE,  where
//     DE = E_endpoint - E_endpoint_calib
//
// 

#include <stdlib.h>
#include <iostream>
#include <map>
#include <set>

#include "DTAGHGeometry.h"

using std::string;
using std::set;

const unsigned int DTAGHGeometry::kCounterCount = 274;

// Only print messages for one thread whenever run number change
static pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
static set<int> runs_announced;
    

//---------------------------------
// DTAGHGeometry    (Constructor)
//---------------------------------
DTAGHGeometry::DTAGHGeometry(const std::shared_ptr<const JEvent>& event)
{
	// keep track of which runs we print out messages for
	auto runnumber = event->GetRunNumber();
	pthread_mutex_lock(&print_mutex);
	bool print_messages = false;
	if(runs_announced.find(runnumber) == runs_announced.end()){
	  print_messages = true;
	  runs_announced.insert(runnumber);
	}
	pthread_mutex_unlock(&print_mutex);

	JCalibrationCCDB *jcalib =  dynamic_cast<JCalibrationCCDB*>( DEvent::GetJCalibration(event) );

	Initialize(jcalib, print_messages);
}

//---------------------------------
// DTAGHGeometry    (Constructor)
//---------------------------------
DTAGHGeometry::DTAGHGeometry(JCalibration *jcalib, int32_t runnumber)
{
	pthread_mutex_lock(&print_mutex);
	bool print_messages = false;
	if(runs_announced.find(runnumber) == runs_announced.end()){
	  print_messages = true;
	  runs_announced.insert(runnumber);
	}
	pthread_mutex_unlock(&print_mutex);

	Initialize(jcalib, print_messages);
}

//---------------------------------
// Initialize
//---------------------------------
void DTAGHGeometry::Initialize(JCalibration *jcalib, bool print_messages)
{
   /* read tagger set endpoint energy from calibdb */
   std::map<string,double> result1;
   jcalib->Get("/PHOTON_BEAM/endpoint_energy", result1);
   if (result1.find("PHOTON_BEAM_ENDPOINT_ENERGY") == result1.end()) {
      std::cerr << "Error in DTAGHGeometry constructor: "
                << "failed to read photon beam endpoint energy "
                << "from calibdb at /PHOTON_BEAM/endpoint_energy" << std::endl;
      m_endpoint_energy_GeV = 0;
   }
   else {
      m_endpoint_energy_GeV = result1["PHOTON_BEAM_ENDPOINT_ENERGY"];
   }

   /* read hodoscope counter energy bounds from calibdb */
   std::vector<std::map<string,double> > result2;
   jcalib->Get("/PHOTON_BEAM/hodoscope/scaled_energy_range", result2);
   if (result2.size() != kCounterCount) {
      jerr << "Error in DTAGHGeometry constructor: "
           << "failed to read photon beam scaled_energy_range table "
           << "from calibdb at /PHOTON_BEAM/hodoscope/scaled_energy_range" << std::endl;
      for (unsigned int i=0; i <= TAGH_MAX_COUNTER; ++i) {
         m_counter_xlow[i] = 0;
         m_counter_xhigh[i] = 0;
      }
   }
   else {
      for (unsigned int i=0; i < result2.size(); ++i) {
	int ctr = (result2[i])["counter"];
         m_counter_xlow[ctr] = (result2[i])["xlow"];
         m_counter_xhigh[ctr] = (result2[i])["xhigh"];
      }
   }

   int status = 0;
   m_endpoint_energy_calib_GeV = 0.;
   
   std::map<string,double> result3;
   status = jcalib->Get("/PHOTON_BEAM/hodoscope/endpoint_calib",result3);
   
   
   if(!status){
     if (result3.find("TAGGER_CALIB_ENERGY") == result3.end()) {
       std::cerr << "Error in DTAGHGeometry constructor: "
		 <<  "failed to read  endpoint_calib field "
		 <<  "from /PHOTON_BEAM/hodoscope/endpoint_calib table" << std::endl;
       
     } else {
       m_endpoint_energy_calib_GeV  = result3["TAGGER_CALIB_ENERGY"];

	   if(print_messages)
       	  jout << " Correct Beam Photon Energy (TAGH) = " << m_endpoint_energy_calib_GeV << " (GeV)" << std::endl;

     }
   }
   
   
   /*
   // read tagger set endpoint energy from calibdb 
   std::map<string,double> result1;
   loop->GetCalib("/PHOTON_BEAM/endpoint_energy", result1);
   if (result1.find("PHOTON_BEAM_ENDPOINT_ENERGY") == result1.end()) {
      std::cerr << "Error in DTAGHGeometry constructor: "
                << "failed to read photon beam endpoint energy "
                << "from calibdb at /PHOTON_BEAM/endpoint_energy" << std::endl;
      m_endpoint_energy_GeV = 0;
   }
   else {
      m_endpoint_energy_GeV = result1["PHOTON_BEAM_ENDPOINT_ENERGY"];
   }

   // read hodoscope counter energy bounds from calibdb 
   std::vector<std::map<string,double> > result2;
   loop->GetCalib("/PHOTON_BEAM/hodoscope/scaled_energy_range", result2);
   if (result2.size() != kCounterCount) {
      jerr << "Error in DTAGHGeometry constructor: "
           << "failed to read photon beam scaled_energy_range table "
           << "from calibdb at /PHOTON_BEAM/hodoscope/scaled_energy_range" << jendl;
      for (unsigned int i=0; i <= TAGH_MAX_COUNTER; ++i) {
         m_counter_xlow[i] = 0;
         m_counter_xhigh[i] = 0;
      }
   }
   else {
      for (unsigned int i=0; i < result2.size(); ++i) {
	int ctr = (result2[i])["counter"];
         m_counter_xlow[ctr] = (result2[i])["xlow"];
         m_counter_xhigh[ctr] = (result2[i])["xhigh"];
      }
   }

   int status = 0;
   m_endpoint_energy_calib_GeV = 0.;
   
   std::map<string,double> result3;
   status = calibration->Get("/PHOTON_BEAM/hodoscope/endpoint_calib",result3);
   
   
   if(!status){
     if (result3.find("TAGGER_CALIB_ENERGY") == result3.end()) {
       std::cerr << "Error in DTAGHGeometry constructor: "
		 <<  "failed to read  endpoint_calib field "
		 <<  "from /PHOTON_BEAM/hodoscope/endpoint_calib table" << std::endl;
       
     } else {
       m_endpoint_energy_calib_GeV  = result3["TAGGER_CALIB_ENERGY"];

	   if(print_messages)
       	  jout << " Correct Beam Photon Energy (TAGH) = " << m_endpoint_energy_calib_GeV << " (GeV)" << std::endl;

     }
   }
   */

}

DTAGHGeometry::~DTAGHGeometry() { }


bool DTAGHGeometry::E_to_counter(double E, unsigned int &counter) const
{  
  for (counter = 1; counter <= kCounterCount; ++counter) {     
    
    double Emin = getElow(counter);
    double Emax = getEhigh(counter);
    
    if ( E >= Emin &&  E <= Emax )
      {
	return true;
      }
  }
  return false;
}


double DTAGHGeometry::getElow(unsigned int counter) const
{
  if (counter > 0 && counter <= kCounterCount){
    if(m_endpoint_energy_calib_GeV  > 0){

      double delta_E  =  m_endpoint_energy_GeV  -  m_endpoint_energy_calib_GeV;
      double Emin     =  m_counter_xlow[counter]*m_endpoint_energy_calib_GeV  +  delta_E;
      
      return Emin;

    } else
      return m_endpoint_energy_GeV * m_counter_xlow[counter];

  }  else
    return 0;
}


double DTAGHGeometry::getEhigh(unsigned int counter) const
{
  if (counter > 0 && counter <= kCounterCount){
    if(m_endpoint_energy_calib_GeV  > 0){
      
      double delta_E  =  m_endpoint_energy_GeV  -  m_endpoint_energy_calib_GeV;
      double Emax     =  m_counter_xhigh[counter]*m_endpoint_energy_calib_GeV  +  delta_E;
      
      return Emax;
      
    } else    
      return m_endpoint_energy_GeV * m_counter_xhigh[counter];
    
  } else
    return 0;
}
