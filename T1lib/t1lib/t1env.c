/*--------------------------------------------------------------------------
  ----- File:        t1env.c 
  ----- Author:      Rainer Menzner (rmz@neuroinformatik.ruhr-uni-bochum.de)
  ----- Date:        1999-03-31
  ----- Description: This file is part of the t1-library. It implements
                     the reading of a configuration file and path-searching
		     of type1-, afm- and encoding files.
  ----- Copyright:   t1lib is copyrighted (c) Rainer Menzner, 1996-1998. 
                     As of version 0.5, t1lib is distributed under the
		     GNU General Public Library Lincense. The
		     conditions can be found in the files LICENSE and
		     LGPL, which should reside in the toplevel
		     directory of the distribution.  Please note that 
		     there are parts of t1lib that are subject to
		     other licenses:
		     The parseAFM-package is copyrighted by Adobe Systems
		     Inc.
		     The type1 rasterizer is copyrighted by IBM and the
		     X11-consortium.
  ----- Warranties:  Of course, there's NO WARRANTY OF ANY KIND :-)
  ----- Credits:     I want to thank IBM and the X11-consortium for making
                     their rasterizer freely available.
		     Also thanks to Piet Tutelaers for his ps2pk, from
		     which I took the rasterizer sources in a format
		     independ from X11.
                     Thanks to all people who make free software living!
--------------------------------------------------------------------------*/
  

#define T1ENV_C


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>


#include "../type1/types.h"
#include "parseAFM.h" 
#include "../type1/objects.h" 
#include "../type1/spaces.h"  
#include "../type1/util.h" 
#include "../type1/fontfcn.h"
#include "../type1/fontmisc.h"

#include "t1types.h"
#include "t1extern.h"
#include "t1env.h"
#include "t1misc.h"
#include "t1base.h"


/* ScanConfigFile(): Read a configuration file and scan and save the
   environment strings used for searching pfa/pfb-, afm- and encoding
   files as well as the name of the font database file. */
int ScanConfigFile( char **pfabenv_ptr, 
		    char **afmenv_ptr,
		    char **encenv_ptr,
		    char **fontdatabase_ptr)
{
  
  char *environ;
  char *linebuf;
  char *usershome;
  char *cnffilepath;
  char *globalcnffilepath;
  char *tmp_ptr;
  
  
  
  FILE *cfg_fp;
  int filesize, i, j, k;
  
  
  /* First, get the string stored in the environment variable: */
  environ=getenv(ENV_CONF_STRING);


  if (environ==NULL){
    /* environment variable not set, try to open default file
       in user's home directory and afterwards global config file */
    usershome=getenv("HOME");
    cnffilepath=(char *)malloc((strlen(usershome) +
					 strlen(T1_CONFIGFILENAME) + 2
					 ) * sizeof(char));
    if (cnffilepath==NULL){
      T1_errno=T1ERR_ALLOC_MEM;
      return(-1);
    }
    strcpy( cnffilepath, usershome);
    strcat( cnffilepath, DIRECTORY_SEP);
    strcat( cnffilepath, T1_CONFIGFILENAME);

    globalcnffilepath=(char*)malloc((strlen(GLOBAL_CONFIG_DIR) +
					      strlen(GLOBAL_CONFIG_FILE) + 2
					      ) * sizeof(char));
    if (globalcnffilepath==NULL){
      T1_errno=T1ERR_ALLOC_MEM;
      return(-1);
    }
    strcpy( globalcnffilepath, GLOBAL_CONFIG_DIR);
    strcat( globalcnffilepath, DIRECTORY_SEP);
    strcat( globalcnffilepath, GLOBAL_CONFIG_FILE);

    if ((cfg_fp=fopen( cnffilepath, "r"))==NULL){
      sprintf( err_warn_msg_buf, "Could not open %s", cnffilepath);
      T1_PrintLog( "ScanConfigFile()", err_warn_msg_buf, T1LOG_STATISTIC);
      /* Try global config file */
      if ((cfg_fp=fopen( globalcnffilepath, "r"))==NULL){
	sprintf( err_warn_msg_buf, "Could not open %s", globalcnffilepath);
	T1_PrintLog( "ScanConfigFile()", err_warn_msg_buf, T1LOG_WARNING);
      }
      else{
	sprintf( err_warn_msg_buf, "Using %s as Configfile (global)",
		 cnffilepath);
	T1_PrintLog( "ScanConfigFile()", err_warn_msg_buf, T1LOG_STATISTIC);
      }
    }
    else{
      sprintf( err_warn_msg_buf, "Using %s as Configfile (user's)",
	       cnffilepath);
      T1_PrintLog( "ScanConfigFile()", err_warn_msg_buf, T1LOG_STATISTIC);
    }
    free( cnffilepath);
    free( globalcnffilepath);
    if (cfg_fp==NULL){
      T1_PrintLog( "ScanConfigFile()",
		   "Neither user's nor global Configfile has been found",
		   T1LOG_WARNING);
      return(0);
    }
  }
  else{
    /* open specified file for reading the configuration */
    if ((cfg_fp=fopen(environ,"r"))==NULL){
      T1_PrintLog( "ScanConfigFile()",
		   "Configfile as specified by Environment has not been found",
		   T1LOG_WARNING);
      return(0);  /* specified file could not be openend
		     => no config paths read */
    }
    else{
      sprintf( err_warn_msg_buf, "Using %s as Configfile (environment)",
	       environ);
      T1_PrintLog( "ScanConfigFile()", err_warn_msg_buf, T1LOG_STATISTIC);
    }
  }
  

  /* cfg_fp points now to a valid config file */
  /* Get the file size */
  fseek( cfg_fp, 0, SEEK_END);
  filesize=ftell(cfg_fp);
  /* Reset fileposition to start */
  fseek( cfg_fp, 0, SEEK_SET);
  
  if ((linebuf=(char *)calloc( filesize,
			       sizeof(char)))==NULL){
    T1_errno=T1ERR_ALLOC_MEM;
    return(-1);
  }
  
  fread((char *)linebuf, sizeof(char), filesize, cfg_fp);
  fclose(cfg_fp);
  
  i=0;
  
  while(i<filesize){
    j=i;     /* Save index of beginning of line */
    while ((linebuf[i]!='=')&&(linebuf[i]!='\n'))
      i++;
    linebuf[i]=0;  /* replace '=' by ASCII-0 */
    if (strcmp( "ENCODING", &linebuf[j])==0){
      /* Check for an explicitly assigned value */
      if (*encenv_ptr==T1_enc){
	k=i+1;       /* points to assigned path string */
	while (linebuf[i] != '\n')
	  i++;
	linebuf[i]=0;  /* replace linefeed by ASCII-0 */
	/* Copy string */
	if ((tmp_ptr=(char *)malloc( strlen(&linebuf[k]) + 1))==NULL){
	  T1_errno=T1ERR_ALLOC_MEM;
	  return(-1);
	}
	strcpy( tmp_ptr, &linebuf[k]);
	*encenv_ptr=tmp_ptr;
      }
      else
	T1_PrintLog( "ScanConfigFile()",
		     "Preserving explicitly assigned ENCODING search path",
		     T1LOG_DEBUG);
    }  
    if (strcmp( "TYPE1", &linebuf[j])==0){
      /* Check for an explicitly assigned value */
      if (*pfabenv_ptr==T1_pfab){
	k=i+1;       /* points to assigned path string */
	while (linebuf[i] != '\n')
	  i++;
	linebuf[i]=0;  /* replace linefeed by ASCII-0 */
	/* Copy string */
	if ((tmp_ptr=(char *)malloc( strlen(&linebuf[k]) + 1))==NULL){
	  T1_errno=T1ERR_ALLOC_MEM;
	  return(-1);
	}
	strcpy( tmp_ptr, &linebuf[k]);
	*pfabenv_ptr=tmp_ptr;
      }
      else
	T1_PrintLog( "ScanConfigFile()",
		     "Preserving explicitly assigned PFAB search path",
		     T1LOG_DEBUG);
    }  
    if (strcmp( "AFM", &linebuf[j])==0){
      /* Check for an explicitly assigned value */
      if (T1_AFM_ptr==T1_afm){
	k=i+1;       /* points to assigned path string */
	while (linebuf[i] != '\n')
	  i++;
	linebuf[i]=0;  /* replace linefeed by ASCII-0 */
	/* Copy string */
	if ((tmp_ptr=(char *)malloc( strlen(&linebuf[k]) + 1))==NULL){
	  T1_errno=T1ERR_ALLOC_MEM;
	  return(-1);
	}
	strcpy( tmp_ptr, &linebuf[k]);
	*afmenv_ptr=tmp_ptr;
      }
      else
	T1_PrintLog( "ScanConfigFile()",
		     "Preserving explicitly assigned AFM search path",
		     T1LOG_DEBUG);
    }  
    if (strcmp( "FONTDATABASE", &linebuf[j])==0){
      /* Check for an explicitly assigned value */
      if (*fontdatabase_ptr==T1_fontdatabase){
	k=i+1;       /* points to assigned path string */
	while (!isspace((int)linebuf[i]))
	  i++;
	if (linebuf[i]=='\n'){
	  linebuf[i]=0;  /* replace linefeed by ASCII-0 */
	}
	else{
	  linebuf[i]=0;  /* replace linefeed by ASCII-0 */
	  /* Step to end of line */
	  while (linebuf[i] != '\n')
	    i++;
	  linebuf[i]=0;
	}
	/* Copy string */
	if ((tmp_ptr=(char *)malloc( strlen(&linebuf[k]) + 1))==NULL){
	  T1_errno=T1ERR_ALLOC_MEM;
	  return(-1);
	}
	strcpy( tmp_ptr, &linebuf[k]);
	*fontdatabase_ptr=tmp_ptr;
      }
      else
	T1_PrintLog( "ScanConfigFile()",
		     "Preserving explicitly assigned FontDataBase",
		     T1LOG_DEBUG);
    }
    i++;
  }
  /*file should now be read in */
  free( linebuf);
  
  return(i);
  
}



/* Env_GetCompletePath( ): Get a full path name from the file specified by
   argument 1 in the environment specified by argument 2. Return the pointer
   to the path string or NULL if no file was found.*/
char *Env_GetCompletePath( char *FileName,
			   char *env_ptr )
{
  struct stat filestats;    /* A structure where fileinfo is stored */
  int fnamelen, enamelen, i, j;
  char save_char;
  char *FullPathName;
  char *res_ptr;
  char *StrippedName;
  

  if (FileName==NULL)
    return(NULL);
  fnamelen=strlen(FileName);
  enamelen=strlen(env_ptr);
  /* We check whether absolute or relative pathname is given. If so,
     stat() it and if appropriate, return that string immediately. */
  if ( (FileName[0]==DIRECTORY_SEP_CHAR) ||
       ((fnamelen>1) && (FileName[0]=='.') &&
	(FileName[1]==DIRECTORY_SEP_CHAR)) ||
       ((fnamelen>2) && (FileName[0]=='.') &&
	(FileName[1]=='.') && (FileName[2]==DIRECTORY_SEP_CHAR))) {
    /* Check for existence of the path: */
    if (!stat( FileName, &filestats)){
      if (t1lib_log_file!=NULL){
	sprintf( err_warn_msg_buf, "stat()'ing complete path %s successful",
		 FileName);
	T1_PrintLog( "Env_GetCompletePath()", err_warn_msg_buf,
		     T1LOG_DEBUG);
      }
      /* Return a copy of the string */
      if ((FullPathName=(char *)malloc( fnamelen + 1))==NULL){
	T1_errno=T1ERR_ALLOC_MEM;
	return(NULL);
      }
      strcpy( FullPathName, FileName);
      return(FullPathName);
    }
    if (t1lib_log_file!=NULL){
      sprintf( err_warn_msg_buf, "stat()'ing complete path %s failed",
	       FileName);
      T1_PrintLog( "Env_GetCompletePath()", err_warn_msg_buf,
		   T1LOG_DEBUG);
    }
    /* Trying to locate absolute path spec. failed. We try to recover
       by removing the path component and searching in the remaining search
       path entries. */
    i=fnamelen-1;
    StrippedName=&(FileName[i]);
    while ( FileName[i]!=DIRECTORY_SEP_CHAR){
      i--;
    }
    i++;
    StrippedName=&FileName[i];
    if (t1lib_log_file!=NULL){
      sprintf( err_warn_msg_buf, "path %s stripped to %s",
	       FileName, StrippedName);
      T1_PrintLog( "Env_GetCompletePath()", err_warn_msg_buf,
		   T1LOG_DEBUG);
    }
  }
  else{ /* We have a relative path name */
    StrippedName=&FileName[0];
  }

  i=0;
  while (i<enamelen){
    j=i;      /* Save start of current path string */
    while ((env_ptr[i]!=':')&&(env_ptr[i]!=0))
      i++;
    /* Save the character that indicated end of path */
    save_char=env_ptr[i];
    env_ptr[i]=0;   /* Set limit for this path element */
    FullPathName=(char *)malloc((i-j+2+fnamelen)*sizeof(char));
    if (FullPathName==NULL){
      T1_errno=T1ERR_ALLOC_MEM;
      return(NULL);
    }
    /* Copy current path element: */
    res_ptr=strcpy( FullPathName, &env_ptr[j]);
    /* Add the directory separator: */
    res_ptr=strcat( FullPathName, DIRECTORY_SEP);
    /* And finally the filename: */
    res_ptr=strcat( FullPathName, StrippedName);
    
    /* Check for existence of the path: */
    if (!stat( FullPathName, &filestats)){
      /* restore the replaced character in order to leave environment
	 string unmodified */
      env_ptr[i]=save_char;
      if (t1lib_log_file!=NULL){
	sprintf( err_warn_msg_buf, "stat()'ing %s successful",
		 FullPathName);
	T1_PrintLog( "Env_GetCompletePath()", err_warn_msg_buf,
		     T1LOG_DEBUG);
      }
      return(FullPathName);
    }
    if (t1lib_log_file!=NULL){
      sprintf( err_warn_msg_buf, "stat()'ing %s failed",
	       FullPathName);
      T1_PrintLog( "Env_GetCompletePath()", err_warn_msg_buf,
		   T1LOG_DEBUG);
    }
    
    /* We didn't find the file --> try next path entry */
    free(FullPathName);
    /* restore the replaced character in order to leave environment string
       unmodified */
    env_ptr[i]=save_char;
    i++;
  }
  /* If we get here, no file was found at all, so return a NULL-pointer */
  return(NULL);
}


/* T1_SetFileSearchPath(): Set the search path to find files of the
   specified type and return 0 if successful and -1 otherwise */
int T1_SetFileSearchPath( int type, char *pathname)
{
  
  if (pathname==NULL){
    T1_errno=T1ERR_INVALID_PARAMETER;
    return(-1);
  }

  /* We do not allow to change the searchpath if the database already
     contains one or more entries. */
  if (T1_Get_no_fonts()>0){
    sprintf( err_warn_msg_buf, "Path %s not set, database is not empty",
	     pathname);
    T1_PrintLog( "T1_SetFileSearchPath()", err_warn_msg_buf,
		 T1LOG_STATISTIC);
    T1_errno=T1ERR_OP_NOT_PERMITTED;
    return(-1);
  }
  
  if (type & T1_PFAB_PATH){
    if ((void *)T1_PFAB_ptr!=(void *)T1_pfab)
      free( T1_PFAB_ptr);
    if ((T1_PFAB_ptr=(char *)malloc( (strlen(pathname) + 1) *
				     sizeof( char)))==NULL){
      T1_errno=T1ERR_ALLOC_MEM;
      return(-1);
    }
    strcpy( T1_PFAB_ptr, pathname);
  }
  if (type & T1_AFM_PATH){
    if ((void *)T1_AFM_ptr!=(void *)T1_afm)
      free( T1_AFM_ptr);
    if ((T1_AFM_ptr=(char *)malloc( (strlen(pathname) + 1) *
				    sizeof( char)))==NULL){
      T1_errno=T1ERR_ALLOC_MEM;
      return(-1);
    }
    strcpy( T1_AFM_ptr, pathname);
  }
  if (type & T1_ENC_PATH){
    if ((void *)T1_ENC_ptr!=(void *)T1_enc)
      free( T1_ENC_ptr);
    if ((T1_ENC_ptr=(char *)malloc( (strlen(pathname) + 1) *
				    sizeof( char)))==NULL){
      T1_errno=T1ERR_ALLOC_MEM;
      return(-1);
    }
    strcpy( T1_ENC_ptr, pathname);
  }
  
  return(0);
  
}



/* T1_GetFileSearchPath(): Return the specified file search path
   or NULL if an error occurred. Note: We  do only one path at a
   time. */
char *T1_GetFileSearchPath( int type)
{
  static char *out_ptr;

  if (out_ptr!=NULL)
    free( out_ptr);
  out_ptr=NULL;
  
  if (type & T1_PFAB_PATH){
    if ((out_ptr=(char *)malloc(strlen(T1_PFAB_ptr)+1))==NULL){
      T1_errno=T1ERR_ALLOC_MEM;
      return( NULL);
    }
    strcpy( out_ptr, T1_PFAB_ptr);
    return( out_ptr);
  }
  if (type & T1_AFM_PATH){
    if ((out_ptr=(char *)malloc(strlen(T1_AFM_ptr)+1))==NULL){
      T1_errno=T1ERR_ALLOC_MEM;
      return( NULL);
    }
    strcpy( out_ptr, T1_AFM_ptr);
    return( out_ptr);
  }
  if (type & T1_ENC_PATH){
    if ((out_ptr=(char *)malloc(strlen(T1_ENC_ptr)+1))==NULL){
      T1_errno=T1ERR_ALLOC_MEM;
      return( NULL);
    }
    strcpy( out_ptr, T1_ENC_ptr);
    return( out_ptr);
  }

  return( NULL);
  
}


/* T1_AddToFileSearchPath(): Add the specified path element to
   the specified search path.
   Return value is 0 if successful and -1 otherwise */
int T1_AddToFileSearchPath( int pathtype, int mode, char *pathname)
{
  int i;
  char *tmp_ptr;
  
  
  if (pathname==NULL)
    return(-1);
  
  if (pathtype & T1_PFAB_PATH){
    i=strlen( T1_PFAB_ptr)+strlen( pathname) + 2;
    if ((tmp_ptr=(char *)malloc(strlen(T1_PFAB_ptr)+
				strlen(pathname)+2))==NULL){
      T1_errno=T1ERR_ALLOC_MEM;
      return(-1);
    }
    if (mode & T1_PREPEND_PATH){ /* prepend */
      strcpy(tmp_ptr, pathname);
      strcat(tmp_ptr, ":");
      strcat(tmp_ptr, T1_PFAB_ptr);
    }
    else{ /* append */
      strcpy(tmp_ptr,T1_PFAB_ptr);
      strcat(tmp_ptr, ":");
      strcat(tmp_ptr, pathname);
    }
    if ((void *)T1_PFAB_ptr!=(void *)&T1_pfab)
      free( T1_PFAB_ptr);
    T1_PFAB_ptr=tmp_ptr;
  }
  if (pathtype & T1_AFM_PATH){
    i=strlen( T1_AFM_ptr)+strlen( pathname) + 2;
    if ((tmp_ptr=(char *)malloc(strlen(T1_AFM_ptr)+
				strlen(pathname)+2))==NULL){
      T1_errno=T1ERR_ALLOC_MEM;
      return(-1);
    }
    if (mode & T1_PREPEND_PATH){ /* prepend */
      strcpy(tmp_ptr, pathname);
      strcat(tmp_ptr, ":");
      strcat(tmp_ptr, T1_AFM_ptr);
    }
    else{ /* append */
      strcpy(tmp_ptr,T1_AFM_ptr);
      strcat(tmp_ptr, ":");
      strcat(tmp_ptr, pathname);
    }
    if ((void *)T1_AFM_ptr!=(void *)&T1_afm)
      free( T1_AFM_ptr);
    T1_AFM_ptr=tmp_ptr;
  }
  if (pathtype & T1_ENC_PATH){
    i=strlen( T1_ENC_ptr)+strlen( pathname) + 2;
    if ((tmp_ptr=(char *)malloc(strlen(T1_ENC_ptr)+
				strlen(pathname)+2))==NULL){
      T1_errno=T1ERR_ALLOC_MEM;
      return(-1);
    }
    if (mode & T1_PREPEND_PATH){ /* prepend */
      strcpy(tmp_ptr, pathname);
      strcat(tmp_ptr, ":");
      strcat(tmp_ptr, T1_ENC_ptr);
    }
    else{ /* append */
      strcpy(tmp_ptr,T1_ENC_ptr);
      strcat(tmp_ptr, ":");
      strcat(tmp_ptr, pathname);
    }
    if ((void *)T1_ENC_ptr!=(void *)&T1_enc)
      free( T1_ENC_ptr);
    T1_ENC_ptr=tmp_ptr;
  }
  return(0);
  
}



