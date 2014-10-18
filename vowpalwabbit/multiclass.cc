#include <limits.h>
#include "multiclass.h"
#include "global_data.h"
#include "vw.h"

namespace MULTICLASS {

  char* bufread_label(multiclass* ld, char* c)
  {
    ld->label = *(uint32_t *)c;
    c += sizeof(ld->label);
    ld->weight = *(float *)c;
    c += sizeof(ld->weight);
    return c;
  }
  
  size_t read_cached_label(shared_data*, void* v, io_buf& cache)
  {
    multiclass* ld = (multiclass*) v;
    char *c;
    size_t total = sizeof(ld->label)+sizeof(ld->weight);
    if (buf_read(cache, c, total) < total) 
      return 0;
    c = bufread_label(ld,c);
    
    return total;
  }
  
  float weight(void* v)
  {
    multiclass* ld = (multiclass*) v;
    return (ld->weight > 0) ? ld->weight : 0.f;
  }
  
  char* bufcache_label(multiclass* ld, char* c)
  {
    *(uint32_t *)c = ld->label;
    c += sizeof(ld->label);
    *(float *)c = ld->weight;
    c += sizeof(ld->weight);
    return c;
  }

  void cache_label(void* v, io_buf& cache)
  {
    char *c;
    multiclass* ld = (multiclass*) v;
    buf_write(cache, c, sizeof(ld->label)+sizeof(ld->weight));
    c = bufcache_label(ld,c);
  }

  void default_label(void* v)
  {
    multiclass* ld = (multiclass*) v;
    ld->label = (uint32_t)-1;
    ld->weight = 1.;
  }

  void delete_label(void* v)
  {
  }

  void parse_label(parser* p, shared_data*, void* v, v_array<substring>& words)
  {
    multiclass* ld = (multiclass*)v;

    switch(words.size()) {
    case 0:
      break;
    case 1:
      ld->label = int_of_substring(words[0]);
      ld->weight = 1.0;
      break;
    case 2:
      ld->label = int_of_substring(words[0]);
      ld->weight = float_of_substring(words[1]);
      break;
    default:
      cerr << "malformed example!\n";
      cerr << "words.size() = " << words.size() << endl;
    }
    if (ld->label == 0)
      {
	cout << "label 0 is not allowed for multiclass.  Valid labels are {1,k}" << endl;
	throw exception();
      }
  }

  label_parser mc_label = {default_label, parse_label, 
				  cache_label, read_cached_label, 
				  delete_label, weight, 
				  NULL,
				  sizeof(multiclass)};
  
  void print_update(vw& all, example &ec)
  {
    if (all.sd->weighted_examples >= all.sd->dump_interval && !all.quiet && !all.bfgs)
      {
        multiclass* ld = (multiclass*) ec.ld;
        char label_buf[32];
        if (ld->label == INT_MAX)
          strcpy(label_buf," unknown");
        else
          sprintf(label_buf,"%8ld",(long int)ld->label);

        if(!all.holdout_set_off && all.current_pass >= 1)
        {
          if(all.sd->holdout_sum_loss == 0. && all.sd->weighted_holdout_examples == 0.)
            fprintf(stderr, " unknown   ");
          else
	    fprintf(stderr, "%-10.6f " , all.sd->holdout_sum_loss/all.sd->weighted_holdout_examples);

          if(all.sd->holdout_sum_loss_since_last_dump == 0. && all.sd->weighted_holdout_examples_since_last_dump == 0.)
            fprintf(stderr, " unknown   ");
          else
	    fprintf(stderr, "%-10.6f " , all.sd->holdout_sum_loss_since_last_dump/all.sd->weighted_holdout_examples_since_last_dump);

            fprintf(stderr, "%8ld %8.1f   %s %8ld %8lu h\n",
	      (long int)all.sd->example_number,
	      all.sd->weighted_examples,
	      label_buf,
	      (long int)ld->prediction,
	      (long unsigned int)ec.num_features);

          all.sd->weighted_holdout_examples_since_last_dump = 0;
          all.sd->holdout_sum_loss_since_last_dump = 0.0;
        }
        else
          fprintf(stderr, "%-10.6f %-10.6f %8ld %8.1f   %s %8ld %8lu\n",
                all.sd->sum_loss/all.sd->weighted_examples,
                all.sd->sum_loss_since_last_dump / (all.sd->weighted_examples - all.sd->old_weighted_examples),
                (long int)all.sd->example_number,
                all.sd->weighted_examples,
                label_buf,
                (long int)ld->prediction,
                (long unsigned int)ec.num_features);
     
        all.sd->sum_loss_since_last_dump = 0.0;
        all.sd->old_weighted_examples = all.sd->weighted_examples;
	fflush(stderr);
        VW::update_dump_interval(all);
      }
  }

  void output_example(vw& all, example& ec)
  {
    multiclass* ld = (multiclass*)ec.ld;

    size_t loss = 1;
    if (ld->label == (uint32_t)ld->prediction)
      loss = 0;

    if(ec.test_only)
    {
      all.sd->weighted_holdout_examples += ld->weight;//test weight seen
      all.sd->weighted_holdout_examples_since_last_dump += ld->weight;
      all.sd->weighted_holdout_examples_since_last_pass += ld->weight;
      all.sd->holdout_sum_loss += loss;
      all.sd->holdout_sum_loss_since_last_dump += loss;
      all.sd->holdout_sum_loss_since_last_pass += loss;//since last pass
    }
    else
    {
      all.sd->weighted_examples += ld->weight;
      all.sd->total_features += ec.num_features;
      all.sd->sum_loss += loss;
      all.sd->sum_loss_since_last_dump += loss;
      all.sd->example_number++;
    }
 
    for (int* sink = all.final_prediction_sink.begin_; sink != all.final_prediction_sink.end_; sink++)
      all.print(*sink, (float)ld->prediction, 0, ec.tag);

    MULTICLASS::print_update(all, ec);
  }
}
