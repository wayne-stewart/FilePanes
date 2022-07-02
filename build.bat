
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

if not exist build (
	mkdir build
)
pushd build
del /s *.obj *.exe *.res *.manifest

rc /fo filepane.res ../assets/resource.rc

rem /MT			- enable multi-threaded and statically linked libraries
rem /02			- optimize for speed
rem /Wall 		- enable all warnings
rem /WX   		- warnings are treated as errors
rem /Qspectre	- add spectre mitigations
cl.exe ../src/_main.cpp /MT /O2 /Wall /WX /Qspectre /FeFilePanes.exe /link filepane.res

FilePanes.exe

