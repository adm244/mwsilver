@ECHO OFF
SETLOCAL
SET data=..\data
SET ship=..\ship

IF [%1]==[] GOTO Error
GOTO CreateShipVersion

:CreateShipVersion
SET fullshippath=%ship%\%1

ECHO: Creating shipping version in %fullshippath%

IF NOT EXIST "%ship%" MKDIR "%ship%"
IF NOT EXIST "%fullshippath%" MKDIR "%fullshippath%"

COPY "%bin%\dinput8.dll" "%fullshippath%"
COPY "%bin%\dinput8.ini" "%fullshippath%"
COPY "%bin%\mwsilver.dll" "%fullshippath%"
COPY "%bin%\mwsilver.ini" "%fullshippath%"

COPY "%data%\mwgold" "%fullshippath%"
COPY "%data%\mwnight" "%fullshippath%"
COPY "%data%\mwspawn" "%fullshippath%"

ECHO: Done!
GOTO:EOF

:Error
ECHO: Enter a version number!
GOTO:EOF
