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

int GraceOpen (const int);
int GraceClose (void);
int GraceFlush (void);
int GracePrintf (const char*, ...);
int GraceCommand (const char*);

#ifdef __cplusplus
}
#endif

#endif /* GRACE_NPIPE_H */

