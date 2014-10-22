

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
    unordered_map<string, float> feature_sum;
    unordered_map<string, float> feature_std;
    unordered_map<string, int> feature_update_times;
    VW::start_parser(*all);

    int sample_num = 0;
    while ( true )
    {
        if ((ec = VW::get_example(all->p)) != NULL)//semiblocking operation.
        {

            sample_num += 1;
            for (auto audit_array : ec->audit_features)
            {
                for (auto audit : audit_array)
                {
                    string feature_name = audit.feature;
                    if (feature_min.find(feature_name) == feature_min.end())
                    {
                        feature_min[feature_name] = audit.x;
                    }
                    else
                    {
                        feature_min[feature_name] = min(
                                feature_min[feature_name],
                                audit.x
                        );
                    }
                    if (feature_max.find(feature_name) == feature_max.end())
                    {
                        feature_max[feature_name] = audit.x;
                    }
                    else
                    {
                        feature_max[feature_name] = max(
                                feature_max[feature_name],
                                audit.x
                        );
                    }

                    if (feature_sum.find(feature_name) == feature_sum.end())
                    {

                        feature_sum[feature_name] = audit.x;
                        float mean = feature_sum[feature_name]/sample_num;
                        float squared_std = pow((0-mean),2)*(sample_num-1) + pow((audit.x-mean),2);

                        feature_std[feature_name] = squared_std/sample_num;
                        feature_update_times[feature_name] = sample_num;
                    }
                    else
                    {

                        float squared_std = feature_std[feature_name];

                        for (int i = feature_update_times[feature_name] + 1; i < sample_num; i++)
                        {
                            float mean = feature_sum[feature_name]/i;
                            squared_std = float(i-2)*squared_std/float(i-1) + pow(-mean,2)/float(i);
                        }

                        feature_sum[feature_name] += audit.x;
                        float mean = feature_sum[feature_name]/sample_num;
                        squared_std = float(sample_num-2)*squared_std/float(sample_num-1) +
                                pow(audit.x-mean,2)/float(sample_num);

                        feature_std[feature_name] = squared_std;
                        feature_update_times[feature_name] = sample_num;
                    }


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



    for (auto x : feature_sum)
    {
        string feature_name = x.first;
        float squared_std = feature_std[feature_name];
        for (int i = feature_update_times[feature_name] + 1; i <= sample_num; i++)
        {
            float mean = feature_sum[feature_name]/i;
            squared_std = float(i-2)*squared_std/float(i-1) + pow(-mean,2)/float(i);
        }

        float mean = feature_sum[feature_name]/sample_num;
        feature_std[feature_name] = squared_std;
        feature_update_times[feature_name] = sample_num;
    }

    cout << "f_name" << "\t" << "min" << "\t" << "max" << "\t" << "mean" << "\t" << "std" << endl;

    for (auto x = feature_min.begin(); x!=feature_min.end(); x++)
    {
        cout << x->first
                << "\t" << feature_min[x->first]
                << "\t" << feature_max[x->first]
                << "\t" << feature_sum[x->first]/sample_num
                << "\t" << sqrt(feature_std[x->first])
                << endl;
    }

}
