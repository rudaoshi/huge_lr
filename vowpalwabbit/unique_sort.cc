/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */

#include "unique_sort.h"
#include "global_data.h"

int order_features(const void* first, const void* second)
{
  return ((feature*)first)->weight_index - ((feature*)second)->weight_index;
}

int order_audit_features(const void* first, const void* second)
{
  return (int)(((audit_data*)first)->weight_index) - (int)(((audit_data*)second)->weight_index);
}

void unique_features(v_array<feature> &features)
{
  if (features.empty())
    return;
  feature* last = features.begin_;
  for (feature* current = features.begin_ +1;
       current != features.end_; current++)
    if (current->weight_index != last->weight_index) 
      *(++last) = *current;
  features.end_ = ++last;
}

void unique_audit_features(v_array<audit_data> &features)
{
  if (features.empty())
    return;
  audit_data* last = features.begin_;
  for (audit_data* current = features.begin_ +1;
       current != features.end_; current++)
    if (current->weight_index != last->weight_index) 
      *(++last) = *current;
  features.end_ = ++last;
}

void unique_sort_features(bool audit, uint32_t parse_mask, example* ae)
{
  ae->sorted=true;
  for (unsigned char* b = ae->indices.begin_; b != ae->indices.end_; b++)
    {
      v_array<feature> features = ae->atomics[*b];

      for (size_t i = 0; i < features.size(); i++)
	features[i].weight_index &= parse_mask;
      qsort(features.begin_, features.size(), sizeof(feature),
	    order_features);
      unique_features(ae->atomics[*b]);
      
      if (audit)
	{
	  v_array<audit_data> afeatures = ae->audit_features[*b];

	  for (size_t i = 0; i < ae->atomics[*b].size(); i++)
	    afeatures[i].weight_index &= parse_mask;
	  
	  qsort(afeatures.begin_, afeatures.size(), sizeof(audit_data),
		order_audit_features);
	  unique_audit_features(afeatures);
	}
    }
}
