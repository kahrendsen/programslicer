
#include "PtsInf.h"

#include "llvm/sfs/sfs-aa/sfs-aa.h"

class PtsSfs: public PtsInf, public SFS
{
public:
  virtual void getPtsSet(Value* V, vector<Value* >& set);
  
  void printVal(Value* V);
};
