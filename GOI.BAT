@echo off
if .%1 == . goto error1
echo The image file '%1' is being faxed...
rl -e %1 | fax -ec | fax | rl | sc

goto exit

:error1
echo 
Please specify the data input file.
goto exit


:exit
