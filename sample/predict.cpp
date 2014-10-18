

#include "parse_example.h"
#include "parse_args.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <unordered_map>
#include <cmath>
using namespace std;
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>


float compute_pos_bias(const map<string, float> & pos_feature)
{
    int idx = 0;
    for ( auto x = pos_feature.begin(); x != pos_feature.end();  x ++)
    {
        if (abs(x->second - 1.0)< 1e-6)
        {
            break;
        }
        idx ++;
    }

    int row = idx / 4;
    int column = idx % 3;

    float PARAMS[3][2] = {-0.1150, 1.0,-0.1110, 0.49,-0.0953, 0.31};

    float param1 = PARAMS[row][0];
    float param2 = PARAMS[row][1];

    return param2 * exp(param1 * column);
}

float compute_model_score(const unordered_map<string, float> & coefs, const unordered_map<string,float> & features)
{
    float score = 0.0;
    for (auto coef : coefs)
    {
        auto feature_name = coef.first;
        auto splitter = feature_name.find('_');
        if ( splitter == feature_name.end())
        {
            auto feature = features.find(coef.first);
            if ( feature != features.end())
            {
                score += coef.second * feature->second;
            }
        }
        else
        {
            auto feature_name1 = feature_name.substr(0, splitter);
            auto feature_name2 = feature_name.substr( splitter + 1);

            auto feature1 = features.find(feature_name1);
            auto feature2 = features.find(feature_name2);

            if ( feature1 != features.end() && feature2 != features.end())
            {
                score += coef.second * feature1->second * feature2->second ;
            }

        }


    }

    return score * features.find("pos_bias")->second;
}

int main(int argc, char *argv[])
{

    //we need to discover raw feature name， and be quiet
    int new_argc = argc + 3;
    vector<char *> new_argv(argc + 3);

    copy(argv, argv+argc, new_argv.begin());

    new_argv[argc] = "--invert_hash";
    new_argv[argc+1] = "/dev/null";
    new_argv[argc+2] = "--quiet";

    auto model_file_param = find(new_argv.begin(), new_argv.end(),string("--model_file"));

    if (model_file_param == new_argv.end() or
            model_file_param +1 == new_argv.end())
    {
        cout << "A model file is needed. " << endl;
        exit(1);
    }


    string model_file_name = *(model_file_param + 1);

    if (! boost::filesystem::exists(model_file_name))
    {
        cout << "The model file " << model_file_name << " doesnot exist. " << endl;
        exit(1);
    }

    new_argv.erase(model_file_param,model_file_param + 2);

    vw * all = parse_args(new_argv.size(), new_argv.data());
    example* ec = NULL;

    unordered_map<string, float> coef;

    ifstream model_file(model_file_name);

    while(model_file)
    {
        string feature_name;
        float coef_val;
        model_file >> feature_name >> coef_val;

        coef[feature_name] = coef_val;

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
            unordered_map<string, float> cur_feature;
            map<string, float> pos_feature;

            for ( auto audit_array : ec->audit_features)
            {
                if (audit_array.size() == 0)
                {
                    continue;
                }

                string group_name =  const_cast<const char *>(audit_array[0].space);


                for (auto audit : audit_array)
                {
                    if (group_name == "l") // position feature
                    {
                        pos_feature[audit.feature] = audit.x;
                    }
                    else
                    {
                        cur_feature[audit.feature] = audit.x;
                    }

                }

            }

            cur_feature["pos_bias"] = compute_pos_bias(pos_feature);

            VW::finish_example(*all,ec);



            cout << compute_model_score(coef, cur_feature) << endl;

        }
        else if (parser_done(all->p))
        {
            all->l->end_examples();
            break;
        }


    }
    VW::end_parser(*all);
}
