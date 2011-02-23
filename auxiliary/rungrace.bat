set GRACE_HOME=%CD%\grace
set GRACE_EDITOR=notepad
set GRACE_PRINT_CMD=print /d:LTP1
set GRACE_HELPVIEWER="@start "" /b %GRACE_HOME%\runhelpviewer.bat"
set HOME=%HOMEDRIVE%%HOMEPATH%
start %GRACE_HOME%\bin\qtgrace.exe
