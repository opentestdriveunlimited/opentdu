#include "tdu_instance.h"

#ifdef OTDU_WIN32
HINSTANCE gHINSTANCE = nullptr;
int32_t gNCmdShow = 0;

int WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) {
    gHINSTANCE = hInstance;
    gNCmdShow = nCmdShow;

    int32_t argCount = 1;
    char* pCmdLineArgs[32];
    memset( pCmdLineArgs, 0, sizeof( char* ) * 32 );
    pCmdLineArgs[0] = "";

    char* pCmdLineIt = lpCmdLine;
    char cmdLineIt = *pCmdLineIt;
    while ( cmdLineIt != '\0' && argCount < 32 ) {
        while ( cmdLineIt == ' ' ) {
            *lpCmdLine = '\0';
            pCmdLineIt++;
            lpCmdLine++;
            cmdLineIt = *pCmdLineIt;
        }

        if ( *lpCmdLine == '\0' ) {
            break;
        }

        pCmdLineArgs[argCount++] = lpCmdLine;

        while ( *lpCmdLine != ' ' ) {
            if ( *lpCmdLine == '\"' ) {
                cmdLineIt = lpCmdLine[1];
                while ( ( lpCmdLine = lpCmdLine + 1, cmdLineIt != '\0' && ( cmdLineIt != '\"' ) ) ) {
                    cmdLineIt = lpCmdLine[1];
                }
            }
            pCmdLineIt = lpCmdLine + 1;
            lpCmdLine++;

            if ( *pCmdLineIt == '\0' ) {
                return TestDrive::InitAndRun( (const char**)pCmdLineArgs, argCount );
            }
        }

        cmdLineIt = *pCmdLineIt;
    }

    return TestDrive::InitAndRun( ( const char** )pCmdLineArgs, argCount );
}
#endif