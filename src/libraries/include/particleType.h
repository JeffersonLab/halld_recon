/*
 * particleType.h
*/

#ifndef particleTypeH_INCLUDED
#define particleTypeH_INCLUDED

#include <math.h>
#include <stdio.h>
#include <string.h>

typedef enum {

  /* An extensive list of the GEANT3 particle
   * codes can be found here:
   * http://wwwasdoc.web.cern.ch/wwwasdoc/geant/node72.html
  */

  Unknown        =  0,
  Gamma          =  1,
  Positron       =  2,
  Electron       =  3,
  Neutrino       =  4,
  MuonPlus       =  5,
  MuonMinus      =  6,
  Pi0            =  7,
  PiPlus         =  8,
  PiMinus        =  9,
  KLong          = 10,
  KPlus          = 11,
  KMinus         = 12,
  Neutron        = 13,
  Proton         = 14,
  AntiProton     = 15,
  KShort         = 16,
  Eta            = 17,
  Lambda         = 18,
  SigmaPlus      = 19,
  Sigma0         = 20,
  SigmaMinus     = 21,
  Xi0            = 22,
  XiMinus        = 23,
  OmegaMinus     = 24,
  AntiNeutron    = 25,
  AntiLambda     = 26,
  AntiSigmaMinus = 27,
  AntiSigma0     = 28,
  AntiSigmaPlus  = 29,
  AntiXi0        = 30,
  AntiXiPlus     = 31,
  AntiOmegaPlus  = 32,
  Deuteron       = 45,
  Triton         = 46,
  Helium         = 47,
  Geantino       = 48,
  He3            = 49,

  Li6            = 61,
  Li7            = 62,
  Be7            = 63,
  Be9            = 64,
  B10            = 65,
  B11            = 66,
  C12            = 67,
  N14            = 68,
  O16            = 69,
  F19            = 70,
  Ne20           = 71,
  Na23           = 72,
  Mg24           = 73,
  Al27           = 74,
  Si28           = 75,
  P31            = 76,
  S32            = 77,
  Cl35           = 78,
  Ar36           = 79,
  K39            = 80,
  Ca40           = 81,
  Sc45           = 82,
  Ti48           = 83,
  V51            = 84,
  Cr52           = 85,
  Mn55           = 86,
  Fe56           = 87,
  Co59           = 88,
  Ni58           = 89,
  Cu63           = 90,
  Zn64           = 91,
  Ge74           = 92,
  Se80           = 93,
  Kr84           = 94,
  Sr88           = 95,
  Zr90           = 96,
  Mo98           = 97,
  Pd106          = 98,
  Cd114          = 99,
  Sn120          = 100,
  Xe132          = 101,
  Ba138          = 102,
  Ce140          = 103,
  Sm152          = 104,
  Dy164          = 105,
  Yb174          = 106,
  W184           = 107,
  Pt194          = 108,
  Au197          = 109,
  Hg202          = 110,
  Pb208          = 111,
  U238           = 112,

  /* the constants defined by GEANT end here */
  
  /*
   * Several particle codes are added below which did
   * not overlap with the original GEANT particle list.
   * However, the link above has several other particle
   * codes added which do actually overlap. Because of 
   * this, each of the values below was increased by
   * 100 so that they don't overlap with any of the new
   * codes and they can be used.
  */
  

  /* These are E852-defined constants */
  Rho0           = 44, // was 157
  RhoPlus        = 42, // was 158
  RhoMinus       = 43, // was 159
  omega          = 33,  // was 160
  phiMeson       = 34,  // was 162
  EtaPrime       = 35,  // was 161
  //Rho0           = 157,
  //RhoPlus        = 158,
  //RhoMinus       = 159,
  //omega          = 160,
  //EtaPrime       = 161,
  //phiMeson       = 162,
  a0_980	 = 163,
  f0_980	 = 164,

  /* These are GlueX-defined constants */

  KStar_892_0 = 165,
  KStar_892_Plus = 166,
  KStar_892_Minus = 167,
  AntiKStar_892_0 = 168,

  K1_1400_Plus = 169,
  K1_1400_Minus = 170,

  b1_1235_Plus = 171,
  Sigma_1385_Minus = 172,
  Sigma_1385_0 = 173,
  Sigma_1385_Plus = 174,

  Jpsi = 183,
  Eta_c = 184,
  Chi_c0 = 185,
  Chi_c1 = 186,
  Chi_c2 = 187,
  Psi2s = 188,
  D0 = 189,
  DPlus = 190,
  Dstar0 = 191,
  DstarPlus = 192,
  Lambda_c = 193,
  AntiD0 = 194, 

  /* These are defined in pythia-geant.map in bggen */

  //Rho0          = 80,  // already defined above (44)
  //omega         = 81,  // already defined above (33)
  DeltaPlusPlus = 182


} Particle_t;

static inline Particle_t RemapParticleID(Particle_t p)
{
  // bggen defines these in pythia-geant.dat. However,
  // they are not the same as the definitions used from
  // E-852 in the enum above. Remap them using this routine
  // which is called from several of the routines below.
  //if(p==80) return Rho0;
  //if(p==81) return omega;
  return p;
}

inline static int IsLepton(Particle_t p)
{
	return ((p == Electron) || (p == Positron) || (p == MuonPlus) || (p == MuonMinus) || (p == Neutrino));
}

inline static char* ParticleType(Particle_t p)
{
  p = RemapParticleID(p);

  switch (p) {
  case Unknown:
    return (char*)"Unknown";
  case Gamma:
    return (char*)"Photon";
  case Positron:
    return (char*)"Positron";
  case Electron:
    return (char*)"Electron";
  case Neutrino:
    return (char*)"Neutrino";
  case MuonPlus:
    return (char*)"Muon+";
  case MuonMinus:
    return (char*)"Muon-";
  case Pi0:
    return (char*)"Pi0";
  case PiPlus:
    return (char*)"Pi+";
  case PiMinus:
    return (char*)"Pi-";
  case KLong:
    return (char*)"KLong";
  case KPlus:
    return (char*)"K+";
  case KMinus:
    return (char*)"K-";
  case Neutron:
    return (char*)"Neutron";
  case Proton:
    return (char*)"Proton";
  case AntiProton:
    return (char*)"AntiProton";
  case KShort:
    return (char*)"KShort";
  case Eta:
    return (char*)"Eta";
  case Lambda:
    return (char*)"Lambda";
  case SigmaPlus:
    return (char*)"Sigma+";
  case Sigma0:
    return (char*)"Sigma0";
  case SigmaMinus:
    return (char*)"Sigma-";
  case Xi0:
    return (char*)"Xi0";
  case XiMinus:
    return (char*)"Xi-";
  case OmegaMinus:
    return (char*)"Omega-";
  case AntiNeutron:
    return (char*)"AntiNeutron";
  case AntiLambda:
    return (char*)"AntiLambda";
  case AntiSigmaMinus:
    return (char*)"AntiSigma-";
  case AntiSigma0:
    return (char*)"AntiSigma0";
  case AntiSigmaPlus:
    return (char*)"AntiSigma+";
  case AntiXi0:
    return (char*)"AntiXi0";
  case AntiXiPlus:
    return (char*)"AntiXi+";
  case AntiOmegaPlus:
    return (char*)"AntiOmega+";
  case Geantino:
    return (char*)"Geantino";
  case Rho0:
    return (char*)"Rho0";
  case RhoPlus:
    return (char*)"Rho+";
  case RhoMinus:
    return (char*)"Rho-";
  case omega:
    return (char*)"Omega";
  case EtaPrime:
    return (char*)"EtaPrime";
  case phiMeson:
    return (char*)"Phi";
  case a0_980:
    return (char*)"a0(980)";
  case f0_980:
    return (char*)"f0(980)";
  case KStar_892_0:
    return (char*)"K*(892)0";
  case KStar_892_Plus:
    return (char*)"K*(892)+";
  case KStar_892_Minus:
    return (char*)"K*(892)-";
  case AntiKStar_892_0:
    return (char*)"antiK*(892)0";
  case K1_1400_Plus:
    return (char*)"K1(1400)+";
  case K1_1400_Minus:
    return (char*)"K1(1400)-";
  case b1_1235_Plus:
    return (char*)"b1(1235)+";
  case Sigma_1385_Minus:
    return (char*)"Sigma(1385)-";
  case Sigma_1385_0:
    return (char*)"Sigma(1385)0";
  case Sigma_1385_Plus:
    return (char*)"Sigma(1385)+";
  case Deuteron:
    return (char*)"Deuteron";
  case Triton:
    return (char*)"Triton";
  case Helium:
    return (char*)"Helium";
  case He3:
    return (char*)"Helium-3";
  case Li6:
    return (char*)"Lithium-6";
  case Li7:
    return (char*)"Lithium-7";
  case Be7:
    return (char*)"Beryllium-7";
  case Be9:
    return (char*)"Beryllium-9";
  case B10:
    return (char*)"Boron-10";
  case B11:
    return (char*)"Boron-11";
  case C12:
    return (char*)"Carbon";
  case N14:
    return (char*)"Nitrogen";
  case O16:
    return (char*)"Oxygen";
  case F19:
    return (char*)"Fluorine";
  case Ne20:
    return (char*)"Neon";
  case Na23:
    return (char*)"Sodium";
  case Mg24:
    return (char*)"Magnesium";
  case Al27:
    return (char*)"Aluminum";
  case Si28:
    return (char*)"Silicon";
  case P31:
    return (char*)"Phosphorous";
  case S32:
    return (char*)"Sulphur";
  case Cl35:
    return (char*)"Chlorine";
  case Ar36:
    return (char*)"Argon";
  case K39:
    return (char*)"Potassium";
  case Ca40:
    return (char*)"Calcium";
  case Sc45:
    return (char*)"Scandium";
  case Ti48:
    return (char*)"Titanium";
  case V51:
    return (char*)"Vanadium";
  case Cr52:
    return (char*)"Chromium";
  case Mn55:
    return (char*)"Manganese";
  case Fe56:
    return (char*)"Iron";
  case Co59:
    return (char*)"Cobalt";
  case Ni58:
    return (char*)"Nickel";
  case Cu63:
    return (char*)"Copper";
  case Zn64:
    return (char*)"Zinc";
  case Ge74:
    return (char*)"Germanium";
  case Se80:
    return (char*)"Selenium";
  case Kr84:
    return (char*)"Krypton";
  case Sr88:
    return (char*)"Strontium";
  case Zr90:
    return (char*)"Zirconium";
  case Mo98:
    return (char*)"Molybdenum";
  case Pd106:
    return (char*)"Palladium";
  case Cd114:
    return (char*)"Cadmium";
  case Sn120:
    return (char*)"Tin";
  case Xe132:
    return (char*)"Xenon";
  case Ba138:
    return (char*)"Barium";
  case Ce140:
    return (char*)"Cesium";
  case Sm152:
    return (char*)"Samerium";
  case Dy164:
    return (char*)"Dysprosium";
  case Yb174:
    return (char*)"Ytterbium";
  case W184:
    return (char*)"Tungsten";
  case Pt194:
    return (char*)"Platium";
  case Au197:
    return (char*)"Gold";
  case Hg202:
    return (char*)"Mercury";
  case Pb208:
    return (char*)"Lead";
  case U238:
    return (char*)"Uranium";
  case DeltaPlusPlus:
    return (char*)"Delta++";
  case Jpsi:
    return (char*)"Jpsi";
  case Eta_c:
    return (char*)"EtaC";
  case Chi_c0:
    return (char*)"ChiC0";
  case Chi_c1:
    return (char*)"ChiC1";
  case Chi_c2:
    return (char*)"ChiC2";
  case Psi2s:
    return (char*)"Psi(2S)";
  case D0:
    return (char*)"D0";
  case AntiD0:
    return (char*)"AntiD0";
  case DPlus:
    return (char*)"D+";
  case Dstar0:
    return (char*)"D*0";
  case DstarPlus:
    return (char*)"D*+";
  case Lambda_c:
    return (char*)"LambdaC";
  default:
    return (char*)"Unknown";
  }
}

inline static char* EnumString(Particle_t p)
{
  //returns string that is exact match to enum name. for auto-generating code
  p = RemapParticleID(p);

  switch (p) {
  case Unknown:
    return (char*)"Unknown";
  case Gamma:
    return (char*)"Gamma";
  case Positron:
    return (char*)"Positron";
  case Electron:
    return (char*)"Electron";
  case Neutrino:
    return (char*)"Neutrino";
  case MuonPlus:
    return (char*)"MuonPlus";
  case MuonMinus:
    return (char*)"MuonMinus";
  case Pi0:
    return (char*)"Pi0";
  case PiPlus:
    return (char*)"PiPlus";
  case PiMinus:
    return (char*)"PiMinus";
  case KLong:
    return (char*)"KLong";
  case KPlus:
    return (char*)"KPlus";
  case KMinus:
    return (char*)"KMinus";
  case Neutron:
    return (char*)"Neutron";
  case Proton:
    return (char*)"Proton";
  case AntiProton:
    return (char*)"AntiProton";
  case KShort:
    return (char*)"KShort";
  case Eta:
    return (char*)"Eta";
  case Lambda:
    return (char*)"Lambda";
  case SigmaPlus:
    return (char*)"SigmaPlus";
  case Sigma0:
    return (char*)"Sigma0";
  case SigmaMinus:
    return (char*)"SigmaMinus";
  case Xi0:
    return (char*)"Xi0";
  case XiMinus:
    return (char*)"XiMinus";
  case OmegaMinus:
    return (char*)"OmegaMinus";
  case AntiNeutron:
    return (char*)"AntiNeutron";
  case AntiLambda:
    return (char*)"AntiLambda";
  case AntiSigmaMinus:
    return (char*)"AntiSigmaMinus";
  case AntiSigma0:
    return (char*)"AntiSigma0";
  case AntiSigmaPlus:
    return (char*)"AntiSigmaPlus";
  case AntiXi0:
    return (char*)"AntiXi0";
  case AntiXiPlus:
    return (char*)"AntiXiPlus";
  case AntiOmegaPlus:
    return (char*)"AntiOmegaPlus";
  case Geantino:
    return (char*)"Geantino";
  case Rho0:
    return (char*)"Rho0";
  case RhoPlus:
    return (char*)"RhoPlus";
  case RhoMinus:
    return (char*)"RhoMinus";
  case omega:
    return (char*)"omega";
  case EtaPrime:
    return (char*)"EtaPrime";
  case phiMeson:
    return (char*)"phiMeson";
  case a0_980:
    return (char*)"a0_980";
  case f0_980:
    return (char*)"f0_980";
  case KStar_892_0:
    return (char*)"KStar_892_0";
  case KStar_892_Plus:
    return (char*)"KStar_892_Plus";
  case KStar_892_Minus:
    return (char*)"KStar_892_Minus";
  case AntiKStar_892_0:
    return (char*)"AntiKStar_892_0";
  case K1_1400_Plus:
    return (char*)"K1_1400_Plus";
  case K1_1400_Minus:
    return (char*)"K1_1400_Minus";
  case b1_1235_Plus:
    return (char*)"b1_1235_Plus";
  case Sigma_1385_Minus:
    return (char*)"Sigma_1385_Minus";
  case Sigma_1385_0:
    return (char*)"Sigma_1385_0";
  case Sigma_1385_Plus:
    return (char*)"Sigma_1385_Plus";
  case Deuteron:
    return (char*)"Deuteron";
  case Triton:
    return (char*)"Triton";
  case Helium:
    return (char*)"Helium";
  case He3:
    return (char*)"Helium-3";
  case Li6:
    return (char*)"Lithium-6";
  case Li7:
    return (char*)"Lithium-7";
  case Be7:
    return (char*)"Beryllium-7";
  case Be9:
    return (char*)"Beryllium-9";
  case B10:
    return (char*)"Boron-10";
  case B11:
    return (char*)"Boron-11";
  case C12:
    return (char*)"Carbon";
  case N14:
    return (char*)"Nitrogen";
  case O16:
    return (char*)"Oxygen";
  case F19:
    return (char*)"Fluorine";
  case Ne20:
    return (char*)"Neon";
  case Na23:
    return (char*)"Sodium";
  case Mg24:
    return (char*)"Magnesium";
  case Al27:
    return (char*)"Aluminum";
  case Si28:
    return (char*)"Silicon";
  case P31:
    return (char*)"Phosphorous";
  case S32:
    return (char*)"Sulphur";
  case Cl35:
    return (char*)"Chlorine";
  case Ar36:
    return (char*)"Argon";
  case K39:
    return (char*)"Potassium";
  case Ca40:
    return (char*)"Calcium";
  case Sc45:
    return (char*)"Scandium";
  case Ti48:
    return (char*)"Titanium";
  case V51:
    return (char*)"Vanadium";
  case Cr52:
    return (char*)"Chromium";
  case Mn55:
    return (char*)"Manganese";
  case Fe56:
    return (char*)"Iron";
  case Co59:
    return (char*)"Cobalt";
  case Ni58:
    return (char*)"Nickel";
  case Cu63:
    return (char*)"Copper";
  case Zn64:
    return (char*)"Zinc";
  case Ge74:
    return (char*)"Germanium";
  case Se80:
    return (char*)"Selenium";
  case Kr84:
    return (char*)"Krypton";
  case Sr88:
    return (char*)"Strontium";
  case Zr90:
    return (char*)"Zirconium";
  case Mo98:
    return (char*)"Molybdenum";
  case Pd106:
    return (char*)"Palladium";
  case Cd114:
    return (char*)"Cadmium";
  case Sn120:
    return (char*)"Tin";
  case Xe132:
    return (char*)"Xenon";
  case Ba138:
    return (char*)"Barium";
  case Ce140:
    return (char*)"Cesium";
  case Sm152:
    return (char*)"Samerium";
  case Dy164:
    return (char*)"Dysprosium";
  case Yb174:
    return (char*)"Ytterbium";
  case W184:
    return (char*)"Tungsten";
  case Pt194:
    return (char*)"Platium";
  case Au197:
    return (char*)"Gold";
  case Hg202:
    return (char*)"Mercury";
  case Pb208:
    return (char*)"Lead";
  case U238:
    return (char*)"Uranium";
  case DeltaPlusPlus:
    return (char*)"DeltaPlusPlus";
  case Jpsi:
    return (char*)"Jpsi";
  case Eta_c:
    return (char*)"Eta_c";
  case Chi_c0:
    return (char*)"Chi_c0";
  case Chi_c1:
    return (char*)"Chi_c1";
  case Chi_c2:
    return (char*)"Chi_c2";
  case Psi2s:
    return (char*)"Psi2s";
  case D0:
    return (char*)"D0";
  case AntiD0:
    return (char*)"AntiD0";
  case DPlus:
    return (char*)"DPlus";
  case Dstar0:
    return (char*)"Dstar0";
  case DstarPlus:
    return (char*)"DstarPlus";
  case Lambda_c:
    return (char*)"Lambda_c";
  default:
    return (char*)"Unknown";
  }
}

inline static char* EvtGenString(Particle_t p)
{
  //returns string that is exact match to enum name. for auto-generating code
  p = RemapParticleID(p);

  switch (p) {
  case Unknown:
    return (char*)"Unknown";
  case Gamma:
    return (char*)"gamma";
  case Positron:
    return (char*)"e+";
  case Electron:
    return (char*)"e-";
  case Neutrino:
    return (char*)"nu_e";
  case MuonPlus:
    return (char*)"mu+";
  case MuonMinus:
    return (char*)"mu-";
  case Pi0:
    return (char*)"pi0";
  case PiPlus:
    return (char*)"pi+";
  case PiMinus:
    return (char*)"pi-";
  case KLong:
    return (char*)"K_L0";
  case KPlus:
    return (char*)"K+";
  case KMinus:
    return (char*)"K-";
  case Neutron:
    return (char*)"n0";
  case Proton:
    return (char*)"p+";
  case AntiProton:
    return (char*)"anti-p-";
  case KShort:
    return (char*)"K_S0";
  case Eta:
    return (char*)"eta";
  case Lambda:
    return (char*)"Lambda0";
  case SigmaPlus:
    return (char*)"Sigma+";
  case Sigma0:
    return (char*)"Sigma0";
  case SigmaMinus:
    return (char*)"Sigma-";
  case Xi0:
    return (char*)"Xi0";
  case XiMinus:
    return (char*)"Xi-";
  case OmegaMinus:
    return (char*)"Omega-";
  case AntiNeutron:
    return (char*)"anti-n0";
  case AntiLambda:
    return (char*)"anti-Lambda0";
  case AntiSigmaMinus:
    return (char*)"anti-Sigma-";
  case AntiSigma0:
    return (char*)"anti-Sigma0";
  case AntiSigmaPlus:
    return (char*)"anti-Sigma+";
  case AntiXi0:
    return (char*)"anti-Xi0";
  case AntiXiPlus:
    return (char*)"anti-Xi+";
  case AntiOmegaPlus:
    return (char*)"anti-Omega+";
  case Geantino:
    return (char*)"geantino";
  case Rho0:
    return (char*)"rho0";
  case RhoPlus:
    return (char*)"rho+";
  case RhoMinus:
    return (char*)"rho-";
  case omega:
    return (char*)"omega";
  case EtaPrime:
    return (char*)"eta'";
  case phiMeson:
    return (char*)"phi";
  case a0_980:
    return (char*)"a_0";
  case f0_980:
    return (char*)"f_0";
  case KStar_892_0:
    return (char*)"K*0";
  case KStar_892_Plus:
    return (char*)"K*+";
  case KStar_892_Minus:
    return (char*)"K*-";
  case AntiKStar_892_0:
    return (char*)"anti-K*0";
  case K1_1400_Plus:
    return (char*)"K'_1+";
  case K1_1400_Minus:
    return (char*)"K'_1-";
  case b1_1235_Plus:
    return (char*)"b_1+";
  case Sigma_1385_Minus:
    return (char*)"Sigma_1385_Minus";
  case Sigma_1385_0:
    return (char*)"Sigma_1385_0";
  case Sigma_1385_Plus:
    return (char*)"Sigma_1385_Plus";
  case Deuteron:
    return (char*)"deuteron";
  case Triton:
    return (char*)"Triton";  // FIX
  case Helium:
    return (char*)"Helium";  // FIX
  case He3:
    return (char*)"He3";
  case Li6:
    return (char*)"Lithium-6";
  case Li7:
    return (char*)"Lithium-7";
  case Be7:
    return (char*)"Beryllium-7";
  case Be9:
    return (char*)"Beryllium-9";
  case B10:
    return (char*)"Boron-10";
  case B11:
    return (char*)"Boron-11";
  case C12:
    return (char*)"Carbon";
  case N14:
    return (char*)"Nitrogen";
  case O16:
    return (char*)"Oxygen";
  case F19:
    return (char*)"Fluorine";
  case Ne20:
    return (char*)"Neon";
  case Na23:
    return (char*)"Sodium";
  case Mg24:
    return (char*)"Magnesium";
  case Al27:
    return (char*)"Aluminum";
  case Si28:
    return (char*)"Silicon";
  case P31:
    return (char*)"Phosphorous";
  case S32:
    return (char*)"Sulphur";
  case Cl35:
    return (char*)"Chlorine";
  case Ar36:
    return (char*)"Argon";
  case K39:
    return (char*)"Potassium";
  case Ca40:
    return (char*)"Calcium";
  case Sc45:
    return (char*)"Scandium";
  case Ti48:
    return (char*)"Titanium";
  case V51:
    return (char*)"Vanadium";
  case Cr52:
    return (char*)"Chromium";
  case Mn55:
    return (char*)"Manganese";
  case Fe56:
    return (char*)"Iron";
  case Co59:
    return (char*)"Cobalt";
  case Ni58:
    return (char*)"Nickel";
  case Cu63:
    return (char*)"Copper";
  case Zn64:
    return (char*)"Zinc";
  case Ge74:
    return (char*)"Germanium";
  case Se80:
    return (char*)"Selenium";
  case Kr84:
    return (char*)"Krypton";
  case Sr88:
    return (char*)"Strontium";
  case Zr90:
    return (char*)"Zirconium";
  case Mo98:
    return (char*)"Molybdenum";
  case Pd106:
    return (char*)"Palladium";
  case Cd114:
    return (char*)"Cadmium";
  case Sn120:
    return (char*)"Tin";
  case Xe132:
    return (char*)"Xenon";
  case Ba138:
    return (char*)"Barium";
  case Ce140:
    return (char*)"Cesium";
  case Sm152:
    return (char*)"Samerium";
  case Dy164:
    return (char*)"Dysprosium";
  case Yb174:
    return (char*)"Ytterbium";
  case W184:
    return (char*)"Tungsten";
  case Pt194:
    return (char*)"Platium";
  case Au197:
    return (char*)"Gold";
  case Hg202:
    return (char*)"Mercury";
  case Pb208:
    return (char*)"Lead";
  case U238:
    return (char*)"Uranium";
  case DeltaPlusPlus:
    return (char*)"Delta++";
  case Jpsi:
    return (char*)"J/psi";
  case Eta_c:
    return (char*)"eta_c";
  case Chi_c0:
    return (char*)"chi_c0";
  case Chi_c1:
    return (char*)"chi_c1";
  case Chi_c2:
    return (char*)"chi_c2";
  case Psi2s:
    return (char*)"psi(2S)";
  case D0:
    return (char*)"D0";
  case AntiD0:
    return (char*)"anti-D0";
  case DPlus:
    return (char*)"D+";
  case Dstar0:
    return (char*)"D*0";
  case DstarPlus:
    return (char*)"D*+";
  case Lambda_c:
    return (char*)"Lambda_c0";
  default:
    return (char*)"Unknown";
  }
}

inline static char* ShortName(Particle_t locPID)
{
  //returns string that is exact match to enum name. for auto-generating code
  locPID = RemapParticleID(locPID);

  switch (locPID) {
  case Unknown:
	return (char*)"x";
  case Gamma:
	return (char*)"g";
  case Positron:
	return (char*)"ep";
  case Electron:
	return (char*)"em";
  case Neutrino:
	return (char*)"nu";
  case MuonPlus:
	return (char*)"mup";
  case MuonMinus:
	return (char*)"mum";
  case Pi0:
	return (char*)"pi0";
  case PiPlus:
	return (char*)"pip";
  case PiMinus:
	return (char*)"pim";
  case KLong:
	return (char*)"kl";
  case KPlus:
	return (char*)"kp";
  case KMinus:
	return (char*)"km";
  case Neutron:
	return (char*)"n";
  case Proton:
	return (char*)"prot";
  case AntiProton:
	return (char*)"antip";
  case KShort:
	return (char*)"ks";
  case Eta:
	return (char*)"eta";
  case Lambda:
	return (char*)"lamb";
  case SigmaPlus:
	return (char*)"sigp";
  case Sigma0:
	return (char*)"sig0";
  case SigmaMinus:
	return (char*)"sigm";
  case Xi0:
	return (char*)"xi0";
  case XiMinus:
	return (char*)"xim";
  case OmegaMinus:
	return (char*)"omegam";
  case AntiNeutron:
	return (char*)"antin";
  case AntiLambda:
	return (char*)"antilamb";
  case AntiSigmaMinus:
	return (char*)"antisigm";
  case AntiSigma0:
	return (char*)"antisig0";
  case AntiSigmaPlus:
	return (char*)"antisigp";
  case AntiXi0:
	return (char*)"antixi0";
  case AntiXiPlus:
	return (char*)"antixip";
  case AntiOmegaPlus:
	return (char*)"antiomegap";
  case Geantino:
	return (char*)"geant";
  case omega:
	return (char*)"omega";
  case EtaPrime:
	return (char*)"etapr";
  case phiMeson:
	return (char*)"phi";
  case Deuteron:
	return (char*)"d";
  case Triton:
	return (char*)"tri";
  case Helium:
	return (char*)"he";
  case He3:
	return (char*)"he3";
  case Li6:
    return (char*)"li6";
  case Li7:
    return (char*)"li7";
  case Be7:
    return (char*)"be7";
  case Be9:
    return (char*)"be9";
  case B10:
    return (char*)"b10";
  case B11:
    return (char*)"b11";
  case C12:
    return (char*)"c12";
  case N14:
    return (char*)"n14";
  case O16:
    return (char*)"o16";
  case F19:
    return (char*)"f19";
  case Ne20:
    return (char*)"ne20";
  case Na23:
    return (char*)"na23";
  case Mg24:
    return (char*)"mg24";
  case Al27:
    return (char*)"al27";
  case Si28:
    return (char*)"si28";
  case P31:
    return (char*)"p31";
  case S32:
    return (char*)"s32";
  case Cl35:
    return (char*)"cl35";
  case Ar36:
    return (char*)"ar36";
  case K39:
    return (char*)"k39";
  case Ca40:
    return (char*)"ca40";
  case Sc45:
    return (char*)"sc45";
  case Ti48:
    return (char*)"ti48";
  case V51:
    return (char*)"v51";
  case Cr52:
    return (char*)"cr52";
  case Mn55:
    return (char*)"mn55";
  case Fe56:
    return (char*)"fe56";
  case Co59:
    return (char*)"co59";
  case Ni58:
    return (char*)"ni58";
  case Cu63:
    return (char*)"cu63";
  case Zn64:
    return (char*)"zn64";
  case Ge74:
    return (char*)"ge74";
  case Se80:
    return (char*)"se80";
  case Kr84:
    return (char*)"kr84";
  case Sr88:
    return (char*)"sr88";
  case Zr90:
    return (char*)"zr90";
  case Mo98:
    return (char*)"mo98";
  case Pd106:
    return (char*)"pd106";
  case Cd114:
    return (char*)"cd114";
  case Sn120:
    return (char*)"sn120";
  case Xe132:
    return (char*)"xe132";
  case Ba138:
    return (char*)"ba138";
  case Ce140:
    return (char*)"ce140";
  case Sm152:
    return (char*)"sm152";
  case Dy164:
    return (char*)"dy164";
  case Yb174:
    return (char*)"yb174";
  case W184:
    return (char*)"w184";
  case Pt194:
    return (char*)"pt194";
  case Au197:
    return (char*)"au197";
  case Hg202:
    return (char*)"hg202";
  case Pb208:
    return (char*)"pb208";
  case U238:
    return (char*)"u238";
  case Jpsi:
	return (char*)"jpsi";
  case Eta_c:
	return (char*)"etac";
  case Chi_c0:
	return (char*)"chic0";
  case Chi_c1:
	return (char*)"chic1";
  case Chi_c2:
	return (char*)"chic2";
  case Psi2s:
	return (char*)"psi2s";
  case D0:
	return (char*)"d0";
  case AntiD0:
	return (char*)"antid0";
  case DPlus:
	return (char*)"dp";
  case Dstar0:
	return (char*)"ds0";
  case DstarPlus:
	return (char*)"dsp";
  case Lambda_c:
	return (char*)"lambc";

  //not really supported
  case Rho0:
	return (char*)"Rho0";
  case RhoPlus:
	return (char*)"RhoPlus";
  case RhoMinus:
	return (char*)"RhoMinus";
  case a0_980:
	return (char*)"a0_980";
  case f0_980:
	return (char*)"f0_980";
  case KStar_892_0:
	return (char*)"KStar_892_0";
  case KStar_892_Plus:
	return (char*)"KStar_892_Plus";
  case KStar_892_Minus:
	return (char*)"KStar_892_Minus";
  case AntiKStar_892_0:
	return (char*)"AntiKStar_892_0";
  case K1_1400_Plus:
	return (char*)"K1_1400_Plus";
  case K1_1400_Minus:
	return (char*)"K1_1400_Minus";
  case b1_1235_Plus:
	return (char*)"b1_1235_Plus";
  case Sigma_1385_Minus:
	return (char*)"Sigma_1385_Minus";
  case Sigma_1385_0:
	return (char*)"Sigma_1385_0";
  case Sigma_1385_Plus:
	return (char*)"Sigma_1385_Plus";
  case DeltaPlusPlus:
	return (char*)"DeltaPlusPlus";

  default:
	return (char*)"x";
  }
}

inline static Particle_t ParticleEnum(const char* locParticleName)
{
  if(strcmp(locParticleName, "Unknown") == 0)
    return Unknown;
  else if(strcmp(locParticleName, "Photon") == 0)
    return Gamma;
  else if(strcmp(locParticleName, "Positron") == 0)
    return Positron;
  else if(strcmp(locParticleName, "Electron") == 0)
    return Electron;
  else if(strcmp(locParticleName, "Neutrino") == 0)
    return Neutrino;
  else if(strcmp(locParticleName, "Muon+") == 0)
    return MuonPlus;
  else if(strcmp(locParticleName, "Muon-") == 0)
    return MuonMinus;
  else if(strcmp(locParticleName, "Pi0") == 0)
    return Pi0;
  else if(strcmp(locParticleName, "Pi+") == 0)
    return PiPlus;
  else if(strcmp(locParticleName, "Pi-") == 0)
    return PiMinus;
  else if(strcmp(locParticleName, "KLong") == 0)
    return KLong;
  else if(strcmp(locParticleName, "K+") == 0)
    return KPlus;
  else if(strcmp(locParticleName, "K-") == 0)
    return KMinus;
  else if(strcmp(locParticleName, "Neutron") == 0)
    return Neutron;
  else if(strcmp(locParticleName, "Proton") == 0)
    return Proton;
  else if(strcmp(locParticleName, "AntiProton") == 0)
    return AntiProton;
  else if(strcmp(locParticleName, "KShort") == 0)
    return KShort;
  else if(strcmp(locParticleName, "Eta") == 0)
    return Eta;
  else if(strcmp(locParticleName, "Lambda") == 0)
    return Lambda;
  else if(strcmp(locParticleName, "Sigma+") == 0)
    return SigmaPlus;
  else if(strcmp(locParticleName, "Sigma0") == 0)
    return Sigma0;
  else if(strcmp(locParticleName, "Sigma-") == 0)
    return SigmaMinus;
  else if(strcmp(locParticleName, "Xi0") == 0)
    return Xi0;
  else if(strcmp(locParticleName, "Xi-") == 0)
    return XiMinus;
  else if(strcmp(locParticleName, "Omega-") == 0)
    return OmegaMinus;
  else if(strcmp(locParticleName, "AntiNeutron") == 0)
    return AntiNeutron;
  else if(strcmp(locParticleName, "AntiLambda") == 0)
    return AntiLambda;
  else if(strcmp(locParticleName, "AntiSigma-") == 0)
    return AntiSigmaMinus;
  else if(strcmp(locParticleName, "AntiSigma0") == 0)
    return AntiSigma0;
  else if(strcmp(locParticleName, "AntiSigma+") == 0)
    return AntiSigmaPlus;
  else if(strcmp(locParticleName, "AntiXi0") == 0)
    return AntiXi0;
  else if(strcmp(locParticleName, "AntiXi+") == 0)
    return AntiXiPlus;
  else if(strcmp(locParticleName, "AntiOmega+") == 0)
    return AntiOmegaPlus;
  else if(strcmp(locParticleName, "Geantino") == 0)
    return Geantino;
  else if(strcmp(locParticleName, "Rho0") == 0)
    return Rho0;
  else if(strcmp(locParticleName, "Rho+") == 0)
    return RhoPlus;
  else if(strcmp(locParticleName, "Rho-") == 0)
    return RhoMinus;
  else if(strcmp(locParticleName, "Omega") == 0)
    return omega;
  else if(strcmp(locParticleName, "EtaPrime") == 0)
    return EtaPrime;
  else if(strcmp(locParticleName, "Phi") == 0)
    return phiMeson;
  else if(strcmp(locParticleName, "a0(980)") == 0)
    return a0_980;
  else if(strcmp(locParticleName, "f0(980)") == 0)
    return f0_980;
  else if(strcmp(locParticleName, "K*(892)0") == 0)
    return KStar_892_0;
  else if(strcmp(locParticleName, "K*(892)+") == 0)
    return KStar_892_Plus;
  else if(strcmp(locParticleName, "K*(892)-") == 0)
    return KStar_892_Minus;
  else if(strcmp(locParticleName, "antiK*(892)0") == 0)
    return AntiKStar_892_0;
  else if(strcmp(locParticleName, "K1(1400)+") == 0)
    return K1_1400_Plus;
  else if(strcmp(locParticleName, "K1(1400)-") == 0)
    return K1_1400_Minus;
  else if(strcmp(locParticleName, "b1(1235)+") == 0)
    return b1_1235_Plus;
  else if(strcmp(locParticleName, "Sigma(1385)-") == 0)
    return Sigma_1385_Minus;
  else if(strcmp(locParticleName, "Sigma(1385)0") == 0)
    return Sigma_1385_0;
  else if(strcmp(locParticleName, "Sigma(1385)+") == 0)
    return Sigma_1385_Plus;
  else if(strcmp(locParticleName, "Deuteron") == 0)
    return Deuteron;
  else if(strcmp(locParticleName, "Triton") == 0)
    return Triton;
  else if(strcmp(locParticleName, "Helium") == 0)
    return Helium;
  else if(strcmp(locParticleName, "Helium-3") == 0)
    return He3;
  else if(strcmp(locParticleName, "Lithium-6") == 0)
    return Li6;
  else if(strcmp(locParticleName, "Lithium-7") == 0)
    return Li7;
  else if(strcmp(locParticleName, "Beryllium-7") == 0)
    return Be7;
  else if(strcmp(locParticleName, "Beryllium-9") == 0)
    return Be9;
  else if(strcmp(locParticleName, "Boron-10") == 0)
    return B10;
  else if(strcmp(locParticleName, "Boron-11") == 0)
    return B11;
  else if(strcmp(locParticleName, "Carbon") == 0)
    return C12;
  else if(strcmp(locParticleName, "Nitrogen") == 0)
    return N14;
  else if(strcmp(locParticleName, "Oxygen") == 0)
    return O16;
  else if(strcmp(locParticleName, "Fluorine") == 0)
    return F19;
  else if(strcmp(locParticleName, "Neon") == 0)
    return Ne20;
  else if(strcmp(locParticleName, "Sodium") == 0)
    return Na23;
  else if(strcmp(locParticleName, "Magnesium") == 0)
    return Mg24;
  else if(strcmp(locParticleName, "Aluminum") == 0)
    return Al27;
  else if(strcmp(locParticleName, "Silicon") == 0)
    return Si28;
  else if(strcmp(locParticleName, "Phosphorous") == 0)
    return P31;
  else if(strcmp(locParticleName, "Sulphur") == 0)
    return S32;
  else if(strcmp(locParticleName, "Chlorine") == 0)
    return Cl35;
  else if(strcmp(locParticleName, "Argon") == 0)
    return Ar36;
  else if(strcmp(locParticleName, "Potassium") == 0)
    return K39;
  else if(strcmp(locParticleName, "Calcium") == 0)
    return Ca40;
  else if(strcmp(locParticleName, "Scandium") == 0)
    return Sc45;
  else if(strcmp(locParticleName, "Titanium") == 0)
    return Ti48;
  else if(strcmp(locParticleName, "Vanadium") == 0)
    return V51;
  else if(strcmp(locParticleName, "Chromium") == 0)
    return Cr52;
  else if(strcmp(locParticleName, "Manganese") == 0)
    return Mn55;
  else if(strcmp(locParticleName, "Iron") == 0)
    return Fe56;
  else if(strcmp(locParticleName, "Cobalt") == 0)
    return Co59;
  else if(strcmp(locParticleName, "Nickel") == 0)
    return Ni58;
  else if(strcmp(locParticleName, "Copper") == 0)
    return Cu63;
  else if(strcmp(locParticleName, "Zinc") == 0)
    return Zn64;
  else if(strcmp(locParticleName, "Germanium") == 0)
    return Ge74;
  else if(strcmp(locParticleName, "Selenium") == 0)
    return Se80;
  else if(strcmp(locParticleName, "Krypton") == 0)
    return Kr84;
  else if(strcmp(locParticleName, "Strontium") == 0)
    return Sr88;
  else if(strcmp(locParticleName, "Zirconium") == 0)
    return Zr90;
  else if(strcmp(locParticleName, "Molybdenum") == 0)
    return Mo98;
  else if(strcmp(locParticleName, "Palladium") == 0)
    return Pd106;
  else if(strcmp(locParticleName, "Cadmium") == 0)
    return Cd114;
  else if(strcmp(locParticleName, "Tin") == 0)
    return Sn120;
  else if(strcmp(locParticleName, "Xenon") == 0)
    return Xe132;
  else if(strcmp(locParticleName, "Barium") == 0)
    return Ba138;
  else if(strcmp(locParticleName, "Cesium") == 0)
    return Ce140;
  else if(strcmp(locParticleName, "Samerium") == 0)
    return Sm152;
  else if(strcmp(locParticleName, "Dysprosium") == 0)
    return Dy164;
  else if(strcmp(locParticleName, "Ytterbium") == 0)
    return Yb174;
  else if(strcmp(locParticleName, "Tungsten") == 0)
    return W184;
  else if(strcmp(locParticleName, "Platium") == 0)
    return Pt194;
  else if(strcmp(locParticleName, "Gold") == 0)
    return Au197;
  else if(strcmp(locParticleName, "Mercury") == 0)
    return Hg202;
  else if(strcmp(locParticleName, "Lead") == 0)
    return Pb208;
  else if(strcmp(locParticleName, "Uranium") == 0)
    return U238;
  else if(strcmp(locParticleName, "Delta++") == 0)
    return DeltaPlusPlus;
  else if(strcmp(locParticleName, "Jpsi") == 0)
    return Jpsi;
  else if(strcmp(locParticleName, "EtaC") == 0)
    return Eta_c;
  else if(strcmp(locParticleName, "ChiC0") == 0)
    return Chi_c0;
  else if(strcmp(locParticleName, "ChiC1") == 0)
    return Chi_c1;
  else if(strcmp(locParticleName, "ChiC2") == 0)
    return Chi_c2;
  else if(strcmp(locParticleName, "Psi(2S)") == 0)
    return Psi2s;
  else if(strcmp(locParticleName, "D0") == 0)
    return D0;
  else if(strcmp(locParticleName, "AntiD0") == 0)
    return AntiD0;
  else if(strcmp(locParticleName, "D+") == 0)
    return DPlus;
  else if(strcmp(locParticleName, "D*0") == 0)
    return Dstar0;
  else if(strcmp(locParticleName, "D*+") == 0)
    return DstarPlus;
  else if(strcmp(locParticleName, "LambdaC") == 0)
    return Lambda_c;
  else
    return Unknown;
}

inline static unsigned short int IsFixedMass(Particle_t p)
{
  switch (p)
  {
  case Gamma:		return 1;
  case Positron:	return 1;
  case Electron:	return 1;
  case Neutrino:	return 1;
  case MuonPlus:	return 1;
  case MuonMinus:	return 1;
  case Pi0:      	return 1;
  case PiPlus:		return 1;
  case PiMinus:		return 1;
  case KShort:		return 1;
  case KLong:		return 1;
  case KPlus:		return 1;
  case KMinus:		return 1;
  case Neutron:		return 1;
  case Proton:		return 1;
  case AntiProton:	return 1;
  case Eta:			return 1;
  case Lambda:		return 1;
  case SigmaPlus:	return 1;
  case Sigma0:		return 1;
  case SigmaMinus:	return 1;
  case Xi0:			return 1;
  case XiMinus:		return 1;
  case OmegaMinus:	return 1;
  case AntiNeutron:	return 1;
  case AntiLambda:	return 1;
  case AntiSigmaMinus:	return 1;
  case AntiSigma0:	return 1;
  case AntiSigmaPlus:	return 1;
  case AntiXi0:		return 1;
  case AntiXiPlus:	return 1;
  case AntiOmegaPlus:	return 1;
  case Geantino:	return 1;
  case EtaPrime:	return 1;
  case Deuteron:	return 1;
  case Triton:   	return 1;
  case Helium:		return 1;
  case He3:		return 1;
  case Li6:     return 1;
  case Li7:     return 1;
  case Be7:     return 1;
  case Be9:     return 1;
  case B10:     return 1;
  case B11:     return 1;
  case C12:     return 1;
  case N14:     return 1;
  case O16:     return 1;
  case F19:     return 1;
  case Ne20:    return 1;
  case Na23:    return 1;
  case Mg24:    return 1;
  case Al27:    return 1;
  case Si28:    return 1;
  case P31:     return 1;
  case S32:     return 1;
  case Cl35:    return 1;
  case Ar36:    return 1;
  case K39:     return 1;
  case Ca40:    return 1;
  case Sc45:    return 1;
  case Ti48:    return 1;
  case V51:     return 1;
  case Cr52:    return 1;
  case Mn55:    return 1;
  case Fe56:    return 1;
  case Co59:    return 1;
  case Ni58:    return 1;
  case Cu63:    return 1;
  case Zn64:    return 1;
  case Ge74:    return 1;
  case Se80:    return 1;
  case Kr84:    return 1;
  case Sr88:    return 1;
  case Zr90:    return 1;
  case Mo98:    return 1;
  case Pd106:   return 1;
  case Cd114:   return 1;
  case Sn120:   return 1;
  case Xe132:   return 1;
  case Ba138:   return 1;
  case Ce140:   return 1;
  case Sm152:   return 1;
  case Dy164:   return 1;
  case Yb174:   return 1;
  case W184:    return 1;
  case Pt194:   return 1;
  case Au197:   return 1;
  case Hg202:   return 1;
  case Pb208:   return 1;
  case U238:    return 1;
  case Jpsi:    return 1;
  //case eta_c:   return 1;
  //case chi_c0:  return 1;
  case Chi_c1:  return 1;
  case Chi_c2:  return 1;
  case Psi2s:   return 1;
  case D0:      return 1;
  case AntiD0:  return 1;
  case DPlus:   return 1;
  case Dstar0:  return 1;
  case DstarPlus:  return 1;
  case Lambda_c: return 1;
  default: return 0;
  }
}

inline static unsigned short int IsResonance(Particle_t p)
{
   p = RemapParticleID(p);

	if(IsFixedMass(p) == 1)
		return 0;
	if(p == Unknown)
		return 0;
	if(p == phiMeson)
		return 0;
	if(p == omega)
		return 0;
	return 1;
}

inline static unsigned short int IsDetachedVertex(Particle_t p)
{
  switch (p)
  {
  case MuonPlus:	return 1;
  case MuonMinus:	return 1;
  case PiPlus:		return 1;
  case PiMinus:		return 1;
  case KShort:		return 1;
  case KLong:		return 1;
  case KPlus:		return 1;
  case KMinus:		return 1;
  case Neutron:		return 1;
  case Lambda:		return 1;
  case SigmaPlus:	return 1;
  case SigmaMinus:	return 1;
  case Xi0:			return 1;
  case XiMinus:		return 1;
  case OmegaMinus:	return 1;
  case AntiNeutron:	return 1;
  case AntiLambda:	return 1;
  case AntiSigmaMinus:	return 1;
  case AntiSigmaPlus:	return 1;
  case AntiXi0:		return 1;
  case AntiXiPlus:	return 1;
  case AntiOmegaPlus:	return 1;
  case Deuteron:	return 1;
  case Triton:  	return 1;
  case Helium:		return 1;
  case He3:  		return 1;
  default: return 0;
  }
}

inline static char* ParticleName_ROOT(Particle_t p)
{
  p = RemapParticleID(p);

  switch (p) {
  case Unknown:
    return (char*)"X";
  case Gamma:
    return (char*)"#gamma";
  case Positron:
    return (char*)"e^{#plus}";
  case Electron:
    return (char*)"e^{#minus}";
  case Neutrino:
    return (char*)"#nu";
  case MuonPlus:
    return (char*)"#mu^{#plus}";
  case MuonMinus:
    return (char*)"#mu^{#minus}";
  case Pi0:
    return (char*)"#pi^{0}";
  case PiPlus:
    return (char*)"#pi^{#plus}";
  case PiMinus:
    return (char*)"#pi^{#minus}";
  case KLong:
    return (char*)"K^{0}_{L}";
  case KPlus:
    return (char*)"K^{#plus}";
  case KMinus:
    return (char*)"K^{#minus}";
  case Neutron:
    return (char*)"n";
  case Proton:
    return (char*)"p";
  case AntiProton:
    return (char*)"#bar{p}";
  case KShort:
    return (char*)"K^{0}_{S}";
  case Eta:
    return (char*)"#eta";
  case Lambda:
    return (char*)"#Lambda";
  case SigmaPlus:
    return (char*)"#Sigma^{#plus}";
  case Sigma0:
    return (char*)"#Sigma^{0}";
  case SigmaMinus:
    return (char*)"#Sigma^{#minus}";
  case Xi0:
    return (char*)"#Xi^{0}";
  case XiMinus:
    return (char*)"#Xi^{#minus}";
  case OmegaMinus:
    return (char*)"#Omega^{#minus}";
  case AntiNeutron:
    return (char*)"#bar^{n}";
  case AntiLambda:
    return (char*)"#bar^{#Lambda}";
  case AntiSigmaMinus:
    return (char*)"#bar{#Sigma}^{#minus}";
  case AntiSigma0:
    return (char*)"#bar{#Sigma}^{0}";
  case AntiSigmaPlus:
    return (char*)"#bar{#Sigma}^{#plus}";
  case AntiXi0:
    return (char*)"#bar{#Xi}^{0}";
  case AntiXiPlus:
    return (char*)"#bar{#Xi}^{#plus}";
  case AntiOmegaPlus:
    return (char*)"#bar{#Omega}^{#plus}";
  case Geantino:
    return (char*)"geantino";
  case Rho0:
    return (char*)"#rho^{0}";
  case RhoPlus:
    return (char*)"#rho^{#plus}";
  case RhoMinus:
    return (char*)"#rho^{#minus}";
  case omega:
    return (char*)"#omega";
  case EtaPrime:
    return (char*)"#eta'";
  case phiMeson:
    return (char*)"#phi";
  case a0_980:
    return (char*)"a_{0}(980)";
  case f0_980:
    return (char*)"f_{0}(980)";
  case KStar_892_0:
    return (char*)"K*(892)^{0}";
  case KStar_892_Plus:
    return (char*)"K*(892)^{#plus}";
  case KStar_892_Minus:
    return (char*)"K*(892)^{#minus}";
  case AntiKStar_892_0:
    return (char*)"#bar{K*}(892)^{0}";
  case K1_1400_Plus:
    return (char*)"K_{1}(1400)^{#plus}";
  case K1_1400_Minus:
    return (char*)"K_{1}(1400)^{#minus}";
  case b1_1235_Plus:
    return (char*)"b_{1}(1235)^{#plus}";
  case Deuteron:
    return (char*)"d";
  case Triton:
    return (char*)"t";
  case Helium:
    return (char*)"He";
  case He3:
    return (char*)"^{3}He";
  case Li6:
    return (char*)"^{6}Li";
  case Li7:
    return (char*)"^{7}Li";
  case Be7:
    return (char*)"^{7}Be";
  case Be9:
    return (char*)"^{9}Be";
  case B10:
    return (char*)"^{10}B";
  case B11:
    return (char*)"^{11}B";
  case C12:
    return (char*)"^{12}C";
  case N14:
    return (char*)"^{14}N";
  case O16:
    return (char*)"^{16}O";
  case F19:
    return (char*)"^{19}F";
  case Ne20:
    return (char*)"^{20}Ne";
  case Na23:
    return (char*)"^{23}Na";
  case Mg24:
    return (char*)"^{24}Mg";
  case Al27:
    return (char*)"^{27}Al";
  case Si28:
    return (char*)"^{28}Si";
  case P31:
    return (char*)"^{31}P";
  case S32:
    return (char*)"^{32}S";
  case Cl35:
    return (char*)"^{35}Cl";
  case Ar36:
    return (char*)"^{36}Ar";
  case K39:
    return (char*)"^{39}K";
  case Ca40:
    return (char*)"^{40}Ca";
  case Sc45:
    return (char*)"^{45}Sc";
  case Ti48:
    return (char*)"^{48}Ti";
  case V51:
    return (char*)"^{51}V";
  case Cr52:
    return (char*)"^{52}Cr";
  case Mn55:
    return (char*)"^{55}Mn";
  case Fe56:
    return (char*)"^{56}Fe";
  case Co59:
    return (char*)"^{59}Co";
  case Ni58:
    return (char*)"^{58}Ni";
  case Cu63:
    return (char*)"^{63}Cu";
  case Zn64:
    return (char*)"^{64}Zn";
  case Ge74:
    return (char*)"^{74}Ge";
  case Se80:
    return (char*)"^{80}Se";
  case Kr84:
    return (char*)"^{84}Kr";
  case Sr88:
    return (char*)"^{88}Sr";
  case Zr90:
    return (char*)"^{90}Zr";
  case Mo98:
    return (char*)"^{98}Mo";
  case Pd106:
    return (char*)"^{106}Pd";
  case Cd114:
    return (char*)"^{114}Cd";
  case Sn120:
    return (char*)"^{120}Sn";
  case Xe132:
    return (char*)"^{132}Xe";
  case Ba138:
    return (char*)"^{138}Ba";
  case Ce140:
    return (char*)"^{140}Ce";
  case Sm152:
    return (char*)"^{152}Sm";
  case Dy164:
    return (char*)"^{164}Dy";
  case Yb174:
    return (char*)"^{174}Yb";
  case W184:
    return (char*)"^{184}W";
  case Pt194:
    return (char*)"^{194}Pt";
  case Au197:
    return (char*)"^{197}Au";
  case Hg202:
    return (char*)"^{202}Hg";
  case Pb208:
    return (char*)"^{208}Pb";
  case U238:
    return (char*)"^{238}U";
  case Sigma_1385_Minus:
    return (char*)"#Sigma(1385)^{#minus}";
  case Sigma_1385_0:
    return (char*)"#Sigma(1385)^{0}";
  case Sigma_1385_Plus:
    return (char*)"#Sigma(1385)^{#plus}";
  case DeltaPlusPlus:
    return (char*)"#Delta(1232)^{#plus#plus}";
  case Jpsi:
    return (char*)"J/#psi";
  case Eta_c:
    return (char*)"#eta_{c}";
  case Chi_c0:
    return (char*)"#chi_{c0}";
  case Chi_c1:
    return (char*)"#chi_{c1}";
  case Chi_c2:
    return (char*)"#chi_{c2}";
  case Psi2s:
    return (char*)"#psi(2S)";
  case D0:
    return (char*)"D^{0}";
  case AntiD0:
    return (char*)"#bar{D^{0}}";
  case DPlus:
    return (char*)"D{^+}";
  case Dstar0:
    return (char*)"D^{*0}";
  case DstarPlus:
    return (char*)"D^{*+}";
  case Lambda_c:
    return (char*)"#Lambda_{c}";
		 
  default:
    return (char*)"X";
  }
}

inline static double ParticleMass(Particle_t p)
{
  p = RemapParticleID(p);

  switch (p) {
  case Unknown:		return HUGE_VAL;
  case Gamma:		return 0;
  case Positron:	return 0.000510998928;
  case Electron:	return 0.000510998928;
  case Neutrino:	return 0;
  case MuonPlus:	return 0.1056583715;
  case MuonMinus:	return 0.1056583715;
  case Pi0:		return 0.1349766;
  case PiPlus:		return 0.13957018;
  case PiMinus:		return 0.13957018;
  case KShort:		return 0.497614;
  case KLong:		return 0.497614;
  case KPlus:		return 0.493677;
  case KMinus:		return 0.493677;
  case Neutron:		return 0.939565379;
  case Proton:		return 0.938272046;
  case AntiProton:	return 0.938272046;
  case Eta:		return 0.547862;
  case Lambda:		return 1.115683;
  case SigmaPlus:	return 1.18937;
  case Sigma0:		return 1.192642;
  case SigmaMinus:	return 1.197449;
  case Xi0:		return 1.31486;
  case XiMinus:		return 1.32171;
  case OmegaMinus:	return 1.67245;
  case AntiNeutron:	return 0.939565379;
  case AntiLambda:	return 1.115683;
  case AntiSigmaMinus:	return 1.18937;
  case AntiSigma0:	return 1.192642;
  case AntiSigmaPlus:	return 1.197449;
  case AntiXi0:		return 1.31486;
  case AntiXiPlus:	return 1.32171;
  case AntiOmegaPlus:	return 1.67245;
  case Geantino:	return 0.0;
  case Rho0:		return 0.7690;  // neutral only, photoproduced and other reactions. e+ e- gives 775.26
  case RhoPlus:		return 0.7665;  // charged only, hadroproduced. tau decays and e+ e- gives 775.11
  case RhoMinus:	return 0.7665;
  case omega:		return 0.78265;
  case EtaPrime:	return 0.95778;
  case phiMeson:	return 1.019455;
  case a0_980:		return 0.980;
  case f0_980:		return 0.990;
  case KStar_892_0: return 0.89581;     // neutral only
  case KStar_892_Plus: return 0.89166;  // charged only, hadroproduced
  case KStar_892_Minus: return 0.89166; // charged only, hadroproduced
  case AntiKStar_892_0: return 0.89581; // neutral only
  case K1_1400_Plus: return 1.403;
  case K1_1400_Minus: return 1.403;
  case b1_1235_Plus: return 1.2295;
  case Deuteron:	return 1.875612859;     // from NIST
  case Triton:	    return 2.808921004;     // from NIST 5.00735630 x 10^-27 kg
  case Helium:		return 3.727379238;     // from NIST 6.64465675 x 10-27 kg
  case He3:  		return 2.809413498;
  case Li6:         return 5.60305;
  case Li7:         return 6.53536;
  case Be7:         return 6.53622;
  case Be9:         return 8.39479;
  case B10:         return 9.32699;
  case B11:         return 10.25510;
  case C12:         return 11.17793;
  case N14:         return 13.04378;
  case O16:         return 14.89917;
  case F19:         return 17.69690;
  case Ne20:        return 18.62284;
  case Na23:        return 21.41483;
  case Mg24:        return 22.34193;
  case Al27:        return 25.13314;
  case Si28:        return 26.06034;
  case P31:         return 28.85188;
  case S32:         return 29.78180;
  case Cl35:        return 32.57328;
  case Ar36:        return 33.50356;
  case K39:         return 36.29447;
  case Ca40:        return 37.22492;
  case Sc45:        return 41.87617;
  case Ti48:        return 44.66324;
  case V51:         return 47.45401;
  case Cr52:        return 48.38228;
  case Mn55:        return 51.17447;
  case Fe56:        return 52.10307;
  case Co59:        return 54.89593;
  case Ni58:        return 53.96644;
  case Cu63:        return 58.61856;
  case Zn64:        return 59.54963;
  case Ge74:        return 68.85715;
  case Se80:        return 74.44178;
  case Kr84:        return 78.16309;
  case Sr88:        return 81.88358;
  case Zr90:        return 83.74571;
  case Mo98:        return 91.19832;
  case Pd106:       return 98.64997;
  case Cd114:       return 106.10997;
  case Sn120:       return 111.68821;
  case Xe132:       return 122.86796;
  case Ba138:       return 128.45793;
  case Ce140:       return 130.32111;
  case Sm152:       return 141.51236;
  case Dy164:       return 152.69909;
  case Yb174:       return 162.02245;
  case W184:        return 171.34924;
  case Pt194:       return 180.67513;
  case Au197:       return 183.47324;
  case Hg202:       return 188.13451;
  case Pb208:	    return 193.72899;       // NIST gives 207.976627 AMU
  case U238:        return 221.74295;
  case Sigma_1385_Minus:	return 1.3872;
  case Sigma_1385_0:		return 1.3837;
  case Sigma_1385_Plus:	return 1.38280;
  case DeltaPlusPlus:   return 1.232;
  case Jpsi:            return 3.069916;
  case Eta_c:           return 2.9836;
  case Chi_c0:          return 3.41475;
  case Chi_c1:          return 3.51066;
  case Chi_c2:          return 3.55620;
  case Psi2s:           return 3.686109;
  case D0:              return 1.86484;
  case AntiD0:          return 1.86484;
  case DPlus:           return 1.86961;
  case Dstar0:          return 2.01026;
  case DstarPlus:       return 2.00696;
  case Lambda_c:        return 2.28646;
  default:
    fprintf(stderr,"ParticleMass: Error: Unknown particle type %d,",p);
    fprintf(stderr," returning HUGE_VAL...\n");
    return HUGE_VAL;
  }
}

inline static int ParticleCharge(Particle_t p)
{
  p = RemapParticleID(p);

  switch (p) {
  case Unknown:		return  0;
  case Gamma:		return  0;
  case Positron:	return +1;
  case Electron:	return -1;
  case Neutrino:	return  0;
  case MuonPlus:	return +1;
  case MuonMinus:	return -1;
  case Pi0:		return  0;
  case PiPlus:		return +1;
  case PiMinus:		return -1;
  case KShort:		return  0;
  case KLong:		return  0;
  case KPlus:		return +1;
  case KMinus:		return -1;
  case Neutron:		return  0;
  case Proton:		return +1;
  case AntiProton:	return -1;
  case Eta:		return  0;
  case Lambda:		return  0;
  case SigmaPlus:	return +1;
  case Sigma0:		return  0;
  case SigmaMinus:	return -1;
  case Xi0:		return  0;
  case XiMinus:		return -1;
  case OmegaMinus:	return -1;
  case AntiNeutron:	return  0;
  case AntiLambda:	return  0;
  case AntiSigmaMinus:	return -1;
  case AntiSigma0:	return  0;
  case AntiSigmaPlus:	return +1;
  case AntiXi0:		return  0;
  case AntiXiPlus:	return +1;
  case AntiOmegaPlus:	return +1;
  case Geantino:	return  0;
  case Rho0:		return  0;
  case RhoPlus:		return +1;
  case RhoMinus:	return -1;
  case omega:		return  0;
  case EtaPrime:	return  0;
  case phiMeson:	return  0;
  case a0_980:		return  0;
  case f0_980:		return  0;
  case KStar_892_0: return  0;
  case KStar_892_Plus: return  1;
  case KStar_892_Minus: return -1;
  case AntiKStar_892_0: return  0;
  case K1_1400_Plus: return  1;
  case K1_1400_Minus: return -1;
  case b1_1235_Plus: return 1;
  case Deuteron:	return 1;
  case Triton:  	return 1;
  case Helium:		return 2;
  case He3:		    return 2;
  case Li6:         return 3;
  case Li7:         return 3;
  case Be7:         return 4;
  case Be9:         return 4;
  case B10:         return 5;
  case B11:         return 5;
  case C12:         return 6;
  case N14:         return 7;
  case O16:         return 8;
  case F19:         return 9;
  case Ne20:        return 10;
  case Na23:        return 11;
  case Mg24:        return 12;
  case Al27:        return 13;
  case Si28:        return 14;
  case P31:         return 15;
  case S32:         return 16;
  case Cl35:        return 17;
  case Ar36:        return 18;
  case K39:         return 19;
  case Ca40:        return 20;
  case Sc45:        return 21;
  case Ti48:        return 22;
  case V51:         return 23;
  case Cr52:        return 24;
  case Mn55:        return 25;
  case Fe56:        return 26;
  case Co59:        return 27;
  case Ni58:        return 28;
  case Cu63:        return 29;
  case Zn64:        return 30;
  case Ge74:        return 32;
  case Se80:        return 34;
  case Kr84:        return 36;
  case Sr88:        return 38;
  case Zr90:        return 40;
  case Mo98:        return 42;
  case Pd106:       return 46;
  case Cd114:       return 48;
  case Sn120:       return 50;
  case Xe132:       return 54;
  case Ba138:       return 56;
  case Ce140:       return 58;
  case Sm152:       return 62;
  case Dy164:       return 66;
  case Yb174:       return 70;
  case W184:        return 74;
  case Pt194:       return 78;
  case Au197:       return 79;
  case Hg202:       return 80;
  case Pb208:       return 82;
  case U238:        return 92;
  case Sigma_1385_Minus:	return -1;
  case Sigma_1385_0:		return 0;
  case Sigma_1385_Plus:	return 1;
  case DeltaPlusPlus: return 2;
  case Jpsi:    return 0;
  case Eta_c:   return 0;
  case Chi_c0:  return 0;
  case Chi_c1:  return 0;
  case Chi_c2:  return 0;
  case Psi2s:   return 0;
  case D0:      return 0;
  case AntiD0:  return 0;
  case DPlus:   return 1;
  case Dstar0:  return 0;
  case DstarPlus:  return 1;
  case Lambda_c:   return 1;

  default:
    fprintf(stderr,"ParticleCharge: Error: Unknown particle type %d,",p);
    fprintf(stderr," returning 0...\n");
    return 0;
  }
}

inline static int PDGtype(Particle_t p)
{
  p = RemapParticleID(p);

  switch (p) {
  case Unknown:		return  0;
  case Gamma:		return  22;
  case Positron:	return -11;
  case Electron:	return  11;
  case Neutrino:	return  121416;
  case MuonPlus:	return -13;
  case MuonMinus:	return  13;
  case Pi0:		return  111;
  case PiPlus:		return  211;
  case PiMinus:		return -211;
  case KShort:		return  310;
  case KLong:		return  130;
  case KPlus:		return  321;
  case KMinus:		return -321;
  case Neutron:		return  2112;
  case Proton:		return  2212;
  case AntiProton:	return -2212;
  case Eta:		return  221;
  case Lambda:		return  3122;
  case SigmaPlus:	return  3222;
  case Sigma0:		return  3212;
  case SigmaMinus:	return  3112;
  case Xi0:		return  3322;
  case XiMinus:		return  3312;
  case OmegaMinus:	return  3334;
  case AntiNeutron:	return -2112;
  case AntiLambda:	return -3122;
  case AntiSigmaMinus:	return -3112;
  case AntiSigma0:	return -3212;
  case AntiSigmaPlus:	return -3222;
  case AntiXi0:		return -3322;
  case AntiXiPlus:	return -3312;
  case AntiOmegaPlus:	return -3334;
  case Geantino:	return  0;
  case Rho0:		return  113;
  case RhoPlus:		return  213;
  case RhoMinus:	return -213;
  case omega:		return  223;
  case EtaPrime:	return  331;
  case phiMeson:	return  333;
  case a0_980:		return  9000110;
  case f0_980:		return  9010221;
  case KStar_892_0: return  313;
  case AntiKStar_892_0: return  -313;
  case KStar_892_Plus: return  323;
  case KStar_892_Minus: return -323;
  case K1_1400_Plus: return  20323;
  case K1_1400_Minus: return  -20323;
  case b1_1235_Plus: return  10213;
  case Deuteron:		return  45;
  case Triton:		return  46;
  case Helium:		return  47;
  case He3:			return  49;
  case Sigma_1385_Minus:	return 3114;
  case Sigma_1385_0:		return 3214;
  case Sigma_1385_Plus:	return 3224;
  case Li6:      return 1000030060;
  case Li7:      return 1000030070;
  case Be7:      return 1000040070;
  case Be9:      return 1000040090;
  case B10:      return 1000050100;
  case B11:      return 1000050110;
  case C12:      return 1000060120;
  case N14:      return 1000070140;
  case O16:      return 1000080160;
  case F19:      return 1000090190;
  case Ne20:     return 1000100200;
  case Na23:     return 1000110230;
  case Mg24:     return 1000120240;
  case Al27:     return 1000130270;
  case Si28:     return 1000140280;
  case P31:      return 1000150310;
  case S32:      return 1000160320;
  case Cl35:     return 1000170350;
  case Ar36:     return 1000180360;
  case K39:      return 1000190390;
  case Ca40:     return 1000200400;
  case Sc45:     return 1000210450;
  case Ti48:     return 1000220480;
  case V51:      return 1000230510;
  case Cr52:     return 1000240520;
  case Mn55:     return 1000250550;
  case Fe56:     return 1000260560;
  case Co59:     return 1000270590;
  case Ni58:     return 1000280580;
  case Cu63:     return 1000290630;
  case Zn64:     return 1000300640;
  case Ge74:     return 1000320740;
  case Se80:     return 1000340800;
  case Kr84:     return 1000360840;
  case Sr88:     return 1000380880;
  case Zr90:     return 1000400900;
  case Mo98:     return 1000420980;
  case Pd106:    return 1000461060;
  case Cd114:    return 1000481140;
  case Sn120:    return 1000501200;
  case Xe132:    return 1000541320;
  case Ba138:    return 1000561380;
  case Ce140:    return 1000581400;
  case Sm152:    return 1000621520;
  case Dy164:    return 1000661640;
  case Yb174:    return 1000701740;
  case W184:     return 1000741840;
  case Pt194:    return 1000781940;
  case Au197:    return 1000791970;
  case Hg202:    return 1000802020;
  case Pb208:    return 1000822080;
  case U238:     return 1000922380;
  case DeltaPlusPlus: return 2224;
  case Jpsi:          return 443;
  case Eta_c:         return 441;
  case Chi_c0:        return 10441;
  case Chi_c1:        return 20443;
  case Chi_c2:        return 445;
  case Psi2s:         return 100443;
  case D0:            return 421;
  case AntiD0:        return -421; //FIX!!
  case DPlus:         return 411;
  case Dstar0:        return 423;
  case DstarPlus:     return 413;
  case Lambda_c:      return 4122;
  default:		return  0;
  }
}

inline static Particle_t PDGtoPType(int locPDG_PID)
{
  switch (locPDG_PID) {
  case 0:				return Unknown;
  case 22:			return Gamma;
  case -11:			return Positron;
  case 11:			return Electron;
  case 121416:		return Neutrino;
  case -13:			return MuonPlus;
  case 13:			return MuonMinus;
  case 111:			return Pi0;
  case 211:			return PiPlus;
  case -211:			return PiMinus;
  case 310:			return KShort;
  case 130:			return KLong;
  case 321:			return KPlus;
  case -321:			return KMinus;
  case 2112:			return Neutron;
  case 2212:			return Proton;
  case -2212:		return AntiProton;
  case 221:			return Eta;
  case 3122:			return Lambda;
  case 3222:			return SigmaPlus;
  case 3212:			return Sigma0;
  case 3112:			return SigmaMinus;
  case 3322:			return Xi0;
  case 3312:			return XiMinus;
  case 3334:			return OmegaMinus;
  case -2112:		return AntiNeutron;
  case -3122:		return AntiLambda;
  case -3112:		return AntiSigmaMinus;
  case -3212:		return AntiSigma0;
  case -3222:		return AntiSigmaPlus;
  case -3322:		return AntiXi0;
  case -3312:		return AntiXiPlus;
  case -3334:		return AntiOmegaPlus;
  case 113:			return Rho0;
  case 213:			return RhoPlus;
  case -213:			return RhoMinus;
  case 223:			return omega;
  case 331:			return EtaPrime;
  case 333:			return phiMeson;
  case 9000110:		return a0_980;
  case 9010221:		return f0_980;
  case 313: 			return KStar_892_0;
  case -313:			return AntiKStar_892_0;
  case 323:			return KStar_892_Plus;
  case -323:			return KStar_892_Minus;
  case 20323:		return K1_1400_Plus;
  case -20323:		return K1_1400_Minus;
  case 10213:		return b1_1235_Plus;
  case 45:			return Deuteron;
  case 46:			return Triton;
  case 47:			return Helium;
  case 49:			return He3;
  case 3114:			return Sigma_1385_Minus;
  case 3214:			return Sigma_1385_0;
  case 3224:			return Sigma_1385_Plus;
  case 1000030060:  return Li6;
  case 1000030070:  return Li7;
  case 1000040070:  return Be7;
  case 1000040090:  return Be9;
  case 1000050100:  return B10;
  case 1000050110:  return B11;
  case 1000060120:  return C12;
  case 1000070140:  return N14;
  case 1000080160:  return O16;
  case 1000090190:  return F19;
  case 1000100200:  return Ne20;
  case 1000110230:  return Na23;
  case 1000120240:  return Mg24;
  case 1000130270:  return Al27;
  case 1000140280:  return Si28;
  case 1000150310:  return P31;
  case 1000160320:  return S32;
  case 1000170350:  return Cl35;
  case 1000180360:  return Ar36;
  case 1000190390:  return K39;
  case 1000200400:  return Ca40;
  case 1000210450:  return Sc45;
  case 1000220480:  return Ti48;
  case 1000230510:  return V51;
  case 1000240520:  return Cr52;
  case 1000250550:  return Mn55;
  case 1000260560:  return Fe56;
  case 1000270590:  return Co59;
  case 1000280580:  return Ni58;
  case 1000290630:  return Cu63;
  case 1000300640:  return Zn64;
  case 1000320740:  return Ge74;
  case 1000340800:  return Se80;
  case 1000360840:  return Kr84;
  case 1000380880:  return Sr88;
  case 1000400900:  return Zr90;
  case 1000420980:  return Mo98;
  case 1000461060:  return Pd106;
  case 1000481140:  return Cd114;
  case 1000501200:  return Sn120;
  case 1000541320:  return Xe132;
  case 1000561380:  return Ba138;
  case 1000581400:  return Ce140;
  case 1000621520:  return Sm152;
  case 1000661640:  return Dy164;
  case 1000701740:  return Yb174;
  case 1000741840:  return W184;
  case 1000781940:  return Pt194;
  case 1000791970:  return Au197;
  case 1000802020:  return Hg202;
  case 1000822080:  return Pb208;
  case 1000922380:  return U238;
  case 2224:         return DeltaPlusPlus;
  case 443:          return Jpsi;
  case 441:          return Eta_c;
  case 10441:        return Chi_c0;
  case 20443:        return Chi_c1;
  case 445:          return Chi_c2;
  case 100443:       return Psi2s;
  case 421:          return D0;
  case -421:         return AntiD0;
  case 411:          return DPlus;
  case 423:          return Dstar0;
  case 413:          return DstarPlus;
  case 4122:         return Lambda_c;
  default:			return Unknown;
  }
}

inline static int Is_FinalStateParticle(Particle_t locPID)
{
	switch(locPID)
	{
		case Gamma:       return 1;
		case Positron:    return 1;
		case Electron:    return 1;
		case Neutrino:    return 1;
		case MuonPlus:    return 1;
		case MuonMinus:   return 1;
		case PiPlus:      return 1;
		case PiMinus:     return 1;
		case KLong:       return 1;
		case KPlus:       return 1;
		case KMinus:      return 1;
		case Neutron:     return 1;
		case Proton:      return 1;
		case AntiProton:  return 1;
		case AntiNeutron: return 1;
		case Deuteron:    return 1;
		case Triton:      return 1;
		case Helium:      return 1;
		case He3:         return 1;
        case Li6:         return 1;
        case Li7:         return 1;
        case Be7:         return 1;
        case Be9:         return 1;
        case B10:         return 1;
        case B11:         return 1;
        case C12:         return 1;
        case N14:         return 1;
        case O16:         return 1;
        case F19:         return 1;
        case Ne20:        return 1;
        case Na23:        return 1;
        case Mg24:        return 1;
        case Al27:        return 1;
        case Si28:        return 1;
        case P31:         return 1;
        case S32:         return 1;
        case Cl35:        return 1;
        case Ar36:        return 1;
        case K39:         return 1;
        case Ca40:        return 1;
        case Sc45:        return 1;
        case Ti48:        return 1;
        case V51:         return 1;
        case Cr52:        return 1;
        case Mn55:        return 1;
        case Fe56:        return 1;
        case Co59:        return 1;
        case Ni58:        return 1;
        case Cu63:        return 1;
        case Zn64:        return 1;
        case Ge74:        return 1;
        case Se80:        return 1;
        case Kr84:        return 1;
        case Sr88:        return 1;
        case Zr90:        return 1;
        case Mo98:        return 1;
        case Pd106:       return 1;
        case Cd114:       return 1;
        case Sn120:       return 1;
        case Xe132:       return 1;
        case Ba138:       return 1;
        case Ce140:       return 1;
        case Sm152:       return 1;
        case Dy164:       return 1;
        case Yb174:       return 1;
        case W184:        return 1;
        case Pt194:       return 1;
        case Au197:       return 1;
        case Hg202:       return 1;
        case Pb208:       return 1;
        case U238:        return 1;
		default: return 0; //decaying
	}
}

inline static char* Get_ShortName(Particle_t locPID)
{
	switch (locPID) {
	case Gamma:
		return (char*)"g";
	case Positron:
		return (char*)"ep";
	case Electron:
		return (char*)"em";
	case Neutrino:
		return (char*)"neut";
	case MuonPlus:
		return (char*)"mup";
	case MuonMinus:
		return (char*)"mum";
	case Pi0:
		return (char*)"pi0";
	case PiPlus:
		return (char*)"pip";
	case PiMinus:
		return (char*)"pim";
	case KLong:
		return (char*)"kl";
	case KPlus:
		return (char*)"kp";
	case KMinus:
		return (char*)"km";
	case Neutron:
		return (char*)"n";
	case Proton:
		return (char*)"p";
	case AntiProton:
		return (char*)"pbar";
	case KShort:
		return (char*)"ks";
	case Eta:
		return (char*)"eta";

	case Lambda:
		return (char*)"lamb";
	case SigmaPlus:
		return (char*)"sigp";
	case Sigma0:
		return (char*)"sig0";
	case SigmaMinus:
		return (char*)"sigm";
	case Xi0:
		return (char*)"xi0";
	case XiMinus:
		return (char*)"xim";
	case OmegaMinus:
		return (char*)"omgm";

	case AntiNeutron:
		return (char*)"nbar";
	case AntiLambda:
		return (char*)"lbar";
	case AntiSigmaMinus:
		return (char*)"sigmbar";
	case AntiSigma0:
		return (char*)"sig0bar";
	case AntiSigmaPlus:
		return (char*)"sigpbar";
	case AntiXi0:
		return (char*)"xi0bar";
	case AntiXiPlus:
		return (char*)"xipbar";
	case AntiOmegaPlus:
		return (char*)"omgpbar";

	case omega:
		return (char*)"omg";
	case EtaPrime:
		return (char*)"etap";
	case phiMeson:
		return (char*)"phi";

	case Deuteron:
		return (char*)"d";
	case Triton:
		return (char*)"tri";
	case Helium:
		return (char*)"he";
	case He3:
		return (char*)"he3";
    case Li6:
        return (char*)"li6";
    case Li7:
        return (char*)"li7";
    case Be7:
        return (char*)"be7";
    case Be9:
        return (char*)"be9";
    case B10:
        return (char*)"b10";
    case B11:
        return (char*)"b11";
    case C12:
        return (char*)"c12";
    case N14:
        return (char*)"n14";
    case O16:
        return (char*)"o16";
    case F19:
        return (char*)"f19";
    case Ne20:
        return (char*)"ne20";
    case Na23:
        return (char*)"na23";
    case Mg24:
        return (char*)"mg24";
    case Al27:
        return (char*)"al27";
    case Si28:
        return (char*)"si28";
    case P31:
        return (char*)"p31";
    case S32:
        return (char*)"s32";
    case Cl35:
        return (char*)"cl35";
    case Ar36:
        return (char*)"ar36";
    case K39:
        return (char*)"k39";
    case Ca40:
        return (char*)"ca40";
    case Sc45:
        return (char*)"sc45";
    case Ti48:
        return (char*)"ti48";
    case V51:
        return (char*)"v51";
    case Cr52:
        return (char*)"cr52";
    case Mn55:
        return (char*)"mn55";
    case Fe56:
        return (char*)"fe56";
    case Co59:
        return (char*)"co59";
    case Ni58:
        return (char*)"ni58";
    case Cu63:
        return (char*)"cu63";
    case Zn64:
        return (char*)"zn64";
    case Ge74:
        return (char*)"ge74";
    case Se80:
        return (char*)"se80";
    case Kr84:
        return (char*)"kr84";
    case Sr88:
        return (char*)"sr88";
    case Zr90:
        return (char*)"zr90";
    case Mo98:
        return (char*)"mo98";
    case Pd106:
        return (char*)"pd106";
    case Cd114:
        return (char*)"cd114";
    case Sn120:
        return (char*)"sn120";
    case Xe132:
        return (char*)"xe132";
    case Ba138:
        return (char*)"ba138";
    case Ce140:
        return (char*)"ce140";
    case Sm152:
        return (char*)"sm152";
    case Dy164:
        return (char*)"dy164";
    case Yb174:
        return (char*)"yb174";
    case W184:
        return (char*)"w184";
    case Pt194:
        return (char*)"pt194";
    case Au197:
        return (char*)"au197";
    case Hg202:
        return (char*)"hg202";
    case Pb208:
        return (char*)"pb208";
    case U238:
        return (char*)"u238";
	case Jpsi:
		return (char*)"jpsi";
	case Eta_c:
		return (char*)"etac";
	case Chi_c0:
		return (char*)"chic0";
	case Chi_c1:
		return (char*)"chic1";
	case Chi_c2:
		return (char*)"chic2";
	case Psi2s:
		return (char*)"psi2S";
	case D0:
		return (char*)"d0";
	case AntiD0:
		return (char*)"d0bar";
	case DPlus:
		return (char*)"dp";
	case Lambda_c:
		return (char*)"lambc";

	default:
		return (char*)"Unknown";
	}
}

inline static int ParticleMultiplexPower(Particle_t locPID)
{
   locPID = RemapParticleID(locPID);

	switch(locPID)
	{
		//FINAL-STATE PARTICLES (+ pi0) (decimal: 10^power):
		case Gamma:       return 0;
		case Positron:    return 1;
		case Electron:    return 2;
		case Neutrino:    return 3;
		case MuonPlus:    return 4;
		case MuonMinus:   return 5;
		case Pi0:         return 6;
		case PiPlus:      return 7;
		case PiMinus:     return 8;
		case KLong:       return 9;
		case KPlus:       return 10;
		case KMinus:      return 11;
		case Neutron:     return 12;
		case Proton:      return 13;
		case AntiProton:  return 14;
		case AntiNeutron: return 15;

		//DECAYING PARTICLES (- pi0): (binary: 2^power)
		case KShort:           return 0;
		case Eta:              return 1;
		case Lambda:           return 2;
		case SigmaPlus:        return 3;
		case Sigma0:           return 4;
		case SigmaMinus:       return 5;
		case Xi0:              return 6;
		case XiMinus:          return 7;
		case OmegaMinus:       return 8;
		case AntiLambda:       return 9;
		case AntiSigmaMinus:   return 10;
		case AntiSigma0:       return 11;
		case AntiSigmaPlus:    return 12;
		case AntiXi0:          return 13;
		case AntiXiPlus:       return 14;
		case AntiOmegaPlus:    return 15;
		case Deuteron:         return 16;
		case Rho0:             return 17;
		case RhoPlus:          return 18;
		case RhoMinus:         return 19;
		case omega:            return 20;
		case EtaPrime:         return 21;
		case phiMeson:         return 22;
		case a0_980:           return 23;
		case f0_980:           return 24;
		case KStar_892_0:      return 25;
		case KStar_892_Plus:   return 26;
		case KStar_892_Minus:  return 27;
		case AntiKStar_892_0:  return 28;
		case K1_1400_Plus:     return 29;
		case K1_1400_Minus:    return 30;
		case b1_1235_Plus:     return 31;
		case Sigma_1385_Minus: return 32;
		case Sigma_1385_0:     return 33;
		case Sigma_1385_Plus:  return 34;
		case DeltaPlusPlus:    return 35;
		case Jpsi:             return 36;
		case Eta_c:            return 37;
		case Chi_c0:           return 38;
		case Chi_c1:           return 39;
		case Chi_c2:           return 40;
		case Psi2s:            return 41;
		case D0:               return 42;
		case DPlus:            return 43;
		case Dstar0:           return 44;
		case DstarPlus:        return 45;
		case Lambda_c:         return 46;
		case AntiD0:           return 47;
		default: return -1;
	}
}

inline static Particle_t DemultiplexPID(int locBit, int locIsDecayingFlag)
{
	//FINAL-STATE PARTICLES (+ pi0) (decimal: 10^power):
	if(locIsDecayingFlag == 0)
	{
		switch(locBit)
		{
			case 0:   return Gamma;
			case 1:   return Positron;
			case 2:   return Electron;
			case 3:   return Neutrino;
			case 4:   return MuonPlus;
			case 5:   return MuonMinus;
			case 6:   return Pi0;
			case 7:   return PiPlus;
			case 8:   return PiMinus;
			case 9:   return KLong;
			case 10:  return KPlus;
			case 11:  return KMinus;
			case 12:  return Neutron;
			case 13:  return Proton;
			case 14:  return AntiProton;
			case 15:  return AntiNeutron;
			default:  return Unknown;
		}
	}

	//DECAYING PARTICLES (- pi0): (binary: 2^power)
	switch(locBit)
	{
		case 0:   return KShort;
		case 1:   return Eta;
		case 2:   return Lambda;
		case 3:   return SigmaPlus;
		case 4:   return Sigma0;
		case 5:   return SigmaMinus;
		case 6:   return Xi0;
		case 7:   return XiMinus;
		case 8:   return OmegaMinus;
		case 9:   return AntiLambda;
		case 10:  return AntiSigmaMinus;
		case 11:  return AntiSigma0;
		case 12:  return AntiSigmaPlus;
		case 13:  return AntiXi0;
		case 14:  return AntiXiPlus;
		case 15:  return AntiOmegaPlus;
		case 16:  return Deuteron;
		case 17:  return Rho0;
		case 18:  return RhoPlus;
		case 19:  return RhoMinus;
		case 20:  return omega;
		case 21:  return EtaPrime;
		case 22:  return phiMeson;
		case 23:  return a0_980;
		case 24:  return f0_980;
		case 25:  return KStar_892_0;
		case 26:  return KStar_892_Plus;
		case 27:  return KStar_892_Minus;
		case 28:  return AntiKStar_892_0;
		case 29:  return K1_1400_Plus;
		case 30:  return K1_1400_Minus;
		case 31:  return b1_1235_Plus;
		case 32:  return Sigma_1385_Minus;
		case 33:  return Sigma_1385_0;
		case 34:  return Sigma_1385_Plus;
		case 35:  return DeltaPlusPlus;
		case 36:  return Jpsi;
		case 37:  return Eta_c;
		case 38:  return Chi_c0;
		case 39:  return Chi_c1;
		case 40:  return Chi_c2;
		case 41:  return Psi2s;
		case 42:  return D0;
		case 43:  return DPlus;
		case 44:  return Dstar0;
		case 45:  return DstarPlus;
		case 46:  return Lambda_c;
      case 47:  return AntiD0;
		default:  return Unknown;
	}
}

typedef enum
{
	d_Charged,
	d_Positive,
	d_Negative,
	d_Neutral,
	d_AllCharges
} Charge_t;

inline static int Is_CorrectCharge(Particle_t locPID, Charge_t locCharge)
{
	if(locPID == Unknown)
		return (locCharge == d_AllCharges);
	int locIntCharge = ParticleCharge(locPID);
	switch(locCharge)
	{
		case d_Neutral:
			return (locIntCharge == 0);
		case d_Positive:
			return (locIntCharge > 0);
		case d_Negative:
			return (locIntCharge < 0);
		case d_Charged:
			return (locIntCharge != 0);
		case d_AllCharges:
			return 1;
		default:
			return 0;
	}
}

// Deduce particle type from charge and mass
inline static Particle_t IDTrack(float locCharge, float locMass)
{
        float locMassTolerance = 0.010;
        if (locCharge > 0.1) // Positive particles
        { 
	  if (fabs(locMass - ParticleMass(Triton)) < locMassTolerance) return Triton;
                if (fabs(locMass - ParticleMass(Deuteron)) < locMassTolerance) return Deuteron;
                if (fabs(locMass - ParticleMass(Proton)) < locMassTolerance) return Proton;
                if (fabs(locMass - ParticleMass(PiPlus)) < locMassTolerance) return PiPlus;
                if (fabs(locMass - ParticleMass(KPlus)) < locMassTolerance) return KPlus;
                if (fabs(locMass - ParticleMass(Positron)) < locMassTolerance) return Positron;
                if (fabs(locMass - ParticleMass(MuonPlus)) < locMassTolerance) return MuonPlus;
        }
        else if(locCharge < -0.1) // Negative particles
        {
                if (fabs(locMass - ParticleMass(PiMinus)) < locMassTolerance) return PiMinus;
                if (fabs(locMass - ParticleMass(KMinus)) < locMassTolerance) return KMinus;
                if (fabs(locMass - ParticleMass(MuonMinus)) < locMassTolerance) return MuonMinus;
                if (fabs(locMass - ParticleMass(Electron)) < locMassTolerance) return Electron;
                if (fabs(locMass - ParticleMass(AntiProton)) < locMassTolerance) return AntiProton;
        }
        else //Neutral Track
        {
                if (fabs(locMass - ParticleMass(Gamma)) < locMassTolerance) return Gamma;
                if (fabs(locMass - ParticleMass(Neutron)) < locMassTolerance) return Neutron;
        }
        return Unknown;
}


#endif
