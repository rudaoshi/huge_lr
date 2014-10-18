#include "global_data.h"
#include "parser.h"
#include "learner.h"

void save_predictor(vw& all, string reg_name, size_t current_pass);

void dispatch_example(vw& all, example& ec)
{
  if (ec.test_only || !all.training)
    all.l->predict(ec);
  else
    all.l->learn(ec);
  all.l->finish_example(all, ec);
}

namespace LEARNER
{
  void generic_driver(vw* all)
  {
    example* ec = NULL;

    all->l->init_driver();
    while ( true )
      {
	if ((ec = VW::get_example(all->p)) != NULL)//semiblocking operation.
	  {
	    if (ec->indices.size() > 1) // 1+ nonconstant feature. (most common case first)
	      dispatch_example(*all, *ec);
	    else if (ec->end_pass)
	      {
		all->l->end_pass();
		VW::finish_example(*all,ec);
	      }
	    else if (ec->tag.size() >= 4 && !strncmp((const char*) ec->tag.begin_, "save", 4))
	      {// save state command

		string final_regressor_name = all->final_regressor_name;
		
		if ((ec->tag).size() >= 6 && (ec->tag)[4] == '_')
		  final_regressor_name = string(ec->tag.begin_ +5, (ec->tag).size()-5);
		
		if (!all->quiet)
		  cerr << "saving regressor to " << final_regressor_name << endl;
		save_predictor(*all, final_regressor_name, 0);
		
		VW::finish_example(*all,ec);
	      }
	    else // empty example
	      dispatch_example(*all, *ec);
	  }
	else if (parser_done(all->p))
	  {
	    all->l->end_examples();
	    return;
	  }
      }
  }
}
