set GRACE_HOME=%~dp0grace
set GRACE_EDITOR=notepad
set GRACE_PRINT_CMD=print /d:LTP1
set GRACE_HELPVIEWER="cd %GRACE_HOME% & @start "" /b runhelpviewer.bat "%%s""
set HOME=%HOMEDRIVE%%HOMEPATH%
set GRACE_HOME=%GRACE_HOME:\=/%
set CD=%CD:\=/%
set FILE_PATH=%1
IF [%FILE_PATH%] NEQ [] (
  set FILE_PATH=%FILE_PATH:\=/%
)
@start "" "%GRACE_HOME%/bin/qtgrace.exe" -wd "%CD%" %FILE_PATH%
