set GRACE_HOME=%~dp0\grace
set GRACE_EDITOR=notepad
set GRACE_PRINT_CMD=print /d:LTP1
set GRACE_HELPVIEWER="cd %GRACE_HOME% & @start "" /b runhelpviewer.bat"
set HOME=%HOMEDRIVE%%HOMEPATH%
cd %GRACE_HOME%
@start bin\qtgrace.exe %1
