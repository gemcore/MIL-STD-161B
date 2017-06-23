rem Project Environment Setup for 161B/fax simulations

set ROOTDIR=D:\PROJECT\161B
C:\DOS\SUBST G: /D
C:\DOS\SUBST G: %ROOTDIR%

set PATH=F:\POLY;E:\ZTC\BIN;E:\ZTC\UTIL;.;\;E:\BAT;E:\BIN;F:\BIN;F:\B;F:\NU;F:\NC;C:\DOS;F:\VTERM;F:\WP\LIB;F:\WP;E:\BCHK
set LIB=E:\ZTC\LIB;
set INCLUDE=G:\LIB;E:\ZTC\INCLUDE;E:\ZTC\TOOLKIT\HPP;
set BCC="ztc -c -ms -a -o+all -g -w -DDEBUG -DROM -oobj\%%s %%s.c"
set BCCPP="ztc -c -ms -a -o+all -g -w -DDEBUG -DROM -oobj\%%s %%s.cpp"
set BCASM="masm -Ml -DNEC -DKNL -DLEDS %%s,,,,"
set BUILTINS=G:\BUILTINS.MAK
set VCSID=ADG
set TMP=d:\tmp
