
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

if not exist build (
	mkdir build
)
pushd build
del /s *.obj *.exe *.res *.manifest *.pdb *.ilk

rc /fo filepane.res ../assets/resource.rc

rem /MT			- enable multi-threaded and statically linked libraries
rem /02			- optimize for speed
rem /Zi			- include debug output pdb file
rem /Wall 		- enable all warnings
rem /WX   		- warnings are treated as errors
rem /Qspectre	- add spectre mitigations

rem cl.exe ../src/FilePane_Main.cpp /MT /O2 /Wall /WX /Qspectre /FeFilePanes.exe /link filepane.res
rem FilePanes.exe

cl.exe ../src/FilePane_Main.cpp /MT /Zi /Wall /WX /Qspectre /FeFilePanes.exe /link /DEBUG:FULL filepane.res



