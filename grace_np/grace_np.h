/*************************************************************************/
/*  (C) 04.08.1997 Henrik Seidel (HS)                                    */
/*  <henrik@itb.biologie.hu-berlin.de>                                   */
/*************************************************************************/

#ifndef GRACE_NPIPE_H_
#define GRACE_NPIPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EXIT_SUCCESS
#  define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
#  define EXIT_FAILURE -1
#endif

/* register a user function to print errors */
/* (the default function appends a newline and prints to standard error) */
typedef void (*GraceErrorFunctionType) (const char *);
GraceErrorFunctionType GraceRegisterErrorFunction(GraceErrorFunctionType f);

/* launch a grace subprocess and a communication channel with it */
int GraceOpen(const int);

/* test if a grace subprocess is currently connected */
int GraceIsOpen();

/* close the communication channel and exit the grace subprocess */
int GraceClose(void);

/* close the communication channel and leave the grace subprocess alone */
int GraceClosePipe(void);

/* flush all the data remaining in the buffer */
int GraceFlush(void);

/* format a command and send it to the grace subprocess */
int GracePrintf(const char*, ...);

/* send an already formated command to the grace subprocess */
int GraceCommand(const char*);

#ifdef __cplusplus
}
#endif

#endif /* GRACE_NPIPE_H */

