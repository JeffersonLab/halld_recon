//
// File: DTAGMGeometry.cc
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
//
// 


#include <stdlib.h>
#include <iostream>
#include <map>

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include "DTAGMGeometry.h"

const unsigned int DTAGMGeometry::kRowCount = 5;
const unsigned int DTAGMGeometry::kColumnCount = 102;
const double DTAGMGeometry::kFiberWidth = 0.2; // cm
const double DTAGMGeometry::kFiberLength = 2.0; // cm


//---------------------------------
// DTAGMGeometry    (Constructor)
//---------------------------------
DTAGMGeometry::DTAGMGeometry(JEventLoop *loop)
{
   /* read tagger set endpoint energy from calibdb */
   std::map<string,double> result1;
   loop->GetCalib("/PHOTON_BEAM/endpoint_energy", result1);
   if (result1.find("PHOTON_BEAM_ENDPOINT_ENERGY") == result1.end()) {
      std::cerr << "Error in DTAGMGeometry constructor: "
                << "failed to read photon beam endpoint energy "
                << "from calibdb at /PHOTON_BEAM/endpoint_energy" << std::endl;
      m_endpoint_energy_GeV = 0;
   }
   else {
      m_endpoint_energy_GeV = result1["PHOTON_BEAM_ENDPOINT_ENERGY"];
   }

   /* read microscope channel energy bounds from calibdb */
   std::vector<std::map<string,double> > result2;
   loop->GetCalib("/PHOTON_BEAM/microscope/scaled_energy_range", result2);
   if (result2.size() != kColumnCount) {
      std::cerr << "Error in DTAGMGeometry constructor: "
                << "failed to read photon beam scaled_energy_range table "
                << "from calibdb at /PHOTON_BEAM/microscope/scaled_energy_range" << std::endl;
      for (unsigned int i=0; i <= TAGM_MAX_COLUMN; ++i) {
         m_column_xlow[i] = 0;
         m_column_xhigh[i] = 0;
      }
   }
   else {
      for (unsigned int i=0; i < result2.size(); ++i) {
         int column = (result2[i])["column"];
         m_column_xlow[column] = (result2[i])["xlow"];
         m_column_xhigh[column] = (result2[i])["xhigh"];
      }
   }


   int status = 0;

   m_endpoint_energy_calib_GeV = 0.;
   
   std::map<string,double> result3;
   status = loop->GetCalib("/PHOTON_BEAM/hodoscope/endpoint_calib",result3);
   
   
   if(!status){
     
     if (result3.find("TAGGER_CALIB_ENERGY") == result3.end()) {
       std::cerr << "Error in DTAGHGeometry constructor: "
		 <<  "failed to read  endpoint_calib field "
		 <<  "from /PHOTON_BEAM/hodoscope/endpoint_calib table" << std::endl;      
       
     } else {
       
       m_endpoint_energy_calib_GeV  = result3["TAGGER_CALIB_ENERGY"];       

       printf(" Correct Beam Photon Energy  (TAGM)   %f \n\n", m_endpoint_energy_calib_GeV);

     }
   }
      
}


DTAGMGeometry::~DTAGMGeometry() { }


bool DTAGMGeometry::E_to_column(double E, unsigned int &column) const
{
   for (column = 1; column <= kColumnCount; ++column) {

     double Emin = getElow(column);
     double Emax = getEhigh(column);
     
     if ( E >= Emin &&  E <= Emax ){

         return true;
      }
   }
   return false;
}

double DTAGMGeometry::getElow(unsigned int column) const
{
  if (column > 0 && column <= kColumnCount){
    
    if(m_endpoint_energy_calib_GeV  > 0){

      double delta_E  =  m_endpoint_energy_GeV  -  m_endpoint_energy_calib_GeV;
      double Emin     =  m_column_xlow[column]*m_endpoint_energy_calib_GeV  +  delta_E;
      
      return Emin;

    } else
      return m_endpoint_energy_GeV * m_column_xlow[column];    
    
  } else
    return 0;
}


double DTAGMGeometry::getEhigh(unsigned int column) const
{
  if (column > 0 && column <= kColumnCount){
    
    if(m_endpoint_energy_calib_GeV  > 0){
      
      double delta_E  =  m_endpoint_energy_GeV  -  m_endpoint_energy_calib_GeV;
      double Emax     =  m_column_xhigh[column]*m_endpoint_energy_calib_GeV  +  delta_E;
      
      return Emax;
      
    } else    
      return m_endpoint_energy_GeV * m_column_xhigh[column];
    
  }  else
    return 0;
}

