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
#include <stack>
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
   class trace;
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
      std::map<std::string, std::map<std::streampos, unsigned int> > blinks;

      // flinks is similar to blinks, except that it points to the first
      // line past the last iteration of the block set in the input stream.
      // If flinks is not set, the input parser has not yet encountered
      // the end of this block set.
      std::map<std::string, std::map<std::streampos, unsigned int> > flinks;

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
         //
         // This constructor is only called from user code, never internally.
 
         dilog &dlog = dilog::get(channel, threadsafe);
         parent = dlog.fBlock;
         prefix = parent->getPath();
         if (dlog.fError.size() > 0)
            dlog.clean_exit("dilog::block constructor");
         if (dlog.fBlocks.find(getPath()) != dlog.fBlocks.end()) {
            dlog.fBlocks[getPath()]->parent = 0;
            delete dlog.fBlocks[getPath()];
         }
         dlog.fBlocks[getPath()] = this;
         if (dlog.fWriting) {
            *dlog.fWriting << "[" << getPath() << "[" << std::endl;
            ++dlog.fLineno;
         }
         else {
            trace t(*this, "constructor", "enter");
            if (!enter()) {
               dlog.clean_exit("dilog::block constructor");
            }
         }
         ireplay = dlog.fRecord.size();
         dlog.fBlock = this;
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
         //
         // This destructor is only called from user code, never internally.
 
         if (traces.size() != 0) {
            std::cerr << "dilog::block destructor error: "
                      << "destructor called from inside the call stack "
                      << "of this object's own methods!" << std::endl
                      << "This is an algorithm bug, "
                      << "with results that are generally catastrophic."
                      << std::endl;
            traceback(std::cerr);
         }
         if (parent == 0)
            return;

         dilog &dlog = dilog::get(chan, false);
         if (dlog.fError.size() > 0)
            return;
         if (dlog.fWriting) {
            *dlog.fWriting << "]" << getPath() << "]" << std::endl;
            ++dlog.fLineno;
         }
         else {
            trace t(*this, "destructor", "exit");
            if (!exit()) {
               // std::cerr << "dilog::block destructor error: " 
               //          << dlog.fError << std::endl;
            }
         }
         dlog.fBlocks[getPath()] = new block(*this);
         dlog.fBlock = parent;
      }

      void print(std::ostream &serr=std::cerr)
      {
         // Print the internal state of the block to output
         // stream serr, useful for diagnostics in debugging
         // user code.
 
         serr << "dilog::block(" << getPath() << ") {" << std::endl
              << "   chan: " << chan << "," << std::endl
              << "   name: " << name << "," << std::endl
              << "   prefix: " << prefix << "," << std::endl
              << "   base: " << base << "," << std::endl
              << "   beginline: " << beginline << "," << std::endl
              << "   ireplay: " << ireplay << "," << std::endl
              << "   parent: " << parent << "," << std::endl
              << "   blinks: { ";
         for (auto blink : blinks) {
            if (blink.second.size() > 0) {
               serr << std::endl
                    << "      " << blink.first << ": {";
               for (auto bpair : blink.second) {
                  serr << std::endl
                       << "         " << bpair.first
                       << ": " << bpair.second << ",";
               }
               serr << "},";
            }
         }
         serr << "}," << std::endl
              << "   flinks: { ";
         for (auto flink : flinks) {
            if (flink.second.size() > 0) {
               serr << std::endl
                    << "      " << flink.first << ": {";
               for (auto fpair : flink.second) {
                  serr << std::endl
                       << "         " << fpair.first
                       << ": " << fpair.second << ",";
               }
               serr << "},";
            }
         }
         serr << "}," << std::endl
              << "}" << std::endl;
      }

    protected:
      block(const block &src)
       : chan(src.chan), name(src.name), prefix(src.prefix),
         base(0), beginline(0), ireplay(0), parent(0)
      {
         // Protected copy constructor, needed to save copies of
         // inactive blocks for potential use during replay.
      }

      bool enter()
      {
         // This method is called when a dilog channel open in read mode
         // enters a new block and descends a level in the block stack.
         // If replay is active on this channel, the value of fReplay 
         // at entry is already set to the value it will need to have
         // once enter() has done its thing.
 
         dilog &dlog = dilog::get(chan, false);
         beginline = dlog.fLineno;
         base = dlog.fReading->tellg();
         std::string mexpected = "[" + getPath() + "[";
         std::string nextmsg;
         for (; !std::getline(*dlog.fReading, nextmsg).bad();) {
            ++dlog.fLineno;
            dlog.verify_line(nextmsg, dlog.fLineno);
            if (nextmsg == mexpected) {
               if (dlog.fReplay) {
                  // std::cerr << "block::enter replays match step " 
                  //          << dlog.fReplay-1 << "/" << dlog.fRecord.size()
                  //          << " at lineno " << dlog.fLineno 
                  //          << ": [[" << getPath() << std::endl;
                  dlog.fMatched[dlog.fReplay-1] = dlog.fLineno;
               }
               else {
                  // std::cerr << "block::enter records match step " 
                  //           << dlog.fRecord.size() << " at lineno "
                  //           << dlog.fLineno << ": [[" << getPath()
                  //           << std::endl;
                  dlog.fMatched[dlog.fRecord.size()] = dlog.fLineno;
                  dlog.fRecord.push_back("[[" + getPath());
                  dlog.logger("[" + getPath() + "[");
               }
               break;
            }
            else if (parent->blinks[getPath()].size() > 0 &&
                     parent->flinks[getPath()].size() == 0)
            {
               parent->flinks[getPath()][base] = beginline;
               base = parent->blinks[getPath()].begin()->first;
               beginline = parent->blinks[getPath()].begin()->second;
               parent->blinks[getPath()].erase(base);
               dlog.fReading->clear();
               dlog.fReading->seekg(base);
               dlog.fLineno = beginline;
               trace t(*this, "enter", "enter");
               return enter();
            }
            else {
               trace t(*this, "enter", "parent.next");
               if (parent->next(nextmsg)) {
                  beginline = dlog.fLineno;
                  base = dlog.fReading->tellg();
                  continue;
               }
            }
            dlog.fError = "dilog::block::enter error: "
                          "expected new block"
                          " \"" + getPath() + "\""
                          " at line " + std::to_string(dlog.fLineno)
                          + " in " + chan + ".dilog "
                          "but found \"" + nextmsg + "\" instead.";
            return false;
         }
         parent->blinks[getPath()].erase(base);
         dlog.fBlock = this;
         return true;
      }

      bool exit()
      {
         // This method is called when a dilog channel open in read mode
         // exits a block and ascends a level in the block stack.

         dilog &dlog = dilog::get(chan, false);
         if (dlog.fBlock != this) {
            dlog.fError = "dilog::block::exit error: "
                          "block nesting error, expected block " +
                          getPath() + " but found " + dlog.fBlock->getPath();
            return false;
         }
         while (blinks[getPath()].size() != 0) {
            trace t(*this, "exit", "next");
            if (!next())
               continue;
            dlog.fError = "dilog::block::exit error: "
                          "no matching blocks found!";
            return false;
         }
         std::string mexpected = "]" + getPath() + "]";
         std::string nextmsg;
         for (; !std::getline(*dlog.fReading, nextmsg).bad();) {
            ++dlog.fLineno;
            dlog.verify_line(nextmsg, dlog.fLineno);
            if (nextmsg == mexpected) {
               if (dlog.fReplay) {
                  // std::cerr << "block::exit replays match step " 
                  //           << dlog.fReplay << "/" << dlog.fRecord.size()
                  //           << " at lineno " << dlog.fLineno 
                  //           << ": ]]" << getPath()
                  //           << " with " << parent->blinks[getPath()].size()
                  //           << " iterations still unmatched" << std::endl;
                  dlog.fMatched[dlog.fReplay] = dlog.fLineno;
               }
               else {
                  // std::cerr << "block::exit records match step " 
                  //           << dlog.fRecord.size() << " at lineno "
                  //           << dlog.fLineno << ": ]]" << getPath()
                  //           << " with " << parent->blinks[getPath()].size()
                  //           << " iterations still unmatched" << std::endl;
                  dlog.fMatched[dlog.fRecord.size()] = dlog.fLineno;
                  dlog.fRecord.push_back("]]" + getPath());
                  dlog.logger("]" + getPath() + "]");
               }
               break;
            }
            else {
               // std::cerr << "block::exit from " << getPath() 
               //           << " expected at line " << dlog.fLineno
               //           << " but found \"" << nextmsg << "\""
               //           << " so try the next iteration." << std::endl;
               trace t(*this, "exit", "next2");
               if (next(nextmsg)) {
                  continue;
               }
            }
            dlog.fError = "dilog::block::exit error: "
                          "expected end of block "
                          "\"" + getPath() + "\" at line " + 
                          std::to_string(dlog.fLineno) + 
                          " in " + chan + ".dilog"
                          " but found \"" + nextmsg + "\""
                          " instead of \"" + mexpected + "\"";
            return false;
         }
         if (parent->parent == 0) {
            dlog.fMatched.clear();
            dlog.fRecord.clear();
         }
         if (parent->flinks[getPath()].size() > 0) {
            if (parent->blinks[getPath()].size() > 0) {
               base = parent->blinks[getPath()].begin()->first;
               beginline = parent->blinks[getPath()].begin()->second;
               dlog.fReading->clear();
               dlog.fReading->seekg(base);
               dlog.fLineno = beginline;
            }
            else {
               base = parent->flinks[getPath()].begin()->first;
               beginline = parent->flinks[getPath()].begin()->second;
               dlog.fReading->clear();
               dlog.fReading->seekg(base);
               dlog.fLineno = beginline;
            }
         }
         dlog.fBlock = parent;
         return true;
      }

      bool next(const std::string lastmsg="")
      {
         // Entry here assumes that a mismatch has been found between the 
         // expected message and the lastmsg string that was last read from
         // the input stream. The task of next_block is to search through
         // all remaining block iterations to see if there might be another
         // chance to fulfill the expectation. If the search is successful,
         // it returns true with the input file and block sequence replayed
         // up to the point immediately preceding the check of lastmsg.
 
         dilog &dlog = dilog::get(chan, false);
         if (parent == 0) {
            dlog.fError = "dilog::block::next error: "
                          "no more iterations to search.";
            return false;
         }
         blinks.clear();
         flinks.clear();
         parent->blinks[getPath()][base] = beginline;
         auto flink = parent->flinks[getPath()];
         if (flink.size() > 0) {
            auto blink = parent->blinks[getPath()];
            auto biter = blink.find(base);
            if (biter != blink.end() && ++biter != blink.end()) {
               dlog.fReading->clear();
               dlog.fReading->seekg(biter->first);
               dlog.fLineno = biter->second;
            }
            else {
               trace t(*this, "next", "parent.next");
               return parent->next(lastmsg);
            }
         }
         else {
            std::string mexpected = "]" + getPath() + "]";
            if (lastmsg != mexpected) {
               std::string nextmsg;
               for (; !std::getline(*dlog.fReading, nextmsg).bad();) {
                  ++dlog.fLineno;
                  dlog.verify_line(nextmsg, dlog.fLineno);
                  if (nextmsg == mexpected) {
                     break;
                  }
               }
            }
         }
         dlog.fReplay = ireplay;
         if (dlog.fError.size() == 0) {
            trace t(*this, "next", "enter");
            if (!enter()) {
               dlog.fError = "dilog::block::next error: " + dlog.fError;
               return false;
            }
         }
         if (dlog.fError.size() == 0) {
            trace t(*this, "next", "replay");
            if (!replay()) {
               dlog.fError = "dilog::block::next error: " + dlog.fError;
               return false;
            }
         }
         if (dlog.fReplay >= dlog.fRecord.size())
            dlog.fReplay = 0;
         return true;
      }

      bool replay()
      {
         // Entry here assumes that this block is currently entered on the
         // input stream and the replay is in progress. The task is to 
         // complete the replay through the end of fRecord, constructing
         // and destroying nested blocks as needed along the way and 
         // return true, or else to die trying.
 
         dilog &dlog = dilog::get(chan, false);
         std::string mexpected;
         for (; dlog.fReplay < dlog.fRecord.size(); ++dlog.fReplay) {
            mexpected = dlog.fRecord[dlog.fReplay];
            if (mexpected.find("[[") == 0) {
               block *binner;
               std::string path = mexpected.substr(2);
               if (dlog.fBlocks.find(path) != dlog.fBlocks.end()) {
                  binner = dlog.fBlocks[path];
                  binner->parent = this;
                  binner->ireplay = ++dlog.fReplay;
                  trace *t = new trace(*this, "replay", "child.enter");
                  binner->enter();
                  delete t;
                  if (dlog.fError.size() > 0) {
                     dlog.fError = "dilog::block::replay error: " + dlog.fError;
                     return false;
                  }
                  t = new trace(*this, "replay", "child.replay");
                  binner->replay();
                  delete t;
                  if (dlog.fError.size() > 0) {
                     dlog.fError = "dilog::block::replay error: " + dlog.fError;
                     return false;
                  }
                  if (dlog.fReplay < dlog.fRecord.size()) {
                     t = new trace(*this, "replay", "child.exit");
                     binner->exit();
                     delete t;
                     if (dlog.fError.size() > 0) {
                        dlog.fError = "dilog::block::replay error: " + dlog.fError;
                        return false;
                     }
                  }
               }
               else {
                  dlog.fError = "dilog::block::replay error: "
                                "prior block " + getPath() + 
                                " missing during replay, cannot continue";
                  return false;
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
               mexpected = "[" + getPath() + "]" + mexpected.substr(2);
               std::string nextmsg;
               for (; !std::getline(*dlog.fReading, nextmsg).bad();) {
                  ++dlog.fLineno;
                  dlog.verify_line(nextmsg, dlog.fLineno);
                  if (nextmsg != mexpected) {
                     trace t(*this, "replay", "next");
                     return next(nextmsg);
                  }
                  // std::cerr << "dilog::block::replay matches step "
                  //           << dlog.fReplay << "/" << dlog.fRecord.size()
                  //           << " at lineno " << dlog.fLineno << ": "
                  //           << mexpected << std::endl;
                  dlog.fMatched[dlog.fReplay] = dlog.fLineno;
                  break;
               }
            }
         }
         return true;
      }

      std::string getPath() const {
         if (name.size() > 0)
            return prefix + "/" + name;
         else
            return chan;
      }

      friend class dilog;

    protected:
      std::stack<trace*> traces;       // stack of currently active trace objects

      friend class trace;
      void traceback(std::ostream &serr) {
         serr << "Traceback on dilog::block " << getPath() << ":" << std::endl;
         std::stack<trace*> spool;
         std::string caller;
         for (unsigned int depth=0; traces.size() > 0; ++depth) {
            trace *t = traces.top();
            serr << depth << ": " << t->callee 
                 << " at line " << t->lineno << " with replay=" << t->replay
                 << std::endl;
            if (caller.size() > 0 && caller != t->callee) {
               serr << depth << ": " << t->caller 
                    << " at line ??? with replay=???"
                    << std::endl;
            }
            std::string caller = t->caller;
            spool.push(t);
            traces.pop();
         }
         while (spool.size() > 0) {
            trace *t = spool.top();
            traces.push(t);
            spool.pop();
         }
      }
   };

   class trace {

    // Helper class for tracing calls to the dilog and dilog::block
    // class methods. These can be disabled once debugging is done.

    private:
      std::string caller;
      std::string callee;
      std::string channel;
      unsigned int lineno;
      unsigned int replay;
      block *target;

    public:
      trace(block &b, std::string from, std::string to)
       : caller(from), callee(to), channel(b.chan), target(&b)
      {
         dilog &dlog = dilog::get(channel, false);
         lineno = dlog.fLineno;
         replay = dlog.fReplay;
         tracefile() << "[" << target->traces.size() << "] "
                     << target->getPath() << "." << caller << " >> " << callee
                     << " at line " << lineno << " with replay=" << replay
                     << std::endl;
         target->traces.push(this);
      }

      ~trace() {
         dilog &dlog = dilog::get(channel, false);
         tracefile() << "[" << target->traces.size() << "] "
                     << target->getPath() << "." << caller << " << " << callee
                     << " at line " << lineno << "=>" << dlog.fLineno
                     << " with replay=" << replay << "=>" << dlog.fReplay
                     << "/" << dlog.fRecord.size() << std::endl;
         if (this == target->traces.top()) {
            target->traces.pop();
         }
         else {
            std::cerr << "dilog::trace destructor error: "
                      << "call trace ordering error." << std::endl
                      << "This indicates an algorithm bug internal to "
                      << "the dilog class, please report to the package "
                      << "maintaniners." << std::endl;
         }
      }

      std::ostream &tracefile() {
         static std::ofstream tfile;
         if (!tfile.good())
            tfile.open("trace.dilog");
         if (tfile.good())
            return tfile;
         else
            return std::cerr;
      }

      friend class block;
   };

   static dilog &get(const std::string &channel, bool threadsafe=true)
   {
    // This is the main factory method for dilog objects. Ownership of
    // the object reference returned is retained by the dilog framework
    // and returned to the user as a borrowed reference. Channel should
    // be a unique name for this dilog channel, and becomes the filename
    // of the dilog output file <channel>.dilog that is read in response
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
         std::cerr << "a fatal error has occurred on channel "
                   << fChannel << ", cannot continue." << std::endl
                   << fError << std::endl;
         clean_exit("dilog::printf");
      }
      const unsigned int max_message_size(999);
      char msg[max_message_size + 1];
      va_list args;
      va_start(args, fmt);
      int bytes = vsnprintf(msg, max_message_size, fmt, args);
      va_end(args);
      char *eos = strchr(msg, '\n');
      if (eos != NULL)
         *eos = 0;
      std::string message = "[" + fBlock->getPath() + "]" + msg;
      if (fWriting) {
         *fWriting << message << std::endl;
         ++fLineno;
      }
      else {
         check_message(msg);
         if (fReplay) {
            // std::cerr << "printf replays match step " 
            //           << fReplay << "/" << fRecord.size()
            //           << " at lineno " << fLineno 
            //           << ": []" << msg << std::endl;
            fMatched[fReplay] = fLineno;
         }
         else {
            // std::cerr << "printf records match step " 
            //           << fRecord.size() << " at lineno "
            //           << fLineno << ": []" << msg
            //           << std::endl;
            fMatched[fRecord.size()] = fLineno;
            fRecord.push_back("[]" + std::string(msg));
            logger("[" + fBlock->getPath() + "]" + msg);
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
    : fLineno(0), fChannel(channel), fReplay(0)
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
      bot->chan = channel;
      fBlocks[channel] = bot;
      fBlock = bot;
   }

   ~dilog() {
      if (fReading)
         delete fReading;
      if (fWriting)
         delete fWriting;
      if (fLogging)
         delete fLogging;
   }

   void check_message(const std::string message)
   {
    // Validate printf message against the next content found in the input
    // file, and report a fatail error if the match fails.
 
      size_t nl;
      std::string msg(message);
      while ((nl = msg.find('\n')) != msg.npos)
         msg.erase(nl);
      std::string mexpected = "[" + fBlock->getPath() + "]" + msg;
      std::string nextmsg;
      for (; !std::getline(*fReading, nextmsg).bad();) {
         ++fLineno;
         verify_line(nextmsg, fLineno);
         int nextline = fLineno;
         if (nextmsg == mexpected) {
            return;
         }
         else {
            if (fBlock->next(nextmsg)) {
               continue;
            }
         }
         fError = "expected dilog message"
                  " \"" + mexpected + "\" at line " + std::to_string(nextline)
                  + " in " + fChannel + ".dilog but found \"" 
                  + nextmsg + "\" instead, search stopped at line " +
                  std::to_string(fLineno);
         clean_exit("dilog::check_message");
      }
      fError = "read error from input file " +
               fChannel + ".dilog after line " + std::to_string(fLineno) +
               ": expected \"" + mexpected + "\" but found end-of-file.";
      clean_exit("dilog::check_message");
   }

   void verify_line(std::string msg, int lineno)
   {
      std::streampos posaved = fReading->tellg();
      fReading->clear();
      fReading->seekg(0);
      int line = 0;
      std::string nextmsg;
      for (; !std::getline(*fReading, nextmsg).bad();) {
         if (++line == lineno)
            break;
      }
      if (line != lineno) {
         fError = "error on file " + fChannel + ".dilog at line " +
                  std::to_string(line) + ": end of file " + fChannel +
                  " found seeking line " + std::to_string(lineno);
         clean_exit("dilog::verify_line");
      }
      else if (nextmsg != msg) {
         fError = "error on line " + std::to_string(line) +
                  ": found \"" + nextmsg + "\" at line " +
                  std::to_string(lineno) + " of file " +
                  fChannel + ", expected \"" + msg + "\"";
         clean_exit("dilog::verify_line");
      }
      fReading->clear();
      fReading->seekg(posaved);
   }

   void clean_exit(std::string src="")
   {
      if (src.size() > 0)
         std::cerr << "Fatal error from " << src << ": ";
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
   unsigned int fReplay;                   // state flag indicating replay in progress
 
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
