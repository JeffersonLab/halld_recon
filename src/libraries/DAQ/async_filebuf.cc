//
//    File: async_filebuf.cc
// Created: Wed May 1 09:22:00 EST 2019
// Creator: richard.t.jones at uconn.edu
//
// Require: --std=c++11 -pthread
//

// This code does not compile on Mac OS X using
// Xcode 10.1 = Apple LLVM version 10.0.0 (clang-1000.10.44.4)
// 6/3/2019 DL
#ifndef __APPLE__

#include <string>
#include <string.h>
#include <stdexcept>

#include "async_filebuf.h"

async_filebuf::async_filebuf(int segsize, int segcount, int lookback)
 : segment_size(segsize), 
   segment_count(segcount),
   segment_lookback(lookback),
   readloop_active(0)
{
#if VERBOSE_ASYNC_FILEBUF
   std::cout << THIS_ASYNCFB << "async_filebuf::async_filebuf(" << segsize << "," << segcount << "," << lookback << ")" << std::endl;
#endif
   if (segment_count < segment_lookback + 2) {
      std::string errmsg("async_filebuf error - insufficient"
                         " segment count for look-back.");
      std::cerr << errmsg << std::endl;
      throw std::range_error(errmsg);
   }
   int bufsize = segment_size * segment_count;
   buffer = new char[bufsize];
   setbuf(buffer, bufsize);
}

async_filebuf::~async_filebuf()
{
#if VERBOSE_ASYNC_FILEBUF
   std::cout << THIS_ASYNCFB << "async_filebuf::~async_filebuf()" << std::endl;
#endif
   if (is_open())
      close();
   delete [] buffer;
}

int async_filebuf::readloop_initiate()
{
#if VERBOSE_ASYNC_FILEBUF
   std::cout << THIS_ASYNCFB << "async_filebuf::readloop_initiate()" << std::endl;
#endif
   if (readloop_active)
      return -1;
   segment_count = 0;
   for (char *p = buffer_start; p < buffer_end; p += segment_size) {
      segment_cond.push_back(sEmpty);
      segment_pos.push_back(0);
      segment_len.push_back(0);
      ++segment_count;
   }
   segment_backstop = 0;
   readloop_active = 1;
   readloop_thread = new std::thread(&async_filebuf::readloop, this);
   buffer_eback = buffer_end;
   buffer_gptr = buffer_end;
   buffer_egptr = buffer_end;
   return 0;
}

int async_filebuf::readloop_terminate()
{
#if VERBOSE_ASYNC_FILEBUF
   std::cout << THIS_ASYNCFB << "async_filebuf::readloop_terminate()" << std::endl;
#endif
   //std::cout << "bang!!!" << std::endl;
   if (readloop_active) {
      std::streampos pos = getpos();
      if (readloop_thread) {
         std::unique_lock<std::mutex> lk(readloop_lock);
         readloop_active = 0;
         readloop_wake.notify_one();
      }
      readloop_thread->join();
      delete readloop_thread;
      readloop_thread = 0;
      std::filebuf::seekpos(pos, std::ios::in);
      buffer_eback = buffer_end;
      buffer_gptr = buffer_end;
      buffer_egptr = buffer_end;
      segment_cond.clear();
      segment_pos.clear();
      segment_len.clear();
   }
   return 0;
}

int async_filebuf::readloop()
{
#if VERBOSE_ASYNC_FILEBUF
   std::cout << THIS_ASYNCFB << "async_filebuf::readloop()" << std::endl;
#endif
   int seg = 0;
   while (readloop_active) {
      std::unique_lock<std::mutex> lk(readloop_lock);
      while (readloop_active && segment_cond[seg] != sEmpty) {
         readloop_wake.wait(lk);
      }
      if (! readloop_active)
         break;
      segment_cond[seg] = sFilling;
      lk.unlock();
      char *sbase = buffer_start + seg * segment_size;
      segment_pos[seg] = this->std::filebuf::seekoff(0, std::ios::cur, std::ios::in);
      std::streamsize nreq = buffer_end - sbase;
      nreq = (nreq > segment_size)? segment_size : nreq;
      segment_len[seg] = std::filebuf::xsgetn(sbase, nreq);
      lk.lock();
      segment_cond[seg] = sFull;
      readloop_woke.notify_one();
      seg = (seg + 1) % segment_count;
   }
   return 0;
}

int async_filebuf::underflow()
{
#if VERBOSE_ASYNC_FILEBUF
   std::cout << THIS_ASYNCFB << "async_filebuf::underflow()" << std::endl;
#endif
   if (!readloop_active) {
      if  (segment_lookback > 0)
         readloop_initiate();
      else
         return std::filebuf::underflow();
   }

   int seg = segment() % segment_count;
   if (segoff() > 0)
      seg = (seg + 1) % segment_count;
   std::unique_lock<std::mutex> lk(readloop_lock);
   while (segment_cond[seg] != sFull) {
      readloop_woke.wait(lk);
   }
   segment_cond[seg] = sEmptying;
   if ((segment_backstop + segment_lookback + 1) % segment_count == seg) {
      segment_cond[segment_backstop] = sEmpty;
      readloop_wake.notify_one();
      segment_backstop = (segment_backstop + 1) % segment_count;
   }
   buffer_eback = buffer_start + seg * segment_size;
   buffer_egptr = buffer_eback + segment_len[seg];
   buffer_gptr = buffer_eback;
   if (segment_len[seg] == 0)
      return EOF;
   return (unsigned char)*buffer_gptr;
}

std::streampos async_filebuf::seekoff(std::streamoff off, std::ios::seekdir way,
                                                          std::ios::openmode which)
{
#if VERBOSE_ASYNC_FILEBUF
   std::cout << THIS_ASYNCFB << "async_filebuf::seekoff(" << off << "," << way << "," << which << ")" << std::endl;
#endif
   if (readloop_active && segment_lookback > 0) {
      if (way == std::ios::beg)
         return seekpos(off, which);
      else if (way == std::ios::cur)
         return seekpos(getpos() + off, which);
      else 
         readloop_terminate();
   }
   return this->std::filebuf::seekoff(off, way, which);
}

std::streampos async_filebuf::seekpos(std::streampos pos, std::ios::openmode which)
{
#if VERBOSE_ASYNC_FILEBUF
   std::cout << THIS_ASYNCFB << "async_filebuf::seekpos(" << pos << "," << which << ")" << std::endl;
#endif
   if (! readloop_active || segment_lookback == 0) {
      return this->std::filebuf::seekpos(pos, which);
   }
   if (pos < std::streampos(0))
      pos = std::streampos(0);
   std::streampos curpos = getpos();
   if (abs(pos - curpos) > segment_lookback * segment_size) {
      if (readloop_active)
         readloop_terminate();
      return this->std::filebuf::seekpos(pos, which);
   }

   int seg = segment();
   while (pos < segment_pos[seg]) {
      segment_cond[seg] = sFull;
      int prevseg = (seg == 0)? segment_count-1 : seg-1;
      if (segment_cond[prevseg] != sEmptying) {
         if (readloop_active)
            readloop_terminate();
         return this->std::filebuf::seekpos(pos, which);
      }
      seg = prevseg;
   }
   while (pos >= segment_pos[seg] + segment_len[seg]) {
      buffer_gptr = buffer_egptr;
      if (underflow() == EOF)
         return std::streampos(std::streamoff(-1));
      seg = (seg + 1) % segment_count;
   }
   int off = pos - segment_pos[seg];
   buffer_eback = buffer_start + seg * segment_size;
   buffer_gptr = buffer_eback + off;
   buffer_egptr = buffer_eback + segment_len[seg];
   return pos;
}

std::streamsize async_filebuf::xsgetn(char* s, std::streamsize n)
{
#if VERBOSE_ASYNC_FILEBUF
   std::cout << THIS_ASYNCFB << "async_filebuf::xsgetn(s," << n << ")" << std::endl;
#endif
   if (segment_lookback == 0) {
      return std::filebuf::xsgetn(s,n);
   }

   std::streamsize nleft=n;
   while (nleft > 0) {
      int nbuf = buffer_egptr - buffer_gptr;
      if (nbuf > 0) {
         nbuf = (nbuf < nleft)? nbuf : nleft; 
         memcpy(s, buffer_gptr, nbuf);
#if VERBOSE_ASYNC_FILEBUF
         std::cout << THIS_ASYNCFB << "memcpy(d, s, " << nbuf << ")" << std::endl;
#endif
#if SHADOW_DEBUG
         shadow_ifs.seekg(getpos());
         char *shadowbuf = new char[nbuf];
         if (shadow_ifs.read(shadowbuf, nbuf) && shadow_ifs.gcount() == nbuf) {
            for (int i=0; i<nbuf; ++i) {
               if (shadowbuf[i] != buffer_gptr[i]) {
                  std::cerr << "Error in async_filebuf::xsgetn - "
                               "data read from buffer does not match "
                               "what reading directly from the file "
                               "gives at the same offset! Cannot continue."
                            << std::endl;
                  exit(6);
               }
            }
         }
         else {
            std::cerr << "Error in async_filebuf::xsgetn - "
                         "error reading from the shadow ifstream input. "
                         "Cannot continue."
                      << std::endl;
            exit(6);
         }
         delete [] shadowbuf;
#endif
         s += nbuf;
         buffer_gptr += nbuf;
         nleft -= nbuf;
      }
      else if (underflow() == EOF) {
         break;
      }
   }
   return n - nleft;
}

#else  // __APPLE__
int async_filebuff_disable_for_mac_osx = 0; // symbol so compiled object isn't empty
#endif // __APPLE__

