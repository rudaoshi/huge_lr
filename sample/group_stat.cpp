

#include "parse_example.h"
#include "parse_args.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
using namespace std;


int main(int argc, char *argv[])
{

    //we need to discover raw feature name， and be quiet
    int new_argc = argc + 3;
    vector<char *> new_argv(argc + 3);

    copy(argv, argv+argc, new_argv.begin());
    new_argv[argc] = "--invert_hash";
    new_argv[argc+1] = "/dev/null";
    new_argv[argc+2] = "--quiet";

    vw * all = parse_args(new_argc, new_argv.data());
    example* ec = NULL;

    unordered_map<string, unordered_set<string>> group;

    VW::start_parser(*all);

    while ( true )
    {
        if ((ec = VW::get_example(all->p)) != NULL)//semiblocking operation.
        {

            if (ec->num_features <= 0)
            {
                continue;
            }
            for (auto audit_array : ec->audit_features)
            {
                if (audit_array.size() == 0)
                {
                    continue;
                }
                string cur_group = audit_array[0].space;
                for (auto audit : audit_array)
                {
                    string feature_name = audit.feature;
                    group[cur_group].insert(feature_name);

//                    cout << audit.x << "\t"   // feature value
//                            << audit.weight_index << "\t"  // hash name
//                            << audit.space << "\t"   // feature group
//                            << audit.feature << endl;  // feature name
                }

            }

            VW::finish_example(*all,ec);

        }
        else if (parser_done(all->p))
        {
            all->l->end_examples();
            break;
        }


    }

    VW::end_parser(*all);

    for (auto x = group.begin(); x!=group.end(); x++)
    {
        auto group_name = x->first;
        vector<string> feature_names(x->second.size());
        copy(x->second.begin(), x->second.end(), feature_names.begin());
        sort(feature_names.begin(), feature_names.end());

        for (auto y : feature_names)
        {
            cout << y << "\t" << group_name << endl;
        }
    }

}
