
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

if not exist build (
	mkdir build
)
pushd build
del /s *.obj *.exe *.res *.manifest

rc /fo filepane.res ../assets/resource.rc

cl.exe ../src/_main.cpp /MT /O2 /Wall /WX /FeFilePanes.exe /link filepane.res

FilePanes.exe

