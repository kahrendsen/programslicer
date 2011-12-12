
#ifndef PTS_INF_H
#define PTS_INF_H
#include "llvm/Value.h"

#include <vector>

using namespace std;
using namespace llvm;
class PtsInf
{
public:
  virtual void getPtsSet(Value* V, vector<Value* >& set) = 0;
};
#endif
