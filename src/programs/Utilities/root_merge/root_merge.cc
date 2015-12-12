/*

  This macro will add histograms from a list of root files and write them
  to a target root file. The target file is newly created and must not be
  identical to one of the source files.

  Author: Sven A. Schmidt, sven.schmidt@cern.ch
  Date:   13.2.2001

  This code is based on the hadd.C example by Rene Brun and Dirk Geppert,
  which had a problem with directories more than one level deep.
  (see macro hadd_old.C for this previous implementation).
  
  The macro from Sven has been enhanced by 
     Anne-Sylvie Nicollerat <Anne-Sylvie.Nicollerat@cern.ch>
   to automatically add Trees (via a chain of trees).
  
  To use this macro, modify the file names in function hadd.
  
  NB: This macro is provided as a tutorial.
      Use $ROOTSYS/bin/hadd to merge many histogram files

 */

#include <stdlib.h>

#include <string>
#include <iostream>
#include <iomanip>
using namespace std;

#include <TChain.h>
#include <TFile.h>
#include <TH1.h>
#include <TTree.h>
#include <TKey.h>
#include <Riostream.h>

TList *FileList;
TFile *Target;

const char *OUTPUT_FILENAME = "merged.root";

void Usage(void);
void ParseCommandLineArguments(int narg, char* argv[]);
void MergeRootfile( TDirectory *target, TList *sourcelist );

//--------------
// main
//--------------
int main(int narg, char* argv[])
{
	// create empty list of input files
	FileList = new TList();

	// Parse command line to fill list
	ParseCommandLineArguments(narg, argv);

	// Open ROOT file for output
	Target = new TFile(OUTPUT_FILENAME,"RECREATE","Produced by root_merge");
	cout<<"Opened ROOT file \""<<OUTPUT_FILENAME<<"\" ..."<<endl;
	
	// Merge input files into output file
	MergeRootfile( Target, FileList );
	
	// Close output file
	Target->Write();
	delete Target;
	cout<<endl<<"Closed ROOT file: "<<OUTPUT_FILENAME<<endl;

	return 0;
}

//--------------
// Usage
//--------------
void Usage(void)
{
	cout<<endl;
	cout<<" root_merge [options] infput_file1 input_file2 input_file3 ..."<<endl;
	cout<<endl;
	cout<<"options:"<<endl;
	cout<<"     -o filename   set output filename (def:"<<OUTPUT_FILENAME<<")"<<endl;
	cout<<"     -h            Print this usage statement"<<endl;
	cout<<endl;
	cout<<endl;
	
	exit(0);
}

//--------------
// ParseCommandLineArguments
//--------------
void ParseCommandLineArguments(int narg, char* argv[])
{
	if(narg==1)Usage();

	for(int i=1; i<narg; i++){
		if(argv[i][0] == '-'){
			switch(argv[i][1]){
				case 'o':
					i++;
					OUTPUT_FILENAME = argv[i];
					break;
				case 'h':
					Usage();
					break;
				default:
					cout<<"Unknown option \""<<argv[i]<<"\". Aborting."<<endl;
					exit(-1);
			}
		}else{
			FileList->Add( TFile::Open(argv[i]));
		}
	}
}

#if 0
void hadd() {
   // in an interactive ROOT session, edit the file names
   // Target and FileList, then
   // root > .L hadd.C
   // root > hadd()
   
  Target = TFile::Open( "result.root", "RECREATE" );
  
  FileList = new TList();
  FileList->Add( TFile::Open("hsimple1.root") );
  FileList->Add( TFile::Open("hsimple2.root") );
  
  MergeRootfile( Target, FileList );

}   
#endif

//--------------
// MergeRootfile
//--------------
void MergeRootfile( TDirectory *target, TList *sourcelist ) {

  //  cout << "Target path: " << target->GetPath() << endl;
  TString path( (char*)strstr( target->GetPath(), ":" ) );
  path.Remove( 0, 2 );

  TFile *first_source = (TFile*)sourcelist->First();
  first_source->cd( path );
  TDirectory *current_sourcedir = gDirectory;

  // loop over all keys in this directory
  TChain *globChain = 0;
  TIter nextkey( current_sourcedir->GetListOfKeys() );
  TKey *key;
  while ( (key = (TKey*)nextkey())) {

    // read object from first source file
    first_source->cd( path );
    TObject *obj = key->ReadObj();

    if ( obj->IsA()->InheritsFrom( "TH1" ) ) {
      // descendant of TH1 -> merge it

      //      cout << "Merging histogram " << obj->GetName() << endl;
      TH1 *h1 = (TH1*)obj;

      // loop over all source files and add the content of the
      // correspondant histogram to the one pointed to by "h1"
      TFile *nextsource = (TFile*)sourcelist->After( first_source );
      while ( nextsource ) {
        
        // make sure we are at the correct directory level by cd'ing to path
        nextsource->cd( path );
        TH1 *h2 = (TH1*)gDirectory->Get( h1->GetName() );
        if ( h2 ) {
          h1->Add( h2 );
          delete h2; // don't know if this is necessary, i.e. if 
                     // h2 is created by the call to gDirectory above.
        }

        nextsource = (TFile*)sourcelist->After( nextsource );
      }
    }
    else if ( obj->IsA()->InheritsFrom( "TTree" ) ) {
      
      // loop over all source files create a chain of Trees "globChain"
      const char* obj_name= obj->GetName();

      globChain = new TChain(obj_name);
      globChain->Add(first_source->GetName());
      TFile *nextsource = (TFile*)sourcelist->After( first_source );
      //      const char* file_name = nextsource->GetName();
      // cout << "file name  " << file_name << endl;
     while ( nextsource ) {
     	  
       globChain->Add(nextsource->GetName());
       nextsource = (TFile*)sourcelist->After( nextsource );
     }

    } else if ( obj->IsA()->InheritsFrom( "TDirectory" ) ) {
      // it's a subdirectory

      cout << "Found subdirectory " << obj->GetName() << endl;

      // create a new subdir of same name and title in the target file
      target->cd();
      TDirectory *newdir = target->mkdir( obj->GetName(), obj->GetTitle() );

      // newdir is now the starting point of another round of merging
      // newdir still knows its depth within the target file via
      // GetPath(), so we can still figure out where we are in the recursion
      MergeRootfile( newdir, sourcelist );

    } else {

      // object is of no type that we know or can handle
      cout << "Unknown object type, name: " 
           << obj->GetName() << " title: " << obj->GetTitle() << endl;
    }

    // now write the merged histogram (which is "in" obj) to the target file
    // note that this will just store obj in the current directory level,
    // which is not persistent until the complete directory itself is stored
    // by "target->Write()" below
    if ( obj ) {
      target->cd();

      //!!if the object is a tree, it is stored in globChain...
      if (obj->IsA()->InheritsFrom( "TTree" ) && globChain)
        globChain->Write( key->GetName() );
      else
        obj->Write( key->GetName() );
    }

  } // while ( ( TKey *key = (TKey*)nextkey() ) )

  // save modifications to target file
  target->Write();

}


