//
//
// dilog - A header-only C++ diagnostic class for finding the point
//         of divergence between two runs of an application that
//         should produce identical results each time it runs, bug
//         does not.
//
// author: richard.t.jones at uconn.edu
// date: january 2, 2021 [rtj]
//
// programmer's notes:
// 1) This class is only intended for diagnostic use once a discrepancy
//    has been found in the output of an application between repeated
//    runs with the same input data. It is not intended to be used in
//    regular producition, or become a permanent part of any application.
// 2) The first time you run your dilog-instrumented application in a
//    given directory, it will write its output file into the cwd. 
//    Running it a second time in the same directory will check the
//    dilog output against the original and report a runtime error
//    at the first appearance of differences.
// 3) Assuming the g++ compiler, you must include the -std=c++11 switch.
//
// usage example:
//     #include <dilog.h> 
//     ...
//     dilog::printf("myapp", "sheep %d in herd %s\n", isheep, herd);
//     ...

#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <assert.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <map>
#include <vector>
#include <mutex>

#define DILOG_LOGO "---DILOG------DILOG------DILOG---"

class dilog;
using dilogs_map_t = std::map<std::string, dilog*>;

class dilog {

 // Objects of class dilog are created through the static dilog::get method,
 // identified by channel name. The first get("myapp") would create a new
 // dilog object and open file myapp.dilog for reading, or for writing if
 // a file by this name does not exist in the cwd. An ordered sequence of
 // messages issued through dilog::printf are read from the input file, or
 // written if open for writing. Any mismatch between a printf message and
 // the corresponding record in myapp.dilog will report a runtime error.

 public:
   class block {

    // Objects of class dilog::block are constructed by the user inside a
    // loop that might be reordered at runtime. The printf message scanner
    // automatically reorders at runtime all contiguous iterations of the
    // code block defined by the lifetime of the user dilog::block object
    // in seeking a match between the contents of the recorded dilog file
    // and the runtime message sequence.

    private:
      std::string chan;        // name of associated channel
      std::string name;        // name of block, eg. "loop1"
      std::string prefix;      // slash-delimited pathname prefix of block
      std::streampos base;     // file offset to current iteration of this block
      unsigned int beginline;  // file line number associated with base
      unsigned int ireplay;    // offset in dilog::fRecord where replay starts
      block *parent;           // pointer to the block containing this one

      // blinks contains information about all of the unmatched iterations
      // of inner blocks that have been encountered so far in searching the
      // current iteration of this block. The blinks are indexed by the
      // block path, and consists of key-value pairs where the key is the
      // stream offset in the input file where the unmatched iteration begins,
      // and the value is the corresponding line number. Entries in blinks
      // are inserted by next() and erased by exit() upon completion
      // of a successful block match.
      std::map<std::string, std::map<std::streampos, int> > blinks;

      // flinks is similar to blinks, except that it points to the first
      // line past the last iteration of the block set in the input stream.
      // If flinks is not set, the input parser has not yet encountered
      // the end of this block set.
      std::map<std::string, std::map<std::streampos, int> > flinks;

      block()
       : base(0), beginline(0), ireplay(0), parent(0)
      {}

    public:
      block(const std::string &channel, const std::string &blockname,
                                        bool threadsafe=true)
       : chan(channel), name(blockname)
      {
         // Initialize a new iteration of block with name blockname on the
         // named dilog channel, generating a new dilog channel if it does
         // not already exist. Normal behavior is to report a runtime error 
         // if access to this channel is attempted from more than one C++
         // execution thread. Set threadsafe = false to suppress this
         // check, at the risk of creating race conditions.
 
         dilog &dlog = dilog::get(channel, threadsafe);
         if (dlog.fError.size() > 0)
            dlog.clean_exit("block constructor");
         parent = dlog.fBlock;
         prefix = parent.getPath();
         if (dlog.fWriting) {
            *dlog.fWriting << "[" << getPath() << "[" << std::endl;
            ++dlog.fLineno;
         }
         else if (!enter()) {
            dlog.clean_exit("block constructor");
         }
         ireplay = dlog.fRecord.size();
      }

      ~block()
      {
         // Terminate an open iteration block. The names of the block and
         // channel are saved in the object. This method is normally called
         // automatically at exit from a loop in user code, as the block
         // object declared inside the loop goes out of scope and garbage
         // collection automatically calls the destructor. The destructor
         // can trigger non-trivial operations on the input file, as it
         // requires that the iteration block terminate immediately with
         // no further messages. If a consistency violation is found, it
         // prints to stderr and sets a fatal error flag so no further
         // dilog searches can take place on this channel.
 
         dilog &dlog = dilog::get(chan, false);
         if (dlog.fError.size() > 0)
            return;
         if (dlog.fWriting) {
            *dlog.fWriting << "]" << getPath() << "]" << std::endl;
            ++dlog.fLineno;
         }
         else if (!exit()) {
            std::cerr << "dilog::block destructor error: " 
                      << dlog.fError << std::endl;
         }
      }

    protected:

      bool enter()
      {
         // This method is called when a dilog channel open in read mode
         // enters a new block and descends a level in the block stack.
 
         dilog &dlog = dilog::get(chan, false);
         beginline = dlog.fLineno;
         base = dlog.fReading->tellg();
         std::string mexpected = "[" + getPath() + "[";
         for (std::string nextmsg; std::getline(*dlog.fReading, nextmsg);) {
            ++dlog.fLineno;
            dlog.verify_line(nextmsg, dlog.fLineno);
            if (nextmsg == mexpected) {
               dlog.logger("[" + getPath() + "[");
               // std::cerr << "block::block records match step " 
               //           << dlog.fRecord.size() << " at lineno "
               //           << dlog.fLineno << ": [[" << getPath()
               //           << std::endl;
               dlog.fMatched[dlog.fRecord.size()] = dlog.fLineno;
               dlog.fRecord.push_back("[[" + getPath());
               break;
            }
            else if (parent->->next(nextmsg)) {
               beginline = dlog.fLineno;
               base = dlog.fReading->tellg();
               continue;
            }
            dlog.fError = "dilog::block::enter error: "
                          "expected new block"
                          " \"" + getPath() + "\""
                          " at line " + std::to_string(dlog.fLineno)
                          + " in " + channel + ".dilog "
                          "but found \"" + nextmsg + "\" instead.";
            return false;
         }
         if (!*dlog.fReading) {
            dlog.fError = "dilog::block::enter error: "
                          "expected new block"
                          " \"" + getPath() + "\""
                          " at line " + std::to_string(dlog.fLineno)
                          + " in " + channel + ".dilog "
                          "but found end-of-file instead.";
            return false;
         }
         dlog.fBlocks[getPath()] = this;
         dlog.fBlock = this;
      }

      bool exit()
      {
         // This method is called when a dilog channel open in read mode
         // exits a block and ascends a level in the block stack.

         if (dlog.fBlock != this) {
            dlog.fError = "dilog::block::exit error: "
                          "block nesting error, cannot continue!";
            return false;
         }
         std::string mexpected = "]" + getPath() + "]";
         for (std::string nextmsg; std::getline(*dlog.fReading, nextmsg);) {
            ++dlog.fLineno;
            dlog.verify_line(nextmsg, dlog.fLineno);
            if (parent->blinks[getPath()].size() != 0) {
               if (dlog.next_block(nextmsg))
                  continue;
            }
            else if (nextmsg == mexpected) {
               dlog.logger("]" + getPath() + "]");
               dlog.fMatched[dlog.fRecord.size()] = dlog.fLineno;
               dlog.fRecord.push_back("]]" + getPath());
               break;
            }
            else if (dlog.next_block(nextmsg)) {
               continue;
            }
            dlog.fError = "dilog::block::exit error: "
                          "expected end of block "
                          "\"" + getPath() + "\" at line " + 
                          std::to_string(lineno) + 
                          " in " + chan + ".dilog"
                          " but found \"" + nextmsg + "\" instead.";
            return false;
         }
         if (!*dlog.fReading) {
            dlog.fError = "dilog::block::exit error: "
                          "expected end of block"
                          " \"" + getPath() + "\" at line "
                          + std::to_string(dlog.fLineno)
                          + " in " + channel + ".dilog "
                          "but found end-of-file instead.";
            return false;
         }
         for (auto flink : parent->flinks[getPath()]) {
            if (flink.second > dlog.fLineno) {
               dlog.fReading->seekg(flink.first);
               dlog.fLineno = flink.second;
            }
         }
         parent->blinks.erase(getPath());
         parent->flinks.erase(getPath());
         dlog.fBlocks.erase(getPath());
         dlog.fBlock = parent;
         if (prefix == dlog.fChannel) {
            dlog.fMatched.clear();
            dlog.fRecord.clear();
         }
      }

      bool next(const std::string lastmsg)
      {
         // Entry here assumes that a mismatch has been found between the 
         // expected message and the lastmsg string that was last read from
         // the input stream. The task of next_block is to search through
         // all remaining block iterations to see if there might be another
         // chance to fulfill the expectation. If the search is successful,
         // it returns true with the input file and block sequence replayed
         // up to the point immediately preceding the check of lastmsg.
 
         dilog &dlog = dilog::get(chan, false);
         auto flink = parent->flinks[getPath()];
         if (flink.size() > 0) {
            auto blink = parent->blinks[getPath()];
            auto biter = blink.find(base);
            if (biter != blink.end() && ++biter != blink.end()) {
               fReading->seekg(biter->first);
               fLineno = biter->second;
            }
            else if () {
            }
            else {
               dlog.fError = "dilog::block::next error: "
                             "expected next block"
                             " \"" + getPath() + "\" at line "
                             + std::to_string(flink.begin().second)
                             + " in " + channel + ".dilog "
                             "but no more instances of this block are found.";
               return false;
            }
         }
         else {
            parent->blinks[getPath()][base] = beginline;
            std::string mexpected = "]" + getPath() + "]";
            if (lastmsg != mexpected) {
               for (std::string nextmsg; std::getline(*fReading, nextmsg);) {
                  ++fLineno;
                  verify_line(nextmsg, fLineno);
                  if (nextmsg == mexpected) {
                     break;
                  }
               }
            }
            if (!*dlog.fReading) {
               dlog.fError = "dilog::block::next error: "
                             "expected next block"
                             " \"" + getPath() + "\" at line "
                             + std::to_string(dlog.fLineno)
                             + " in " + channel + ".dilog "
                             "but found end-of-file instead.";
               return false;
            }
         }

         if (!enter()) {
            dlog.fError = "dilog::block::next error: "
                          "expected next block"
                          " \"" + getPath() + "\" at line "
                          + std::to_string(dlog.fLineno)
                          + " in " + channel + ".dilog "
                          "but no more instances of this block are found.";
            return false;
         }
         return replay(ireplay);
      }

      bool replay(unsigned int &irep)
      {
         // Entry here assumes that this block is currently entered on the
         // input stream and the replay is in progress. The task is to 
         // complete the replay through the end of fRecord, constructing
         // and destroying nested blocks as needed along the way and 
         // return true, or else to die trying.
 
         std::string mexpected;
         for (; irep < fRecord.size(); ++irep) {
            mexpected = fRecord[irep];
            if (mexpected.find("[[") == 0) {
               block *binner;
               std::string path = mexpected.substr(2);
               if (dlog.fBlocks.find(path) == dlog.fBlocks.end()) {
                  std::string bname = mexpected.substr(2);
                  bname = bname.substr(bname.rfind("/") + 1);
                  binner = new block(chan, bname);
                  if (dlog.fError.size() > 0) {
                     dlog.fError = "dilog::block::replay error: " + dlog.fError;
                     return false;
                  }
                  binner->ireplay = irep + 1;
                  if (!binner->replay(irep)) {
                     dlog.fError = "dilog::block::replay error: " + dlog.fError;
                     return false;
                  }
                  delete binner;
                  if (dlog.fError.size() > 0) {
                     dlog.fError = "dilog::block::replay error: " + dlog.fError;
                     return false;
                  }
               }
               else {
                  binner = dlog.fBlocks[path];
                  if (!binner->enter()) {
                     dlog.fError = "dilog::block::replay error: " + dlog.fError;
                     return false;
                  }
                  binner->ireplay = irep + 1;
                  if (!binner->replay(irep)) {
                     dlog.fError = "dilog::block::replay error: " + dlog.fError;
                     return false;
                  }
                  if (!binner->exit()) {
                     dlog.fError = "dilog::block::replay error: " + dlog.fError;
                     return false;
                  }
               }
            }
            else if (mexpected.find("]]") == 0) {
               if (mexpected.substr(2) == getPath()) {
                  return true;
               }
               else {
                  dlog.fError = "dilog::block::replay error: " + dlog.fError;
                  return false;
               }
            }
            else {
               mexpected = "[" + prefix + "]" + mexpected.substr(2);
               for (std::string nextmsg; std::getline(*fReading, nextmsg);) {
                  ++fLineno;
                  verify_line(nextmsg, fLineno);
                  if (nextmsg != mexpected) {
                     return next_block(nextmsg);
                  }
                  // std::cerr << "next_block replays match step "
                  //           << ireplay << "/" << fRecord.size()
                  //           << " at lineno " << fLineno << ": "
                  //           << mexpected << std::endl;
                  fMatched[ireplay] = fLineno;
                  break;
               }
            }
         }
         if (!*dlog.fReading) {
            dlog.fError = "dilog::block::replay error: "
                          "expected next line in block"
                          " \"" + getPath() + "\" at line "
                          + std::to_string(dlog.fLineno)
                          + " in " + channel + ".dilog"
                          + " to be " + mexpected +
                          "but found end-of-file instead.";
            return false;
         }
         return true;
      }

      std::string getPath() { return prefix + "/" + name; }

      friend class dilog;
   };
   
   static dilog &get(const std::string &channel, bool threadsafe=true)
   {
    // This is the main factory method for dilog objects. Ownership of
    // the object reference returned is retained by the dilog framework
    // and returned to the user as a borrowed reference. Channel should
    // be a unique name for this dilog channel, and becomes the filename
    // of the dilog trace file <channel>.dilog that is read in response
    // to subsequent printf calls on this channel, or written if the
    // file does not exist at runtime.
    //
    // Normally a given dilog channel should only be accessed from within
    // the same execution thread that created it. Sometimes this rule is
    // violated by static object cleanup running in the main thread at
    // program exit, or some other thread locking mechanism may be in
    // place that prevents cross-thead access violations. In that case,
    // invoke get with threadsafe=false to suppress these unwanted
    // errors, and take care of verifying thread safety yourself.
 
      static std::mutex mutex;
      std::lock_guard<std::mutex> guard(mutex);
      dilogs_map_t &dilogs = get_map();
      std::thread::id tid = std::this_thread::get_id();
      if (dilogs.find(channel) == dilogs.end()) {
         dilogs[channel] = new dilog(channel);
         dilogs[channel]->fThread_id = tid;
      }
      if (threadsafe && dilogs[channel]->fThread_id != tid) {
         dilogs[channel]->fError = "dilog::get error: access to channel"
                                   " \"" + channel + "\" attempted"
                                   " from more than one thread";
         std::cerr << dilogs[channel]->fError << std::endl;
      }
      return *dilogs[channel];
   }

   int printf(const char* fmt, ...)
   {
    // This is the primary user-callable method of dilog. Normally it
    // is invoked on a temporary reference to a dilog object returned
    // by get, eg. dilog::get(channel).printf(...). The arguments to 
    // printf are identical to the stdio printf function. If called
    // with a pending error in fError from a previous failed operation,
    // report the error again.
 
      if (fError.size() > 0) {
         std::cerr << "dilog::printf error: "
                   << "a fatal error has occurred this channel "
                   << fChannel << ", cannot continue." << std::endl
                   << fError << std::endl;
         clean_exit();
      }
      const unsigned int max_message_size(999);
      char msg[max_message_size + 1];
      va_list args;
      va_start(args, fmt);
      int bytes = vsnprintf(msg, max_message_size, fmt, args);
      va_end(args);
      block &top = *fBlocks.top();
      char *eos = strchr(msg, '\n');
      if (eos != NULL)
         *eos = 0;
      std::string message = "[" + top.prefix + "]" + msg;
      if (fWriting) {
         *fWriting << message << std::endl;
         ++fLineno;
      }
      else {
         logger(message);
         check_message(msg);
         if (fBlocks.size() > 1) {
            // std::cerr << "printf records match step " 
            //           << fRecord.size() << " at lineno "
            //           << fLineno << ": []" << msg
            //           << std::endl;
            fMatched[fRecord.size()] = fLineno;
            fRecord.push_back("[]" + std::string(msg));
         }
      }
      return bytes;
   }

   int get_lineno() const {
      return fLineno;
   }

 protected:
   dilog() = delete;
   dilog(const std::string& channel)
    : fLineno(0), fChannel(channel)
   {
      std::string fname(channel);
      fname += ".dilog";
      fReading = new std::ifstream(fname.c_str());
      if (fReading->good()) {
         fWriting = 0;
         fLogging = new std::ofstream((fname + "2").c_str());
      }
      else {
         fReading = 0;
         fLogging = 0;
         fWriting = new std::ofstream(fname.c_str());
      }
      block *bot = new block;
      bot->prefix = channel;
      fBlocks.push(bot);
      fBlock = bot;
   }

   ~dilog() {
      if (fReading)
         delete fReading;
      if (fWriting)
         delete fWriting;
      if (fLogging)
         delete fLogging;
      delete fBlocks.top();
   }

   void check_message(const std::string message)
   {
    // Validate printf message against the next content found in the input
    // file, and report a fatail error if the match fails.
 
      size_t nl;
      std::string msg(message);
      while ((nl = msg.find('\n')) != msg.npos)
         msg.erase(nl);
      block &top = *fBlocks.top();
      std::string mexpected = "[" + top.prefix + "]" + msg;
      for (std::string nextmsg; std::getline(*fReading, nextmsg);) {
         ++fLineno;
         verify_line(nextmsg, fLineno);
         int nextline = fLineno;
         if (nextmsg == mexpected)
            return;
         else if (next_block(nextmsg))
            continue;
         fError = "dilog::check_message error: expected dilog message"
                  " \"" + msg + "\" at line " + std::to_string(nextline)
                  + " in " + fChannel + ".dilog but found \"" 
                  + nextmsg + "\" instead, search stopped at line " +
                  std::to_string(fLineno);
         std::cerr << fError << std::endl;
         clean_exit();
      }
      fError = "dilog::check_message error: read error from input file " +
               fChannel + ".dilog after line " + std::to_string(fLineno) +
               ": expected \"" + mexpected + "\" but found end-of-file.";
      std::cerr << fError << std::endl;
      clean_exit();
   }

   void verify_line(std::string msg, int lineno)
   {
      std::streampos posaved = fReading->tellg();
      fReading->seekg(0);
      int line = 0;
      std::string nextmsg;
      for (; std::getline(*fReading, nextmsg);) {
         if (++line == lineno)
            break;
      }
      if (line != lineno) {
         fError = "dilog::verify_line error on line " + 
                  std::to_string(line) + ": end of file " + fChannel +
                  " found seeking line " + std::to_string(lineno);
         clean_exit();
      }
      else if (nextmsg != msg) {
         fError = "dilog::verify_line error on line " + std::to_string(line) +
                  ": found \"" + nextmsg + "\" at line " +
                  std::to_string(lineno) + " of file " +
                  fChannel + ", expected \"" + msg + "\"";
         clean_exit();
      }
      fReading->seekg(posaved);
   }

   void clean_exit()
   {
      std::cerr << fError << std::endl;
      exit(9);
   }

   void logger(std::string line)
   {
      if (fLogging) {
         *fLogging << line << " at line " << fLineno << std::endl;
      }
   }

   unsigned int fLineno;                   // current line number in dilog file
   std::string fChannel;                   // name of this channel
   std::ifstream *fReading;                // non-zero if reading
   std::ofstream *fWriting;                // non-zero if writing
   std::ofstream *fLogging;                // non-zero if writing
   block *fBlock;                          // current innermost block
   std::map<std::string, block*> fBlocks;  // table of all active blocks
   std::vector<std::string> fRecord;       // record of block actions for replay
   std::thread::id fThread_id;             // thread where this channel was created
   std::string fError;                     // pending error message on this channel
 
   // fMatched is a record of the line numbers in the input file that
   // last matched the lines of the fRecord replay vector, keyed by index
   // into the fRecord vector.
   std::map<unsigned int, unsigned int> fMatched;

 private:
   class dilogs_holder {
    public:
      dilogs_map_t fDilogs;
      dilogs_holder() {}
      ~dilogs_holder() {
         dilogs_map_t &dilogs = get_map();
         for (auto iter : dilogs) {
            delete iter.second;
            dilogs.erase(iter.first);
         }
      }
   };

   static dilogs_map_t& get_map() {
      static dilogs_holder holder;
      return holder.fDilogs;
   }
};
