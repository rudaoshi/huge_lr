/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include "search_hooktask.h"

// this is used for the C++ library and python library hook; hopefully
// it can be used for any foreign library too!
namespace HookTask {
  Search::search_task task = { "hook", run, initialize, finish, NULL, NULL  };

  void initialize(Search::search& sch, size_t& num_actions, po::variables_map& vm) {
    task_data *td = new task_data;
    td->run_f = NULL;
    td->run_object = NULL;
    td->delete_run_object = NULL;
    td->delete_extra_data = NULL;
    td->var_map = new po::variables_map(vm);
    td->num_actions = num_actions;
    sch.set_task_data<task_data>(td);
  }

  void finish(Search::search& sch) {
    task_data *td = sch.get_task_data<task_data>();
    if (td->run_object && td->delete_run_object) td->delete_run_object(td->run_object);
    if (td->delete_extra_data) td->delete_extra_data(*td);
    delete td->var_map;
    delete td;
  }

  void run(Search::search& sch, vector<example*>& ec) {
    task_data *td = sch.get_task_data<task_data>();
    if (td->run_f)
      td->run_f(sch);
    else
      cerr << "warning: HookTask::structured_predict called before hook is set" << endl;
  }
}

