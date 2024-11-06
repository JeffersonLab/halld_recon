// $Id$
//
//    File: DGeometry.cc
// Created: Thu Apr  3 08:43:06 EDT 2008
// Creator: davidl (on Darwin swire-d95.jlab.org 8.11.1 i386)
//

#include <algorithm>

#include <JANA/Calibrations/JCalibrationManager.h>
#include <JANA/Compatibility/JGeometryXML.h>

#include "DGeometry.h"
#include "FDC/DFDCWire.h"
#include "FDC/DFDCGeometry.h"
#include <ansi_escape.h>

using namespace std;

#ifndef M_TWO_PI
#define M_TWO_PI 6.28318530717958647692
#endif

//---------------------------------
// DGeometry    (Constructor)
//---------------------------------
DGeometry::DGeometry(JGeometry *jgeom, DGeometryManager *dgeoman, JApplication* app, int32_t runnumber)
{
	this->jgeom = jgeom;
	this->dgeoman = dgeoman;
	this->app = app;
	this->jcalman = app->GetService<JCalibrationManager>();
	this->bfield = nullptr; // don't ask for B-field object before we're asked for it
	this->runnumber = runnumber;
	this->materialmaps_read = false;
	this->materials_read = false;
	
	pthread_mutex_init(&bfield_mutex, nullptr);
	pthread_mutex_init(&materialmap_mutex, nullptr);
	pthread_mutex_init(&materials_mutex, nullptr);

	ReadMaterialMaps();
}

//---------------------------------
// ~DGeometry    (Destructor)
//---------------------------------
DGeometry::~DGeometry()
{
	pthread_mutex_lock(&materials_mutex);
	for(unsigned int i=0; i<materials.size(); i++)delete materials[i];
	materials.clear();
	pthread_mutex_unlock(&materials_mutex);

	pthread_mutex_lock(&materialmap_mutex);
	for(unsigned int i=0; i<materialmaps.size(); i++)delete materialmaps[i];
	materialmaps.clear();
	pthread_mutex_unlock(&materialmap_mutex);
}

//---------------------------------
// GetBfield
//---------------------------------
DMagneticFieldMap* DGeometry::GetBfield(void) const
{
	pthread_mutex_lock(&bfield_mutex);
	if(bfield == NULL) bfield = dgeoman->GetBfield(runnumber);
	pthread_mutex_unlock(&bfield_mutex);

	return bfield;
}

//---------------------------------
// GetLorentzDeflections
//---------------------------------
DLorentzDeflections* DGeometry::GetLorentzDeflections(void)
{
	return dgeoman->GetLorentzDeflections(runnumber);
}

//---------------------------------
// GetMaterialMapVector
//---------------------------------
vector<DMaterialMap*> DGeometry::GetMaterialMapVector(void) const
{
//	ReadMaterialMaps();

	return materialmaps;
}

//---------------------------------
// ReadMaterialMaps
//---------------------------------
void DGeometry::ReadMaterialMaps(void) const
{
	/// This gets called by several "FindMat" methods below. It
	/// will return immediately if the material maps have already
	/// been read in. If they have not been read in, then it reads
	/// them and sets a flag so that they are only read in once.
	///
	/// Orginally, this code resided in the constructor, but was
	/// moved here so that programs not requiring the material
	/// maps could start up quicker and skip reading them in altogether.

	// Lock mutex so we can check flag to see if maps have already
	// been read in
	pthread_mutex_lock(&materialmap_mutex);
	if(materialmaps_read){
		// Maps are already read. Unlock mutex and return now
		pthread_mutex_unlock(&materialmap_mutex);
		return;
	}

	JCalibration * jcalib = app->GetService<JCalibrationManager>()->GetJCalibration(runnumber);
	if(!jcalib){
		_DBG_<<"ERROR:  Unable to get JCalibration object!"<<endl;
		pthread_mutex_unlock(&materialmap_mutex);
		return;
	}

	// Get a list of all namepaths so we can find all of the material maps.
	// We want to read in all material maps with a standard naming scheme
	// so the number and coverage of the piece-wise maps can be changed
	// without requiring recompilation.
	//vector<DMaterialMap*> material_maps;
	vector<string> namepaths;
	jcalib->GetListOfNamepaths(namepaths);
	vector<string> material_namepaths;
	for(unsigned int i=0; i<namepaths.size(); i++){
		if(namepaths[i].find("Material/material_map")==0)material_namepaths.push_back(namepaths[i]);
	}
	
	// Sort alphabetically so user controls search sequence by map name
	sort(material_namepaths.begin(), material_namepaths.end());
	
	// Inform user what's happening
	if(material_namepaths.size()==0){
		jerr<<"No material maps found in calibration DB!!"<<jendl;
		pthread_mutex_unlock(&materialmap_mutex);
		return;
	}
	jout<<"Found "<<material_namepaths.size()<<" material maps in calib. DB"<<jendl;
	
	if(false){ // save this to work off configuration parameter
		jout<<"Will read in the following:"<<jendl;
		for(unsigned int i=0; i<material_namepaths.size(); i++){
			jout<<"  "<<material_namepaths[i]<<jendl;
		}
	}

	// Actually read in the maps
	uint32_t Npoints_total=0;
	//cout<<endl; // make empty line so material map can overwrite it below
	for(unsigned int i=0; i<material_namepaths.size(); i++){
		// DMaterialMap constructor prints line so we conserve real
		// estate by having each recycle the line
		//cout<<ansi_up(1)<<string(85, ' ')<<"\r";
		DMaterialMap *mat = new DMaterialMap(material_namepaths[i], jcalib, app->GetJParameterManager());
		if( ! mat->IS_VALID ) {
			// This particular map may not exist for this run/variation
			// (e.g. CPP uses maps downstream of TOF)
			delete mat;
			continue;
		}
		materialmaps.push_back(mat);
		Npoints_total += (unsigned int)(mat->GetNr()*mat->GetNz());
	}
	//cout<<ansi_up(1)<<string(85, ' ')<<"\r";
	jout<<"Read in "<<materialmaps.size()<<" material maps for run "<<runnumber<<" containing "<<Npoints_total<<" grid points total"<<jendl;

	// Set flag that maps have been read and unlock mutex
	materialmaps_read = true;
	pthread_mutex_unlock(&materialmap_mutex);
}

//---------------------------------
// FindNodes
//---------------------------------
void DGeometry::FindNodes(string xpath, vector<xpathparsed_t> &matched_xpaths) const
{
	/// Find all nodes that match the specified xpath and return them as
	/// fully parsed lists of the nodes and attributes.
	///
	/// The matched_xpaths variable has 4 levels of STL containers nested
	/// together! The node_t data type contains 2 of them and represents
	/// a single tag with the "first" member being the tag name
	/// and the "second" member being a map of all of the attributes
	/// of that tag along with their values.
	///
	/// The xpathparsed_t data type is a STL vector of node_t objects
	/// that comprises a complete xpath. The data type of matched_xpaths
	/// is a STL vector if xpathparsed_t objects and so comprises a
	/// list of complete xpaths.
	
	/// We do this by calling the GetXPaths() method of JGeometry to get
	/// a list of all xpaths. Then we pass all of those in to 
	/// JGeometryXML::ParseXPath() to get a parsed list for each. This
	/// is compared to the parsed values of the xpath passed to us
	/// (also parsed by JGeometryXML::ParseXPath()) to find matches.
	
	// Make sure matched_xpaths is empty
	matched_xpaths.clear();
	
	// Cast JGeometry into a JGeometryXML
	JGeometryXML *jgeomxml = dynamic_cast<JGeometryXML*>(jgeom);
	
	// Parse our target xpath
	xpathparsed_t target;
	string unused_string;
	unsigned int unused_int;
	jgeomxml->ParseXPath(xpath, target, unused_string, unused_int);
	
	// Get all xpaths for current geometry source
	vector<string> allxpaths;
	jgeom->GetXPaths(allxpaths, JGeometry::attr_level_all);
	
	// Loop over xpaths
	for(unsigned int i=0; i<allxpaths.size(); i++);
}

//---------------------------------
// FindMatALT1 - parameters for alt1 fitter.
//---------------------------------
jerror_t DGeometry::FindMatALT1(DVector3 &pos, DVector3 &mom,double &KrhoZ_overA, 
				double &rhoZ_overA,double &LnI,
				double &X0, double *s_to_boundary) const
{
//	ReadMaterialMaps();

	for(unsigned int i=0; i<materialmaps.size(); i++){
		jerror_t err = materialmaps[i]->FindMatALT1(pos,KrhoZ_overA, rhoZ_overA,LnI,X0);
		if(err==NOERROR){
			// We found the material map containing this point. If a non-NULL 
			// pointer was passed in for s_to_boundary, then search through all
			// material maps above and including this one to find the estimated
			// distance to the nearest boundary the swimmer should step to. Maps
			// after this one would only be considered outside of this one so
			// there is no need to check them.
			if(s_to_boundary==NULL)return NOERROR;	// User doesn't want distance to boundary
			*s_to_boundary = 1.0E6;
			for(unsigned int j=0; j<=i; j++){
				double s = materialmaps[j]->EstimatedDistanceToBoundary(pos, mom);
				if(s<*s_to_boundary)*s_to_boundary = s;
			}
			return NOERROR;
		}
	}
	return RESOURCE_UNAVAILABLE;
}




//---------------------------------
// FindMatKalman - Kalman filter needs slightly different set of parms.
//---------------------------------
jerror_t DGeometry::FindMatKalman(const DVector3 &pos,const DVector3 &mom,
				  double &KrhoZ_overA, 
				  double &rhoZ_overA, 
				  double &LnI,double &Z,
				  double &chi2c_factor,double &chi2a_factor,
				  double &chi2a_corr,
				  unsigned int &last_index,
				  double *s_to_boundary) const
{
//	ReadMaterialMaps();

  //last_index=0;
  for(unsigned int i=last_index; i<materialmaps.size(); i++){
    jerror_t err = materialmaps[i]->FindMatKalman(pos,KrhoZ_overA,
						  rhoZ_overA,LnI,chi2c_factor,
						  chi2a_factor,chi2a_corr,Z);
    if(err==NOERROR){
      if(i==materialmaps.size()-1) last_index=0;
      else last_index=i;
      if(s_to_boundary==NULL)return NOERROR;	// User doesn't want distance to boundary

      *s_to_boundary = 1.0E6;
      // If we are in the main mother volume, search through all the maps for
      // the nearest boundary
      if(last_index==0){
	for(unsigned int j=0; j<materialmaps.size();j++){
	  double s = materialmaps[j]->EstimatedDistanceToBoundary(pos, mom);
	  if(s<*s_to_boundary){
	    *s_to_boundary = s;
	  }
	}
      }
      else{
	// otherwise, we found the material map containing this point. 
	double s = materialmaps[last_index]->EstimatedDistanceToBoundary(pos, mom);
	if(s<*s_to_boundary)*s_to_boundary = s;
      }
      return NOERROR;
    }
  }
       
return RESOURCE_UNAVAILABLE;
}

jerror_t DGeometry::FindMatKalman(const DVector3 &pos,const DVector3 &mom,
				  double &KrhoZ_overA, 
				  double &rhoZ_overA, 
				  double &LnI,double &Z,
				  unsigned int &last_index,
				  double *s_to_boundary) const
{
//	ReadMaterialMaps();

  //last_index=0;
  for(unsigned int i=last_index; i<materialmaps.size(); i++){
    jerror_t err = materialmaps[i]->FindMatKalman(pos,KrhoZ_overA,
						  rhoZ_overA,LnI,Z);
    if(err==NOERROR){
      if(i==materialmaps.size()-1) last_index=0;
      else last_index=i;
      if(s_to_boundary==NULL)return NOERROR;	// User doesn't want distance to boundary

      *s_to_boundary = 1.0E6;
      // If we are in the main mother volume, search through all the maps for
      // the nearest boundary
      if(last_index==0){
	for(unsigned int j=0; j<materialmaps.size();j++){
	  double s = materialmaps[j]->EstimatedDistanceToBoundary(pos, mom);
	  if(s<*s_to_boundary){
	    *s_to_boundary = s;
	  }
	}
      }
      else{
	// otherwise, we found the material map containing this point. 
	double s = materialmaps[last_index]->EstimatedDistanceToBoundary(pos, mom);
	if(s<*s_to_boundary)*s_to_boundary = s;
      }
      return NOERROR;
    }
  }
       
return RESOURCE_UNAVAILABLE;
}

//---------------------------------
jerror_t DGeometry::FindMatKalman(const DVector3 &pos,
				  double &KrhoZ_overA, 
				  double &rhoZ_overA, 
				  double &LnI, double &Z,double &chi2c_factor, 
				  double &chi2a_factor, double &chi2a_corr,
				  unsigned int &last_index) const
{
//	ReadMaterialMaps();

  //last_index=0;
  for(unsigned int i=last_index; i<materialmaps.size(); i++){
    jerror_t err = materialmaps[i]->FindMatKalman(pos,KrhoZ_overA,
						  rhoZ_overA,LnI,
						  chi2c_factor,chi2a_factor,
						  chi2a_corr,Z);
    if(err==NOERROR){
      if(i==materialmaps.size()-1) last_index=0;
      else last_index=i;
      return err;
    }
  }
       
  return RESOURCE_UNAVAILABLE;
}

//---------------------------------
// Get material properties needed for dEdx
jerror_t DGeometry::FindMatKalman(const DVector3 &pos,
				  double &KrhoZ_overA, 
				  double &rhoZ_overA, 
				  double &LnI, double &Z,
				  unsigned int &last_index) const
{
//	ReadMaterialMaps();

  //last_index=0;
  for(unsigned int i=last_index; i<materialmaps.size(); i++){
    jerror_t err = materialmaps[i]->FindMatKalman(pos,KrhoZ_overA,
						  rhoZ_overA,LnI,Z);
    if(err==NOERROR){
      if(i==materialmaps.size()-1) last_index=0;
      else last_index=i;
      return err;
    }
  }
       
  return RESOURCE_UNAVAILABLE;
}




//---------------------------------
// FindMat
//---------------------------------
jerror_t DGeometry::FindMat(DVector3 &pos, double &rhoZ_overA, double &rhoZ_overA_logI, double &RadLen) const
{
//	ReadMaterialMaps();

	for(unsigned int i=0; i<materialmaps.size(); i++){
		jerror_t err = materialmaps[i]->FindMat(pos, rhoZ_overA, rhoZ_overA_logI, RadLen);
		if(err==NOERROR)return NOERROR;
	}
	return RESOURCE_UNAVAILABLE;
}

//---------------------------------
// FindMat
//---------------------------------
jerror_t DGeometry::FindMat(DVector3 &pos, double &density, double &A, double &Z, double &RadLen) const
{
//	ReadMaterialMaps();

	for(unsigned int i=0; i<materialmaps.size(); i++){
		jerror_t err = materialmaps[i]->FindMat(pos, density, A, Z, RadLen);
		if(err==NOERROR)return NOERROR;
	}
	return RESOURCE_UNAVAILABLE;
}

//---------------------------------
// FindMatNode
//---------------------------------
//const DMaterialMap::MaterialNode* DGeometry::FindMatNode(DVector3 &pos) const
//{
//
//}

//---------------------------------
// FindDMaterialMap
//---------------------------------
const DMaterialMap* DGeometry::FindDMaterialMap(DVector3 &pos) const
{
//	ReadMaterialMaps();

	for(unsigned int i=0; i<materialmaps.size(); i++){
		const DMaterialMap* map = materialmaps[i];
		if(map->IsInMap(pos))return map;
	}
	return NULL;
}

//====================================================================
// Convenience Methods
//
// Below are defined some methods to make it easy to extract certain
// key values about the GlueX detector geometry from the XML source.
// Note that one can still use the generic Get(string xpath, ...) 
// methods. This just packages some of them up for convenience.
//
// The one real gotcha here is that these methods must be kept
// in sync with the XML structure by hand. If volumes are renamed
// or their location within the hierachy is modified, then these
// routines will need to be modified as well. That, or course, is
// also true if you are using the generic Get(...) methods.
//
// What these methods are useful for is when minor changes are made
// to the XML (such as the locations of the FDC packages) they
// are automatically reflected here.
//====================================================================

//---------------------------------
// GetDMaterial
//---------------------------------
const DMaterial* DGeometry::GetDMaterial(string name) const
{
	/// Get a pointer to the DMaterial object with the specified name.
	// Only fill materials member when one is actually requested
	// and then, only fill it once.

	// Lock mutex so we can check flag to see if maps have already
	// been read in
	pthread_mutex_lock(&materials_mutex);
	if(!materials_read) GetMaterials();
	pthread_mutex_unlock(&materials_mutex);

	for(unsigned int i=0; i<materials.size(); i++){
		if(materials[i]->GetName() == name)return materials[i];
	}
	
	//_DBG_<<"No material \""<<name<<"\" found ("<<materials.size()<<" materials defined)"<<endl;
	
	return NULL;
}

//---------------------------------
// GetMaterials
//---------------------------------
void DGeometry::GetMaterials(void) const
{
	/// Read in all of the materials from the geometry source and create
	/// a DMaterial object for each one.
	
	//=========== elements ===========
	string filter = "//materials/element/real[@name=\"radlen\"]";
	
	// Get list of all xpaths
	vector<string> xpaths;
	jgeom->GetXPaths(xpaths, JGeometry::attr_level_all, filter);
	
	// Look for xpaths that have "/materials[" in them
	for(unsigned int i=0; i<xpaths.size(); i++){
		// Get start of "element" section
		string::size_type pos = xpaths[i].find("/element[");
		if(pos == string::npos)continue;
		
		// Get name attribute
		string::size_type start_pos = xpaths[i].find("@name=", pos);
		start_pos = xpaths[i].find("'", start_pos);
		string::size_type end_pos = xpaths[i].find("'", start_pos+1);
		if(end_pos==string::npos)continue;
		string name = xpaths[i].substr(start_pos+1, end_pos-(start_pos+1));

		// Get values
		char xpath[256];

		double A;
		sprintf(xpath,"//materials/element[@name='%s']/[@a]", name.c_str());
		if(!Get(xpath, A))continue;

		double Z;
		sprintf(xpath,"//materials/element[@name='%s']/[@z]", name.c_str());
		if(!Get(xpath, Z))continue;

		double density;
		sprintf(xpath,"//materials/element[@name='%s']/real[@name='density']/[@value]", name.c_str());
		if(!Get(xpath, density))continue;

		double radlen;
		sprintf(xpath,"//materials/element[@name='%s']/real[@name='radlen']/[@value]", name.c_str());
		if(!Get(xpath, radlen))continue;

		DMaterial *mat = new DMaterial(name, A, Z, density, radlen);
		materials.push_back(mat);
		
		cout<<mat->toString();
	}

	//=========== composites ===========
	filter = "//materials/composite[@name]";
	
	// Get list of all xpaths
	jgeom->GetXPaths(xpaths, JGeometry::attr_level_all, filter);

	// Look for xpaths that have "/materials[" in them
	for(unsigned int i=0; i<xpaths.size(); i++){
		// Get start of "composite" section
		string::size_type pos = xpaths[i].find("/composite[");
		if(pos == string::npos)continue;
		
		// Get name attribute
		string::size_type start_pos = xpaths[i].find("@name=", pos);
		start_pos = xpaths[i].find("'", start_pos);
		string::size_type end_pos = xpaths[i].find("'", start_pos+1);
		if(end_pos==string::npos)continue;
		string name = xpaths[i].substr(start_pos+1, end_pos-(start_pos+1));

		if(GetDMaterial(name))continue; // skip duplicates

		// Get values
		char xpath[256];

		// We should calculate an effective A and Z .... but we don't
		bool found_all=true;
		double A=0;
		double Z=0;

		double density;
		sprintf(xpath,"//materials/composite[@name='%s']/real[@name='density']/[@value]", name.c_str());
		found_all &= Get(xpath, density);

		double radlen;
		sprintf(xpath,"//materials/composite[@name='%s']/real[@name='radlen']/[@value]", name.c_str());
		found_all &= Get(xpath, radlen);
		
		// If we didn't find the info we need (radlen and density) in the 
		// composite tag itself. Try calculating them from the components
		if(!found_all)found_all = GetCompositeMaterial(name, density, radlen);
		
		// If we weren't able to get the values needed to make the DMaterial object
		// then skip this one.
		if(!found_all)continue;

		DMaterial *mat = new DMaterial(name, A, Z, density, radlen);
		materials.push_back(mat);
		
		cout<<mat->toString();
	}
	
	materials_read = true;
}

//---------------------------------
// GetCompositeMaterial
//---------------------------------
bool DGeometry::GetCompositeMaterial(const string &name, double &density, double &radlen) const
{
	// Get list of all xpaths with "addmaterial" and "fractionmass"
	char filter[512];
	sprintf(filter,"//materials/composite[@name='%s']/addmaterial/fractionmass[@fraction]", name.c_str());
	vector<string> xpaths;
	jgeom->GetXPaths(xpaths, JGeometry::attr_level_all, filter);
	
	// Loop over components of this composite
_DBG_<<"Components for compsite "<<name<<endl;
	for(unsigned int i=0; i<xpaths.size(); i++){
		// Get component material name
		string::size_type start_pos = xpaths[i].find("@material=", 0);
		start_pos = xpaths[i].find("'", start_pos);
		string::size_type end_pos = xpaths[i].find("'", start_pos+1);
		if(end_pos==string::npos)continue;
		string mat_name = xpaths[i].substr(start_pos+1, end_pos-(start_pos+1));

		// Get component mass fraction
		start_pos = xpaths[i].find("fractionmass[", 0);
		start_pos = xpaths[i].find("@fraction=", start_pos);
		start_pos = xpaths[i].find("'", start_pos);
		end_pos = xpaths[i].find("'", start_pos+1);
		if(end_pos==string::npos)continue;
		string mat_frac_str = xpaths[i].substr(start_pos+1, end_pos-(start_pos+1));
		double fractionmass = atof(mat_frac_str.c_str());

		_DBG_<<"   "<<xpaths[i]<<"  fractionmass="<<fractionmass<<endl;
	}
	
	return true;
}

//---------------------------------
// GetTraversedMaterialsZ
//---------------------------------
//void DGeometry::GetTraversedMaterialsZ(double q, const DVector3 &pos, const DVector3 &mom, double z_end, vector<DMaterialStep> &materialsteps)
//{
	/// Find the materials traversed by a particle swimming from a specific
	/// position with a specific momentum through the magnetic field until
	/// it reaches a specific z-location. Energy loss is not included in
	/// the swimming since this method itself is assumed to be one of the
	/// primary means of determining energy loss. As such, one should not
	/// pass in a value of z_end that is far from pos.
	///
	/// The vector materialsteps will be filled with DMaterialStep objects
	/// corresponding to each of the materials traversed.
	///
	/// The real work here is done by the DMaterialStepper class
	
//}

//---------------------------------
// GetCDCStereoWires
//---------------------------------
/// Extract the stereo wire data from the XML
bool DGeometry::GetCDCStereoWires(unsigned int ring,unsigned int ncopy,
				  double zcenter,double dz,  
				  vector<vector<cdc_offset_t> >&cdc_offsets,
				  vector<DCDCWire*> &stereowires,
				  vector<double>&rot_angles,double dx,
				  double dy) const{
  stringstream r_z_s,phi0_s,rot_s;
  
  // Create search strings for the straw geometrical properties 
  r_z_s << "//mposPhi[@volume='CDCstrawLong']/@R_Z/ring[@value='" << ring << "']";
  phi0_s << "//mposPhi[@volume='CDCstrawLong']/@Phi0/ring[@value='" << ring << "']";
  rot_s << "//mposPhi[@volume='CDCstrawLong']/@rot/ring[@value='" << ring << "']";

  vector<double>r_z;
  double phi0;
  vector<double>rot;
 
  // Extract data from the XML
  if(!Get(r_z_s.str(), r_z)) return false; 
  if(!Get(phi0_s.str(), phi0)) return false; 
  if(!Get(rot_s.str(), rot)) return false; 

  // Angular quantities
  const double deg2rad=M_PI/180.;
  double dphi=2*M_PI/double(ncopy);
  phi0*=deg2rad;

  // Extract data from close-packed straws
  double stereo=0.,stereo_sign=1.;
  stereo=deg2rad*rot[0];
  if (stereo<0.) stereo_sign=-1.;

  // Loop over the number of straws
  for (unsigned int i=0;i<ncopy;i++){
     DCDCWire *w=new DCDCWire;
     double phi=phi0+double(i)*dphi;
     w->ring=ring;
     w->straw=i+1;

     // Find the nominal wire position and direction from the XML
     DVector3 origin,udir;
     origin.SetX(r_z[0]*cos(phi)+dx);
     origin.SetY(r_z[0]*sin(phi)+dy);    
     origin.SetZ(zcenter);

     // Here, we need to define a coordinate system for the wire
     // in which the wire runs along one axis. We call the directions
     // of the axes in this coordinate system s,t, and u with
     // the wire running in the "u" direction. The "s" direction
     // will be defined by the direction pointing from the beamline
     // to the midpoint of the wire.
     udir.SetXYZ(0.0, 0.0,1.0);	
     udir.RotateX(stereo);	
     udir.RotateZ(phi);     

     // Apply offsets in x and y
     double half_dz=0.5*dz;
     double x0=origin.x(),y0=origin.y();
     double ux=udir.x()/udir.z();
     double uy=udir.y()/udir.z();
     unsigned int ringid=ring-1;
     DVector3 downstream(x0+half_dz*ux+cdc_offsets[ringid][i].dx_d,
           y0+half_dz*uy+cdc_offsets[ringid][i].dy_d,
           zcenter+half_dz);
     DVector3 upstream(x0-half_dz*ux+cdc_offsets[ringid][i].dx_u,
           y0-half_dz*uy+cdc_offsets[ringid][i].dy_u,
           zcenter-half_dz);
     w->origin=0.5*(upstream+downstream);
     w->origin.RotateX(rot_angles[0]);
     w->origin.RotateY(rot_angles[1]);
     w->origin.RotateZ(rot_angles[2]);

     w->phi=w->origin.Phi();

     // Wire direction
     w->udir=downstream-upstream;
     w->udir.RotateX(rot_angles[0]);
     w->udir.RotateY(rot_angles[1]);
     w->udir.RotateZ(rot_angles[2]);

     // For derivatives
     w->udir_mag=w->udir.Mag();

     w->udir.SetMag(1.);
     w->stereo=stereo_sign*w->udir.Angle(DVector3(0.,0.,1.));
     // other directions for our wire coordinate system
     w->sdir=w->origin;
     w->sdir.SetMag(1.);
     w->tdir = w->udir.Cross(w->sdir);

     // Some values needed for alignment derivatives
     w->x0=dx; w->y0=dy; w->z0=zcenter;
     w->phiX=rot_angles[0]; w->phiY=rot_angles[1]; w->phiZ=rot_angles[2];
     w->r0=r_z[0]; w->phiStraw=phi; w->stereo_raw=stereo;

     stereowires.push_back(w);
  }

  return true;
}

//---------------------------------
// GetCDCAxialWires
//---------------------------------
/// Extract the axial wire data from the XML
bool DGeometry::GetCDCAxialWires(unsigned int ring,unsigned int ncopy,
      double zcenter,double dz,
      vector<vector<cdc_offset_t> >&cdc_offsets,
      vector<DCDCWire*> &axialwires,
      vector<double>&rot_angles,double dx,
      double dy) const{
   stringstream phi0_s,r_z_s;

   // Create search strings for the number of straws and the straw geometrical properties 
   phi0_s << "//mposPhi[@volume='CDCstrawShort']/@Phi0/ring[@value='" << ring << "']";
   r_z_s << "//mposPhi[@volume='CDCstrawShort']/@R_Z/ring[@value='" << ring << "']";

   double phi0;
   vector<double>r_z;

   // Extract the data from the XML
   if(!Get(phi0_s.str(), phi0)) return false; 
   if(!Get(r_z_s.str(), r_z)) return false;

   // Angular quantities
   double dphi=2*M_PI/double(ncopy);
   phi0*=M_PI/180.;

   // Loop over the number of straws
   for (unsigned int i=0;i<ncopy;i++){
      DCDCWire *w=new DCDCWire;
      double phi=phi0+double(i)*dphi;
      w->ring=ring;
      w->straw=i+1;

      // Find the nominal wire position from the XML
      double x0=r_z[0]*cos(phi)+dx;
      double y0=r_z[0]*sin(phi)+dy;

      // Apply offsets in x and y
      double half_dz=0.5*dz;
      unsigned int ringid=ring-1;
      DVector3 downstream(x0+cdc_offsets[ringid][i].dx_d,
            y0+cdc_offsets[ringid][i].dy_d,
            zcenter+half_dz);
      DVector3 upstream(x0+cdc_offsets[ringid][i].dx_u,
            y0+cdc_offsets[ringid][i].dy_u,
            zcenter-half_dz);
      w->origin=0.5*(upstream+downstream);
      w->origin.RotateX(rot_angles[0]);
      w->origin.RotateY(rot_angles[1]);
      w->origin.RotateZ(rot_angles[2]);
      w->phi=w->origin.Phi();

      // Here, we need to define a coordinate system for the wire
      // in which the wire runs along one axis. We call the directions
      // of the axes in this coordinate system s,t, and u with
      // the wire running in the "u" direction. The "s" direction
      // will be defined by the direction pointing from the beamline
      // to the midpoint of the wire.
      w->udir=downstream-upstream;
      w->udir.RotateX(rot_angles[0]);
      w->udir.RotateY(rot_angles[1]);
      w->udir.RotateZ(rot_angles[2]);

      // For derivatives
      w->udir_mag=w->udir.Mag();

      w->udir.SetMag(1.);

      w->stereo=w->udir.Angle(DVector3(0.,0.,1.));
      // other directions for our wire coordinate system
      w->sdir=w->origin;
      w->sdir.SetMag(1.);
      w->tdir = w->udir.Cross(w->sdir);

      // Some values needed for alignment derivatives
      w->x0=dx; w->y0=dy; w->z0=zcenter;
      w->phiX=rot_angles[0]; w->phiY=rot_angles[1]; w->phiZ=rot_angles[2];
      w->r0=r_z[0]; w->phiStraw=phi; w->stereo_raw=0.0;

      axialwires.push_back(w);
   }

   return true;
}

//---------------------------------
// GetCDCWires
//---------------------------------
bool DGeometry::GetCDCWires(vector<vector<DCDCWire *> >&cdcwires) const{
   // Get nominal geometry from XML
   vector<double>cdc_origin;
   vector<double>cdc_length;
   Get("//posXYZ[@volume='CentralDC']/@X_Y_Z",cdc_origin);
   Get("//tubs[@name='STRW']/@Rio_Z",cdc_length);

   // Get CDC rotation angles
   vector<double>rot_angles;
   Get("//posXYZ[@volume='CentralDC']/@rot", rot_angles);
   rot_angles[0]*=M_PI/180.;
   rot_angles[1]*=M_PI/180.;
   rot_angles[2]*=M_PI/180.; 

   double dX=0.0, dY=0.0, dZ=0.0;
   double dPhiX=0.0,dPhiY=0.0,dPhiZ=0.0;

   JCalibration * jcalib = jcalman->GetJCalibration(runnumber);
   vector<map<string,double> >vals;
   if (jcalib->Get("CDC/global_alignment",vals)==false){
      map<string,double> &row = vals[0];
      dX=row["dX"];
      dY=row["dY"];
      dZ=row["dZ"];
      dPhiX=row["dPhiX"];
      dPhiY=row["dPhiY"];
      dPhiZ=row["dPhiZ"];
   }


   // Shift the CDC origin according to alignment
   cdc_origin[0]+=dX;
   cdc_origin[1]+=dY;
   cdc_origin[2]+=dZ;

   // Shift the CDC rotation according to the alignment
   rot_angles[0]+=dPhiX;
   rot_angles[1]+=dPhiY;
   rot_angles[2]+=dPhiZ;

   double zmin=cdc_origin[2];
   double zmax=zmin+cdc_length[2];
   double zcenter=0.5*(zmin+zmax);
   double L=zmax-zmin;

   // Get number of straws for each layer from the XML
   unsigned int numstraws[28];  
   stringstream ncopy_s;

   // Get number of straws for each ring
   for (unsigned int ring=1;ring<=28;ring++){
      // Create search string for the number of straws 
      ncopy_s << "//section[@name='CentralDC']/composition/mposPhi/@ncopy/ring[@value='" << ring << "']";
      Get(ncopy_s.str(),numstraws[ring-1]);
      ncopy_s.str("");
      ncopy_s.clear();
   }

   // Get straw-by-straw offsets from calibration database
   vector<cdc_offset_t>tempvec;
   vector<vector<cdc_offset_t> >cdc_offsets;

   if (jcalib->Get("CDC/wire_alignment",vals)==false){
      unsigned int straw_count=0,ring_count=0;
      for(unsigned int i=0; i<vals.size(); i++){
         map<string,double> &row = vals[i];

         // put the vector of offsets for the current ring into the offsets vector
         if (straw_count==numstraws[ring_count]){
            straw_count=0;
            ring_count++;

            cdc_offsets.push_back(tempvec);

            tempvec.clear();
         }

         // Get the offsets from the calibration database 
         cdc_offset_t temp;
         temp.dx_u=row["dxu"];
         //temp.dx_u=0.;

         temp.dy_u=row["dyu"];
         //temp.dy_u=0.;

         temp.dx_d=row["dxd"];
         //temp.dx_d=0.;

         temp.dy_d=row["dyd"];
         //temp.dy_d=0.;

         tempvec.push_back(temp);

         straw_count++;
      }
      cdc_offsets.push_back(tempvec);
   }
   else{
      jerr<< "CDC wire alignment table not available... bailing... " <<jendl;
      exit(0);
   }

   // First axial layer
   for (unsigned int ring=1;ring<5;ring++){ 
      vector<DCDCWire*>straws;
      if (!GetCDCAxialWires(ring,numstraws[ring-1],zcenter,L,cdc_offsets,straws,
               rot_angles,cdc_origin[0],cdc_origin[1])) return false;    
      cdcwires.push_back(straws);
   }  

   // First set of stereo layers
   for (unsigned int i=0;i<8;i++){
      vector<DCDCWire*>straws;
      if (!GetCDCStereoWires(i+5,numstraws[i+4],zcenter,L,cdc_offsets,straws,
               rot_angles,cdc_origin[0],cdc_origin[1])) return false;
      cdcwires.push_back(straws);
   }

   // Second axial layer
   for (unsigned int ring=13;ring<17;ring++){ 
      vector<DCDCWire*>straws;
      if (!GetCDCAxialWires(ring,numstraws[ring-1],zcenter,L,cdc_offsets,straws,
               rot_angles,cdc_origin[0],cdc_origin[1])) return false;    
      cdcwires.push_back(straws);
   }

   // Second set of stereo layers
   for (unsigned int i=8;i<16;i++){
      vector<DCDCWire*>straws;
      if (!GetCDCStereoWires(i+9,numstraws[i+8],zcenter,L,cdc_offsets,straws,
               rot_angles,cdc_origin[0],cdc_origin[1])) return false;
      cdcwires.push_back(straws);
   }

   // Third axial layer
   for (unsigned int ring=25;ring<29;ring++){ 
      vector<DCDCWire*>straws;
      if (!GetCDCAxialWires(ring,numstraws[ring-1],zcenter,L,cdc_offsets,straws,
               rot_angles,cdc_origin[0],cdc_origin[1])) return false;    
      cdcwires.push_back(straws);
   }

   // Calculate wire lengths and compute "s" and "t" direction vectors (orthogonal to "u")
   for (unsigned int i=0;i<cdcwires.size();i++){ 
      for (unsigned int j=0;j<cdcwires[i].size();j++){
         DCDCWire *w=cdcwires[i][j];
         w->L=L/cos(w->stereo);

         // With the addition of close-packed stereo wires, the vector connecting
         // the center of the wire to the beamline ("s" direction) is not necessarily
         // perpendicular to the beamline. By definition, we want the "s" direction
         // to be perpendicular to the wire direction "u" and pointing at the beamline.
         // 
         // NOTE: This extensive comment is here because the result, when implmented
         // below caused a WORSE residual distribution in the close-packed stereo
         // layers. Owing to lack of time currently to track the issue down (most
         // likely in DReferenceTrajectory) I'm commenting out the "correct" calculation
         // of s, but leaving this comment so the issue can be revisited later. This
         // error leads to around 100 micron errors in the C.P.S. wires, but they
         // are completely washed out when the position smearing of 150 microns is applied
         // making the error unnoticable except when position smearing is not applied.
         //
         // April 2, 2009  D.L.
         //
         // Here is how this is calculated -- We define a vector equation with 2 unknowns
         // Z and S:
         //
         //    Zz + Ss = W
         //
         // where:  z = unit vector in z direction
         //         s = unit vector in "s" direction
         //         W = vector pointing to center of wire in lab coordinates
         //
         //  Rearranging, we get:
         //
         //     s = (W - Zz)/S
         //
         //  Since s must be perpendicular to u, we take a dot product of s and u
         // and set it equal to zero to determine Z:
         //
         //    u.s = 0 = u.(W - Zz)/S  =>  u.W = Zu.z
         //
         //   or
         //
         //     Z = u.W/u.z
         //
         //  Thus, the s direction is just given by (W - (u.W/u.z)z)
         //

         //w->sdir=w->origin-DVector3(0,0,w->origin.Z());
         w->sdir = w->origin - DVector3(0.0, 0.0, w->udir.Dot(w->origin)/w->udir.Z());  // see above comments
         w->sdir.SetMag(1.0);

         w->tdir = w->udir.Cross(w->sdir);
         w->tdir.SetMag(1.0); // This isn't really needed
      }
   }  

   return true;
}


//---------------------------------
// GetFDCCathodes
//---------------------------------
bool DGeometry::GetFDCCathodes(vector<vector<DFDCCathode *> >&fdccathodes) const{
   // Get offsets tweaking nominal geometry from calibration database
   JCalibration * jcalib = jcalman->GetJCalibration(runnumber);
   vector<map<string,double> >vals;
   vector<fdc_cathode_offset_t>fdc_cathode_offsets;
   if (jcalib->Get("FDC/cathode_alignment",vals)==false){
      for(unsigned int i=0; i<vals.size(); i++){
         map<string,double> &row = vals[i];

         // Get the offsets from the calibration database 
         fdc_cathode_offset_t temp;
         temp.du=row["dU"];
         //temp.du=0.;

         temp.dphi=row["dPhiU"];
         //temp.dphi=0.;

         fdc_cathode_offsets.push_back(temp);

         temp.du=row["dV"];
         //temp.du=0.;

         temp.dphi=row["dPhiV"];
         //temp.dphi=0.;

         fdc_cathode_offsets.push_back(temp);
      }
   }
   vector< vector<double> >fdc_cathode_pitches;
   if (jcalib->Get("FDC/strip_pitches_v2",vals)==false){
      for(unsigned int i=0; i<vals.size(); i++){
         map<string,double> &row = vals[i];

         vector<double> uvals;
         // Get the offsets from the calibration database 
         uvals.push_back(row["U_SP_1"]);
         uvals.push_back(row["U_G_1"]);
         uvals.push_back(row["U_SP_2"]);
         uvals.push_back(row["U_G_2"]);
         uvals.push_back(row["U_SP_3"]);

         fdc_cathode_pitches.push_back(uvals);

         vector<double> vvals;
         // Get the offsets from the calibration database
         vvals.push_back(row["V_SP_1"]);
         vvals.push_back(row["V_G_1"]);
         vvals.push_back(row["V_SP_2"]);
         vvals.push_back(row["V_G_2"]);
         vvals.push_back(row["V_SP_3"]);

         fdc_cathode_pitches.push_back(vvals);
      }
   }
   else{
      jerr << "Strip pitch calibration unavailable -- setting default..." <<jendl;
      // set some sensible default
      for (unsigned int i=0;i<2*FDC_NUM_LAYERS;i++){

         vector<double> val;
         for (int j = 0; j < 5; j++){
            val.push_back(STRIP_SPACING);
         }

         fdc_cathode_pitches.push_back(val);
      }
   }

   // Generate the vector of cathode plane parameters
   for (int i=0;i<2*FDC_NUM_LAYERS; i++){
      double angle=(i%2)?(M_PI-CATHODE_ROT_ANGLE):(CATHODE_ROT_ANGLE);

      angle+=fdc_cathode_offsets[i].dphi;
      double SP1 = fdc_cathode_pitches[i][0];
      double SG1 = fdc_cathode_pitches[i][1];
      double SP2 = fdc_cathode_pitches[i][2];
      double SG2 = fdc_cathode_pitches[i][3];
      double SP3 = fdc_cathode_pitches[i][4];

      vector<DFDCCathode *>temp;
      for (int j=0; j<STRIPS_PER_PLANE; j++){
         DFDCCathode *c = new DFDCCathode;
         c->layer = i+1;
         c->strip = j+1;
         c->angle = angle;
         if (j<48) c->u=(-47.5*SP2 - SG1 + (j-47)*SP1) + fdc_cathode_offsets[i].du;
         else if (j<144) c->u=(double(j)-95.5)*SP2 + fdc_cathode_offsets[i].du;
         else c->u=(47.5*SP2 + SG2 + (j-144)*SP3) + fdc_cathode_offsets[i].du;

         temp.push_back(c);
      }
      fdccathodes.push_back(temp);
   }

   return true;
}

//---------------------------------
// GetFDCWires
//---------------------------------
bool DGeometry::GetFDCWires(vector<vector<DFDCWire *> >&fdcwires) const{
   // Get geometrical information from database
   vector<double>z_wires;
   vector<double>stereo_angles;

   if(!GetFDCZ(z_wires)) return false;
   if(!GetFDCStereo(stereo_angles)) return false;

   // Get package rotation angles
   double ThetaX[4],ThetaY[4],ThetaZ[4];
   vector<double>rot_angles;
   Get("//posXYZ[@volume='forwardDC_package_1']/@rot", rot_angles);
   ThetaX[0]=rot_angles[0]*M_PI/180.;
   ThetaY[0]=rot_angles[1]*M_PI/180.;
   ThetaZ[0]=rot_angles[2]*M_PI/180.; 
   Get("//posXYZ[@volume='forwardDC_package_2']/@rot", rot_angles);
   ThetaX[1]=rot_angles[0]*M_PI/180.;
   ThetaY[1]=rot_angles[1]*M_PI/180.;
   ThetaZ[1]=rot_angles[2]*M_PI/180.;  
   Get("//posXYZ[@volume='forwardDC_package_3']/@rot", rot_angles);
   ThetaX[2]=rot_angles[0]*M_PI/180.;
   ThetaY[2]=rot_angles[1]*M_PI/180.;
   ThetaZ[2]=rot_angles[2]*M_PI/180.; 
   Get("//posXYZ[@volume='forwardDC_package_4']/@rot", rot_angles);
   ThetaX[3]=rot_angles[0]*M_PI/180.;
   ThetaY[3]=rot_angles[1]*M_PI/180.;
   ThetaZ[3]=rot_angles[2]*M_PI/180.;
   // Get package offsets
   double dX[4],dY[4];
   vector<double>offsets;
   Get("//posXYZ[@volume='forwardDC_package_1']/@X_Y_Z",offsets);
   dX[0]=offsets[0];
   dY[0]=offsets[1];
   Get("//posXYZ[@volume='forwardDC_package_2']/@X_Y_Z",offsets);
   dX[1]=offsets[0];
   dY[1]=offsets[1]; 
   Get("//posXYZ[@volume='forwardDC_package_3']/@X_Y_Z",offsets);
   dX[2]=offsets[0];
   dY[2]=offsets[1];
   Get("//posXYZ[@volume='forwardDC_package_4']/@X_Y_Z",offsets);
   dX[3]=offsets[0];
   dY[3]=offsets[1];

   // Get offsets tweaking nominal geometry from calibration database
   JCalibration * jcalib = jcalman->GetJCalibration(runnumber);
   vector<map<string,double> >vals;
   vector<fdc_wire_offset_t>fdc_wire_offsets;
   if (jcalib->Get("FDC/wire_alignment",vals)==false){
      for(unsigned int i=0; i<vals.size(); i++){
         map<string,double> &row = vals[i];

         // Get the offsets from the calibration database 
         fdc_wire_offset_t temp;
         temp.du=row["dU"];
         //temp.du=0.;

         temp.dphi=row["dPhi"];
         //temp.dphi=0.;

         temp.dz=row["dZ"];
         //  temp.dz=0.;

         fdc_wire_offsets.push_back(temp);
      }
   }

   vector<fdc_wire_rotation_t>fdc_wire_rotations;
   if (jcalib->Get("FDC/cell_rotations",vals)==false){
      for(unsigned int i=0; i<vals.size(); i++){
         map<string,double> &row = vals[i];

         // Get the offsets from the calibration database
         fdc_wire_rotation_t temp;
         temp.dPhiX=row["dPhiX"];
         temp.dPhiY=row["dPhiY"];
         temp.dPhiZ=row["dPhiZ"];

         fdc_wire_rotations.push_back(temp);
      }
   }

   // Generate the vector of wire plane parameters
   for(int i=0; i<FDC_NUM_LAYERS; i++){
      double angle=-stereo_angles[i]*M_PI/180.+fdc_wire_offsets[i].dphi;

      vector<DFDCWire *>temp;
      for(int j=0; j<WIRES_PER_PLANE; j++){
         unsigned int pack_id=i/6;

         DFDCWire *w = new DFDCWire;
         w->layer = i+1;
         w->wire = j+1;
         w->angle = angle;

         // find coordinates of center of wire in rotated system
         float u = U_OF_WIRE_ZERO + WIRE_SPACING*(float)(j);
         w->u=u+fdc_wire_offsets[i].du;

         // Rotate coordinates into lab system and set the wire's origin
         // Note that the FDC measures "angle" such that angle=0
         // corresponds to the anode wire in the vertical direction
         // (i.e. at phi=90 degrees).
         float x = u*sin(angle + M_PI/2.0);
         float y = u*cos(angle + M_PI/2.0);
         w->origin.SetXYZ(x,y,0.);
         w->origin.RotateX(ThetaX[pack_id]+fdc_wire_rotations[i].dPhiX);
         w->origin.RotateY(ThetaY[pack_id]+fdc_wire_rotations[i].dPhiY);
         w->origin.RotateZ(ThetaZ[pack_id]+fdc_wire_rotations[i].dPhiZ);
         DVector3 globalOffsets(dX[pack_id],dY[pack_id],z_wires[i]+fdc_wire_offsets[i].dz);
         w->origin+=globalOffsets;

         // Length of wire is set by active radius
         w->L = 2.0*sqrt(pow(FDC_ACTIVE_RADIUS,2.0) - u*u);

         // Set directions of wire's coordinate system with "udir"
         // along wire.
         w->udir.SetXYZ(sin(angle),cos(angle),0.0);
         w->udir.RotateX(ThetaX[pack_id]+fdc_wire_rotations[i].dPhiX);
         w->udir.RotateY(ThetaY[pack_id]+fdc_wire_rotations[i].dPhiY);
         w->udir.RotateZ(ThetaZ[pack_id]+fdc_wire_rotations[i].dPhiZ);
         w->angles.SetXYZ(ThetaX[pack_id]+fdc_wire_rotations[i].dPhiX,
               ThetaY[pack_id]+fdc_wire_rotations[i].dPhiY,
               ThetaZ[pack_id]+fdc_wire_rotations[i].dPhiZ);
         w->u+=dX[pack_id]*w->udir.y()-dY[pack_id]*w->udir.x();

         // "s" points in direction from beamline to midpoint of
         // wire. This happens to be the same direction as "origin"
         w->sdir = w->origin;
         w->sdir.SetMag(1.0);

         w->tdir = w->udir.Cross(w->sdir);
         w->tdir.SetMag(1.0); // This isn't really needed
         temp.push_back(w);
      }
      fdcwires.push_back(temp);
   }

   return true;
}

//---------------------------------
// GetFDCZ
//---------------------------------
bool DGeometry::GetFDCZ(vector<double> &z_wires) const
{
   // The FDC geometry is defined as 4 packages, each containing 2
   // "module"s and each of those containing 3 "chambers". The modules
   // are placed as multiple copies in Z using mposZ, but none of the
   // others are (???).
   //
   // This method is currently hardwired to assume 4 packages and
   // 3 chambers. (The number of modules is discovered via the
   // "ncopy" attribute of mposZ.)

   vector<double> ForwardDC;
   vector<double> forwardDC;
   vector<double> forwardDC_package[4];
   vector<double> forwardDC_module[4];
   vector<double> forwardDC_chamber[4][6];

   if(!Get("//section/composition/posXYZ[@volume='ForwardDC']/@X_Y_Z", ForwardDC)) return false;
   if(!Get("//composition[@name='ForwardDC']/posXYZ[@volume='forwardDC']/@X_Y_Z", forwardDC)) return false;
   if(!Get("//posXYZ[@volume='forwardDC_package_1']/@X_Y_Z", forwardDC_package[0])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_package_2']/@X_Y_Z", forwardDC_package[1])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_package_3']/@X_Y_Z", forwardDC_package[2])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_package_4']/@X_Y_Z", forwardDC_package[3])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_module_1']/@X_Y_Z", forwardDC_module[0])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_module_2']/@X_Y_Z", forwardDC_module[1])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_module_3']/@X_Y_Z", forwardDC_module[2])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_module_4']/@X_Y_Z", forwardDC_module[3])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_1']/@X_Y_Z/layer[@value='1']", forwardDC_chamber[0][0])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_1']/@X_Y_Z/layer[@value='2']", forwardDC_chamber[0][1])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_1']/@X_Y_Z/layer[@value='3']", forwardDC_chamber[0][2])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_1']/@X_Y_Z/layer[@value='4']", forwardDC_chamber[0][3])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_1']/@X_Y_Z/layer[@value='5']", forwardDC_chamber[0][4])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_1']/@X_Y_Z/layer[@value='6']", forwardDC_chamber[0][5])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_2']/@X_Y_Z/layer[@value='1']", forwardDC_chamber[1][0])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_2']/@X_Y_Z/layer[@value='2']", forwardDC_chamber[1][1])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_2']/@X_Y_Z/layer[@value='3']", forwardDC_chamber[1][2])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_2']/@X_Y_Z/layer[@value='4']", forwardDC_chamber[1][3])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_2']/@X_Y_Z/layer[@value='5']", forwardDC_chamber[1][4])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_2']/@X_Y_Z/layer[@value='6']", forwardDC_chamber[1][5])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_3']/@X_Y_Z/layer[@value='1']", forwardDC_chamber[2][0])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_3']/@X_Y_Z/layer[@value='2']", forwardDC_chamber[2][1])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_3']/@X_Y_Z/layer[@value='3']", forwardDC_chamber[2][2])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_3']/@X_Y_Z/layer[@value='4']", forwardDC_chamber[2][3])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_3']/@X_Y_Z/layer[@value='5']", forwardDC_chamber[2][4])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_3']/@X_Y_Z/layer[@value='6']", forwardDC_chamber[2][5])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_4']/@X_Y_Z/layer[@value='1']", forwardDC_chamber[3][0])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_4']/@X_Y_Z/layer[@value='2']", forwardDC_chamber[3][1])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_4']/@X_Y_Z/layer[@value='3']", forwardDC_chamber[3][2])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_4']/@X_Y_Z/layer[@value='4']", forwardDC_chamber[3][3])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_4']/@X_Y_Z/layer[@value='5']", forwardDC_chamber[3][4])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_4']/@X_Y_Z/layer[@value='6']", forwardDC_chamber[3][5])) return false;

   // Offset due to global FDC envelopes
   double zfdc = ForwardDC[2] + forwardDC[2];

   // Loop over packages
   for(int package=1; package<=4; package++){
      double z_package = forwardDC_package[package-1][2];

      // Each "package" has 1 "module" which is currently positioned at 0,0,0 but
      // that could in principle change so we add the module z-offset
      double z_module = forwardDC_module[package-1][2];

      // Loop over chambers in this module
      for(int chamber=1; chamber<=6; chamber++){
         double z_chamber = forwardDC_chamber[package-1][chamber-1][2];

         double z = zfdc + z_package + z_module + z_chamber;				
         z_wires.push_back(z);
      }
   }

   return true;
}

//---------------------------------
// GetFDCStereo
//---------------------------------
bool DGeometry::GetFDCStereo(vector<double> &stereo_angles) const
{
   // The FDC geometry is defined as 4 packages, each containing 2
   // "module"s and each of those containing 3 "chambers". The modules
   // are placed as multiple copies in Z using mposZ, but none of the
   // others are (???).
   //
   // This method is currently hardwired to assume 4 packages and
   // 3 chambers. (The number of modules is discovered via the
   // "ncopy" attribute of mposZ.)
   //
   // Stereo angles are assumed to be rotated purely about the z-axis
   // and the units are not specified, but the XML currently uses degrees.

   vector<double> forwardDC_module[4];
   vector<double> forwardDC_chamber[4][6];

   if(!Get("//posXYZ[@volume='forwardDC_module_1']/@X_Y_Z", forwardDC_module[0])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_module_2']/@X_Y_Z", forwardDC_module[1])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_module_3']/@X_Y_Z", forwardDC_module[2])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_module_4']/@X_Y_Z", forwardDC_module[3])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_1']/@rot/layer[@value='1']", forwardDC_chamber[0][0])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_1']/@rot/layer[@value='2']", forwardDC_chamber[0][1])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_1']/@rot/layer[@value='3']", forwardDC_chamber[0][2])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_1']/@rot/layer[@value='4']", forwardDC_chamber[0][3])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_1']/@rot/layer[@value='5']", forwardDC_chamber[0][4])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_1']/@rot/layer[@value='6']", forwardDC_chamber[0][5])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_2']/@rot/layer[@value='1']", forwardDC_chamber[1][0])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_2']/@rot/layer[@value='2']", forwardDC_chamber[1][1])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_2']/@rot/layer[@value='3']", forwardDC_chamber[1][2])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_2']/@rot/layer[@value='4']", forwardDC_chamber[1][3])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_2']/@rot/layer[@value='5']", forwardDC_chamber[1][4])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_2']/@rot/layer[@value='6']", forwardDC_chamber[1][5])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_3']/@rot/layer[@value='1']", forwardDC_chamber[2][0])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_3']/@rot/layer[@value='2']", forwardDC_chamber[2][1])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_3']/@rot/layer[@value='3']", forwardDC_chamber[2][2])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_3']/@rot/layer[@value='4']", forwardDC_chamber[2][3])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_3']/@rot/layer[@value='5']", forwardDC_chamber[2][4])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_3']/@rot/layer[@value='6']", forwardDC_chamber[2][5])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_4']/@rot/layer[@value='1']", forwardDC_chamber[3][0])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_4']/@rot/layer[@value='2']", forwardDC_chamber[3][1])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_4']/@rot/layer[@value='3']", forwardDC_chamber[3][2])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_4']/@rot/layer[@value='4']", forwardDC_chamber[3][3])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_4']/@rot/layer[@value='5']", forwardDC_chamber[3][4])) return false;
   if(!Get("//posXYZ[@volume='forwardDC_chamber_4']/@rot/layer[@value='6']", forwardDC_chamber[3][5])) return false;

   // Loop over packages
   for(int package=1; package<=4; package++){

      // Loop over chambers
      for(int chamber=1; chamber<=6; chamber++){
         // if (chamber==4) forwardDC_chamber[package-1][chamber-1][2]+=15.0;
         stereo_angles.push_back(forwardDC_chamber[package-1][chamber-1][2]);
      }
   }

   return true;
}

//---------------------------------
// GetFDCRmin
//---------------------------------
bool DGeometry::GetFDCRmin(vector<double> &rmin_packages) const
{
   vector<double> FDA[4];

   if(!Get("//section[@name='ForwardDC']/tubs[@name='FDA1']/@Rio_Z", FDA[0])) return false;
   if(!Get("//section[@name='ForwardDC']/tubs[@name='FDA2']/@Rio_Z", FDA[1])) return false;
   if(!Get("//section[@name='ForwardDC']/tubs[@name='FDA3']/@Rio_Z", FDA[2])) return false;
   if(!Get("//section[@name='ForwardDC']/tubs[@name='FDA4']/@Rio_Z", FDA[3])) return false;

   rmin_packages.push_back(FDA[0][0]);
   rmin_packages.push_back(FDA[1][0]);
   rmin_packages.push_back(FDA[2][0]);
   rmin_packages.push_back(FDA[3][0]);

   return true;
}

//---------------------------------
// GetFDCRmax
//---------------------------------
bool DGeometry::GetFDCRmax(double &rmax_active_fdc) const
{
   // We assume that all packages have the same outer radius of the
   // active area.
   vector<double> FDA1;

   bool good = Get("//section[@name='ForwardDC']/tubs[@name='FDA1']/@Rio_Z", FDA1);

   if(!good){
      _DBG_<<"Unable to retrieve FDC Rmax values."<<endl;
      return good;
   }

   rmax_active_fdc = FDA1[1];

   return good;
}

//---------------------------------
// GetCDCOption
//---------------------------------
bool DGeometry::GetCDCOption(string &cdc_option) const
{
   bool good = Get("//section[@name='CentralDC_s']/composition/posXYZ/@volume", cdc_option);

   if(!good){
      _DBG_<<"Unable to retrieve CDC option string."<<endl;
   }

   return good;
}

//---------------------------------
// GetCDCCenterZ
//---------------------------------
bool DGeometry::GetCDCCenterZ(double &cdc_center_z) const
{

   return false;
}

//---------------------------------
// GetCDCAxialLength
//---------------------------------
bool DGeometry::GetCDCAxialLength(double &cdc_axial_length) const
{
   vector<double> Rio_Z;
   bool good = Get("//section[@name='CentralDC']/tubs[@name='STRW']/@Rio_Z", Rio_Z);
   cdc_axial_length = Rio_Z[2];

   if(!good){
      _DBG_<<"Unable to retrieve CDC axial wire length"<<endl;
   }

   return false;
}

//---------------------------------
// GetCDCStereo
//---------------------------------
bool DGeometry::GetCDCStereo(vector<double> &cdc_stereo) const
{

   return false;
}

//---------------------------------
// GetCDCRmid
//---------------------------------
bool DGeometry::GetCDCRmid(vector<double> &cdc_rmid) const
{

   return false;
}

//---------------------------------
// GetCDCNwires
//---------------------------------
bool DGeometry::GetCDCNwires(vector<int> &cdc_nwires) const
{
	cdc_nwires.push_back(42);
	cdc_nwires.push_back(42);
	cdc_nwires.push_back(54);
	cdc_nwires.push_back(54);
	cdc_nwires.push_back(66);
	cdc_nwires.push_back(66);
	cdc_nwires.push_back(80);
	cdc_nwires.push_back(80);
	cdc_nwires.push_back(93);
	cdc_nwires.push_back(93);
	cdc_nwires.push_back(106);
	cdc_nwires.push_back(106);
	cdc_nwires.push_back(123);
	cdc_nwires.push_back(123);
	cdc_nwires.push_back(135);
	cdc_nwires.push_back(135);
	cdc_nwires.push_back(146);
	cdc_nwires.push_back(146);
	cdc_nwires.push_back(158);
	cdc_nwires.push_back(158);
	cdc_nwires.push_back(170);
	cdc_nwires.push_back(170);
	cdc_nwires.push_back(182);
	cdc_nwires.push_back(182);
	cdc_nwires.push_back(197);
	cdc_nwires.push_back(197);
	cdc_nwires.push_back(209);
	cdc_nwires.push_back(209);
	
   return false;
}


//---------------------------------
// GetCDCEndplate
//---------------------------------
bool DGeometry::GetCDCEndplate(double &z,double &dz,double &rmin,double &rmax)
   const{

      vector<double>cdc_origin;
      vector<double>cdc_center;
      vector<double>cdc_layers_offset;
      vector<double>cdc_endplate_pos;
      vector<double>cdc_endplate_dim;

      if(!Get("//posXYZ[@volume='CentralDC'/@X_Y_Z",cdc_origin)) return false;
      if(!Get("//posXYZ[@volume='centralDC']/@X_Y_Z",cdc_center)) return false;
      if(!Get("//posXYZ[@volume='CDPD']/@X_Y_Z",cdc_endplate_pos)) return false;
      if(!Get("//tubs[@name='CDPD']/@Rio_Z",cdc_endplate_dim)) return false;
      if(!Get("//posXYZ[@volume='CDClayers']/@X_Y_Z",cdc_layers_offset)) return false;

      if(cdc_origin.size()<3){
         _DBG_<<"cdc_origin.size()<3 !"<<endl;
         return false;
      }
      if(cdc_center.size()<3){
         _DBG_<<"cdc_center.size()<3 !"<<endl;
         return false;
      }
      if(cdc_endplate_pos.size()<3){
         _DBG_<<"cdc_endplate_pos.size()<3 !"<<endl;
         return false;
      }
      if(cdc_endplate_dim.size()<3){
         _DBG_<<"cdc_endplate_dim.size()<3 !"<<endl;
         return false;
      }
      if (cdc_layers_offset.size()<3){
         _DBG_<<"cdc_layers_offset.size()<3 !"<<endl;
         return false;
      }

      z=cdc_origin[2]+cdc_center[2]+cdc_endplate_pos[2]+cdc_layers_offset[2];
      dz=cdc_endplate_dim[2];
      rmin=cdc_endplate_dim[0];
      rmax=cdc_endplate_dim[1];

      return true;
   }
//---------------------------------
// GetBCALRmin
//---------------------------------
// Including the support plate
bool DGeometry::GetBCALRmin(float &bcal_rmin) const
{
   vector<float> bcal_mother_Rio_Z;
   bool good = Get("//section[@name='BarrelEMcal']/tubs[@name='BCAL']/@Rio_Z", bcal_mother_Rio_Z);
   if(!good){
      _DBG_<<"Unable to retrieve BCAL mother RioZ info."<<endl;
      bcal_rmin = 0.0;
      return false;
   }
   if(bcal_mother_Rio_Z.size() == 3){
      bcal_rmin = bcal_mother_Rio_Z[0];
      return true;
   }
   else{
      _DBG_<<"Wrong vector size for BCAL mother RioZ!!!"<<endl;
      bcal_rmin = 0.0;
      return false;
   }
}

//---------------------------------
// GetBCALfADCRadii
//---------------------------------
bool DGeometry::GetBCALfADCRadii(vector<float> &fADC_radii) const
{
   vector<float> BM[5];

   if(!Get("//section[@name='BarrelEMcal']/tubs[@name='BM01']/@Rio_Z", BM[0])) return false;
   if(!Get("//section[@name='BarrelEMcal']/tubs[@name='BM02']/@Rio_Z", BM[1])) return false;
   if(!Get("//section[@name='BarrelEMcal']/tubs[@name='BM04']/@Rio_Z", BM[2])) return false;
   if(!Get("//section[@name='BarrelEMcal']/tubs[@name='BMF7']/@Rio_Z", BM[3])) return false;
   if(!Get("//section[@name='BarrelEMcal']/tubs[@name='BMFA']/@Rio_Z", BM[4])) return false;

   fADC_radii.push_back(BM[0][0]);
   fADC_radii.push_back(BM[1][0]);
   fADC_radii.push_back(BM[2][0]);
   fADC_radii.push_back(BM[3][0]);
   fADC_radii.push_back(BM[4][1]);

   return true;
}

//---------------------------------
// GetBCALNmodules
//---------------------------------
bool DGeometry::GetBCALNmodules(unsigned int &bcal_nmodules) const
{
   vector<unsigned int> ncopy;
   bool good = Get("//section[@name='BarrelEMcal']/composition/mposPhi/@ncopy", ncopy);
   if(!good){
      _DBG_<<"Unable to retrieve BCAL barrelModule ncopy info."<<endl;
      bcal_nmodules = 0;
      return false;
   }
   if(ncopy.size() == 1){
      bcal_nmodules = ncopy[0];
      return true;
   }else{
      _DBG_<<"Wrong vector size for BCAL barrelModule ncopy!!!"<<endl;
      bcal_nmodules = 0;
      return false;
   }
}

//---------------------------------
// GetBCALCenterZ
//---------------------------------
bool DGeometry::GetBCALCenterZ(float &bcal_center_z) const
{
   vector<float> z0;
   bool good = Get("//section[@name='BarrelEMcal']/parameters/real[@name='z0']/@value", z0);
   if(!good){
      _DBG_<<"Unable to retrieve BCAL parameters z0 info."<<endl;
      bcal_center_z = 0.0;
      return false;
   }
   if(z0.size() == 1){
      bcal_center_z = z0[0];
      return true;
   }else{
      _DBG_<<"Wrong vector size for BCAL parameters z0!!!"<<endl;
      bcal_center_z = 0.0;
      return false;
   }

}

//---------------------------------
// GetBCALLength
//---------------------------------
// The lightguides are not included
bool DGeometry::GetBCALLength(float &bcal_length) const
{
   vector<float> module_length;
   bool good = Get("//section[@name='BarrelEMcal']/tubs[@name='BM01']/@Rio_Z", module_length);
   if(!good){
      _DBG_<<"Unable to retrieve BCAL submodule RioZ info."<<endl;
      bcal_length = 0.0;
      return false;
   }
   if(module_length.size() == 3){
      bcal_length = module_length[2];
      return true;
   }
   else{
      _DBG_<<"Wrong vector size for BCAL submodule RioZ!!!"<<endl;
      bcal_length = 0.0;
      return false;
   }
}

//---------------------------------
// GetBCALDepth
//---------------------------------
// Including the support plate and the support bar
bool DGeometry::GetBCALDepth(float &bcal_depth) const
{
   vector<float> bcal_moth_Rio_Z;
   bool good = Get("//section[@name='BarrelEMcal']/tubs[@name='BCAL']/@Rio_Z", bcal_moth_Rio_Z);
   if(!good){
      _DBG_<<"Unable to retrieve BCAL mother RioZ info."<<endl;
      bcal_depth = 0.0;
      return false;
   }
   if(bcal_moth_Rio_Z.size() == 3){
      bcal_depth = bcal_moth_Rio_Z[1] - bcal_moth_Rio_Z[0];
      return true;
   }
   else{
      _DBG_<<"Wrong vector size for BCAL mother RioZ!!!"<<endl;
      bcal_depth = 0.0;
      return false;
   }

}

//---------------------------------
// GetBCALPhiShift
//---------------------------------
bool DGeometry::GetBCALPhiShift(float &bcal_phi_shift) const
{
   vector<float> Phi0;
   bool good = Get("//section[@name='BarrelEMcal']/composition/mposPhi/@Phi0", Phi0);
   if(!good) return false;
   if(Phi0.size() == 1){
      bcal_phi_shift = Phi0[0];
      return true;
   }else{
      bcal_phi_shift = 0.0;
      return false;
   }
}


//---------------------------------
// GetECALZ
//---------------------------------
bool DGeometry::GetECALZ(double &z_ecal) const
{
  z_ecal=0;
  if (GetFCALZ(z_ecal)){    
    jgeom->SetVerbose(0);   // don't print error messages for optional detector elements
    vector<double>CrystalEcalpos;
    if (Get("//section/composition/posXYZ[@volume='CrystalECAL']/@X_Y_Z", CrystalEcalpos)){
      vector<double> FCALCenter;
      Get("//section/composition/posXYZ[@volume='forwardEMcal]/@X_Y_Z", FCALCenter);
      vector<double>block;
      Get("//box[@name='XTBL']/@X_Y_Z",block);

      jgeom->SetVerbose(1);   // reenable error messages
     
      z_ecal += FCALCenter[2]+CrystalEcalpos[2]-0.5*block[2];
      return true;
    }
  }
  return false;
}

//---------------------------------
// GetCCALZ
//---------------------------------
bool DGeometry::GetCCALZ(double &z_ccal) const
{
   vector<double> ComptonEMcalpos;
   jgeom->SetVerbose(0);   // don't print error messages for optional detector elements
   bool good = Get("//section/composition/posXYZ[@volume='ComptonEMcal']/@X_Y_Z", ComptonEMcalpos);
   jgeom->SetVerbose(1);   // reenable error messages

   if(!good){
	  // NEED TO RETHINK ERROR REPORTING FOR OPTIONAL DETECTOR ELEMENTS
      //_DBG_<<"Unable to retrieve ComptonEMcal position."<<endl;  
      z_ccal = 1279.376;   
      return false;
   }else{
	   z_ccal = ComptonEMcalpos[2];
      return true;
   }
}



//---------------------------------
// GetCTOFZ
//---------------------------------
bool DGeometry::GetCTOFZ(double &z) const {
  z=1000; // cm; initialize to a large value
  vector<double> CppScintPos;
  jgeom->SetVerbose(0);   // don't print error messages for optional detector elements
  bool good = Get("//section/composition/posXYZ[@volume='CppScint']/@X_Y_Z", CppScintPos);
  jgeom->SetVerbose(1);   // reenable error messages
  if (!good){  
    //_DBG_<<"Unable to retrieve CPP scintillator position."<<endl;
    return false;
  }
  z=CppScintPos[2];
  return true;
}

//---------------------------------
// GetCTOFPositions
//---------------------------------
bool DGeometry::GetCTOFPositions(vector<DVector3>&posvec) const{
  vector<double>origin;
  jgeom->SetVerbose(0);   // don't print error messages for optional detector elements
  bool good = Get("//section/composition/posXYZ[@volume='CppScint']/@X_Y_Z",origin);
  jgeom->SetVerbose(1);   // reenable error messages
  if (!good) return false;
  DVector3 pos(origin[0],origin[1],origin[2]);
  for (unsigned int paddle=1;paddle<5;paddle++){
    vector<double>local_pos;
    Get(Form("//posXYZ[@volume='CPPPaddle']/@X_Y_Z/column[@value='%d']",paddle),local_pos);
    DVector3 dpos(local_pos[0],local_pos[1],local_pos[2]);
    posvec.push_back(pos+dpos);
  }

  return true;
}

//---------------------------------
// GetFMWPCZ
//---------------------------------
bool DGeometry::GetFMWPCZ_vec(vector<double>&zvec_fmwpc) const
{
  vector<double> ForwardMWPCpos;
  jgeom->SetVerbose(0);   // don't print error messages for optional detector elements
  bool good = Get("//section/composition/posXYZ[@volume='ForwardMWPC']/@X_Y_Z", ForwardMWPCpos);
  jgeom->SetVerbose(1);   // reenable error messages
  if (!good){
    //_DBG_<<"Unable to retrieve ForwardMWPC position."<<endl;
    return false;
  }

  vector<double>CPPChamberPos;
  Get("//posXYZ[@volume='CPPChamber']/@X_Y_Z/layer[@value='1']", CPPChamberPos);
  zvec_fmwpc.push_back(ForwardMWPCpos[2]+CPPChamberPos[2]);
  Get("//posXYZ[@volume='CPPChamber']/@X_Y_Z/layer[@value='2']", CPPChamberPos);
  zvec_fmwpc.push_back(ForwardMWPCpos[2]+CPPChamberPos[2]);
  Get("//posXYZ[@volume='CPPChamber']/@X_Y_Z/layer[@value='3']", CPPChamberPos);
  zvec_fmwpc.push_back(ForwardMWPCpos[2]+CPPChamberPos[2]);
  Get("//posXYZ[@volume='CPPChamber']/@X_Y_Z/layer[@value='4']", CPPChamberPos);
  zvec_fmwpc.push_back(ForwardMWPCpos[2]+CPPChamberPos[2]);
  Get("//posXYZ[@volume='CPPChamber']/@X_Y_Z/layer[@value='5']", CPPChamberPos);
  zvec_fmwpc.push_back(ForwardMWPCpos[2]+CPPChamberPos[2]);
  Get("//posXYZ[@volume='CPPChamber']/@X_Y_Z/layer[@value='6']", CPPChamberPos);
  zvec_fmwpc.push_back(ForwardMWPCpos[2]+CPPChamberPos[2]);

  return true;
}

//---------------------------------
// GetFMWPCXY
//---------------------------------
bool DGeometry::GetFMWPCXY_vec(vector<double>&xvec_fmwpc, vector<double>&yvec_fmwpc) const
{
  vector<double> ForwardMWPCpos;
  jgeom->SetVerbose(0);   // don't print error messages for optional detector elements
  bool good = Get("//section/composition/posXYZ[@volume='ForwardMWPC']/@X_Y_Z", ForwardMWPCpos);
  jgeom->SetVerbose(1);   // reenable error messages
  if (!good){
    //_DBG_<<"Unable to retrieve ForwardMWPC position."<<endl;
    return false;
  }

  // Get offsets tweaking nominal geometry from calibration database
  // JCalibration * jcalib = dapp->GetJCalibration(runnumber);

  vector<double>CPPChamberPos;
  Get("//posXYZ[@volume='CPPChamber']/@X_Y_Z/layer[@value='1']", CPPChamberPos);
  xvec_fmwpc.push_back(ForwardMWPCpos[0]+CPPChamberPos[0]);
  yvec_fmwpc.push_back(ForwardMWPCpos[1]+CPPChamberPos[1]);
  Get("//posXYZ[@volume='CPPChamber']/@X_Y_Z/layer[@value='2']", CPPChamberPos);
  xvec_fmwpc.push_back(ForwardMWPCpos[0]+CPPChamberPos[0]);
  yvec_fmwpc.push_back(ForwardMWPCpos[1]+CPPChamberPos[1]);
  Get("//posXYZ[@volume='CPPChamber']/@X_Y_Z/layer[@value='3']", CPPChamberPos);
  xvec_fmwpc.push_back(ForwardMWPCpos[0]+CPPChamberPos[0]);
  yvec_fmwpc.push_back(ForwardMWPCpos[1]+CPPChamberPos[1]);
  Get("//posXYZ[@volume='CPPChamber']/@X_Y_Z/layer[@value='4']", CPPChamberPos);
  xvec_fmwpc.push_back(ForwardMWPCpos[0]+CPPChamberPos[0]);
  yvec_fmwpc.push_back(ForwardMWPCpos[1]+CPPChamberPos[1]);
  Get("//posXYZ[@volume='CPPChamber']/@X_Y_Z/layer[@value='5']", CPPChamberPos);
  xvec_fmwpc.push_back(ForwardMWPCpos[0]+CPPChamberPos[0]);
  yvec_fmwpc.push_back(ForwardMWPCpos[1]+CPPChamberPos[1]);
  Get("//posXYZ[@volume='CPPChamber']/@X_Y_Z/layer[@value='6']", CPPChamberPos);
  xvec_fmwpc.push_back(ForwardMWPCpos[0]+CPPChamberPos[0]);
  yvec_fmwpc.push_back(ForwardMWPCpos[1]+CPPChamberPos[1]);

  // Currently, not all chambers have a 'rot' field in hdds
  // vector<double>CPPChamberRot;
  // Get("//posXYZ[@volume='CPPChamber']/@rot/layer[@value='1']", CPPChamberRot);
  // rot_fmwpc.push_back(CPPChamberRot[2]);
  // Get("//posXYZ[@volume='CPPChamber']/@rot/layer[@value='2']", CPPChamberRot);
  // rot_fmwpc.push_back(CPPChamberRot[2]);
  // Get("//posXYZ[@volume='CPPChamber']/@rot/layer[@value='3']", CPPChamberRot);
  // rot_fmwpc.push_back(CPPChamberRot[2]);
  // Get("//posXYZ[@volume='CPPChamber']/@rot/layer[@value='4']", CPPChamberRot);
  // rot_fmwpc.push_back(CPPChamberRot[2]);
  // Get("//posXYZ[@volume='CPPChamber']/@rot/layer[@value='5']", CPPChamberRot);
  // rot_fmwpc.push_back(CPPChamberRot[2]);
  // Get("//posXYZ[@volume='CPPChamber']/@rot/layer[@value='6']", CPPChamberRot);
  // rot_fmwpc.push_back(CPPChamberRot[2]);

  return true;
}

//---------------------------------
// GetFMWPCSize -- use the dimensions of the frame
//---------------------------------
bool DGeometry::GetFMWPCSize(double &xy_fmwpc) const
{
  vector<double> ForwardMWPCdimensions;
  jgeom->SetVerbose(0);   // don't print error messages for optional detector elements
  bool good = Get("//section[@name='ForwardMWPC']/box[@name='CPPF']/@X_Y_Z", ForwardMWPCdimensions);
  jgeom->SetVerbose(1);   // reenable error messages
  if (!good){  
    xy_fmwpc=0.0;
    return false;
  }
  xy_fmwpc=0.5*ForwardMWPCdimensions[0];

  return true;
}

//---------------------------------
// GetFMWPCWireSpacing -- space between wires in cm
//---------------------------------
bool DGeometry::GetFMWPCWireSpacing(double &fmwpc_wire_spacing) const
{
    fmwpc_wire_spacing = 1.016;

    return true;
}

//---------------------------------
// GetFMWPCWireSpacing -- space between wires in cm
//---------------------------------
bool DGeometry::GetFMWPCWireOrientation(vector<fmwpc_wire_orientation_t> &fmwpc_wire_orientation) const
{
    fmwpc_wire_orientation.clear();
    fmwpc_wire_orientation.push_back( kFMWPC_WIRE_ORIENTATION_VERTICAL );
    fmwpc_wire_orientation.push_back( kFMWPC_WIRE_ORIENTATION_HORIZONTAL );
    fmwpc_wire_orientation.push_back( kFMWPC_WIRE_ORIENTATION_VERTICAL );
    fmwpc_wire_orientation.push_back( kFMWPC_WIRE_ORIENTATION_HORIZONTAL );
    fmwpc_wire_orientation.push_back( kFMWPC_WIRE_ORIENTATION_VERTICAL );
    fmwpc_wire_orientation.push_back( kFMWPC_WIRE_ORIENTATION_HORIZONTAL );

    return true;
}

//---------------------------------
// GetFCALZ
//---------------------------------
bool DGeometry::GetFCALZ(double &z_fcal) const
{
   vector<double> ForwardEMcalpos;
   bool good = Get("//section/composition/posXYZ[@volume='ForwardEMcal']/@X_Y_Z", ForwardEMcalpos);

   if(!good){
      _DBG_<<"Unable to retrieve ForwardEMcal position."<<endl;
      z_fcal=0.0;
      return false;
   }else{
      z_fcal = ForwardEMcalpos[2];
      return true;
   }
}


//---------------------------------
// GetFCALPosition
//---------------------------------
bool DGeometry::GetFCALPosition(double &x,double &y,double &z) const
{
  vector<double> ForwardEMcalpos;
  bool good = Get("//section/composition/posXYZ[@volume='ForwardEMcal']/@X_Y_Z", ForwardEMcalpos);

  if(!good){
    _DBG_<<"Unable to retrieve ForwardEMcal position."<<endl;
    x=0.,y=0.,z=0.;
    return false;
  }else{
    x=ForwardEMcalpos[0],y=ForwardEMcalpos[1],z=ForwardEMcalpos[2];
    //_DBG_ << "FCAL position: (x,y,z)=(" << x <<"," << y << "," << z << ")"<<endl;
    return true;
  }
}

//---------------------------------
// GetECALPosition
//---------------------------------
bool DGeometry::GetECALPosition(double &x,double &y,double &z) const
{
  vector<double> CrystalEcalpos;
  jgeom->SetVerbose(0);   // don't print error messages for optional detector elements
  bool good = Get("//section/composition/posXYZ[@volume='CrystalECAL']/@X_Y_Z", CrystalEcalpos);
  jgeom->SetVerbose(1);   // reenable error messages
 
  if(!good){
    //_DBG_<<"Unable to retrieve ECAL position."<<endl;
    x=0.,y=0.,z=0.;
    return false;
  }else{
    x=CrystalEcalpos[0],y=CrystalEcalpos[1],z=CrystalEcalpos[2]; 
    //_DBG_ << "ECAL position: (x,y,z)=(" << CrystalECALpos[0] <<","
	// << CrystalECALpos[1]<<","<<CrystalECALpos[2]<< ")" << endl;
    return true;
  }
}


//---------------------------------
// GetCCALPosition
//---------------------------------
bool DGeometry::GetCCALPosition(double &x,double &y,double &z) const
{
  vector<double> ComptonEMcalpos;
  jgeom->SetVerbose(0);   // don't print error messages for optional detector elements
  bool good = Get("//section/composition/posXYZ[@volume='ComptonEMcal']/@X_Y_Z", ComptonEMcalpos);
  jgeom->SetVerbose(1);   // reenable error messages
 
  if(!good){
    //_DBG_<<"Unable to retrieve ComptonEMcal position."<<endl;
    x=0.,y=0.,z=0.;
    return false;
  }else{
    x=ComptonEMcalpos[0],y=ComptonEMcalpos[1],z=ComptonEMcalpos[2]; 
    //_DBG_ << "CCAL position: (x,y,z)=(" << ComptonEMcalpos[0] <<","
	// << ComptonEMcalpos[1]<<","<<ComptonEMcalpos[2]<< ")" << endl;
    return true;
  }
}

// Check for presence of FCAL2 insert
bool DGeometry::HaveInsert() const{
  int ncopy=0;
  jgeom->SetVerbose(0);
  bool have_insert
    =Get("//composition[@name='XTrow0']/mposX[@volume='XTModule']/@ncopy",
	 ncopy);
  jgeom->SetVerbose(1);
  return have_insert;
}

//---------------------------------
// GetFCALInsertRowSize
//---------------------------------
bool DGeometry::GetFCALInsertRowSize(int &insert_row_size) const
{
   jgeom->SetVerbose(0);   // don't print error messages for optional detector elements

   bool good = Get("//composition[@name='XTrow0']/mposX[@volume='XTModule']/@ncopy",insert_row_size);
   // For backward compatibility with prototype geometry definition:
   if (!good){
     good = Get("//composition[@name='LeadTungstateFullRow']/mposX[@volume='LTBLwrapped']/@ncopy",insert_row_size);
   }

   jgeom->SetVerbose(1);   // reenable error messages

   if(!good){
      insert_row_size = 0;   
      return false;
   }else{
      return true;
   }
}

//---------------------------------
// GetFCALInsertSize
//---------------------------------
double DGeometry::GetFCALInsertSize() const{
  int insertRowSize=0;
  if (GetFCALInsertRowSize(insertRowSize)){
    vector<double>insertBlock;
    // Use nominal crystal separation to be on the safe side.  This is slightly
    // larger than the actual measured separations from the survey.
    return 0.5*double(insertRowSize)*2.09;
  }

  return 0.;
}

//---------------------------------
// GetFCALBlockSize
//---------------------------------
bool DGeometry::GetFCALBlockSize(vector<double> &block) const
{
   jgeom->SetVerbose(0);   // don't print error messages for optional detector elements
   bool good = Get("//box[@name='LGBU']/@X_Y_Z",block);
   jgeom->SetVerbose(1);   // reenable error messages

   if(!good){
	  // NEED TO RETHINK ERROR REPORTING FOR OPTIONAL DETECTOR ELEMENTS
      //_DBG_<<"Unable to retrieve ComptonEMcal position."<<endl;  
      return false;
   }else{
      return true;
   }
}

//---------------------------------
// GetFCALInsertBlockSize
//---------------------------------
bool DGeometry::GetFCALInsertBlockSize(vector<double> &block) const
{
   jgeom->SetVerbose(0);   // don't print error messages for optional detector elements
   bool good = Get("//box[@name='XTMD']/@X_Y_Z",block);
   // for backward compatiblity
   if (!good) good=Get("//box[@name='LTB1']/@X_Y_Z",block);
   jgeom->SetVerbose(1);   // reenable error messages

   if(!good){
      return false;
   }else{
      return true;
   }
}

//---------------------------------
// GetDIRCZ
//---------------------------------
bool DGeometry::GetDIRCZ(double &z_dirc) const
{
  vector<double> dirc_face;
  jgeom->SetVerbose(0);   // don't print error messages for optional detector elements
  bool good = Get("//section/composition/posXYZ[@volume='DIRC']/@X_Y_Z",dirc_face);
  jgeom->SetVerbose(1);   // reenable error messages

  if(!good){
    //_DBG_<<"Unable to retrieve DIRC position."<<endl;
    z_dirc=0.0;
    return false;
  }
  else{ 
    vector<double>dirc_plane;
    vector<double>dirc_shift;
    vector<double>bar_plane;
    Get("//composition[@name='DRCC']/posXYZ[@volume='DCML10']/@X_Y_Z/plane[@value='1']", dirc_plane);
    Get("//composition[@name='DIRC']/posXYZ[@volume='DRCC']/@X_Y_Z", dirc_shift);
    z_dirc=dirc_face[2]+dirc_plane[2]+dirc_shift[2] + 0.8625; // last shift is the average center of quartz bar (585.862)

    //jout << "DIRC z position = " << z_dirc << " cm." << endl;
    return true;
  }
}

//---------------------------------
// GetTRDZ
//---------------------------------
bool DGeometry::GetTRDZ(vector<double> &z_trd) const
{
   vector<double> trd_origin;
   jgeom->SetVerbose(0);   // don't print error messages for optional detector elements
   bool good = Get("//section/composition/posXYZ[@volume='TRDGEM']/@X_Y_Z",trd_origin);
   jgeom->SetVerbose(1);   // reenable error messages

   if(!good){
     //_DBG_<<"Unable to retrieve TRD position."<<endl;
     return false;
   }
   else{ 
     vector<double>trd_G10;
     Get("//composition[@name='TRDGEM']/posXYZ[@volume='TGPK']/@X_Y_Z",trd_G10);

     jout << "TRD z positions = ";
     for(int i=0; i<5; i++) {
       vector<double>trd_plane;
       Get(Form("//composition[@name='TGPK']/posXYZ[@volume='TRDG']/@X_Y_Z/plane[@value='%d']",i),trd_plane);
       double z_trd_plane=trd_origin[2]+trd_G10[2]+trd_plane[2];
       jout << z_trd_plane << ", ";
       z_trd.push_back(z_trd_plane);
     }
     jout << "cm" << jendl;
   }

   return true;
}
//---------------------------------
// GetTOFZ
//---------------------------------
bool DGeometry::GetTOFZ(double &CenterVPlane,double &CenterHPlane,
			double &CenterMPlane) const{
  // Store the z position for both planes
  vector<double>tof_face;
  if (!Get("//section/composition/posXYZ[@volume='ForwardTOF']/@X_Y_Z",tof_face)){
    return false;
  }
  vector<double>tof_plane0;
  Get("//composition[@name='ForwardTOF']/posXYZ[@volume='forwardTOF']/@X_Y_Z/plane[@value='0']", tof_plane0);
  vector<double>tof_plane1;
  Get("//composition[@name='ForwardTOF']/posXYZ[@volume='forwardTOF']/@X_Y_Z/plane[@value='1']", tof_plane1);
  CenterVPlane=tof_face[2]+tof_plane1[2];
  CenterHPlane=tof_face[2]+tof_plane0[2];
  // also save position midway between the two planes
  CenterMPlane=0.5*(CenterHPlane+CenterVPlane);

  return true;
}

//---------------------------------
// GetTOFZ
//---------------------------------
bool DGeometry::GetTOFZ(vector<double> &z_tof) const
{
   vector<double> ForwardTOF;
   vector<double> forwardTOF[2];
   vector<double> FTOC;

   if(!Get("//section/composition/posXYZ[@volume='ForwardTOF']/@X_Y_Z", ForwardTOF)) return false;
   if(!Get("//composition[@name='ForwardTOF']/posXYZ[@volume='forwardTOF']/@X_Y_Z/plane[@value='0']", forwardTOF[0])) return false;
   if(!Get("//composition[@name='ForwardTOF']/posXYZ[@volume='forwardTOF']/@X_Y_Z/plane[@value='1']", forwardTOF[1])) return false;
   if(!Get("//box[@name='FTOC' and sensitive='true']/@X_Y_Z", FTOC)) return false;

   z_tof.push_back(ForwardTOF[2] + forwardTOF[0][2] - FTOC[2]/2.0);
   z_tof.push_back(ForwardTOF[2] + forwardTOF[1][2] - FTOC[2]/2.0);

	//cerr << "DGeometry::GetTOFZ() = " << z_tof[0] << " " << z_tof[1] << endl;

   return true;
}

//---------------------------------
// GetTOFPaddleParameters
//---------------------------------
bool DGeometry::GetTOFPaddleParameters(map<string,double> &paddle_params) const
{
	vector<double> xyz_bar;

	// load the number of bars in each area
    int num_bars1 = 0;
    if(!Get("//composition[@name='forwardTOF_bottom1']/mposY[@volume='FTOC']/@ncopy",num_bars1)) return false; 
    int num_narrow_bars1 = 0;
    if(!Get("//composition[@name='forwardTOF_bottom2']/mposY[@volume='FTOX']/@ncopy",num_narrow_bars1)) return false; 
    int num_single_end_bars1 = 0;
    if(!Get("//composition[@name='forwardTOF_north']/mposY[@volume='FTOH']/@ncopy",num_single_end_bars1)) return false;

    jgeom->SetVerbose(0);   // don't print error messages for optional detector elements
    int num_narrower_bars1 = 0;   // optional - added during upgrade
    Get("//composition[@name='forwardTOF_bottom3']/mposY[@volume='FTOL']/@ncopy",num_narrower_bars1); 
    int num_narrower_bars2 = 0;   // optional  - added during upgrade
    Get("//composition[@name='forwardTOF_top3']/mposY[@volume='FTOL']/@ncopy",num_narrower_bars2); 
    jgeom->SetVerbose(1);   // reenable error messages

    int num_narrow_bars2 = 0;
    if(!Get("//composition[@name='forwardTOF_top2']/mposY[@volume='FTOX']/@ncopy",num_narrow_bars2)) return false;
    int num_bars2 = 0;
    if(!Get("//composition[@name='forwardTOF_top1']/mposY[@volume='FTOC']/@ncopy",num_bars2)) return false;
    int num_single_end_bars2 = 0;
    if(!Get("//composition[@name='forwardTOF_south']/mposY[@volume='FTOH']/@ncopy",num_single_end_bars2)) return false;

	int NLONGBARS = num_bars1 + num_bars2 + num_narrow_bars1 + num_narrow_bars2
						 + num_narrower_bars1 + num_narrower_bars2;
	int NSHORTBARS = num_single_end_bars1 + num_single_end_bars2;
	int FIRSTSHORTBAR = num_bars1 + num_narrow_bars1 + num_narrower_bars1 + 1;
	int LASTSHORTBAR = FIRSTSHORTBAR + NSHORTBARS/2 - 1;

	// load bar sizes
	//Get("//composition[@name='forwardTOF_bottom1']/mposY[@volume='FTOC']/@X_Y_Z",xyz_bar);
    if(!Get("//box[@name='FTOC' and sensitive='true']/@X_Y_Z", xyz_bar)) return false;
	double LONGBARLENGTH = xyz_bar[0];
	double BARWIDTH  = xyz_bar[1];
	//Get("//composition[@name='forwardTOF_bottom1']/mposY[@volume='FTOH']/@X_Y_Z",xyz_bar);
    if(!Get("//box[@name='FTOH' and sensitive='true']/@X_Y_Z", xyz_bar)) return false;
	double SHORTBARLENGTH = xyz_bar[0];

	
	// load up the structure containing the parameters for the calling function
	paddle_params["NLONGBARS"] = NLONGBARS;
	paddle_params["NSHORTBARS"] = NSHORTBARS;
	paddle_params["BARWIDTH"] = BARWIDTH;

	paddle_params["LONGBARLENGTH"] = LONGBARLENGTH;
	paddle_params["HALFLONGBARLENGTH"] = LONGBARLENGTH/2.;
	paddle_params["SHORTBARLENGTH"] = SHORTBARLENGTH;
	paddle_params["HALFSHORTBARLENGTH"] = SHORTBARLENGTH/2.;

	paddle_params["FIRSTSHORTBAR"] = FIRSTSHORTBAR;
	paddle_params["LASTSHORTBAR"] = LASTSHORTBAR;

	//cout << "In DGeometry::GetTOFPaddleParameters() ..." << endl;
	//for(auto el : paddle_params) {
   	//	std::cout << el.first << " " << el.second << endl;
	//}	
	
	return true;
}


//---------------------------------
// GetTOFPaddlePerpPositions
//---------------------------------
bool DGeometry::GetTOFPaddlePerpPositions(vector<double> &y_tof, vector<double> &y_widths) const
{
	// add in a dummy entry, since we are indexing by paddle number, which starts at 1
	// maybe change this some day?
	y_tof.push_back(0);
	y_widths.push_back(0);
	
  	// Next fill array of bar positions within a plane
  	// y_tof[barnumber] gives y position in the center of the bar. [currently barnumber = 1 - 46]
  	double y0,dy;

	// load the number of bars
    int num_bars=1;   // start counting at 1
    int num_bars1 = 0;
    Get("//composition[@name='forwardTOF_bottom1']/mposY[@volume='FTOC']/@ncopy",num_bars1); 
    int num_narrow_bars1 = 0;
    Get("//composition[@name='forwardTOF_bottom2']/mposY[@volume='FTOX']/@ncopy",num_narrow_bars1); 
    int num_single_end_bars1 = 0;
    Get("//composition[@name='forwardTOF_north']/mposY[@volume='FTOH']/@ncopy",num_single_end_bars1); 

    jgeom->SetVerbose(0);   // don't print error messages for optional detector elements
    int num_narrower_bars1 = 0;
    Get("//composition[@name='forwardTOF_bottom3']/mposY[@volume='FTOL']/@ncopy",num_narrower_bars1); 
    int num_narrower_bars2 = 0;
    Get("//composition[@name='forwardTOF_top3']/mposY[@volume='FTOL']/@ncopy",num_narrower_bars2); 
    jgeom->SetVerbose(1);   // reenable error messages

    int num_narrow_bars2 = 0;
    Get("//composition[@name='forwardTOF_top2']/mposY[@volume='FTOX']/@ncopy",num_narrow_bars2); 
    int num_bars2 = 0;
    Get("//composition[@name='forwardTOF_top1']/mposY[@volume='FTOC']/@ncopy",num_bars2); 
    int num_single_end_bars2 = 0;
    Get("//composition[@name='forwardTOF_south']/mposY[@volume='FTOH']/@ncopy",num_single_end_bars2); 
	
  	// First 19 long bars
  	Get("//composition[@name='forwardTOF_bottom1']/mposY/@Y0",y0);
  	Get("//composition[@name='forwardTOF_bottom1']/mposY/@dY",dy);
  	vector<double>tof_bottom1;
  	Get("//composition[@name='forwardTOF']/posXYZ[@volume='forwardTOF_bottom1']/@X_Y_Z",tof_bottom1);  
  	for (int k=num_bars;k<num_bars+num_bars1;k++){
    	y_tof.push_back(y0+tof_bottom1[1]+dy*double(k-num_bars));
    	y_widths.push_back(dy);
  	}
  	num_bars+=num_bars1;
  
  	// two narrow long bars
  	Get("//composition[@name='forwardTOF_bottom2']/mposY/@Y0",y0);
  	Get("//composition[@name='forwardTOF_bottom2']/mposY/@dY",dy);
  	vector<double>tof_bottom2;
  	Get("//composition[@name='forwardTOF']/posXYZ[@volume='forwardTOF_bottom2']/@X_Y_Z",tof_bottom2);  
  	for (int k=num_bars;k<num_bars+num_narrow_bars1;k++){
    	y_tof.push_back(y0+tof_bottom2[1]+dy*double(k-num_bars));
    	y_widths.push_back(dy);
  	}
  	num_bars+=num_narrow_bars1;
  	
  	// two narrower long bars - added by upgrade
  	if(num_narrower_bars1 > 0) {
		Get("//composition[@name='forwardTOF_bottom3']/mposY/@Y0",y0);
		Get("//composition[@name='forwardTOF_bottom3']/mposY/@dY",dy);
		vector<double>tof_bottom3;
		Get("//composition[@name='forwardTOF']/posXYZ[@volume='forwardTOF_bottom3']/@X_Y_Z",tof_bottom3);  
		for (int k=num_bars;k<num_bars+num_narrower_bars1;k++){
			y_tof.push_back(y0+tof_bottom3[1]+dy*double(k-num_bars));
 	   		y_widths.push_back(dy);
		}
		num_bars+=num_narrow_bars1;
  	}

  	// two short wide bars  (4 wide in upgrade)
  	Get("//composition[@name='forwardTOF_north']/mposY/@Y0",y0);
  	Get("//composition[@name='forwardTOF_north']/mposY/@dY",dy);
  	vector<double>tof_north;
  	Get("//composition[@name='forwardTOF']/posXYZ[@volume='forwardTOF_north']/@X_Y_Z",tof_north);  
  	for (int k=num_bars;k<num_bars+num_single_end_bars1;k++){
    	y_tof.push_back(y0+tof_north[1]+dy*double(k-num_bars));
    	y_widths.push_back(dy);
  	}
  	num_bars+=num_single_end_bars1;

  	// two narrower long bars - added by upgrade
  	if(num_narrower_bars2 > 0) {
		Get("//composition[@name='forwardTOF_top3']/mposY/@Y0",y0);
		Get("//composition[@name='forwardTOF_top3']/mposY/@dY",dy);
		vector<double>tof_top3;
		Get("//composition[@name='forwardTOF']/posXYZ[@volume='forwardTOF_top3']/@X_Y_Z",tof_top3);  
		for (int k=num_bars;k<num_bars+num_narrower_bars2;k++){
			y_tof.push_back(y0+tof_top3[1]+dy*double(k-num_bars));
    		y_widths.push_back(dy);
		}
		num_bars+=num_narrow_bars2;
  	}

  	// two narrow long bars
  	Get("//composition[@name='forwardTOF_top2']/mposY/@Y0",y0);
  	Get("//composition[@name='forwardTOF_top2']/mposY/@dY",dy);
  	vector<double>tof_top2;
  	Get("//composition[@name='forwardTOF']/posXYZ[@volume='forwardTOF_top2']/@X_Y_Z",tof_top2);  
  	for (int k=num_bars;k<num_bars+num_narrow_bars2;k++){
    	y_tof.push_back(y0+tof_top2[1]+dy*double(k-num_bars));
    	y_widths.push_back(dy);
  	}
  	num_bars+=num_narrow_bars2;

  	// Last 19 long bars
  	Get("//composition[@name='forwardTOF_top1']/mposY/@Y0",y0);
  	Get("//composition[@name='forwardTOF_top1']/mposY/@dY",dy);
  	vector<double>tof_top1;
  	Get("//composition[@name='forwardTOF']/posXYZ[@volume='forwardTOF_top1']/@X_Y_Z",tof_top1);  
  	for (int k=num_bars;k<num_bars+num_bars2;k++){
   	 	y_tof.push_back(y0+tof_top1[1]+dy*double(k-num_bars));
    	y_widths.push_back(dy);
  	}
  	num_bars+=num_bars2;

	/*
  	// two more short wide bars - IGNORE FOR NOW, ASSUME SAME Y AS OTHER SINGLE ENDED
  	Get("//composition[@name='forwardTOF_south']/mposY/@Y0",y0);
  	Get("//composition[@name='forwardTOF_south']/mposY/@dY",dy);
  	vector<double>tof_south;
  	Get("//composition[@name='forwardTOF']/posXYZ[@volume='forwardTOF_south']/@X_Y_Z",tof_south);  
  	for (unsigned int k=45;k<47;k++){
    	y_tof.push_back(y0+tof_south[1]+dy*double(k-45));
  	}
    */
    
	return true;
}

//---------------------------------
// GetTargetZ
//---------------------------------
bool DGeometry::GetTargetZ(double &z_target) const
{
   // Default to nominal center of GlueX target
   z_target=65.;

   jgeom->SetVerbose(0);   // don't print error messages for optional detector elements

   // Check GlueX target is defined
   bool gluex_target_exists = true;
   vector<double> xyz_vessel;
   vector<double> xyz_target;
   vector<double> xyz_detector;
   if(gluex_target_exists) gluex_target_exists = Get("//composition[@name='targetVessel']/posXYZ[@volume='targetTube']/@X_Y_Z", xyz_vessel);
   if(gluex_target_exists) gluex_target_exists = Get("//composition[@name='Target']/posXYZ[@volume='targetVessel']/@X_Y_Z", xyz_target);
   if(gluex_target_exists) gluex_target_exists = Get("//posXYZ[@volume='Target']/@X_Y_Z", xyz_detector);
   if(gluex_target_exists) {
     z_target = xyz_vessel[2] + xyz_target[2] + xyz_detector[2];
     jgeom->SetVerbose(1);   // reenable error messages
     return true;
   }

   // Check if CPP target is defined
   bool cpp_target_exists = true;
   vector<double> xyz_TGT0;
   vector<double> xyz_TARG;
   vector<double> xyz_TargetCPP;
   if(cpp_target_exists) cpp_target_exists = Get("//composition/posXYZ[@volume='TGT0']/@X_Y_Z", xyz_TGT0);
   if(cpp_target_exists) cpp_target_exists = Get("//composition/posXYZ[@volume='TARG']/@X_Y_Z", xyz_TARG);
   if(cpp_target_exists) cpp_target_exists = Get("//composition/posXYZ[@volume='TargetCPP']/@X_Y_Z", xyz_TargetCPP);
   if(cpp_target_exists) {
      z_target = xyz_TGT0[2] + xyz_TARG[2] + xyz_TargetCPP[2];
      jgeom->SetVerbose(1);   // reenable error messages
      return true;
   }
   
   // Check if PrimEx Be target is defined
   bool primex_target_exists = true;
   vector<double> xyz_BETG;
   if(primex_target_exists) primex_target_exists = Get("//composition[@name='targetVessel']/posXYZ[@volume='BETG']/@X_Y_Z", xyz_BETG);
   if(primex_target_exists) primex_target_exists = Get("//composition[@name='Target']/posXYZ[@volume='targetVessel']/@X_Y_Z", xyz_target);
   if(primex_target_exists) primex_target_exists = Get("//posXYZ[@volume='Target']/@X_Y_Z", xyz_detector);
   if(primex_target_exists) {

     z_target = xyz_BETG[2] + xyz_target[2] + xyz_detector[2];

     //cout << " PrimEx Be targer selected. Z target =   = " << z_target << endl;
     
     jgeom->SetVerbose(1);   // reenable error messages
     return true;
   }
   
   // Check if SRC carbon foils are defined.  Return the center of the ladder  
   vector<double>ladder_xyz;
   if (Get("//composition[@name='targetVessel']/posXYZ[@volume='carbonLadder']/@X_Y_Z",ladder_xyz)){
     Get("//composition[@name='Target']/posXYZ[@volume='targetVessel']/@X_Y_Z", xyz_target);
     Get("//posXYZ[@volume='Target']/@X_Y_Z", xyz_detector);
     
     vector<double>foil_pos;
     Get("//composition[@name='carbonLadder']/posXYZ[@volume='carbonTarget1']/@X_Y_Z",foil_pos);
     z_target=0.5*foil_pos[2];
     Get("//composition[@name='carbonLadder']/posXYZ[@volume='carbonTarget8']/@X_Y_Z",foil_pos);
     z_target+=0.5*foil_pos[2];
     z_target+=xyz_target[2]+xyz_detector[2]+ladder_xyz[2];

     jgeom->SetVerbose(1);   // reenable error messages
     return true;
   }

   // Only print warning for one thread whenever run number change
   static pthread_mutex_t empty_target_mutex = PTHREAD_MUTEX_INITIALIZER;
   static set<int> empty_target_runs_announced;

   // keep track of which runs we print out warnings for
   pthread_mutex_lock(&empty_target_mutex);
   bool empty_target_warning = false;
   if(empty_target_runs_announced.find(runnumber) == empty_target_runs_announced.end()){
     empty_target_warning = true;
     empty_target_runs_announced.insert(runnumber);
   }
   pthread_mutex_unlock(&empty_target_mutex);

   if (empty_target_warning)
     jout << " WARNING: Unable to get target location from XML for any of GlueX, PrimEx, or CPP targets. It's likely an empty target run. Using default of " <<
       z_target << " cm" << endl;

   jgeom->SetVerbose(1);   // reenable error messages

   return false;
}

//---------------------------------
// GetTargetLength
//---------------------------------
bool DGeometry::GetTargetLength(double &target_length) const
{
 
  target_length=0.;
  vector<double> zLength;  
  jgeom->SetVerbose(0);   // don't print error messages for optional detector elements
  
  // Regular GlueX target 
  if (Get("//section[@name='Target']/pcon[@name='LIH2']/real[@name='length']/[@value]", zLength)){
    target_length=zLength[0];
    jgeom->SetVerbose(1);   // reenable error messages
    return true;
  }
  
  // Liquid helium target
  if (Get("//section[@name='Target']/pcon[@name='LIHE']/real[@name='length']/[@value]", zLength)){
    target_length=zLength[0];
    jgeom->SetVerbose(1);   // reenable error messages
    return true;
  }
  
  // Beryllium target
  vector<double>rio_z;
  if (Get("//section[@name='Target']/tubs[@name='BETG']/@Rio_Z",rio_z)){
    target_length=rio_z[2];
    jgeom->SetVerbose(1);   // reenable error messages
    return true;
  }
  
  // Carbon foils for SRC; length is distance between first and last foil
  vector<double>foil_pos;
  if (Get("//composition[@name='carbonLadder']/posXYZ[@volume='carbonTarget1']/@X_Y_Z",foil_pos)){
    target_length-=foil_pos[2];
    Get("//composition[@name='carbonLadder']/posXYZ[@volume='carbonTarget8']/@X_Y_Z",foil_pos);
    target_length+=foil_pos[2];
    jgeom->SetVerbose(1);   // reenable error messages
    return true;
  }

  // Lead target for CPP
  if (Get("//section[@name='TargetCPP']/tubs[@name='TGT0']/@Rio_Z",rio_z)){
    target_length=rio_z[2];
    jgeom->SetVerbose(1);   // reenable error messages
    return true;
  }
  
  jgeom->SetVerbose(1);   // reenable error messages
  return false;
}

// Get vectors of positions and norm vectors for start counter from XML
bool DGeometry::GetStartCounterGeom(vector<vector<DVector3> >&pos,
				    vector<vector<DVector3> >&norm
				    ) const
{
				    
  JCalibration *jcalib = jcalman->GetJCalibration(runnumber);

  // Check if Start Counter geometry is present
  vector<double> sc_origin;
  bool got_sc = Get("//posXYZ[@volume='StartCntr']/@X_Y_Z", sc_origin);
  if(got_sc){
    // Offset from beam center
    vector<double>sc_origin_delta;
    Get("//posXYZ[@volume='startCntr']/@X_Y_Z", sc_origin_delta);
    double dx=sc_origin_delta[0];
    double dy=sc_origin_delta[1];

    // z-position at upstream face of scintillators.
    double z0=sc_origin[2];
    
    // Get rotation angles
    vector<double>sc_rot_angles;
    Get("//posXYZ[@volume='startCntr']/@rot", sc_rot_angles);
    double ThetaX=sc_rot_angles[0]*M_PI/180.;
    double ThetaY=sc_rot_angles[1]*M_PI/180.;
    Get("//posXYZ[@volume='StartCntr']/@rot", sc_rot_angles);
    //double ThetaX=sc_rot_angles[0]*M_PI/180.;
    //double ThetaY=sc_rot_angles[1]*M_PI/180.;
    double ThetaZ=sc_rot_angles[2]*M_PI/180.;

	// Get overall alignment shifts from CCDB
    map<string,double> sc_global_offsets;
    if (jcalib->Get("START_COUNTER/global_alignment_parms",sc_global_offsets)==false) {
   		// translations
   		dx += sc_global_offsets["SC_ALIGN_X"];
   		dy += sc_global_offsets["SC_ALIGN_Y"];
   		z0 += sc_global_offsets["SC_ALIGN_Z"];

		// rotations
		ThetaX += sc_global_offsets["SC_ALIGN_ROTX"]*M_PI/180.;
		ThetaY += sc_global_offsets["SC_ALIGN_ROTY"]*M_PI/180.;
		ThetaZ += sc_global_offsets["SC_ALIGN_ROTZ"]*M_PI/180.;
    }

    double num_paddles;
    Get("//mposPhi[@volume='STRC']/@ncopy",num_paddles); 
    double dSCdphi = M_TWO_PI/num_paddles;
    
    vector<vector<double> > sc_rioz;
    GetMultiple("//pgon[@name='STRC']/polyplane/@Rio_Z", sc_rioz);
    
    // Get individual paddle alignment parameters
    vector< map<string,double> > sc_paddle_offsets;
	bool loaded_paddle_offsets = false;
    if (jcalib->Get("START_COUNTER/paddle_alignment_parms",sc_paddle_offsets)==false)
    	loaded_paddle_offsets = true;
    
    // Create vectors of positions and normal vectors for each paddle
    for (unsigned int i=0;i<30;i++){
      double phi=ThetaZ+dSCdphi*(double(i)+0.5);
      double sinphi=sin(phi);
      double cosphi=cos(phi);
      double r=0.5*(sc_rioz[0][0]+sc_rioz[0][1]);
      DVector3 oldray;
      // Rotate by phi and take into account the tilt
      double x=r*cosphi;//+dx;
      double y=r*sinphi;//+dy;
      DVector3 ray(x,y,sc_rioz[0][2]);
      ray.RotateX(ThetaX);
      ray.RotateY(ThetaY);
      
      // Create stl-vectors to store positions and norm vectors
      vector<DVector3>posvec;
      vector<DVector3>dirvec;
      // Loop over radial/z positions describing start counter geometry from xml
      for(unsigned int k = 1; k < sc_rioz.size(); ++k){
	oldray=ray;
	r=0.5*(sc_rioz[k][0]+sc_rioz[k][1]);
	// Point in plane of scintillator
	x=r*cosphi;//+dx;
	y=r*sinphi;//+dy;
	ray.SetXYZ(x,y,sc_rioz[k][2]); 
	ray.RotateX(ThetaX);
	ray.RotateY(ThetaY);  
	// Apply alignment parameters
	if(loaded_paddle_offsets) {
      // allow for a maximum extent of the paddle in z
	  double max_z = sc_paddle_offsets[i]["SC_MAX_Z"];
	  if(ray.Z() > max_z) {
		ray.SetZ(max_z);
	  }
	  // allow for a modification of the bend angle of the paddle (18.5 deg from horizontal)
	  // this should just be a perturbation around this angle, so assume a linear interpolation
	  double delta_theta = sc_paddle_offsets[i]["SC_CURVE_THETA"];   // in degrees, r of curvature = 120 cm
	  ray.SetX(ray.X()+delta_theta*1.65);   // 1 degree ~ 1.65 cm in x
	  ray.SetY(ray.Y()-delta_theta*0.55);   // 1 degree ~ 0.55 cm in y
	}
	// Second point in the plane of the scintillator
	DVector3 ray2(r,10.,sc_rioz[k][2]);
	ray2.RotateZ(phi+0.5*dSCdphi*(1.+1./15.*((i>14)?29-i:i)));	
	ray2.RotateX(ThetaX);
	ray2.RotateY(ThetaY);  
	// Compute normal vector to plane
	DVector3 dir=(ray-oldray).Cross(ray2-oldray);
	dir.SetMag(1.);
	dirvec.push_back(dir);
	posvec.push_back(DVector3(oldray.X()+dx,oldray.Y()+dy,oldray.Z()+z0));
      }
      posvec.push_back(DVector3(ray.X()+dx,ray.Y()+dy,ray.Z()+z0)); //SAVE THE ENDPOINT OF THE LAST PLANE
      pos.push_back(posvec);
      norm.push_back(dirvec);
		  
      posvec.clear();
      dirvec.clear();
    }
    
  }
  return got_sc;
}

