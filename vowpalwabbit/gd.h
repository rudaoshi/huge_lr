/*
  Copyright (c) by respective owners including Yahoo!, Microsoft, and
  individual contributors. All rights reserved.  Released under a BSD
  license as described in the file LICENSE.
*/
#ifndef GRADIENT_DESCENT_H
#define GRADIENT_DESCENT_H

#ifdef __FreeBSD__
#include <sys/socket.h>
#endif

#include "example.h"
#include "parse_regressor.h"
#include "parser.h"
#include "v_array.h"
#include "constant.h"

namespace GD{
  void print_result(int f, float res, v_array<char> tag);
  void print_audit_features(regressor &reg, example& ec, size_t offset);
  float finalize_prediction(shared_data* sd, float ret);
  void print_audit_features(vw&, example& ec);
  void train_one_example(regressor& r, example* ex);
  void train_offset_example(regressor& r, example* ex, size_t offset);
  void compute_update(example* ec);
  void offset_train(regressor &reg, example* &ec, float update, size_t offset);
  void train_one_example_single_thread(regressor& r, example* ex);
  LEARNER::learner* setup(vw& all, po::variables_map& vm);
  void save_load_regressor(vw& all, io_buf& model_file, bool read, bool text);
  void output_and_account_example(example* ec);

  template <class R, void (*T)(R&, const float, float&)>
  inline void foreach_feature(weight* weight_vector, size_t weight_mask, feature* begin, feature* end, R& dat, uint32_t offset=0, float mult=1.)
  {
    for (feature* f = begin; f!= end; f++)
      T(dat, mult*f->x, weight_vector[(f->weight_index + offset) & weight_mask]);
  }

 template <class R, void (*T)(R&, float, uint32_t)>
   void foreach_feature(weight* weight_vector, size_t weight_mask, feature* begin, feature* end, R&dat, uint32_t offset=0, float mult=1.)
   {
     for (feature* f = begin; f!= end; f++) 
       T(dat, mult*f->x, f->weight_index + offset);
   }
 
 template <class R, class S, void (*T)(R&, float, S)>
  inline void foreach_feature(vw& all, example& ec, R& dat)
  {
    uint32_t offset = ec.ft_offset;

    for (unsigned char* i = ec.indices.begin_; i != ec.indices.end_; i++)
      foreach_feature<R,T>(all.reg.weight_vector, all.reg.weight_mask, ec.atomics[*i].begin_, ec.atomics[*i].end_, dat, offset);
     
    for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++) {
      if (ec.atomics[(int)(*i)[0]].size() > 0) {
        v_array<feature> temp = ec.atomics[(int)(*i)[0]];
        for (; temp.begin_ != temp.end_; temp.begin_++)
        {
          uint32_t halfhash = quadratic_constant * (temp.begin_->weight_index + offset);
       
          foreach_feature<R,T>(all.reg.weight_vector, all.reg.weight_mask, ec.atomics[(int)(*i)[1]].begin_, ec.atomics[(int)(*i)[1]].end_, dat,
                               halfhash, temp.begin_->x);
        }
      }
    }
     
    for (vector<string>::iterator i = all.triples.begin(); i != all.triples.end();i++) {
      if ((ec.atomics[(int)(*i)[0]].size() == 0) || (ec.atomics[(int)(*i)[1]].size() == 0) || (ec.atomics[(int)(*i)[2]].size() == 0)) { continue; }
      v_array<feature> temp1 = ec.atomics[(int)(*i)[0]];
      for (; temp1.begin_ != temp1.end_; temp1.begin_++) {
        v_array<feature> temp2 = ec.atomics[(int)(*i)[1]];
        for (; temp2.begin_ != temp2.end_; temp2.begin_++) {
           
          uint32_t halfhash = cubic_constant2 * (cubic_constant * (temp1.begin_->weight_index + offset) + temp2.begin_->weight_index + offset);
          float mult = temp1.begin_->x * temp2.begin_->x;
          foreach_feature<R,T>(all.reg.weight_vector, all.reg.weight_mask, ec.atomics[(int)(*i)[2]].begin_, ec.atomics[(int)(*i)[2]].end_, dat, halfhash, mult);
        }
      }
    }
  }

template <class R, void (*T)(R&, float, float&)>
  inline void foreach_feature(vw& all, example& ec, R& dat)
  {
    foreach_feature<R,float&,T>(all, ec, dat);
  }

 inline void vec_add(float& p, const float fx, float& fw) { p += fw * fx; }

  inline float inline_predict(vw& all, example& ec)
  {
    label_data* ld = (label_data*)ec.ld;
    float temp = ld->initial;
    foreach_feature<float, vec_add>(all, ec, temp);
    return temp;
  }
}

#endif
