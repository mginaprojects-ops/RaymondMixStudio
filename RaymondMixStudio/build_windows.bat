@echo off
echo === Raymond Mix Studio : build ya Windows ===
cmake -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 goto fail
cmake --build build --config Release
if errorlevel 1 goto fail
echo.
echo IMEKAMILIKA. Plugin iko hapa:
echo build\RaymondMixStudio_artefacts\Release\VST3\Raymond Mix Studio.vst3
echo Standalone iko hapa:
echo build\RaymondMixStudio_artefacts\Release\Standalone\Raymond Mix Studio.exe
pause
exit /b 0
:fail
echo.
echo HITILAFU. Soma ujumbe ulio juu na uutume kwa Claude ili urekebishwe.
pause
exit /b 1
