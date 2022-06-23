#include "utilities.h"
#include "common.h"
#include "ICVar.h"


namespace Hack
{
	int getVBE();

    class ConVars {
    public:
        void InitConvars();

        ConCommandBase* camera_distance;
        ConCommandBase* range_display;
        ConCommandBase* r_farz;
        ConCommandBase* fog_enable;
        ConCommandBase* weather;
        ConCommandBase* particle_hack;
    };

}