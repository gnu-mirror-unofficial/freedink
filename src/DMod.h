#ifndef DMOD_H_
#define DMOD_H_

#include <string>
using namespace std;

#include "EditorMap.h"

class DMod {
public:
	DMod();
	EditorMap map;
};

extern DMod g_dmod;

#endif
