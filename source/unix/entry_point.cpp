#include "tdu_instance.h"

#ifdef OTDU_MACOS
int main( int argc, const char** argv ) 
{
    return TestDrive::InitAndRun( argv, argc );
}
#endif
