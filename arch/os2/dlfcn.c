/* dlfcn.c */ 
/* Implementation of dlopen() interface for OS/2               */
/* This code is released into public domain                    */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* We only need parts of the whole OS/2-specific stuff */
#define INCL_DOSMODULEMGR     /* Module Manager values */
#define INCL_DOSERRORS        /* Error values */
#include <os2.h>

#define LM_LENGTH 256
#define MAXDLLOPEN 256

static ULONG LoadErrorLength = LM_LENGTH;
static int   LoadErrorFlag   = FALSE;
UCHAR LoadError[LM_LENGTH]   = "";  /* this is being referenced from outside
                                       this module */


void *dlopen( const char *filename, int flag)

{
  HMODULE DLLHandle;
  APIRET rc;
 
  rc = DosLoadModule( LoadError,
                      LoadErrorLength-1,
                      filename,
                      &DLLHandle);
  if (rc != NO_ERROR)
   {
    sprintf(LoadError,
            "DosLoadModule(\"%s\") = %lu\n", filename, rc);
    LoadErrorFlag = TRUE;
    return NULL;
    } else {
    LoadErrorFlag = FALSE;
    return (void*)DLLHandle;
   }
}


char *dlerror(void)
{

  if (!LoadErrorFlag) {
    return (char *)NULL;
    }
  else {
    LoadErrorFlag = FALSE;
    return LoadError;
    }
}


void *dlsym(void *handle, char *symbol)
{

  APIRET rc;  
  PFN FuncAddress;
  
  rc = DosQueryProcAddr( (HMODULE) handle,
                             0L,
                             symbol,
                             &FuncAddress);
  if (rc != NO_ERROR)
   {
    switch (rc)
      {
       case ERROR_INVALID_HANDLE:
        {
         sprintf(LoadError,
                "DosQueryProcAddr(\"%s\")=ERROR_INVALID_HANDLE (%lu)\n",
                 symbol, rc);
          break;
         }
       case ERROR_ENTRY_IS_CALLGATE: /* I have no idea what this is ... */
        {
         sprintf(LoadError,
                "DosQueryProcAddr(\"%s\")=ERROR_INVALID_HANDLE (%lu)\n",
                 symbol, rc);
          break;
         }
       default:
        {
         sprintf(LoadError,
                "DosQueryProcAddr(\"%s\")=%lu\n", symbol, rc);
          break;
         }
       } /* end switch(rc) */

    LoadErrorFlag = TRUE;
    return NULL;
   } else {
    LoadErrorFlag = FALSE;
    return (void*)FuncAddress;
   }
}


int dlclose( void *handle )

{
  APIRET rc;

  rc = DosFreeModule( (HMODULE)handle );
  if (rc != NO_ERROR)
    {
     sprintf(LoadError,
            "DosFreeModule()=%lu\n", rc);
     LoadErrorFlag = TRUE;
     return 2;
   } else {
    LoadErrorFlag = FALSE;
    return 0;
   }
}
