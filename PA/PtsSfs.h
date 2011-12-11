
#include "PtsInf.h"

#include "llvm/sfs/include/sfs-aa/sfs-aa.h"

class PtsAnders: public PtsInf, public Anders
{
public:
  virtual void getPtsSet(Value* V, vector<Value* >& set);
  
  void printVal(Value* V);
};
