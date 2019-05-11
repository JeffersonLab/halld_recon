//
//    File: async_filebuf.h
// Created: Wed May 1 09:22:00 EST 2019
// Creator: richard.t.jones at uconn.edu
//

#ifndef _ASYNC_FILEBUF_
#define _ASYNC_FILEBUF_

#include <fstream>
#include <iostream>
#include <cstdio>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

//#define VERBOSE_ASYNC_FILEBUF 1
//#define SHADOW_DEBUG 1

#define THIS_ASYNCFB "(" << (void*)this << ")"

class async_filebuf : public std::filebuf {

 public:
   async_filebuf(int segsize=1000000, int segcount=3, int lookback=1);
   virtual ~async_filebuf();

   async_filebuf* open(const std::string fname, std::ios::openmode mode) {
#if VERBOSE_ASYNC_FILEBUF
      std::cout << THIS_ASYNCFB << "async_filebuf::open(" << fname << "," << mode << ")" << std::endl;
#endif
      std::filebuf::open(fname, mode);
#if SHADOW_DEBUG
      shadow_ifs.open(fname, mode);
#endif
      return this;
   }

   async_filebuf* close() {
#if VERBOSE_ASYNC_FILEBUF
      std::cout << THIS_ASYNCFB << "async_filebuf::close()" << std::endl;
#endif
      if (readloop_active)
         readloop_terminate();
      std::filebuf::close();
#if SHADOW_DEBUG
      shadow_ifs.close();
#endif
      return this;
   }

   std::streamsize in_avail() {
#if VERBOSE_ASYNC_FILEBUF
      std::cout << THIS_ASYNCFB << "async_filebuf::in_avail()" << std::endl;
#endif
      if (readloop_active)
         return buffer_egptr - buffer_gptr;
      return std::filebuf::in_avail();
   }

   int snextc() {
#if VERBOSE_ASYNC_FILEBUF
      std::cout << THIS_ASYNCFB << "async_filebuf::snextc()" << std::endl;
#endif
      if (readloop_active) {
         if (buffer_gptr < buffer_egptr)
            return (unsigned char)*(++buffer_gptr);
         if (underflow() == EOF)
            return EOF;
         return snextc();
      }
      return std::filebuf::snextc();
   }

   int sbumpc() {
      if (readloop_active) {
         if (buffer_gptr < buffer_egptr)
            return (unsigned char)*(buffer_gptr++);
         if (underflow() == EOF)
            return EOF;
         return sbumpc();
      }
      return std::filebuf::sbumpc();
   }

   int sgetc() {
      if (readloop_active) {
         if (buffer_gptr == buffer_egptr)
            return underflow();
         else
            return (unsigned char)*buffer_gptr;
      }
      return std::filebuf::sgetc();
   }

   int sputbackc(int c) {
      if (readloop_active) {
         int seg = (buffer_gptr - buffer_start) / segment_size;
         char *eback = buffer_start + seg * segment_size;
         if (buffer_gptr > eback || c == (unsigned char)buffer_gptr[-1])
            return (unsigned char)*(--buffer_gptr);
         else
            return pbackfail();
      }
      return std::filebuf::sputbackc(c);
   }

   int sungetc() {
      if (readloop_active) {
         int seg = (buffer_gptr - buffer_start) / segment_size;
         char *eback = buffer_start + seg * segment_size;
         if (buffer_gptr > eback)
            return (unsigned char)*(--buffer_gptr);
         else
            return pbackfail();
      }
      return std::filebuf::sungetc();
   }

 protected:
   int pbackfail(char c=EOF) {
      if (readloop_active)
         return EOF;
      return std::filebuf::pbackfail(c);
   }

   virtual std::streambuf* setbuf(char* s, std::streamsize n) {
#if VERBOSE_ASYNC_FILEBUF
      std::cout << THIS_ASYNCFB << "async_filebuf::setbuf(s," << n << ")" << std::endl;
#endif
      if (! readloop_active) {
         buffer_start = s;
         buffer_end = s + n;
         buffer_eback = buffer_end;
         buffer_gptr = buffer_end;
         buffer_egptr = buffer_end;
      }
      return this;
   }

   virtual std::streamsize showmanyc() {
      if (readloop_active) {
         if (buffer_gptr == buffer_egptr)
            underflow();
         return buffer_egptr - buffer_gptr;
      }
      return std::filebuf::showmanyc();
   }

   virtual int uflow() {
#if VERBOSE_ASYNC_FILEBUF
      std::cout << THIS_ASYNCFB << "async_filebuf::uflow()" << std::endl;
#endif
      if (readloop_active) {
         if (underflow() == EOF)
            return EOF;
         return (unsigned char)*buffer_gptr++;
      }
      return std::filebuf::uflow();
   }

   virtual int underflow();

   virtual std::streamsize xsgetn(char* s, std::streamsize n);

   virtual std::streampos seekoff(std::streamoff off, std::ios::seekdir way,
                                  std::ios::openmode which);

   virtual std::streampos seekpos(std::streampos pos, std::ios::openmode which);

 protected:
   char *buffer;
   char *buffer_start;
   char *buffer_end;
   char *buffer_eback;
   char *buffer_gptr;
   char *buffer_egptr;

   enum segment_state {sFull, sEmptying, sEmpty, sFilling};
   std::vector<segment_state> segment_cond;
   std::vector<std::streampos> segment_pos;
   std::vector<std::streamsize> segment_len;
   int segment_size;
   int segment_count;
   int segment_lookback;
   int segment_backstop;
   int readloop_active;
   std::mutex readloop_lock;
   std::condition_variable readloop_wake;
   std::condition_variable readloop_woke;
   std::thread *readloop_thread;

#if SHADOW_DEBUG
   std::ifstream shadow_ifs;
#endif

 protected:
   int readloop_initiate();
   int readloop_terminate();
   int readloop();

   int segment() { return (buffer_gptr - buffer_start) / segment_size; }
   int segoff()  { return (buffer_gptr - buffer_start) % segment_size; }

   std::streampos getpos() {
      if (! readloop_active)
         return this->std::filebuf::seekoff(0, std::ios::cur, std::ios::in);
      else if (buffer_gptr == buffer_egptr) {
         if (segment_len[segment()] > 0)
            underflow();
         if (buffer_gptr == buffer_egptr)
            return this->std::filebuf::seekoff(0, std::ios::cur, std::ios::in);
      }
      std::streampos pos = segment_pos[segment()] + std::streamsize(segoff());
#if VERBOSE_ASYNC_FILEBUF
      std::cout << THIS_ASYNCFB << "async_filebuf::getpos() returns " << pos << " in segment " << segment() << std::endl;
#endif
      return pos;
   }
};

#endif
