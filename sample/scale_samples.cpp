

#include "parse_example.h"
#include "parse_args.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
using namespace std;
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>


int main(int argc, char *argv[])
{

    //we need to discover raw feature name， and be quiet
    int new_argc = argc + 3;
    vector<char *> new_argv(argc + 3);

    copy(argv, argv+argc, new_argv.begin());

    new_argv[argc] = "--invert_hash";
    new_argv[argc+1] = "/dev/null";
    new_argv[argc+2] = "--quiet";

    auto scale_file_param = find(new_argv.begin(), new_argv.end(),string("--scale_file"));

    if (scale_file_param == new_argv.end() or
            scale_file_param +1 == new_argv.end())
    {
        cout << "A scale file is needed. " << endl;
        exit(1);
    }


    string scale_file_name = *(scale_file_param + 1);

    if (! boost::filesystem::exists(scale_file_name))
    {
        cout << "The scale file " << scale_file_name << " donot exist. " << endl;
        exit(1);
    }

    new_argv.erase(scale_file_param,scale_file_param + 2);

    vw * all = parse_args(new_argv.size(), new_argv.data());
    example* ec = NULL;

    unordered_map<string, float> feature_shift;
    unordered_map<string, float> feature_scale;

    ifstream scale_file(scale_file_name);

    while(scale_file)
    {
        string feature_name;
        float feature_shift_val, feature_scale_val;
        scale_file >> feature_name >> feature_shift_val >> feature_scale_val;

        feature_shift[feature_name] = feature_shift_val;

        if (abs(feature_scale_val) <= 1e-8)
        {
            feature_scale_val = 1;
        }

        feature_scale[feature_name] = feature_scale_val;


    }

    VW::start_parser(*all);

    while ( true )
    {
        if ((ec = VW::get_example(all->p)) != NULL)//semiblocking operation.
        {

            if (ec->num_features <= 0)
            {
                continue;
            }

            label_data* ld = (label_data*)ec->ld;
            string feature_str = boost::lexical_cast<string>(ld->label);

            for ( auto audit_array : ec->audit_features)
            {
                if (audit_array.size() == 0)
                {
                    continue;
                }

                feature_str += string(" |") + const_cast<const char *>(audit_array[0].space);
                for (auto audit : audit_array)
                {
                    string feature_name = audit.feature;
                    if (feature_min.find(feature_name) == feature_min.end())
                    {
                        continue;
                    }
                    audit.x = (audit.x - feature_min[feature_name]) / feature_scale[feature_name];

                    feature_str += " " + feature_name + ":" +
                            boost::lexical_cast<string>(audit.x);
//                    cout << audit.x << "\t"   // feature value
//                            << audit.weight_index << "\t"  // hash name
//                            << audit.space << "\t"   // feature group
//                            << audit.feature << endl;  // feature name
                }

            }

            VW::finish_example(*all,ec);
            cout << feature_str << endl;

        }
        else if (parser_done(all->p))
        {
            all->l->end_examples();
            break;
        }


    }
    VW::end_parser(*all);
}
