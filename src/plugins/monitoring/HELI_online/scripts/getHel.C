//example function to find the helicity for run,event from lookup tables
//tables must be called : HelTable_<runNo>.txt
//of the form:
// #event no  helicity
// 263111681  0
// 263439201 -1
// 263441772  1
// 263447041 -1
// 263449685  1

// (#comments are skipped when reading the table)

// Assumme events in approximate order of event number (maybe with some shuffling due to threaded analysis)
// Hence expect the event to be in the currenly selected range, but sometimes in previous range, and then at fairly regular
// intervals move the current range up to the next one in the table as needed.
// So try and do that eficiently.

//some globals 
int64_t   *events    = NULL;
char   *helicities   = NULL;
int   haveHelTable   = 0;
int    tableLength   = 0;
int         curr_r   = 0;
int         prev_r   = 0;
int         next_r   = 0;

void readHelTable(uint run){                                //read in the helicity table 
  FILE *fp = NULL;
  char helTableName[200];
  char shellCommand[200];
  char tableString[60];
  uint64_t nread=0;
  
  sprintf(helTableName,"HelTable_%d.txt",run);              //work out no of lines in table from shell command
  sprintf(shellCommand,"grep -v \\# %s | wc -l", helTableName);
  tableLength = atoi(gSystem->GetFromPipe(shellCommand));

  if(events)     delete events;                             //delete previous table
  if(helicities) delete helicities;
  events      = new int64_t[tableLength];                   //and make new - may be large.
  helicities  = new char[tableLength];
  
  fp = fopen(helTableName,"r");                             //open file and scan the lines
  while(fgets(tableString,60,fp)){
    if(!strstr(tableString,"#")){
      sscanf(tableString,"%ld%hhd",&events[nread],&helicities[nread]);
      nread++;
    }
  }
  fclose(fp);
  prev_r  = 0;                                              // reset incides to range in table
  curr_r  = 0;
  next_r  = 1;
  haveHelTable = run;
}


int getHel(uint run, uint64_t event){
  int helicity = 0;                                          //default to 0 (undefined)

  if(haveHelTable!=run) readHelTable(run);                   //get the table if needed

  if((event>=events[curr_r])&&(event<events[next_r])){       //if event in the current window (most likely)
    helicity = helicities[curr_r];
  }
  else if((event>=events[prev_r])&&(event<events[curr_r])){  //if event in previous window (possible if threaded, non sequential)
    helicity = helicities[prev_r];
  }
  else if((event>=events[next_r])&&(event<events[next_r+1])){ //at end of current range, move to next
    helicity = helicities[next_r];
    if(next_r<(tableLength-1)){
      next_r++;
      prev_r++;
      curr_r++;
    }
  }
  else if((event<events[prev_r])||(event>events[next_r])){  //not in the current, previous or next range (shouldn't happen)
    curr_r=-1;                                              //search the whole table from the start
    for(int n=0;n<(tableLength-1);n++){
      if((event>=events[n])&&(event<events[n+1])){
  	curr_r = n;
  	prev_r = n-1;
  	next_r = n+1;
      }
    }
    if(curr_r > -1){
      helicity = helicities[curr_r];     
    }
    else{
      helicity = 0;
      cout << "not found anywhere" << endl;
    }
  }
  else{                                                      //there shouldn't be anything else.
    helicity=0;
  }
  cout << curr_r << "  " << events[curr_r] << endl;
  return helicity;
}
