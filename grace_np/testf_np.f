       PROGRAM TESTF_NP
C
       IMPLICIT NONE
       INTEGER i, GraceOpenf
       CHARACTER*64 buf
C
C      Start Grace with a buffer size of 2048 and open the pipe
C
       IF (GraceOpenf(2048) .EQ. -1) THEN
           WRITE (*,*) 'Can not run xmgr.'
           CALL EXIT (1)
       ENDIF
C
C      Send some initialization commands to Grace
C
       CALL GraceCommandf ('world xmax 100')
       CALL GraceCommandf ('world ymax 10000')
       CALL GraceCommandf ('xaxis tick major 20')
       CALL GraceCommandf ('xaxis tick minor 10')
       CALL GraceCommandf ('yaxis tick major 2000')
       CALL GraceCommandf ('yaxis tick minor 1000')
       CALL GraceCommandf ('s0 on')
       CALL GraceCommandf ('s1 on')
       CALL GraceCommandf ('sets symbol 2')
       CALL GraceCommandf ('sets symbol fill 1')
       CALL GraceCommandf ('sets symbol size 0.3')
C
C      Display sample data
C
       DO i = 1, 100, 1
           WRITE (buf, 1) i, i
           CALL GraceCommandf (buf)
           WRITE (buf, 2) i, i**2
           CALL GraceCommandf (buf)
C
C          Update the Grace display after every ten steps
C
           IF (10*(i / 10) .EQ. I) THEN
               CALL GraceCommandf ('redraw')
C                Wait a second, just to simulate some time needed for
C                   calculations. Your real application shouldn't wait.
               CALL SLEEP (1)
           ENDIF
       ENDDO

C
C      Tell Grace to save the data
C
       CALL GraceCommandf ('saveall "sample.gr"')
C
C      Flush the output buffer and close the pipe
C
       CALL GraceClosef ()
C
C      We are done
C
       CALL EXIT (0)
C      
 1     FORMAT ('g0.s0 point ', I6, ' , ', I6)
 2     FORMAT ('g0.s1 point ', I6, ' , ', I6)
C
       END

