$ ! This command file reads DOTEST. and executes the GRACE commands.
$ !
$ GRACE = GRACE + " -noask"
$ !
$ OPEN IN DOTEST.
$LOOP:
$ READ/END=DONE IN REC
$ IF (F$EXTRACT (0, 6, REC) .EQS. "$GRACE")
$ THEN
$   REC = REC - "$"
$   WRITE SYS$OUTPUT "$ ", REC
$   'REC'
$ ENDIF
$ GOTO LOOP
$DONE:
$ CLOSE IN
$ EXIT
