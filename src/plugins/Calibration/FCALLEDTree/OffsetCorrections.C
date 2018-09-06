#include "TString.h"
#include "TTree.h"

#include <limits>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;


// *******************************************************************************************
// *** Script to find +/-4 ns timing offsets in FCAL ***
// *** Script goes through a user-provided list of runs, and searches for 4 ns timing offsets for the FCAL ***
// *** Script creates a text file for each run, stating which crates, slots and channels are offset and in which direction ***
// *******************************************************************************************


void OffsetCorrections()
{
    
    
    // ****************************************************************************************
    // *** SECTION 1: CRATE SORTING ***
    // ****************************************************************************************
    
    // *** MapFile contains info on which chan/position is in which crate ***
    ifstream MapFile("/w/halld-scifs17exp/home/gleasonc/Software/halld_recon/src/plugins/Calibration/FCALLEDTree/CrateOrder.txt");
    string line;
    
    // *** Variables to hold values from MapFile ***
    int xx = 0;
    int yy = 0;
    int crate= 0;
    int slot = 0;
    
    // *** Variables used for keeping track of data in MapFile ***
    int TrackLine = 1;
    
    int TrackCrate = 1;
    int TrackChan = 1;
    int TrackPositions = 1;
    
    string Crates;
    string Chans;
    string Positions;
    
    
    // *** Holds value of crate/slot number for each x,y position ***
    int xyCrates[59][59] = {0};
    int xySlots[59][59] = {0};
    
    // *** Hold crate and slot numbers for each channel ***
    int ChannelCrates[2800] = {0};
    int ChannelSlots[2800] = {0};
    
    // *** Go through MapFile, find x,y positions and their corresponding crates ***
    while ( !MapFile.eof() )
    {
        // *** If statement selects crate/slot/chan section of line in MapFile ***
        if ( (TrackLine + 2)%3 == 0)
        {
            
            MapFile >> Crates;
            TrackCrate++;
            //cout << "Crates: " << Crates << endl;
            
            stringstream stream(Crates);
            int i;
            
            vector<int> vect1;
            
            
            while ( stream >> i)
            {
                vect1.push_back(i);
                
                if ( stream.peek() == '/' )
                {
                    stream.ignore();
                }
                
            } // *** end while i ***
            
            
            for (i = 0; i < vect1.size(); i++)
            {
                
                if ( (i+3) % 3 == 0 )
                {
                    //cout << "Crates = " << vect1.at(i) << endl;
                    crate = vect1.at(i);
                    
                }
                if ( (i+2) % 3 == 0 )
                {
                    slot = vect1.at(i);
                    //cout << "slot = " << vect1.at(i) << endl;
                }
                if ( (i+1) % 3 == 0 )
                {
                    //cout << "'chan' = " << vect1.at(i) << endl;
                    // *** This loop does nothing but cycle through the unused string section ***
                }
                
            } // *** end for i ***
            
        } // *** end if TrackLine + 2 ***
        
        // *** selects channel_no section in MapFile ***
        if ( (TrackLine + 1) % 3 == 0 )
        {
            MapFile >> Chans;
            TrackChan++;
            //cout << "Chans: " << Chans << endl;
        } // *** End if trackLine + 1 ***
        
        // *** Selects x/y section of MapFile ***
        if ( (TrackLine) % 3 == 0 )
        {
            MapFile >> Positions;
            TrackPositions++;
            //cout << "Positions: " << Positions << endl;
            
            vector<int> vect;
            
            stringstream stream(Positions);
            int n;
            
            while( stream >> n )
            {
                vect.push_back(n);
                if ( stream.peek() == '/' )
                {
                    stream.ignore();
                }
            } // *** end while stream >> n ***
            
            for (n = 0; n < vect.size(); n++)
            {
                if (n%2 == 0)
                {
                    xx = vect.at(n);
                }
                else
                {
                    yy = vect.at(n);
                }
            } // *** end for n ***
            
            xyCrates[xx+29][yy+29] = crate;
            xySlots[xx+29][yy+29] = slot;
        } // *** end if trackline + 0 ***
        TrackLine++;
    } // *** End of for MapFile while ***

    // **************************************************************************
    // *** SECTION 2: CONSTRUCTING REFERENCE TIMES ARRAY ***
    // **************************************************************************
    
    // *** Change this to prompt for txt file ***
    string UserRunList;
    cout << "Please enter path to a text file containing list of run paths (run path example: /path/to/run/Run04000/led_Run04000.root): ";
    cin >> UserRunList;
    
    string UserOutputDirectory;
    cout << "Please enter path to the directory for output: ";
    cin >> UserOutputDirectory;
    
    ifstream RunList(UserRunList);
    string line2;
    
    // *** Declare variables for values in fcalBlockHits tree ***
    int nHits = 0;
    int chan[7000] = {0};
    float x[7000] = {0};
    float y[7000] = {0};
    float t[7000] = {0};
    int run = 0;
    float eTot = 0;
    
    int HowManyRuns = 0;
    
    // *** Save channel locations, convenient later ***
    int chanPosition[59][59] = {0};
    double chanXPos[2800] = {0};
    double chanYPos[2800] = {0};
    
    // *** Count how many runs are listed in the file selected
    while ( !RunList.eof() )
    {
        RunList >> line2;
        ++HowManyRuns;
    }
    
    // *** So we can start from top of file again later ***
    RunList.close();
    
    cout << "There are " << HowManyRuns << " in selected file" << endl;
    
    // *** Array to hold 'good time' values ***
    double GoodTimesArray[2800] = {0};
    double GoodCratesArray[12] = {0};
    double GoodSlotsArray[13][18] = {0};
    
    
    // *** For calculating ***
    double ChanTimeAvg[HowManyRuns][2800];
    double CrateAvg[HowManyRuns][12];
    double SlotAvg[HowManyRuns][13][18];
    
    memset( ChanTimeAvg, 0, sizeof(int)*HowManyRuns*2800 );
    memset( CrateAvg, 0, sizeof(int)*HowManyRuns*12 );
    memset( SlotAvg, 0, sizeof(int)*HowManyRuns*13*18 );
    
    // *** Same file as before ***
    ifstream RunList2(UserRunList);
    
    // *** Run over all files in user selected txt file, find good time value for each channel and save in array GoodTimesArray ***
    for (int iRun = 0; iRun < HowManyRuns; ++iRun)
    {
        RunList2 >> line;
        TString fcalFileNamer( line );
        
        TFile *fcalFile = new TFile (fcalFileNamer);
        cout << "Current file: " << fcalFileNamer << endl;
        
        TTree *fcalTree = (TTree*)fcalFile->Get("fcalBlockHits");
        
        fcalTree->SetBranchAddress("chan", chan);
        fcalTree->SetBranchAddress("nHits", &nHits);
        fcalTree->SetBranchAddress("x", x);
        fcalTree->SetBranchAddress("y", y);
        fcalTree->SetBranchAddress("t", t);
        fcalTree->SetBranchAddress("run", &run);
        fcalTree->SetBranchAddress("eTot", &eTot);
        
        // *** Calculation variables ***
        double ChanTimeAdder[2800] = {0};
        double ChanTimeCounter[2800] = {0};
        
        double CrateAdder[12] = {0};
        double CrateCounter[12] = {0};
        
        double SlotAdder[13][18] = {0};
        double SlotCounter[13][18] = {0};
        
        // *** For loop goes through, adds up times for use in average calculations ***
        for (int iEntry = 0; iEntry < fcalTree->GetEntries(); ++iEntry)
        {
            fcalTree->GetEntry(iEntry);
            
            if (iEntry%10000 == 0)
            {
                cout << iEntry << " events processed" << endl;
            }
            
            for (int iHits = 0; iHits < nHits; ++iHits)
            {
                
                int row = (int)round((x[iHits] + 116)/4.0157);
                int column = (int)round((y[iHits] + 116)/4.0157);
                
                chanPosition[row][column] = chan[iHits];
                
                chanXPos[ chan[iHits] ] = row;
                chanYPos[ chan[iHits] ] = column;
                
                if ( eTot > 0  && t[iHits] > 0 )
                {
                    
                    ChanTimeAdder[ chan[iHits] ] += t[iHits];
                    ChanTimeCounter[ chan[iHits] ]++;
                    
                    CrateAdder[ xyCrates[row][column] ] += t[iHits];
                    CrateCounter[ xyCrates[row][column] ] ++;
                    
                    SlotAdder[ xyCrates[row][column] ][ xySlots[row][column] ] += t[iHits];
                    SlotCounter[ xyCrates[row][column] ][ xySlots[row][column] ] ++;
                    
                    // *** holding values for crate and slot for each channel for later ***
                    ChannelCrates[ chan[iHits] ] = xyCrates[row][column];
                    ChannelSlots[ chan[iHits] ] = xySlots[row][column];
                }
            } // *** End of iHits for loop ***
        } // *** End of iEntry for loop ***
        
        // *** Calculating averages ***
        
        for (int iChan = 0; iChan < 2800; ++iChan)
        {
            if (ChanTimeCounter[iChan] != 0)
            {
                ChanTimeAvg[iRun][iChan] = ChanTimeAdder[iChan] / ChanTimeCounter[iChan];
                
            } // ** End if ChanTimeCounter **
        }
        
        for (int iCrate = 0; iCrate < 12; iCrate++)
        {
            if (CrateCounter[iCrate] != 0)
            {
                CrateAvg[iRun][iCrate] = CrateAdder[iCrate] / CrateCounter[iCrate];
            }
            
            // *** Testing purposes ***
            cout << "CrateAdder[" << iCrate << "] = " << CrateAdder[iCrate] << " and CrateCounter[" << iCrate << "] = " << CrateCounter[iCrate] << endl;
            cout << "CrateAvg[" << iRun << "][" << iCrate << "] = " << CrateAvg[iRun][iCrate] << endl;
            
            
            for (int iSlot = 3; iSlot < 19; iSlot++)
            {
                
                if ( SlotCounter[iCrate][iSlot] != 0 )
                {
                    SlotAvg[iRun][iCrate][iSlot] = SlotAdder[iCrate][iSlot] / SlotCounter[iCrate][iSlot];
                }
                //cout << "SlotAdder[" << iCrate << "][" << iSlot << "] = " << SlotAdder[iCrate][iSlot] << " and SlotCounter[" << iCrate << "][" << iSlot << "] = " << SlotCounter[iCrate][iSlot] << endl;
                //cout << "SlotAvg[" << iRun << "][" << iCrate << "][" << iSlot << "] = " << SlotAvg[iRun][iCrate][iSlot] << "\n" << endl;
            }
        } // *** End for (iCrate) ***
    } // *** End iRun ***
    
    RunList2.close(); // *** Done with text file for now ***
    
    // *** File to hold good times for slots and such ***
    TString GoodTimesFileNamer =  UserOutputDirectory;
    GoodTimesFileNamer += "/";
    GoodTimesFileNamer += "GoodTimesData.txt";
    
    ofstream GoodTimesFile;
    GoodTimesFile.open(GoodTimesFileNamer);

    // *** Histograms will be used several times, once for each channel/crate/slot. Histograms are filled with time values and most common time value is selected as 'good time' ***
    TH1* channelTimes = new TH1F("channelTimes", "Chan times", 2000, 0, 100);
    TH1* crateTimes = new TH1F("crateTimes", "crate times", 2000, 0, 100);
    TH1* slotTimes = new TH1F("slotTimes", "slot times", 2000, 0, 100);
    
    GoodTimesFile  << "Crate GoodTime" << endl;
    
    // *** Find good times for crates ***
    
    for (int iCrates = 0; iCrates < 12; ++iCrates)
    {
        
        for (int iRuns = 0; iRuns < HowManyRuns; ++iRuns)
        {
            
            if ( CrateAvg[iRuns][iCrates] != 0 )
            {
                crateTimes->Fill( CrateAvg[iRuns][iCrates] );
            }
            
        } //*** end iRuns ***
        
        // *** Selecting the most occuring time as the 'good time' for a crate ***
        double maxCrates = crateTimes->GetMaximumBin();
        
        // *** The 1/5 factor is because we have bins in the crateTimes histogram such that every .2 ns is a bin, so 5 bins per ns ***
        GoodCratesArray[iCrates] = ( maxCrates / 20 );
        
        // *** Testing purposes ***
        cout << "GoodCratesArray[" << iCrates << "] = " << GoodCratesArray[iCrates] << "\n" << endl;
        
        GoodTimesFile << ( iCrates + 1 ) << "    " << GoodCratesArray[iCrates] << endl;
        
        crateTimes->Reset(); // *** Clear out histogram so its good for next crate ***
    } // *** End iCrates ***
    
    GoodTimesFile << "\n\n\n" << "Crate/Slot GoodTime" << endl;
    
    // *** Find good times for slots ***
    
    for (int iCrater = 0; iCrater < 12; ++iCrater)
    {
        for (int iSlots = 3; iSlots < 19; ++iSlots)
        {
            for (int iRuns = 0; iRuns < HowManyRuns; ++iRuns)
            {
                
                if (SlotAvg[iRuns][iCrater][iSlots] != 0)
                {
                    slotTimes->Fill( SlotAvg[iRuns][iCrater][iSlots] );
                }
                
            } // *** end iRuns ***
            
            // *** Selecting the most occuring time as the 'good time' for a slot ***
            double maxSlots = slotTimes->GetMaximumBin();
            
            // *** The 1/5 factor is because we have bins in the slotTimes histogram such that every .2 ns is a bin, so 5 bins per ns ***
            GoodSlotsArray[iCrater][iSlots] = ( maxSlots / 20 );
            
            cout << "GoodSlotsArray[" << iCrater << "][" << iSlots << "] = " << GoodSlotsArray[iCrater][iSlots] << endl;
            
            GoodTimesFile << ( iCrater + 1 ) << "/" << iSlots << "   " << GoodSlotsArray[iCrater][iSlots] << endl;
            
            slotTimes->Reset(); // *** Clear out histogram so its good for next slot ***
        } // *** End iSlots ***
        
    } // *** End iCrates (for iSlots good time) ***
    
    GoodTimesFile << "\n\n\n" << "Chan GoodTimes" << endl;
    
    // *** Find good times for channels ***
    for (int iChans = 0; iChans < 2800; ++iChans)
    {
        
        for (int iRuns = 0; iRuns < HowManyRuns; ++iRuns)
        {
            
            if (ChanTimeAvg[iRuns][iChans] != 0)
            {
                channelTimes->Fill( ChanTimeAvg[iRuns][iChans] );
            }
            
        } //*** end iRuns ***
        
        // *** Selecting the most occuring time as the 'good time' for a channel ***
        double max = channelTimes->GetMaximumBin();
        
        // *** The 1/5 factor is because we have bins in the channelTimes histogram such that every .2 ns is a bin, so 5 bins per ns ***
        GoodTimesArray[iChans] = ( max / 20 );
        
        GoodTimesFile << ( iChans + 1 ) << "    " << GoodTimesArray[iChans] << endl;
        
        channelTimes->Reset(); // *** Clear out histogram so its good for next channel ***
    } // *** End iChans ***

    // *** For testing ***
    cout << "\n" << "Testing, after good slots loop, GoodCratesArray[0] = " << GoodCratesArray[0] << "\n" << endl;
 
    // ****************************************************************************************
    // *** SECTION 3: COMPARING EACH RUN TO THE REFERENCE, FINDING OFFSETS ***
    // ****************************************************************************************

    // *** Access text file list of runs again ***
    ifstream RunList3(UserRunList);
    
    for (int iRun = 0; iRun < HowManyRuns; ++iRun)
    {
        
        RunList3 >> line;
        TString fcalFileNamer2( line );
        
        TFile *fcalFile2 = new TFile (fcalFileNamer2);
        cout << "Current file: " << fcalFileNamer2 << endl;
        
        TTree *fcalTree2 = (TTree*)fcalFile2->Get("fcalBlockHits");
        
        // *** Reseting variables and declaring trees ***
        nHits = 0;
        run = 0;
        eTot = 0;
        
        memset( chan, 0, sizeof(int)*7000 );
        memset( x, 0, sizeof(int)*7000 );
        memset( y, 0, sizeof(int)*7000 );
        memset( t, 0, sizeof(int)*7000 );
        
        fcalTree2->SetBranchAddress("chan", chan);
        fcalTree2->SetBranchAddress("nHits", &nHits);
        fcalTree2->SetBranchAddress("x", x);
        fcalTree2->SetBranchAddress("y", y);
        fcalTree2->SetBranchAddress("t", t);
        fcalTree2->SetBranchAddress("run", &run);
        fcalTree2->SetBranchAddress("eTot", &eTot);
        
        // *** RunHold is used for naming the output text files ***
        int RunHold = 99999;
        
        // *** Arrays for run by run calculations ***
        double RunChannelTimeAdder[2800] = {0};
        double RunChannelTimeCounter[2800] = {0};
        double RunChannelTimeAvg[2800] = {0};
        
        double RunCrateTimeAdder[12] = {0};
        double RunCrateTimeCounter[12] = {0};
        double RunCrateTimeAvg[12] = {0};
        
        double RunSlotTimeAdder[13][18] = {0};
        double RunSlotTimeCounter[13][18] = {0};
        double RunSlotTimeAvg[13][18] = {0};
        
        
        
        // *** Loop over entries for current run,  ***
        for (int iEntry = 0; iEntry < fcalTree2->GetEntries(); ++iEntry)
        {
            fcalTree2->GetEntry(iEntry);
            
            RunHold = run;
            
            // *** Outputs every 2000 entries processed ***
            if (iEntry%10000 == 0) cout << iEntry << " events processed" << endl;
            
            for (int iHits = 0; iHits < nHits; ++iHits)
            {
                
                int row = (int)round((x[iHits] + 116)/4.0157);
                int column = (int)round((y[iHits] + 116)/4.0157);
                
                // *** Assigning each position the proper channel number, useful later ***
                chanPosition[row][column] = chan[iHits];
                
                // *** Adding up times for crates, slots and channels ***
                if ( eTot > 0 && t[iHits] > 0 )
                {
                    
                    RunChannelTimeAdder[chan[iHits]] += t[iHits];
                    RunChannelTimeCounter[chan[iHits]]++;
                    
                    RunCrateTimeAdder[ xyCrates[row][column] ] += t[iHits];
                    RunCrateTimeCounter[ xyCrates[row][column] ] ++;
                    
                    RunSlotTimeAdder[ xyCrates[row][column] ][ xySlots[row][column] ] += t[iHits];
                    RunSlotTimeCounter[ xyCrates[row][column] ][ xySlots[row][column] ] ++;
                    
                }
                
            } // *** End of iHits for loop ***
            
        } // *** End of iEntry ***

        // *** Testing ***
        cout << "\n\n" << "At end of section 3, GoodCratesArray[0] = " << GoodCratesArray[0] << "\n\n" << endl;
        
        // ****************************************************************
        // *** SECTION 4: WRITING OUTPUT FILES ***
        // ****************************************************************
        
        // *** Create an output text file ***
        
        TString OutputFileNamer =  UserOutputDirectory;
        OutputFileNamer += "/";
        OutputFileNamer += RunHold;
        OutputFileNamer += "_Timing_Offsets.txt";
        
        ofstream OffsetOutputFile;
        OffsetOutputFile.open(OutputFileNamer);
        
        cout << "OutputFileNamer is " << OutputFileNamer << endl;
        
        TString OutputChanCCDB = UserOutputDirectory;
        OutputChanCCDB += "/";
        OutputChanCCDB += RunHold;
        OutputChanCCDB += "_Channel_CCDB_Offsets.txt";
        
        ofstream OutputChanCCDBFile;
        OutputChanCCDBFile.open(OutputChanCCDB);
        
        cout << "OutputChanCCDB is " << OutputChanCCDB << endl;
        
        // *** Array for difference between current run and good time values ***
        double RunChannelOffsets[2800] = {0};
        double RunCrateOffsets[12] = {0};
        double RunSlotOffsets[13][18] = {0};
        
        // *** Stores either +4, -4 or 0 ***
        int ChanOffsetCorrections[2800] = {0};
        int CrateOffsetCorrections[12] = {0};
        int SlotOffsetCorrections[13][18] = {0};

        // *** Finding Crate Offsets ***
        OffsetOutputFile  << "Crate Offset" << endl;
        
        // *** Loop through each crate and calculate the average ***
        for (int iCrate = 0; iCrate < 12; iCrate++)
        {
            // *** Calculate avg time for each crate and how different it is from 'good time' value ***
            if (RunCrateTimeCounter[iCrate] != 0)
            {
                RunCrateTimeAvg[iCrate] = RunCrateTimeAdder[iCrate] / RunCrateTimeCounter[iCrate];
            }
            
            // *** Time average difference for current run vs reference ***
            RunCrateOffsets[iCrate] = RunCrateTimeAvg[iCrate] - GoodCratesArray[iCrate];
            
            // *** Testing purposes ***
            cout << "RunCrateTimeAdder[" << iCrate << "] = " << RunCrateTimeAdder[iCrate] << " and RunCrateTimeCounter[" << iCrate << "] = " << RunCrateTimeCounter[iCrate] << endl;
            cout << "RunCrateTimeAvg[" << iCrate << "] = " << RunCrateTimeAvg[iCrate] << " and GoodCratesArray[" << iCrate << "] = " << GoodCratesArray[iCrate] << endl;
            cout << "RunCrateOffsets[" << iCrate << "] = " << RunCrateOffsets[iCrate] << endl;
            
            cout << "\n" << endl;
            
            // *** Determine if channel is offset or not, and assign each channel either 0, +4 or -4 offset ***
            if (RunCrateOffsets[iCrate] > 3)
            {
                CrateOffsetCorrections[iCrate] = +4;
            }
            else if (RunCrateOffsets[iCrate] < -3)
            {
                CrateOffsetCorrections[iCrate] = -4;
            }
            else
            {
                CrateOffsetCorrections[iCrate] = 0;
            }
            // *** Fill the output file with correction values ***
            OffsetOutputFile << ( iCrate + 1 ) << "    " << CrateOffsetCorrections[iCrate] << endl;
        } // *** End for(iCrate) ***
        
        // *** Finding Slot Offsets ***
        OffsetOutputFile << "\n\n\n" << "Crate/Slot Offset" << endl;
        for (int iCrate = 0; iCrate < 12; ++iCrate)
        {
            // *** Go through slots for each crate, find offsets ***
            for (int iSlot = 3; iSlot < 19; iSlot++)
            {
                // *** Calculate avg time for each slot and how different it is from 'good time' value ***
                if (RunSlotTimeCounter[iCrate][iSlot] != 0)
                {
                    RunSlotTimeAvg[iCrate][iSlot] = RunSlotTimeAdder[iCrate][iSlot] / RunSlotTimeCounter[iCrate][iSlot];
                }
                
                RunSlotOffsets[iCrate][iSlot] = RunSlotTimeAvg[iCrate][iSlot] - GoodSlotsArray[iCrate][iSlot];
                
                // *** Testing purposes ***
                
                if (iCrate % 1 == 0 && iSlot % 111111 == 0)
                {
                    cout << "RunSlotTimeAdder[" << iCrate << "][" << iSlot << "]  = " << RunSlotTimeAdder[iCrate][iSlot] << " and RunSlotTimeCounter[" << iCrate << "][" << iSlot << " = " << RunSlotTimeCounter[iCrate][iSlot] << endl;
                    cout << "RunSlotTimeAvg[" << iCrate << "][" << iSlot << "] = " << RunSlotTimeAvg[iCrate][iSlot] << " and GoodSlotsArray[" << iCrate << "][" << iSlot << "] = " << GoodSlotsArray[iCrate][iSlot] << endl;
                    cout << "RunSlotOffsets[" << iCrate << "][" << iSlot << "] = " << RunSlotOffsets[iCrate][iSlot] << endl;
                    cout << "\n\n" << endl;
                } // *** End testing if ***

                // *** Determine if channel is offset or not, and assign each channel either 0, +4 or -4 offset ***
                if (RunSlotOffsets[iCrate][iSlot] > 3)
                {
                    SlotOffsetCorrections[iCrate][iSlot] = +4;
                }
                else if (RunSlotOffsets[iCrate][iSlot] < -3)
                {
                    SlotOffsetCorrections[iCrate][iSlot] = -4;
                }
                else
                {
                    SlotOffsetCorrections[iCrate][iSlot] = 0;
                }
                
                // *** Fill the output file with correction values ***
                OffsetOutputFile << ( iCrate + 1 ) << "/" << iSlot << "   " << SlotOffsetCorrections[iCrate][iSlot] << endl;
            } // *** End iSlot ***
        } // *** End for (iCrate) (iSlot loop) ***

        // *** Finding Channel Offsets ***
        OffsetOutputFile << "\n\n\n" << "Chan Offset" << endl;

        // *** Loop through each channel and calculate the average ***
        for (int iChannel = 0; iChannel < 2800; iChannel++)
        {
            // *** Calculate avg time for each channel and how different it is from 'good time' value ***
            if (RunChannelTimeCounter[iChannel] != 0)
            {
                RunChannelTimeAvg[iChannel] = RunChannelTimeAdder[iChannel] / RunChannelTimeCounter[iChannel];
            }
            
            RunChannelOffsets[iChannel] = RunChannelTimeAvg[iChannel] - GoodTimesArray[iChannel];

            // *** Determine if channel is offset or not, and assign each channel either 0, +4 or -4 offset ***
            if (RunChannelOffsets[iChannel] > 3)
            {
                ChanOffsetCorrections[iChannel] = +4;
            }
            else if (RunChannelOffsets[iChannel] < -3)
            {
                ChanOffsetCorrections[iChannel] = -4;
            }
            else
            {
                ChanOffsetCorrections[iChannel] = 0;
            }

            // *** Forcing individual channels to offset of the slot they are in if said slot has offset ***
            if (  SlotOffsetCorrections[ ChannelCrates[iChannel] ][ ChannelSlots[iChannel] ] != 0 )
            {
                //ChanOffsetCorrections[iChannel] = SlotOffsetCorrections[ ChannelCrates[iChannel] ][ ChannelSlots[iChannel] ];
            }
        } // *** End for(iChannel) ***

        for (int iChannel = 0; iChannel < 2800; iChannel++)
        {
            // *** If statement for problem slot 4 crate 7, use nearest neighbor approach to compare it to slot to right and slo below ***
            if ( ChannelCrates[iChannel] == 6 && ChannelSlots[iChannel] == 4 )
            {
                cout << "RunChannelTimeAvg[" << iChannel << "] = " << RunChannelTimeAvg[iChannel] << " and RunChannelTimeAvg[" << chanPosition[ (int)round( chanXPos[iChannel] + 3 ) ][ (int)round(chanYPos[iChannel]) ] << "] = " << RunChannelTimeAvg[ chanPosition[ (int)round( chanXPos[iChannel] + 3 ) ][ (int)round(chanYPos[iChannel]) ] ] << endl;
                
                cout << "RunChannelTimeAvg[" << iChannel << "] = " << RunChannelTimeAvg[iChannel] << " and RunChannelTimeAvg[" << chanPosition[ (int)round( chanXPos[iChannel]) ][ (int)round(chanYPos[iChannel] - 3 ) ] << "] = " << RunChannelTimeAvg[ chanPosition[ (int)round( chanXPos[iChannel]  ) ][ (int)round(chanYPos[iChannel] - 3 ) ] ] << endl;
                
                cout << "Difference between bad slot channel and neighboring slot channel, right: " <<  RunChannelTimeAvg[iChannel] - RunChannelTimeAvg[ chanPosition[ (int)round( chanXPos[iChannel] + 3 ) ][ (int)round(chanYPos[iChannel]) ] ] << " and below: " << RunChannelTimeAvg[iChannel] - RunChannelTimeAvg[ chanPosition[ (int)round( chanXPos[iChannel] ) ][ (int)round( chanYPos[iChannel] - 3 ) ] ] << endl;
                
                cout << "Chan X and Y pos: " << chanXPos[iChannel] << " & " << chanYPos[iChannel] << endl;
                cout << "Neighbor positions, right: " << chanXPos[iChannel] + 3 << ", " << chanYPos[iChannel] << " & below: " << chanXPos[iChannel] << ", " << chanYPos[iChannel] - 3 << endl;

                // *** Compare channels in bad slot to channels in slot to the right, and if they are lower then bump them up ***
                if ( ( RunChannelTimeAvg[iChannel] - RunChannelTimeAvg[ chanPosition[ (int)round( chanXPos[iChannel] + 3 ) ][ (int)round(chanYPos[iChannel]) ] ] ) < -2.5 )
                {
                    ChanOffsetCorrections[iChannel] = -4;
                }
                
                // *** Compare channels in bad slot to channels in slot below, and if they are lower then bump them up ***
                if ( ( RunChannelTimeAvg[iChannel] - RunChannelTimeAvg[ chanPosition[ (int)round( chanXPos[iChannel] ) ][ (int)round( chanYPos[iChannel] - 3 ) ] ] ) < -2.5 )
                {
                    ChanOffsetCorrections[iChannel] = -4;
                }
            } // *** End if crate 7 slot 4 ***
            
            // *** Fill the output file with correction values ***
            OffsetOutputFile << ( iChannel + 1 ) << "    " << ChanOffsetCorrections[iChannel] << endl;
            OutputChanCCDBFile << ChanOffsetCorrections[iChannel] << endl;
            
        } // *** End for iChannel 2 ***

        OffsetOutputFile.close();
        OutputChanCCDBFile.close();
        
    } // *** End of iRun for loop ***
    
    
    
} // *** End of main ***




