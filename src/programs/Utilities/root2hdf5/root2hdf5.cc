//
// root2hdf5 - utility for automatic conversion of a ROOT tree
//             into a hdf5 data file for input into tensorflow.
//
// author: richard.t.jones at uconn.edu
// version: may 8, 2023

#include <iostream>
#include <string>
#include <sstream>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TLeaf.h>
#include <TLeafC.h>
#include <TLeafB.h>

#include "root2hdf5.h"

std::vector<column_*> columns;

hsize_t column_::rowcount(0);
hsize_t column_::chunksize(0);
std::map<TLeaf*, column_*> column_::leaves;

void usage()
{
   std::cout << "usage: root2hdf5 [options] <rootfile> <tree>" << std::endl
             << " where <rootfile> is the path to the input root file" << std::endl
             << "       <tree> is the path to the input tree in <rootfile>" << std::endl
             << " and options may be any combination of the following:" << std::endl
             << "  -?, -h, --help: print this message and exit" << std::endl
             << "  -o, --output <outfile>: name of the output hdf5 file" << std::endl
             << "  -c, --columns <colon_separated_list>: list of column names" << std::endl
             << "        from the root tree to write into the output datasets [all]" << std::endl
             << "  -n, --maxrows <maxcount>: maximum number of rows to write" << std::endl
             << "        to the output datasets [ULONG_MAX]" << std::endl
             << "  -s, --skiprows <skipcount>: rows in the input root tree" << std::endl
             << "        to skip before starting to write to output datasets [0]" << std::endl
             << "  -h, --chunksize <count>: chunksize for output datasets [1000]" << std::endl;
   exit(1);
}

int main(int argc, char *argv[])
{
   std::map<std::string, int> colnames;
   unsigned long maxrows(ULONG_MAX);
   unsigned long skiprows(0);
   unsigned long chunksize(1000);
   std::string treepath;
   std::string rootfile;
   std::string hdf5file;
   for (int i=1; i < argc; ++i) {
      if (strcmp(argv[i], "-?") == 0)
         usage();
      else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
         usage();
      else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0)
         hdf5file = argv[++i];
      else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--columns") == 0) {
         std::stringstream ss(argv[++i]);
         std::string var;
         while (std::getline(ss, var, ':'))
            colnames[var] = 1;
      }
      else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--maxrows") == 0)
         maxrows = std::atoi(argv[++i]);
      else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--skiprows") == 0)
         skiprows = std::atoi(argv[++i]);
      else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--chunksize") == 0)
         chunksize = std::atoi(argv[++i]);
      else if (rootfile.size() == 0)
         rootfile = argv[i];
      else if (treepath.size() == 0)
         treepath = argv[i];
      else
         usage();
   }
   if (rootfile.size() == 0)
      usage();
   else if (treepath.size() == 0)
      usage();
   else if (hdf5file.size() == 0) {
      if (treepath.rfind('/') == treepath.npos)
         hdf5file = treepath + ".hdf5";
      else
         hdf5file = treepath.substr(treepath.rfind('/')) + ".hdf5";
   }

   TFile *fin = TFile::Open(rootfile.c_str(), "read");
   TTree *tree = (TTree*)fin->Get(treepath.c_str());
   if (tree == 0) {
      std::cerr << "Error in root2hdf5 - tree " << treepath
                << " not found in " << rootfile << std::endl;
      exit(2);
   }

   hid_t fout_id = H5Fcreate(hdf5file.c_str(), H5F_ACC_TRUNC,
                             H5P_DEFAULT, H5P_DEFAULT);

   unsigned long nentries(tree->GetEntries());
   unsigned long maxentries(skiprows + maxrows);
   maxentries = (maxentries < nentries)? maxentries : nentries;
   column_::setRowCount(maxentries - skiprows);
   unsigned long maxchunk = maxentries - skiprows;
   chunksize = (chunksize < maxchunk)? chunksize : maxchunk;
   column_::setChunkSize(chunksize);
   TObjArray *leaves = tree->GetListOfLeaves();
   for (int ileaf=0; ileaf < leaves->GetEntries(); ++ileaf) {
      TLeaf *leaf = (TLeaf*)(*leaves)[ileaf];
      const char *colname = leaf->GetBranch()->GetName();
      if (colnames.size() > 0 && colnames.find(colname) == colnames.end())
         continue;
      else if (leaf->IsA() == TLeafC::Class())
         columns.push_back(new column_cstring(leaf));
      else if (leaf->IsA() == TLeafB::Class())
         columns.push_back(new column_int8(leaf));
      else if (strcmp(leaf->GetTypeName(), "UChar_t") == 0)
         columns.push_back(new column_uint8(leaf));
      else if (strcmp(leaf->GetTypeName(), "Short_t") == 0)
         columns.push_back(new column_int16(leaf));
      else if (strcmp(leaf->GetTypeName(), "UShort_t") == 0)
         columns.push_back(new column_uint16(leaf));
      else if (strcmp(leaf->GetTypeName(), "Int_t") == 0)
         columns.push_back(new column_int32(leaf));
      else if (strcmp(leaf->GetTypeName(), "UInt_t") == 0)
         columns.push_back(new column_uint32(leaf));
      else if (strcmp(leaf->GetTypeName(), "Float_t") == 0)
         columns.push_back(new column_float(leaf));
      else if (strcmp(leaf->GetTypeName(), "Float16_t") == 0)
         columns.push_back(new column_float(leaf));
      else if (strcmp(leaf->GetTypeName(), "Double_t") == 0)
         columns.push_back(new column_double(leaf));
      else if (strcmp(leaf->GetTypeName(), "Double32_t") == 0)
         columns.push_back(new column_double(leaf));
      else if (strcmp(leaf->GetTypeName(), "Long64_t") == 0)
         columns.push_back(new column_int64(leaf));
      else if (strcmp(leaf->GetTypeName(), "ULong64_t") == 0)
         columns.push_back(new column_uint64(leaf));
      else if (strcmp(leaf->GetTypeName(), "Long_t") == 0)
         columns.push_back(new column_int64(leaf));
      else if (strcmp(leaf->GetTypeName(), "ULong_t") == 0)
         columns.push_back(new column_uint64(leaf));
      else if (strcmp(leaf->GetTypeName(), "Bool_t") == 0)
         columns.push_back(new column_bool(leaf));
      else {
         std::cerr << "root2hdf5 error - unknown leaf type " << leaf->GetTypeName()
                   << " found in tree column " << colname
                   << ", cannot continue." << std::endl;
         exit(3);
      }
   }

   unsigned long int bytecount(0);
   unsigned long nrow = 0;
   for (unsigned long entry=0; entry < (unsigned long)tree->GetEntries(); ++entry) {
      if (entry < skiprows)
         continue;
      else if (entry > maxentries)
          break;
      std::vector<column_*>::iterator icolumn;
      for (icolumn = columns.begin(); icolumn != columns.end(); ++icolumn) {
         (*icolumn)->leaf->GetBranch()->SetAddress((*icolumn)->address(nrow));
      }
      tree->GetEntry(entry);
      if (++nrow == column_::chunksize || entry == maxentries - 1) {
         for (icolumn = columns.begin(); icolumn != columns.end(); ++icolumn) {
            int varcount(nrow);
            if ((*icolumn)->vleaf) {
               varcount = nrow * 32 / ((*icolumn)->dsize() * (*icolumn)->width);
               for (unsigned int i=0; i < nrow; ++i)
                  varcount += column_::leaves[(*icolumn)->vleaf]->getvalue(i);
            }
            bytecount += (*icolumn)->dsize() * (*icolumn)->width * varcount;
            (*icolumn)->write(fout_id, nrow);
         }
         nrow = 0;
      }
   }

   std::vector<column_*>::iterator icolumn;
   for (icolumn = columns.begin(); icolumn != columns.end(); ++icolumn) {
      std::cout << "(" << (*icolumn)->leaf->GetTypeName() << ")" 
                << (*icolumn)->getname();
      if ((*icolumn)->width > 1)
         std::cout << "[" << (*icolumn)->width << "]";
      else if ((*icolumn)->vleaf != 0)
         std::cout << "[" << (*icolumn)->vleaf->GetName() << "]";
      std::cout << " has " << (*icolumn)->dset_extent << " rows" << std::endl;
      delete *icolumn;
   }
   std::cout << "total of " << bytecount << " bytes written" << std::endl;
   H5Fclose(fout_id);
}
