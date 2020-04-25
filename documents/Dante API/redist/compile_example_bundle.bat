@ECHO OFF

SET CANDLE=%WIX%\bin\candle.exe
SET LIGHT=%WIX%\bin\light.exe

SET INPUT=ConmonExampleBundle.wxs
SET OBJECT=ConmonExampleBundle.obj
SET OUTPUT=ConmonExampleBundle.exe

echo "Comiling %INPUT% to %OBJECT%"
"%CANDLE%" -nologo -ext WixBalExtension -o %OBJECT% %INPUT%

echo "Linking %OBJECT% to %OUTPUT%"
"%LIGHT%" -nologo -ext WixBalExtension -o %OUTPUT% %OBJECT%
