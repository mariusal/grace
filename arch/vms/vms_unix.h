/* vms_unix.h */
/* Rolf Niepraschk, 11/97, niepraschk@ptb.de */

#  if __VMS_VER < 70000000 
#    define O_NONBLOCK O_NDELAY
     char *getlogin();
#  endif

int system_spawn(const char *command);

