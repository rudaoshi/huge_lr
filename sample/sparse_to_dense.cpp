

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


int main(int argc, char *argv[])
{

    //we need to discover raw feature name， and be quiet
    int new_argc = argc + 3;
    vector<char *> new_argv(argc + 3);

    copy(argv, argv+argc, new_argv.begin());

    new_argv[argc] = "--invert_hash";
    new_argv[argc+1] = "/dev/null";
    new_argv[argc+2] = "--quiet";

    auto dim_param = find(new_argv.begin(), new_argv.end(),string("--dim "));

    if (dim_param == new_argv.end() or
            dim_param +1 == new_argv.end())
    {
        cout << "A dimension param is needed. " << endl;
        exit(1);
    }


    int dimension =  boost::lexical_cast<int>(*(dim_param + 1));


    new_argv.erase(dim_param,dim_param + 2);

    vw * all = parse_args(new_argv.size(), new_argv.data());
    example* ec = NULL;

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

            cout << ld->label;
            vector<float> feature(dimension, 0.0);

            for ( auto audit_array : ec->audit_features)
            {
                if (audit_array.size() == 0)
                {
                    continue;
                }

                for (auto audit : audit_array)
                {
                    int idx = boost::lexical_cast<int>(audit.feature);
                    feature[idx] = our_feature[idx] = audit.x;

                }

            }

            VW::finish_example(*all,ec);

            for (auto x : feature)
            {
                cout << " " << x;
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
