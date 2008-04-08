SET POSTFIX=package\
copy "..\bin\*.exe" src
copy "..\src\Resources\lang\*.vpl" lang
del lang\new.vpl
cabwiz.exe gpsVPppc.inf
cabwiz.exe gpsVP.inf
cabwiz.exe gpsVPppc2003.inf
cabwiz.exe gpsVP2003.inf
zip.exe -r -9 %POSTFIX%gpsVPppc.zip gpsVPppc.CAB lang
zip.exe -r -9 %POSTFIX%gpsVP.zip gpsVP.CAB lang
zip.exe -r -9 %POSTFIX%gpsVPppc2003.zip gpsVPppc2003.CAB lang
zip.exe -r -9 %POSTFIX%gpsVP2003.zip gpsVP2003.CAB lang
copy src\VisualPainter.exe gpsVPxp.exe
zip.exe -r -9 %POSTFIX%gpsVPxp.zip gpsVPxp.exe lang
cd lang
..\zip.exe -9 ..\lang.zip *.vpl
cd ..