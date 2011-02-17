@echo off
set x=%1
set x=%x:/=\%
explorer.exe %x%
exit
