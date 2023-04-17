
rem call "C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"

if not exist build (
	mkdir build
)
pushd build
del /s *.obj *.exe *.res *.manifest *.pdb *.ilk

rem compile the resource (.rc) file so it can be embedded in the exe
rc /fo filepane.res ../assets/resource.rc

rem COMPILER FLAGS
rem /MT			- enable multi-threaded and statically linked libraries
rem /02			- optimize for speed
rem /Zi			- include debug output pdb file
rem /Wall 		- enable all warnings
rem /WX   		- warnings are treated as errors
rem /Qspectre	- add spectre mitigations

rem LINKER FLAGS
rem /DEBUG:FULL		- compiles the exe for debug with separate pdb file, use with /Zi
rem /MANIFEST:EMBED	- embeds the manifiest in the exe instead of creating a .manifest file

set arg1=%1
set arg2=%2

REM BUILD AND RUN SHELL IMAGES
if %arg1%.==img. goto IMG
goto SKIP_IMG

:IMG
cl.exe ../src/shell_images.cpp /MT /Zi /Wall /WX /Qspectre /FeShellImages_DEBUG.exe /link /DEBUG:FULL filepane.res /MANIFEST:EMBED
if %arg2%.==run. goto RUN_IMG
goto END

:RUN_IMG
ShellImages_DEBUG.exe
goto END

:SKIP_IMG

REM BUILD AND RUN FILE PANES
if %arg1%.==fp. goto FP
goto SKIP_FP
:FP
cl.exe ../src/FilePane_Main.cpp /MT /O2 /Wall /WX /Qspectre /FeFilePanes.exe /link filepane.res /MANIFEST:EMBED
cl.exe ../src/FilePane_Main.cpp /MT /Zi /Wall /WX /Qspectre /FeFilePanes_DEBUG.exe /link /DEBUG:FULL filepane.res /MANIFEST:EMBED

if %arg2%.==run. goto RUN
if %arg2%.==debug. goto DEBUG
goto END

:RUN
FilePanes.exe
goto END

:DEBUG
FilePanes_DEBUG.exe
goto END

:SKIP_FP


:END