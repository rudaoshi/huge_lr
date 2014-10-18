

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

    auto group_file_param = find(new_argv.begin(), new_argv.end(),string("--group_file"));

    if (group_file_param == new_argv.end() or
            group_file_param +1 == new_argv.end())
    {
        cout << "A group file is needed. " << endl;
        exit(1);
    }


    string group_file_name = *(group_file_param + 1);

    if (! boost::filesystem::exists(group_file_name))
    {
        cout << "The scale file " << group_file_name << " donot exist. " << endl;
        exit(1);
    }

    new_argv.erase(group_file_param, group_file_param + 2);

    vw * all = parse_args(new_argv.size(), new_argv.data());
    example* ec = NULL;

    unordered_map<string, string> feature_group_map;


    ifstream group_file(group_file_name);

    while(group_file)
    {
        string feature_name, group_name;
        group_file >> feature_name >> group_name ;

        feature_group_map[feature_name] = group_name;
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
            string label_str = boost::lexical_cast<string>(ld->label);

            unordered_map<string, float> features;

            for ( auto audit_array : ec->audit_features)
            {
                if (audit_array.size() == 0)
                {
                    continue;
                }

                for (auto audit : audit_array)
                {
                    string feature_name = audit.feature;
                    features[feature_name] = audit.x;
                }

            }

            VW::finish_example(*all,ec);

            unordered_map<string, string> group_feature_strs;

            for (auto x : features)
            {
                if (feature_group_map.find(x.first) == feature_group_map.end())
                {
                    // the feature is removed
                    continue;
                }
                auto group_name = feature_group_map[x.first];
                auto cur_feature_str = x.first + ":" + boost::lexical_cast<string>(x.second);
                group_feature_strs[group_name] += " " + cur_feature_str;
            }

            cout << label_str;
            for (auto x : group_feature_strs)
            {
                cout << " |" << x.first << x.second;
            }
            cout << endl;

        }
        else if (parser_done(all->p))
        {
            all->l->end_examples();
            break;
        }


    }
    VW::end_parser(*all);
}
