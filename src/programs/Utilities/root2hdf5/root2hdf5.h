//
// root2hdf5 - utility for automatic conversion of a ROOT tree
//             into a hdf5 data file for input into tensorflow.
//
// author: richard.t.jones at uconn.edu
// version: may 8, 2023

#ifndef ROOT2HDF5_INCLUDE_
#define ROOT2HDF5_INCLUDE_ 1

#include <iostream>
#include <map>

#include <TLeaf.h>
#include <hdf5.h>

#define MAXLEN_CSTRING 65535
#define MAXLEN_VARRAY 65535

class column_ {
 public:
   column_(TLeaf *column_leaf)
    : leaf(column_leaf), dtype_id(0), mtype_id(0), dset_id(0), chunk_id(0)
   {
      vleaf = leaf->GetLeafCounter(width);
      if (vleaf != 0)
         varwidth = std::min(vleaf->GetMaximum(), MAXLEN_VARRAY);
      else
         varwidth = 1;
      hsize_t dims[2] = {hsize_t(width), 1};
      hsize_t maxdims[2] = {hsize_t(width), rowcount};
      if (width == 1) {
         dspace_id = H5Screate_simple(1, dims + 1, maxdims + 1);
         mspace_id = H5Screate_simple(1, dims + 1, maxdims + 1);
      }
      else {
         dspace_id = H5Screate_simple(2, dims, maxdims);
         mspace_id = H5Screate_simple(2, dims, maxdims);
      }
      mspace_extent = dims[1];
      dspace_extent = 0;
      dset_extent = 0;
      if (vleaf != 0)
         vlen = new hvl_t[chunksize];
      else
         vlen = 0;
      leaves[leaf] = this;
   }
   virtual ~column_() {
      if (dset_id)
         H5Dclose(dset_id);
      if (mspace_id)
         H5Sclose(mspace_id);
      if (dspace_id)
         H5Sclose(dspace_id);
   }

   virtual herr_t write(hid_t file_id, hsize_t nrows) {
      if (leaves.find(vleaf) != leaves.end()) {
         for (unsigned int i=0; i < nrows; ++i) {
            vlen[i].len = leaves[vleaf]->getvalue(i);
            vlen[i].p = address(i);
         }
      }
      if (chunk_id == 0) {
         chunk_id = H5Pcreate(H5P_DATASET_CREATE);
         hsize_t chunks[2] = {hsize_t(width), chunksize};
         if (width == 1)
            H5Pset_chunk(chunk_id, 1, chunks + 1);
         else
            H5Pset_chunk(chunk_id, 2, chunks);
      }
      if (dset_id == 0) {
         dset_id = H5Dcreate(file_id, leaf->GetName(),
                             dtype_id, dspace_id,
                             H5P_DEFAULT, chunk_id, H5P_DEFAULT);
         dset_offset = 0;
         dset_extent = dspace_extent;
      }
      if (dset_offset + nrows > dset_extent) {
         dset_extent = dset_offset + nrows;
         hsize_t new_extent[2] = {hsize_t(width), hsize_t(dset_extent)};
         if (width == 1)
            H5Dset_extent(dset_id, new_extent + 1);
         else
            H5Dset_extent(dset_id, new_extent);
         H5Sclose(dspace_id);
         dspace_id = H5Dget_space(dset_id);
      }
      if (nrows > chunksize) {
         std::cerr << "column_::write error - nrows > chunksize,"
                   << "cannot continue" << std::endl;
      }
      else if (mspace_extent != nrows) {
         H5Sclose(mspace_id);
         hsize_t dims[2] = {hsize_t(width), hsize_t(nrows)};
         hsize_t maxdims[2] = {hsize_t(width), H5S_UNLIMITED};
         if (width == 1)
            mspace_id = H5Screate_simple(1, dims + 1, maxdims + 1);
         else
            mspace_id = H5Screate_simple(2, dims, maxdims);
         mspace_extent = dims[1];
      }
      hsize_t count[2] = {hsize_t(width), hsize_t(nrows)};
      hsize_t offset[2] = {0, dset_offset};
      dset_offset += nrows;
      if (width == 1)
         H5Sselect_hyperslab(dspace_id, H5S_SELECT_SET, offset + 1,
                             NULL, count + 1, NULL);
      else
         H5Sselect_hyperslab(dspace_id, H5S_SELECT_SET, offset,
                             NULL, count, NULL);
      if (vlen == 0)
         return H5Dwrite(dset_id, mtype_id, mspace_id, 
                         dspace_id, H5P_DEFAULT, address(0));
      else
         return H5Dwrite(dset_id, mtype_id, mspace_id, 
                         dspace_id, H5P_DEFAULT, vlen);
   }

   virtual void print() {
      std::cout << "branch " << leaf->GetBranch()->GetName()
                << ", column name " << leaf->GetName()
                << ", type " << leaf->GetTypeName();
      if (width != 1)
         std::cout << "[" << width << "]";
      std::cout << std::endl;
   }

   virtual void *address(int irow) {
      std::cerr << "column_::address() - this method should never be called" << std::endl;
      return 0;
   }

   virtual float getvalue(int irow) {
      std::cerr << "column_::getvalue() - this method should never be called" << std::endl;
      return 0;
   }

   virtual int dsize() {
      std::cerr << "column_::dsize() - this method should never be called" << std::endl;
      return 0;
   }

   TLeaf *leaf;
   TLeaf *vleaf;
   hvl_t *vlen;
   int width;
   int varwidth;
   hid_t dtype_id;
   hid_t mtype_id;
   hid_t mspace_id;
   hid_t dspace_id;
   hid_t dset_id;
   hid_t chunk_id;
   hsize_t dset_offset;
   hsize_t dset_extent;
   hsize_t mspace_extent;
   hsize_t dspace_extent;

   static std::map<TLeaf*, column_*> leaves;
   static hsize_t rowcount;
   static hsize_t chunksize;

   static void setChunkSize(int chunk) {
      chunksize = chunk;
   }
   static void setRowCount(int nrows) {
      rowcount = nrows;
   }

 private:
   column_();
};

class column_float : public column_ {
 public:
   column_float(TLeaf *column_leaf)
    : column_(column_leaf) {
      if (vleaf == 0) {
         mtype_id = H5T_NATIVE_FLOAT;
         dtype_id = H5T_IEEE_F32LE;
      }
      else {
         mtype_id = H5Tvlen_create(H5T_NATIVE_FLOAT);
         dtype_id = H5Tvlen_create(H5T_IEEE_F32LE);
      }
      row = new float[varwidth * width * chunksize];
   }
   ~column_float() {
      delete row;
   }
   virtual void *address(int irow) {
      return row + irow * width;
   }
   virtual float getvalue(int irow) {
      return row[irow * width];
   }
   virtual int dsize() {
      return sizeof(*row);
   }
 private:
   float *row;
};

class column_double : public column_ {
 public:
   column_double(TLeaf *column_leaf)
    : column_(column_leaf) {
      if (vleaf == 0) {
         mtype_id = H5T_NATIVE_DOUBLE;
         dtype_id = H5T_IEEE_F64LE;
      }
      else {
         mtype_id = H5Tvlen_create(H5T_NATIVE_DOUBLE);
         dtype_id = H5Tvlen_create(H5T_IEEE_F64LE);
      }
      row = new double[varwidth * width * chunksize];
   }
   ~column_double() {
      delete row;
   }
   virtual void *address(int irow) {
      return row + irow * width;
   }
   virtual float getvalue(int irow) {
      return row[irow * width];
   }
   virtual int dsize() {
      return sizeof(*row);
   }
 private:
   double *row;
};

class column_int32 : public column_ {
 public:
   column_int32(TLeaf *column_leaf)
    : column_(column_leaf) {
      if (vleaf == 0) {
         mtype_id = H5T_NATIVE_INT;
         dtype_id = H5T_STD_I32LE;
      }
      else {
         mtype_id = H5Tvlen_create(H5T_NATIVE_INT);
         dtype_id = H5Tvlen_create(H5T_STD_I32LE);
      }
      row = new int[varwidth * width * chunksize];
   }
   ~column_int32() {
      delete row;
   }
   virtual void *address(int irow) {
      return row + irow * width;
   }
   virtual float getvalue(int irow) {
      return row[irow * width];
   }
   virtual int dsize() {
      return sizeof(*row);
   }
 private:
   int *row;
};

class column_uint32 : public column_ {
 public:
   column_uint32(TLeaf *column_leaf)
    : column_(column_leaf) {
      if (vleaf == 0) {
         mtype_id = H5T_NATIVE_UINT;
         dtype_id = H5T_STD_U32LE;
      }
      else {
         mtype_id = H5Tvlen_create(H5T_NATIVE_UINT);
         dtype_id = H5Tvlen_create(H5T_STD_U32LE);
      }
      row = new unsigned int[varwidth * width * chunksize];
   }
   ~column_uint32() {
      delete row;
   }
   virtual void *address(int irow) {
      return row + irow * width;
   }
   virtual float getvalue(int irow) {
      return row[irow * width];
   }
   virtual int dsize() {
      return sizeof(*row);
   }
 private:
   unsigned int *row;
};

class column_int64 : public column_ {
 public:
   column_int64(TLeaf *column_leaf)
    : column_(column_leaf) {
      if (vleaf == 0) {
         mtype_id = H5T_NATIVE_LONG;
         dtype_id = H5T_STD_I64LE;
      }
      else {
         mtype_id = H5Tvlen_create(H5T_NATIVE_LONG);
         dtype_id = H5Tvlen_create(H5T_STD_I64LE);
      }
      row = new long[varwidth * width * chunksize];
   }
   ~column_int64() {
      delete row;
   }
   virtual void *address(int irow) {
      return row + irow * width;
   }
   virtual float getvalue(int irow) {
      return row[irow * width];
   }
   virtual int dsize() {
      return sizeof(*row);
   }
 private:
   long *row;
};

class column_uint64 : public column_ {
 public:
   column_uint64(TLeaf *column_leaf)
    : column_(column_leaf) {
      if (vleaf == 0) {
         mtype_id = H5T_NATIVE_ULONG;
         dtype_id = H5T_STD_U64LE;
      }
      else {
         mtype_id = H5Tvlen_create(H5T_NATIVE_ULONG);
         dtype_id = H5Tvlen_create(H5T_STD_U64LE);
      }
      row = new unsigned long[varwidth * width * chunksize];
   }
   ~column_uint64() {
      delete row;
   }
   virtual void *address(int irow) {
      return row + irow * width;
   }
   virtual float getvalue(int irow) {
      return row[irow * width];
   }
   virtual int dsize() {
      return sizeof(*row);
   }
 private:
   unsigned long *row;
};

class column_int16 : public column_ {
 public:
   column_int16(TLeaf *column_leaf)
    : column_(column_leaf) {
      if (vleaf == 0) {
         mtype_id = H5T_NATIVE_SHORT;
         dtype_id = H5T_STD_I16LE;
      }
      else {
         mtype_id = H5Tvlen_create(H5T_NATIVE_SHORT);
         dtype_id = H5Tvlen_create(H5T_STD_I16LE);
      }
      row = new short[varwidth * width * chunksize];
   }
   ~column_int16() {
      delete row;
   }
   virtual void *address(int irow) {
      return row + irow * width;
   }
   virtual float getvalue(int irow) {
      return row[irow * width];
   }
   virtual int dsize() {
      return sizeof(*row);
   }
 private:
   short *row;
};

class column_uint16 : public column_ {
 public:
   column_uint16(TLeaf *column_leaf)
    : column_(column_leaf) {
      if (vleaf == 0) {
         mtype_id = H5T_NATIVE_USHORT;
         dtype_id = H5T_STD_U16LE;
      }
      else {
         mtype_id = H5Tvlen_create(H5T_NATIVE_USHORT);
         dtype_id = H5Tvlen_create(H5T_STD_U16LE);
      }
      row = new unsigned short[varwidth * width * chunksize];
   }
   ~column_uint16() {
      delete row;
   }
   virtual void *address(int irow) {
      return row + irow * width;
   }
   virtual float getvalue(int irow) {
      return row[irow * width];
   }
   virtual int dsize() {
      return sizeof(*row);
   }
 private:
   unsigned short *row;
};

class column_int8 : public column_ {
 public:
   column_int8(TLeaf *column_leaf)
    : column_(column_leaf) {
      if (vleaf == 0) {
         mtype_id = H5T_NATIVE_CHAR;
         dtype_id = H5T_STD_I8LE;
      }
      else {
         mtype_id = H5Tvlen_create(H5T_NATIVE_CHAR);
         dtype_id = H5Tvlen_create(H5T_STD_I8LE);
      }
      row = new char[varwidth * width * chunksize];
   }
   ~column_int8() {
      delete row;
   }
   virtual void *address(int irow) {
      return row + irow * width;
   }
   virtual float getvalue(int irow) {
      return row[irow * width];
   }
   virtual int dsize() {
      return sizeof(*row);
   }
 private:
   char *row;
};

class column_uint8 : public column_ {
 public:
   column_uint8(TLeaf *column_leaf)
    : column_(column_leaf) {
      if (vleaf == 0) {
         mtype_id = H5T_NATIVE_UCHAR;
         dtype_id = H5T_STD_U8LE;
      }
      else {
         mtype_id = H5Tvlen_create(H5T_NATIVE_UCHAR);
         dtype_id = H5Tvlen_create(H5T_STD_U8LE);
      }
      row = new unsigned char[varwidth * width * chunksize];
   }
   ~column_uint8() {
      delete row;
   }
   virtual void *address(int irow) {
      return row + irow * width;
   }
   virtual float getvalue(int irow) {
      return row[irow * width];
   }
   virtual int dsize() {
      return sizeof(*row);
   }
 private:
   unsigned char *row;
};

class column_bool : public column_ {
 public:
   column_bool(TLeaf *column_leaf)
    : column_(column_leaf) {
      if (vleaf == 0) {
         mtype_id = H5T_NATIVE_CHAR;
         dtype_id = H5T_STD_U8LE;
      }
      else {
         mtype_id = H5Tvlen_create(H5T_NATIVE_CHAR);
         dtype_id = H5Tvlen_create(H5T_STD_U8LE);
      }
      row = new char[varwidth * width * chunksize];
   }
   ~column_bool() {
      delete row;
   }
   virtual void *address(int irow) {
      return row + irow * width;
   }
   virtual float getvalue(int irow) {
      return row[irow * width];
   }
   virtual int dsize() {
      return sizeof(*row);
   }
 private:
   char *row;
};

class column_cstring : public column_ {
 public:
   column_cstring(TLeaf *column_leaf)
    : column_(column_leaf) {
      if (width == 1)
         width = MAXLEN_CSTRING;
      row = new char[width * chunksize];
      mtype_id = H5Tcopy(H5T_C_S1);
      H5Tset_size(mtype_id, H5T_VARIABLE);
      dtype_id = mtype_id;
   }
   ~column_cstring() {
      delete row;
   }
   virtual void *address(int irow) {
      return row + irow * width;
   }
   virtual float getvalue(int irow) {
      return row[irow * width];
   }
   virtual int dsize() {
      return sizeof(*row);
   }
 private:
   char *row;
};

#endif
