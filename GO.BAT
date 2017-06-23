@echo off
if .%1 == . goto error1
echo The image file '%1' is being faxed...
rl -qe %1 | fax -qec | fax -q | rl -q | bc -c64

goto exit

:error1
echo 
Please specify the data input file.
goto exit


:exit
