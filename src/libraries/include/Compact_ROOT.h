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
      typename std::map<TH*, T>::iterator iter;
      for (iter = mothers.begin(); iter != mothers.end(); ++iter) {
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
      typename std::map<TH*, T>::iterator iter;
      for (iter = mothers.begin(); iter != mothers.end(); ++iter) {
         if (iter->first->GetNbinsX() == nbinsx &&
             iter->first->GetNbinsY() == nbinsy)
         {
            ++iter->second;
            return iter->first;
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
      typename std::map<TH*, T>::iterator iter;
      for (iter = mothers.begin(); iter != mothers.end(); ++iter) {
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
   void fill_mother(TH* mother, double xlim[2]) {
      mother->SetName(fName);
      mother->SetTitle(fTitle);
      mother->GetXaxis()->SetLimits(xlim[0], xlim[1]);
      replay(mother);
   }
   void fill_mother(TH* mother, double xlim[2], double ylim[2]) {
      mother->SetName(fName);
      mother->SetTitle(fTitle);
      mother->GetXaxis()->SetLimits(xlim[0], xlim[1]);
      mother->GetYaxis()->SetLimits(ylim[0], ylim[1]);
      replay(mother);
   }
   void fill_mother(TH* mother, double xlim[2], double ylim[2], double zlim[2]) {
      mother->SetName(fName);
      mother->SetTitle(fTitle);
      mother->GetXaxis()->SetLimits(xlim[0], xlim[1]);
      mother->GetYaxis()->SetLimits(ylim[0], ylim[1]);
      mother->GetZaxis()->SetLimits(zlim[0], zlim[1]);
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
   virtual void fill_bin(int bin, double w) = 0;
   void replay(TH* mother) {
      mother->Sumw2(fSumw2);
      mother->Reset();
      typename std::map<T, std::vector<unsigned char> >::iterator ib;
      for (ib = fDataByte.begin(); ib != fDataByte.end(); ++ib) {
         for (int i=0; i < ib->second.size(); ++i) {
            fill_bin(ib->second[i], ib->first);
         }
      }
      typename std::map<T, std::vector<unsigned short> >::iterator is;
      for (is = fDataShort.begin() ; is != fDataShort.end(); ++is) {
         for (int i=0; i < is->second.size(); ++i) {
            fill_bin(is->second[i], is->first);
         }
      }
      typename std::map<T, std::vector<unsigned int> >::iterator ii;
      for (ii = fDataInt.begin() ; ii != fDataInt.end(); ++ii) {
         for (int i=0; i < ii->second.size(); ++i) {
            fill_bin(ii->second[i], ii->first);
         }
      }
      typename std::map<T, std::vector<unsigned long> >::iterator il;
      for (il = fDataLong.begin() ; il != fDataLong.end(); ++il) {
         for (int i=0; i < il->second.size(); ++i) {
            fill_bin(il->second[i], il->first);
         }
      }
   }
   virtual int Write(const char *name=0, int option=0, int bufsize=0) = 0;
};

// Wrappers for ROOT histogram classes

class Compact_TH2I : public Compact_TH<TH2I, int> {
 public:
   Compact_TH2I(std::string name, std::string title, 
                int nbinsx, double xlow, double xup,
                int nbinsy, double ylow, double yup)
   : Compact_TH<TH2I, int>(name, title),
     fXlim{xlow,xup}, fYlim{ylow,yup}, fStats{0}
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
      fMother->GetXaxis()->SetLimits(fXlim[0], fXlim[1]);
      fMother->GetYaxis()->SetLimits(fYlim[0], fYlim[1]);
      if (w != 0) {
         fMother->PutStats(&fStats.sumw);
         fill(fMother->Fill(x,y),w);
         fMother->GetStats(&fStats.sumw);
      }
      else {
         fill(fMother->Fill(x,y),w);
      }
   }
   virtual void fill_bin(int bin, double w) {
      int ix, iy, iz;
      fMother->GetBinXYZ(bin, ix, iy, iz);
      double x = fMother->GetXaxis()->GetBinCenter(ix);
      double y = fMother->GetYaxis()->GetBinCenter(iy);
      fMother->Fill(x,y,w);
   }
   virtual int Write(const char *name=0, int option=0, int bufsize=0) {
      fill_mother(fMother, fXlim, fYlim);
      fMother->PutStats(&fStats.sumw);
      return fMother->Write(name, option, bufsize);
   }
   TH2I* Clone(const char* newname) {
      fill_mother(fMother, fXlim, fYlim);
      fMother->PutStats(&fStats.sumw);
      return (TH2I*)fMother->Clone(newname);
   }

 protected:
   Compact_TH2I() {}

 private:
   TH2I *fMother;
   double fXlim[2];
   double fYlim[2];
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
