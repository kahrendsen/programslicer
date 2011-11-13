// TODO: Add header

#include "CDG.h"

bool CDG::runOnFunction(Function &F)
{
  errs() << "Function: " << F << "\n";

  PostDominatorTree &PDT = getAnalysis<PostDominatorTree>();
  return false;
}

