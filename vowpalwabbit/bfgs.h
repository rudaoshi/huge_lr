/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef BFGS_H
#define BFGS_H

namespace BFGS {
  LEARNER::learner* setup(vw& all, po::variables_map& vm);
}

#endif