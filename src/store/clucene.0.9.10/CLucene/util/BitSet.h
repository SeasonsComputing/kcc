#ifndef _lucene_util_BitSet_
#define _lucene_util_BitSet_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif


CL_NS_DEF(util)
  class BitSet:LUCENE_BASE {
    int32_t _size;
    bool *bits;
  public:
    ///Create a bitset with the specified size
    BitSet ( int32_t size ):
      _size(size)
    {
      bits = _CL_NEWARRAY(bool,size);
      for ( int32_t i=0;i<_size;i++ )
        bits[i] = false;
    }
    ///Destructor for the bit set
    ~BitSet(){
      _CLDELETE_ARRAY(bits);
    }
    ///get the value of the specified bit
    bool get(const int32_t bit) const{
        if ( bit < 0 || bit > _size-1 )
          _CLTHROWA(CL_ERR_IllegalArgument, "bit out of range" );
      return bits[bit];
    }
    ///set the value of the specified bit
    void set(const int32_t bit){
      if ( bit < 0 || bit >= _size )
        _CLTHROWA(CL_ERR_IllegalArgument, "bit out of range" );
      bits[bit] = true;
    }
    ///returns the size of the bitset
    int32_t size() const {
      return _size;
    }
  };
CL_NS_END
#endif
