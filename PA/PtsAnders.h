
#include "PtsInf.h"

//#include "llvm/sfs/include/anders.h"
//#include "/home/kendall/source/llvm/projects/ptr-analysis/include/anders.h"
#include "llvm/Transforms/anders.h"

class PtsAnders: public PtsInf, public Anders
{
public:
  virtual void getPtsSet(Value* V, vector<Value* >& set);
  
  void printVal(Value* V);
};
