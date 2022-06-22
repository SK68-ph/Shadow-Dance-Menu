#include "utilities.h"
#include "common.h"
#include "ICVar.h"


namespace Hack
{
	int getVBE();
    int InitVbe();

    class ConVars {
    public:
        void FindConVars();

        ConCommandBase* sv_cheats;
        ConCommandBase* camera_distance;
        ConCommandBase* range_display;
        ConCommandBase* r_farz;
        ConCommandBase* fog_enable;
        ConCommandBase* cl_weather;
        ConCommandBase* particle_hack;
    };

}