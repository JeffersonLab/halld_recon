//
// CompactRoot.h - a set of thin wrappers around ROOT a subset
//                 of ROOT histogram types that reduces the memory
//                 footprint in a context of many instances that
//                 share the same size and shape.
//
// author: richard.t.jones at uconn.edu
// version: december 26, 2020
//
// usage:    #include <Compact_ROOT.h>
//           #if USE_COMPACT_TH2I
//           #  define TH2I Compact_TH2I
//           #endif
//
// programmer's notes:
// 1) The use case for these wrapper classes is when a large number
//    of similar histograms is being created for monitoring analysis
//    results from a large data set being processed in parallel. In
//    such a case, a single thread only contains a small fraction of
//    the total sample statistics, and the occupation of bins within
//    a given histogram in memory can be low. In this case one can
//    save a lot of memory by buffering calls to Fill during data
//    processing, and then instantiating and filling the ROOT object
//    when it is being written to a file at the end of the thread.
// 2) The basic idea is to share a single instance of the underlying
//    ROOT histogram object of a given type, size, and shape between
//    multiple similar objects in memory. Calls to Fill on any given
//    instance of this class are saved in a memory buffer, and then
//    replayed into the shared ROOT object at the point it is written
//    out or displayed.
// 3) Compact_XXX object methods have the same thread safety features
//    as the underlying ROOT objects, ie. they must be interlocked
//    against conflicting updates in a multithreaded runtime context.
// 4) Not all ROOT histogram types have been implemented, just the
//    ones that appeared to be most useful to the author. It can be
//    easily extended to new types by imitation.

#ifndef _CompactRoot_h
#define _CompactRoot_h

#include <string>
#include <vector>
#include <map>

#include <TROOT.h>
#include <TDirectory.h>
#include <TH2I.h>

// Base class for CompactRoot objects

template <class TH, class T>
class Compact_TH : public TNamed {
 protected:
   Compact_TH() {}
   bool fSumw2;
   TDirectory *fDirectory;
   std::map<T, std::vector<unsigned char> > fDataByte;
   std::map<T, std::vector<unsigned short> > fDataShort;
   std::map<T, std::vector<unsigned int> > fDataInt;
   std::map<T, std::vector<unsigned long> > fDataLong;

 public:
   Compact_TH(std::string name, std::string title)
    : TNamed(name.c_str(), title.c_str()),
      fSumw2(TH1::GetDefaultSumw2()),
      fDirectory(0)
   {
      if (TH1::AddDirectoryStatus()) {
         fDirectory = gDirectory;
         if (fDirectory) {
            fDirectory->Append(this,kTRUE);
         }
      }
   }
   virtual ~Compact_TH() {
      if (fDirectory)
         fDirectory->Remove(this);
   }
   void Sumw2(bool flag=true) {
      fSumw2 = flag;
   }
   static std::map<TH*, T> &get_mothers() {
      static std::map<TH*, T> mothers;
      return mothers;
   }
   TH* find_mother(std::string name, std::string title, int nbinsx) {
      std::map<TH*, T> &mothers = get_mothers();
      for (auto iter : mothers) {
         if (iter->first->GetNbinsX() == nbinsx) {
            ++iter->second;
            return iter->first;
         }
      }
      TH* mother;
      if (TH1::AddDirectoryStatus()) {
         TH1::AddDirectory(kFALSE);
         mother = new TH(name.c_str(), title.c_str(), nbinsx, 0., 1.);
         TH1::AddDirectory(kTRUE);
      }
      else {
         mother = new TH(name.c_str(), title.c_str(), nbinsx, 0., 1.);
      }
      mothers[mother] = 1;
      return mother;
   }
   TH* find_mother(std::string name, std::string title, int nbinsx,
                                                        int nbinsy) {
      std::map<TH*, T> &mothers = get_mothers();
      for (auto iter : mothers) {
         if (iter.first->GetNbinsX() == nbinsx &&
             iter.first->GetNbinsY() == nbinsy)
         {
            ++iter.second;
            return iter.first;
         }
      }
      TH* mother;
      if (TH1::AddDirectoryStatus()) {
         TH1::AddDirectory(kFALSE);
         mother = new TH(name.c_str(), title.c_str(),
                         nbinsx, 0., 1., nbinsy, 0., 1.);
         TH1::AddDirectory(kTRUE);
      }
      else {
         mother = new TH(name.c_str(), title.c_str(),
                         nbinsx, 0., 1., nbinsy, 0., 1.);
      }
      mothers[mother] = 1;
      return mother;
   }
   TH* find_mother(std::string name, std::string title, int nbinsx,
                                                        int nbinsy,
                                                        int nbinsz) {
      std::map<TH*, T> &mothers = get_mothers();
      for (auto iter : mothers) {
         if (iter->first->GetNbinsX() == nbinsx &&
             iter->first->GetNbinsY() == nbinsy &&
             iter->first->GetNbinsZ() == nbinsz)
         {
            ++iter->second;
            return iter->first;
         }
      }
      TH* mother;
      if (TH1::AddDirectoryStatus()) {
         TH1::AddDirectory(kFALSE);
         mother = new TH(name.c_str(), title.c_str(),
                         nbinsx, 0., 1., nbinsy, 0., 1., nbinsz, 0., 1.);
         TH1::AddDirectory(kTRUE);
      }
      else {
         mother = new TH(name.c_str(), title.c_str(),
                         nbinsx, 0., 1., nbinsy, 0., 1., nbinsz, 0., 1.);
      }
      mothers[mother] = 1;
      return mother;
   }
   void fill_mother(TH* mother, TAxis &xaxis) {
      mother->SetName(fName);
      mother->SetTitle(fTitle);
      mother->GetXaxis()->Copy(xaxis);
      replay(mother);
   }
   void fill_mother(TH* mother, TAxis &xaxis, TAxis &yaxis) {
      mother->SetName(fName);
      mother->SetTitle(fTitle);
      mother->GetXaxis()->Copy(xaxis);
      mother->GetYaxis()->Copy(yaxis);
      replay(mother);
   }
   void fill_mother(TH* mother, TAxis &xaxis, TAxis &yaxis, TAxis &zaxis) {
      mother->SetName(fName);
      mother->SetTitle(fTitle);
      mother->GetXaxis()->Copy(xaxis);
      mother->GetYaxis()->Copy(yaxis);
      mother->GetZaxis()->Copy(zaxis);
      replay(mother);
   }
   void fill(unsigned int bin, double wgt) {
      if (bin <= 0xff)
         fDataByte[wgt].push_back(bin);
      else if (bin <= 0xffff)
         fDataShort[wgt].push_back(bin);
      else if (bin <= 0xffffffff)
         fDataInt[wgt].push_back(bin);
      else
         fDataLong[wgt].push_back(bin);
   }
   void clear_bin(unsigned int bin) {
      for (auto ib : fDataByte) {
         for (int i=ib.second.size(); i > 0;) {
            if (ib.second[--i] == bin) {
               ib.second.erase(ib.second.begin() + i);
            }
         }
      }
      for (auto is : fDataShort) {
         for (int i=is.second.size(); i > 0;) {
            if (is.second[--i] == bin) {
               is.second.erase(is.second.begin() + i);
            }
         }
      }
      for (auto ii : fDataInt) {
         for (int i=0; i < ii.second.size(); ++i) {
            if (ii.second[--i] == bin) {
               ii.second.erase(ii.second.begin() + i);
            }
         }
      }
      for (auto il : fDataLong) {
         for (int i=0; i < il.second.size(); ++i) {
            if (il.second[--i] == bin) {
               il.second.erase(il.second.begin() + i);
            }
         }
      }
   }
   virtual void fill_bin(int bin, double w) = 0;
   void replay(TH* mother) {
      mother->Sumw2(fSumw2);
      mother->Reset();
      for (auto ib : fDataByte) {
         for (int i=0; i < ib.second.size(); ++i) {
            fill_bin(ib.second[i], ib.first);
         }
      }
      for (auto is : fDataShort) {
         for (int i=0; i < is.second.size(); ++i) {
            fill_bin(is.second[i], is.first);
         }
      }
      for (auto ii : fDataInt) {
         for (int i=0; i < ii.second.size(); ++i) {
            fill_bin(ii.second[i], ii.first);
         }
      }
      for (auto il : fDataLong) {
         for (int i=0; i < il.second.size(); ++i) {
            fill_bin(il.second[i], il.first);
         }
      }
   }
   virtual int Write(const char *name=0, int option=0, int bufsize=0) = 0;
   operator TH2*() { return (TH2*)this; }
};

// Wrappers for ROOT histogram classes

class Compact_TH2I : public Compact_TH<TH2I, int> {
 public:
   Compact_TH2I(std::string name, std::string title, 
                int nbinsx, double xlow, double xup,
                int nbinsy, double ylow, double yup)
   : Compact_TH<TH2I, int>(name, title),
     fXaxis(nbinsx, xlow, xup),
     fYaxis(nbinsy, ylow, yup),
     fStats{0}
   {
      fMother = find_mother(name, title, nbinsx, nbinsy);
   }
   Compact_TH2I(std::string name, std::string title, 
                int nbinsx, double *xbounds,
                int nbinsy, double *ybounds)
   : Compact_TH<TH2I, int>(name, title),
     fXaxis(nbinsx, xbounds),
     fYaxis(nbinsy, ybounds),
     fStats{0}
   {
      fMother = find_mother(name, title, nbinsx, nbinsy);
   }

   virtual ~Compact_TH2I() {
      std::map<TH2I*, int> &mothers = get_mothers();
      if (--mothers[fMother] == 0) {
         mothers.erase(fMother);
         delete fMother;
      }
   }
   void Fill(double x, double y, int w=1) {
      fMother->GetXaxis()->Copy(fXaxis);
      fMother->GetYaxis()->Copy(fYaxis);
      if (fStats.sumw == 0)
         fStats.sumw = 1e-299;
      fMother->PutStats(&fStats.sumw);
      fill(fMother->Fill(x,y),w);
      fMother->GetStats(&fStats.sumw);
   }
   virtual void fill_bin(int bin, double w) {
      int ix, iy, iz;
      fMother->GetBinXYZ(bin, ix, iy, iz);
      double x = fMother->GetXaxis()->GetBinCenter(ix);
      double y = fMother->GetYaxis()->GetBinCenter(iy);
      fMother->Fill(x,y,w);
   }
   virtual int Write(const char *name=0, int option=0, int bufsize=0) {
      fill_mother(fMother, fXaxis, fYaxis);
      fMother->PutStats(&fStats.sumw);
      return fMother->Write(name, option, bufsize);
   }
   TObject* Clone(const char* newname) {
      fill_mother(fMother, fXaxis, fYaxis);
      fMother->PutStats(&fStats.sumw);
      return (TH2I*)fMother->Clone(newname);
   }
   virtual TAxis *GetXaxis() { return &fXaxis; }
   virtual TAxis *GetYaxis() { return &fYaxis; }
   virtual void SetXTitle(const char *title) {
      fXaxis.SetTitle(title);
   }
   virtual void SetYTitle(const char *title) {
      fYaxis.SetTitle(title);
   }
   virtual void LabelsOption(Option_t *option="h") {
      fXaxis.LabelsOption(option);
   }

   virtual int GetBinContent(unsigned int bin) {
      fill_mother(fMother, fXaxis, fYaxis);
      return fMother->GetBinContent(bin);
   }
   virtual int GetBinContent(unsigned int binx, unsigned int biny) {
      fill_mother(fMother, fXaxis, fYaxis);
      return fMother->GetBinContent(binx, biny);
   }
   virtual void SetBinContent(unsigned int bin, double val) {
      fill_mother(fMother, fXaxis, fYaxis);
      if (fMother->GetBinContent(bin) != 0) {
         clear_bin(bin);
      }
      fill_bin(bin, val);
   }
   virtual void SetBinContent(unsigned int binx, unsigned int biny, double val) {
      fMother->GetXaxis()->Copy(fXaxis);
      fMother->GetYaxis()->Copy(fYaxis);
      SetBinContent(fMother->GetBin(binx, biny), val);
   }
   TH1D *ProjectionX(const char *name="_px", int firstybin=0, int lastybin=-1, Option_t *option="") {
      fill_mother(fMother, fXaxis, fYaxis);
      return fMother->ProjectionX(name, firstybin, lastybin, option);
   }
   TH1D *ProjectionY(const char *name="_py", int firstybin=0, int lastybin=-1, Option_t *option="") {
      fill_mother(fMother, fXaxis, fYaxis);
      return fMother->ProjectionY(name, firstybin, lastybin, option);
   }

   static TClass *Class() {
      static TClass myclass("Compact_TH2I");
      return &myclass;
   }
   TClass *IsA() {
      return Class();
   }
   bool InheritsFrom(const char *name) {
      if (strstr(name, "Compact_TH2I") == 0)
         return true;
      else if (strstr(name, "TH2I") == 0)
         return true;
      return TNamed::InheritsFrom(name);
   }

 protected:
   Compact_TH2I() {}

 private:
   TH2I *fMother;
   TAxis fXaxis;
   TAxis fYaxis;
   struct TH2_stats_t {
      double sumw;
      double sumw2;
      double sumwx;
      double sumwx2;
      double sumwy;
      double sumwy2;
      double sumwxy;
   } fStats;
};


#endif
