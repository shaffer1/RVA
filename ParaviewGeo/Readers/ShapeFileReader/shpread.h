/**************************************************************************
 * shpread.h  declarations for shpread.c, extracted from shpopen.c
 **************************************************************************/
 
#include "shapefil.h"

extern SHPInfo* SHPOpen( const char * pszLayer, const char * pszAccess );

extern void SHPClose(SHPHandle psSHP );

extern SHPObject* SHPReadObject( SHPHandle psSHP, int hEntity );

extern void SHPDestroyObject( SHPObject * psShape );
