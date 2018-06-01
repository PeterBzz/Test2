
#ifndef State_H
#define State_H

#include <vector>
#include <sstream>

using namespace std;

struct State
{
    public:
        int symbolsCount;
        vector< vector<string> > copies;
        vector<float> fullness;
        int copiesCount;
        int fullnessNotNominalCount;
};

#endif
