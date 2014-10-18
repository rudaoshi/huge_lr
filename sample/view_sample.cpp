

#include "parse_example.h"
#include "parse_args.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
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

    unordered_map<string, float> feature_min;
    unordered_map<string, float> feature_max;

    VW::start_parser(*all);

    while ( true )
    {
        if ((ec = VW::get_example(all->p)) != NULL)//semiblocking operation.
        {
            label_data* ld = (label_data*)ec->ld;

            cout << "Label = " << ld->label << endl;
            cout << "Feature Num = " << ec->ft_offset << endl;
            for (auto audit_array : ec->audit_features)
            {
                for (auto audit : audit_array)
                {

                    cout << audit.x << "\t"   // feature value
                            << audit.weight_index << "\t"  // hash name
                            << audit.space << "\t"   // feature group
                            << audit.feature << endl;  // feature name
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
    for (auto x = feature_min.begin(); x!=feature_min.end(); x++)
    {
        cout << x->first << "\t" << feature_min[x->first] << "\t" << feature_max[x->first] << endl;
    }

}
