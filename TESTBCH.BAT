@echo off
echo BCH(51,12) Encoder/Decoder tests
if .%1 == . goto error4
if .%2 == . goto bch0
if .%2 == .0 goto bch0
if .%2 == .1 goto bch1
if .%2 == .2 goto bch2
if .%2 == .3 goto bch3
goto error5
:bch0
echo Method: LFSR encoder, syndrome table match decoder.

goto start

:bch1
echo Method: LFSR encoder, parity matrix solution decoder.

goto start

:bch2
echo Method: Parallel encoder, syndrome table match decoder.

goto start

:bch3
echo Method: LFSR encoder, power sum lookup table decoder.

goto start

:start
echo Generating (%1) test vectors...
r -n%1 -ot1

echo 
Testing 0 bit errors...
bch -es%2 t1 | bch -s%2 -ot2
diff t1 t2 >>td1
if not errorlevel 0 goto error0
type td1

echo 
Testing 1 bit errors...
bch -es%2 t1 | err -r1 | bch -s%2 -ot2
diff t1 t2 >>td1
if not errorlevel 0 goto error1
type td1

echo 
Testing 2 bit errors...
bch -es%2 t1 | err -r2 | bch -s%2 -ot2
diff t1 t2 >>td2
if not errorlevel 0 goto error2
type td2

echo 
Testing interleaved 4 bit burst errors with a period of 2...
bch -es%2 t1 | inter -qe | err -r1 -b2,4 | inter -q | bch -s%2 -ot2
diff t1 t2 >>td2
if not errorlevel 0 goto error3
type td2

echo Done
goto exit

:error0
echo 
1 0 Bit error test failed!
goto exit

:error1
echo 
1 1 Bit error test failed!
goto exit

:error2
echo 
2 2 Bit error test failed!
goto exit

:error3
echo 
2 2 Bit burst error test failed!
goto exit

:error4
echo 
Please specify the number of test vectors to generate.
goto exit

:error5
echo 
Please specify a solution type 0-3.
goto exit

:exit
if exist t1 del t1
if exist t2 del t2
if exist td1 del td1
if exist td2 del td2

