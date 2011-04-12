set GRACE_HOME=%~dp0grace
set GRACE_EDITOR=notepad
set GRACE_PRINT_CMD=print /d:LTP1
set GRACE_HELPVIEWER="cd %GRACE_HOME% & @start "" /b runhelpviewer.bat "%%s""
set HOME=%HOMEDRIVE%%HOMEPATH%
@start "" "%GRACE_HOME%\bin\qtgrace.exe" -wd "%CD%" %1
