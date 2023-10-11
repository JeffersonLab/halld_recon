// Author: David Lawrence  June 25, 2004
//
//
// MyProcessor.cc
//

#include <iostream>
using namespace std;

#include <stdio.h>
#include <unistd.h>

#include <JANA/JApplication.h>
#include <DANA/DStatusBits.h>

#include "MyProcessor.h"


bool PAUSE_BETWEEN_EVENTS = true;
bool SKIP_BORING_EVENTS = false;
bool PRINT_ALL = false;
bool PRINT_CORE = false;
bool LIST_ASSOCIATED_OBJECTS = false;
bool PRINT_SUMMARY_ALL = false;
bool PRINT_SUMMARY_HEADER = false;
bool PRINT_STATUS_BITS = false;
bool ACTIVATE_TAGGED_FOR_SUMMARY = false;
bool QUIT_AFTER_FINDING_NTH = false;
extern bool SPARSIFY_SUMMARY;

int N_TO_FIND=0; // default to processing all events

int n_found=0;

set<string> toprint;
set<string> tosummarize;

#define ansi_escape		((char)0x1b)
#define ansi_bold 		ansi_escape<<"[1m"
#define ansi_black		ansi_escape<<"[30m"
#define ansi_red			ansi_escape<<"[31m"
#define ansi_green		ansi_escape<<"[32m"
#define ansi_blue			ansi_escape<<"[34m"
#define ansi_normal		ansi_escape<<"[0m"
#define ansi_up(A)		ansi_escape<<"["<<(A)<<"A"
#define ansi_down(A)		ansi_escape<<"["<<(A)<<"B"
#define ansi_forward(A)	ansi_escape<<"["<<(A)<<"C"
#define ansi_back(A)		ansi_escape<<"["<<(A)<<"D"

//------------------------------------------------------------------
// BeginRun
//------------------------------------------------------------------
void MyProcessor::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	cout << "Beginning run" << endl;
	vector<string> factory_names;
	for (auto factory : event->GetFactorySet()->GetAllFactories()) {
		factory_names.push_back(factory->GetObjectName());
	}

	usleep(100000); //this just gives the Main thread a chance to finish printing the "Launching threads" message
	cout<<endl;

	// If int PRINT_ALL is set then add EVERYTHING.
	if(PRINT_ALL){
		for(auto fac_name : factory_names) toprint.insert(fac_name);
		SKIP_BORING_EVENTS = 0; // with PRINT_ALL, nothing is boring!
	}else if(PRINT_SUMMARY_ALL){
		for(auto fac_name : factory_names) tosummarize.insert(fac_name);
		SKIP_BORING_EVENTS = 0; // with PRINT_ALL, nothing is boring!
	}else if(PRINT_CORE){
		
		// Make list of "core" factories. n.b. these high level
		// objects will automatically activate lower level ones
		set<string> core_factories;

		tosummarize.insert("DChargedTrack");
		tosummarize.insert("DNeutralShower");

		tosummarize.insert("DMCTrackHit");
		tosummarize.insert("DMCThrown");
		tosummarize.insert("DMCTrajectoryPoint");
		tosummarize.insert("DMCReaction");

        tosummarize.insert("DTrigger");
        tosummarize.insert("DEventHitStatistics");
		
	}else{
		// make sure factories exist for all requested data types
		// If a factory isn't found, but one with a "D" prefixed
		// is, go ahead and correct the name.
		set<string> really_toprint;
		for(auto fac_name : toprint){
			bool found  = false;
			bool dfound = false;
			for(unsigned int j=0;j<factory_names.size();j++){
				if(factory_names[j] == fac_name)found = true;
				if(factory_names[j] == "D" + fac_name)dfound = true;
			}
			if(found)
				really_toprint.insert(fac_name);
			else if(dfound)
				really_toprint.insert("D" + fac_name);
			else
				cout<<ansi_red<<"WARNING:"<<ansi_normal
					<< " Couldn't find factory for \""
					<< ansi_bold << fac_name << ansi_normal
					<< "\"!"<<endl;
		}
		
		toprint = really_toprint;
	}

	// Make sure all factories to print are also included in summary
	tosummarize.insert(toprint.begin(), toprint.end());
	
	// At this point, toprint should contain a list of all factories
	// in dataClassName:tag format, that both exist and were requested.
	// Separate the tag from the name and fill the fac_info vector.
	fac_info.clear();
	for(auto fac_name : toprint){
		string tag = "";
		unsigned int pos = fac_name.rfind(":",fac_name.size()-1);
		if(pos != (unsigned int)string::npos){
			tag = fac_name.substr(pos+1,fac_name.size());
			fac_name.erase(pos);
		}
		factory_info_t f;
		f.dataClassName = fac_name;
		f.tag = tag;
		f.fac = event->GetFactory(f.dataClassName, f.tag.c_str());
		fac_info.push_back(f);
	}
	
	cout<<endl;
}

//------------------------------------------------------------------
// Process
//------------------------------------------------------------------
void MyProcessor::Process(const std::shared_ptr<const JEvent>& event)
{

	// If we're skipping boring events (events with no rows for any of
	// the types we're printing) we must find out first if the event is
	// "boring".
	int event_is_boring = 1;

	for(unsigned int i=0;i<toprint.size();i++){

		string name =fac_info[i].dataClassName;
		string tag = fac_info[i].tag;
		JFactory *factory = event->GetFactory(name,tag.c_str());
		if(!factory)factory = event->GetFactory("D" + name,tag.c_str());
		if(factory){
			try{
				if(factory->Create(event, event->GetJApplication(), event->GetRunNumber()) > 0){
					event_is_boring=0;
					if(PRINT_SUMMARY_HEADER)break;
				}
			}catch(...){
				// someone threw an exception
			}
		}
	}
	
	if(SKIP_BORING_EVENTS && event_is_boring)return;
	if(!SKIP_BORING_EVENTS)event_is_boring= 0;
	
	// Print event separator
	cout<<"================================================================"<<endl;
	cout<<"Event: "<<event->GetEventNumber()<<endl;

	// We want to print info about all factories results, even if we aren't
	// printing the actual data. To make sure the informational messages often
	// printed during brun are printed first, call the Create() method of all factories
	// ourself first.
	if(PRINT_SUMMARY_HEADER){
		for( auto fac : event->GetFactorySet()->GetAllFactories() ){
			string name = fac->GetObjectName();
			string tag  = fac->GetTag();
			if(tag.size()>0) name += ":" + tag;
			if( tosummarize.count(name) ) fac->Create(event, event->GetJApplication(), event->GetRunNumber());
		}
	}
	
	if(PRINT_STATUS_BITS) cout << event->GetSingleStrict<DStatusBits>()->ToString() << endl;
	if(PRINT_SUMMARY_HEADER) PrintSummaryHeader(event, SPARSIFY_SUMMARY ? 2:0);

	// Print data for all specified factories
	for(unsigned int i=0;i<fac_info.size();i++){
		try{
			string name =fac_info[i].dataClassName;
			string tag = fac_info[i].tag;
			PrintFactoryData(event, name, tag);
			if(LIST_ASSOCIATED_OBJECTS)PrintAssociatedObjects(event, &fac_info[i]);
		}catch(...){
			// exception thrown
		}
	}
	
        n_found++; // keep count of number of events printed

        if (n_found==N_TO_FIND) GetApplication()->Quit();

	// If the program is quitting, then don't bother waiting for the user
	if(event->GetJApplication()->IsQuitting()) return;
	
	// Wait for user input if pausing
	if(PAUSE_BETWEEN_EVENTS && !event_is_boring){
		cerr.flush();
		cout<<endl<<"< Hit return for the next event (P=prev. Q=quit) >";
		cout.flush();
		char c = getchar(); // see note in hd_dump.cc:main()
		if(c=='\n')cout<<ansi_up(1);
		cout<<endl;
		switch(toupper(c)){
			case 'Q':
				GetApplication()->Quit();
				break;
			case 'P':
				//event->GotoEvent(eventnumber-1);
				break;
			case 'N':
				break;
		}

		cout<<ansi_up(1)<<"\r                                                     \r";
		cout.flush();
	}
}

//------------------------------------------------------------------
// PrintSummaryHeader
//------------------------------------------------------------------
void MyProcessor::PrintSummaryHeader(const std::shared_ptr<const JEvent>& event, int sparsify) {
	/// Print a list of all registered factories to the screen
	/// along with a little info about each. The value of
	/// "sparsify" controls what gets printed and whether
	/// factories are activated by this method.
	///
	/// sparsify==0  all factories are activated and
	///              a line printed for each
	///
	/// sparsify==1  all factories are activated but
	///              a line is printed only for those
	///              with at least one object
	///
	/// sparsify==2  factories are not activated, but
	///              a line is printed for any that
	///              already have at least one object

	bool do_not_call_get = (sparsify==2);

	auto factories = event->GetFactorySet()->GetAllFactories();
	jout<<endl;
	jout<<"Registered factories: ("<<factories.size()<<" total)"<<endl;
	jout<<endl;
	jout<<"Name:                nrows:  tag:"<<endl;
	jout<<"------------------- ------- --------------"<<endl;

	for(JFactory* factory : factories) {

		if (!do_not_call_get) {
			// Make sure this factory has been activated
			factory->Create(event, event->GetJApplication(), event->GetRunNumber());
		}
		// Retrieve number of rows (whether it has been activated or not)
		size_t nrows = factory->GetAs<JObject>().size(); // GetAs doesn't trigger creation

		try{
			if(sparsify)
				if(nrows<1)continue;
		}catch(...){}

		// To make things look pretty, copy all values into the buffer "str"
		string str(79,' ');
		string name = factory->GetObjectName();
		str.replace(0, name.size(), name);

		char num[32]="";
		try{
			sprintf(num, "%zu", nrows);
		}catch(...){}
		str.replace(25-strlen(num), strlen(num), num);

		string tag = factory->GetTag();
		//const char *tag = factory->Tag();
		if(tag != ""){
			tag = "\"" + tag + "\"";
			//char tag_str[256];
			//sprintf(tag_str, "\"%s\"", tag);
			//str.replace(26, strlen(tag_str), tag_str);
			str.replace(29, tag.size(), tag);
		}

		jout<<str<<endl;
	}

	jout<<endl;
}

//------------------------------------------------------------------
// PrintFactoryData
//------------------------------------------------------------------
void MyProcessor::PrintFactoryData(const std::shared_ptr<const JEvent>& event, std::string fac_name, std::string fac_tag) {
	/// Dump the data to stdout for the specified factory
	///
	/// Find the factory corresponding to data_name and send
	/// the return value of its toString() method to stdout.

	// Search for specified factory and return pointer to it's data container
	JFactory* factory = event->GetFactory(fac_name, fac_tag);
	if(!factory){
		jerr<<" ERROR -- Factory not found for class \""<<fac_name<<"\""<<endl;
		return;
	}

	string str = PrintFactory(factory);
	if(str=="")return;

	jout<<fac_name<<":"<<fac_tag<<endl;
	jout<<str<<endl;;
}

std::string MyProcessor::PrintFactory(JFactory* factory) {
	/// Return a string containing the data for all of the objects
	/// produced by this factory for the current event. Note that
	/// this does not actually activate the factory to create the
	/// objects if they don't already exist. One should call the
	/// Get() method first if they wish to ensure the factory has
	/// been activated.
	///
	/// If no objects exist, then an empty string is returned. If objects
	/// do exist, then the string returned will contain column names
	/// and a separator line.
	///
	/// The string is built using values obtained via JObject::toStrings().


	// Get data in the form of strings from the sub-class which knows
	// the data type we are.
	vector<vector<pair<string,string> > > allitems;

	JObjectSummary summary;
	for (auto obj : factory->GetAs<JObject>()) {
		obj->Summarize(summary);
		vector<pair<string,string>> oldsummary;
		for (auto field : summary.get_fields()) {
			oldsummary.push_back({field.name, field.value});
		}
		allitems.push_back(oldsummary);
	}
	if(allitems.size()==0)return "";

	// Make reference to first map which we'll use to get header info
	vector<pair<string,string> > &h = allitems[0];
	if(h.size()==0)return string("");

	// Make list of column names and simultaneously capture the string lengths
	vector<unsigned int> colwidths;
	vector<string> headers;
	vector<pair<string,string> >::iterator hiter = h.begin();
	for(; hiter!=h.end(); hiter++){
		headers.push_back(hiter->first);
		colwidths.push_back(hiter->first.length()+2);
	}

	assert(headers.size()==colwidths.size());

	// Look at all elements to find the maximum width of each column
	for(unsigned int i=0; i<allitems.size(); i++){
		vector<pair<string,string> > &a = allitems[i];

		assert(a.size()==colwidths.size());

		for(unsigned int j=0; j<a.size(); j++){
			pair<string,string> &b = a[j];

			unsigned int len = b.second.length()+2;
			if(len>colwidths[j])colwidths[j] = len;
		}
	}

	stringstream ss;

	// Print header
	unsigned int header_width=0;
	for(unsigned int i=0; i<colwidths.size(); i++)header_width += colwidths[i];
	string header = string(header_width,' ');
	unsigned int pos=0;
	for(unsigned int i=0; i<colwidths.size(); i++){
		header.replace(pos+colwidths[i]-headers[i].length()-1, headers[i].length()+1, headers[i]+":");
		pos += colwidths[i];
	}
	ss<<header<<endl;

	ss<<string(header_width,'-')<<endl;

	// Print data
	for(unsigned int i=0; i<allitems.size(); i++){
		vector<pair<string,string> > &a = allitems[i];
		assert(a.size()==colwidths.size());

		string row = string(header_width,' ');

		unsigned int pos=0;
		for(unsigned int j=0; j<a.size(); j++){
			pair<string,string> &b = a[j];

			row.replace(pos+colwidths[j]-b.second.length()-1, b.second.length(), b.second);
			pos += colwidths[j];
		}

		ss<<row<<endl;
	}

	return ss.str();

}

//------------------------------------------------------------------
// FindOwner
//------------------------------------------------------------------
JFactory* MyProcessor::FindOwner(const std::shared_ptr<const JEvent>& event, const JObject* find_me) {
	for (JFactory* factory : event->GetFactorySet()->GetAllFactories()) {
		for (JObject* obj : factory->GetAs<JObject>()) {
			if (find_me == obj) {  // Use pointer equality instead of value equality, consider using index for efficiency
				// TODO: NWB: Original code did the following:
				// const JObject *my_obj = factories[i]->GetByID(obj->id);
				// GetByID assumes obj->id is valid and unique, which makes me wonder if this code ever worked
				return factory;
			}
		}
	}
	return nullptr;
}

//------------------------------------------------------------------
// PrintAssociatedObjects
//------------------------------------------------------------------
void MyProcessor::PrintAssociatedObjects(const std::shared_ptr<const JEvent>& event, const factory_info_t *fac_info)
{
	// cast away const-ness of JFactory class pointer
	JFactory *fac = const_cast<JFactory*>(fac_info->fac);
	if(!fac)return;

	// Get list of all objects from this factory
	vector<JObject*> objs = fac->GetAs<JObject>();

	// Loop over objects from this factory
	for(unsigned int i=0; i<objs.size(); i++){
	
		// First, get a list of all associated objects
		vector<const JObject*> aobjs;
		objs[i]->GetT(aobjs);
		// If no associated objects, just go on to the next object
		if(aobjs.size()==0)continue;
		
		// Print separator
		cout<<"  [== Associated objects for row "<<i<<" ==]"<<endl;

		// Make a list of all factories that made objects associated to this one
		map<JFactory*, vector<const JObject*> > aofacs;
		for(unsigned int j=0; j<aobjs.size(); j++){
			JFactory *aofac = FindOwner(event, aobjs[j]);
			
			map<JFactory*, vector<const JObject*> >::iterator iter = aofacs.find(aofac);
			if(iter==aofacs.end()){
				vector<const JObject*> tmp;
				aofacs[aofac] = tmp;
			}
			// Record this object as belonging to this factory
			aofacs[aofac].push_back(aobjs[j]);
		}
		// Figure out number of spaces to indent objects based on factory name length
		map<JFactory*, vector<const JObject*> >::iterator iter;
		unsigned int indent=4; // some minimal string length
		for(iter=aofacs.begin(); iter!=aofacs.end(); iter++){
			JFactory *fac = iter->first;
			string name = fac->GetObjectName();
			if(fac->GetTag() != "") name += string(":") + fac->GetTag();
			if(name.length()>indent)indent=name.length();
		}
		indent += 4; // indent the factory name itself
		
		// Loop over factories that produced associated objects for this object and
		// list the objects it created
		for(iter=aofacs.begin(); iter!=aofacs.end(); iter++){
			JFactory *fac = iter->first;
			vector<const JObject*> &ptrs = iter->second;
			
			// Print out factory name
			string name = fac->GetObjectName();
			if(fac->GetTag() != "") name += string(":") + fac->GetTag();
			cout<<string(indent-name.length()-1,' ');
			cout<<name<<" ";
			
			// Loop over objects from this factory that are in the list
			for(unsigned int j=0; j<ptrs.size(); j++){
				if(j!=0)cout<<string(indent,' ');
				cout<<"0x"<<hex<<(unsigned long)ptrs[j]<<dec<<endl;
			}
		}
	}
	
}

