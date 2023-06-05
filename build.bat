
if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
	call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
	call "C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
) else (
	echo "Visual Studio not found"
	exit /b 1
)

if not exist build (
	mkdir build
)
pushd build
del /s *.obj *.exe *.res *.manifest *.pdb *.ilk

rem compile the resource (.rc) file so it can be embedded in the exe
rc /fo resource_filepanes.res ../assets/resource_filepanes.rc
rc /fo resource_png_to_ico.res ../assets/resource_png_to_ico.rc

rem COMPILER FLAGS
rem /MT			- enable multi-threaded and statically linked libraries
rem /02			- optimize for speed
rem /Zi			- include debug output pdb file
rem /Wall 		- enable all warnings
rem /WX   		- warnings are treated as errors
rem /Qspectre	- add spectre mitigations

rem LINKER FLAGS
rem /DEBUG:FULL			- compiles the exe for debug with separate pdb file, use with /Zi
rem /MANIFEST:EMBED		- embeds the manifiest in the exe instead of creating a .manifest file
rem <resource_file>.res	- embeds the resource file in the exe

cl.exe ../src/png_to_ico.cpp /MT /O2 /Wall /WX /Qspectre /Fepng_to_ico.exe /link resource_png_to_ico.res /MANIFEST:EMBED
rem cl.exe ../src/png_to_ico.cpp /MT /Zi /Wall /WX /Qspectre /Feptoi.exe /link /DEBUG:FULL resource_png_to_ico.res /MANIFEST:EMBED

cl.exe ../src/shell_images.cpp /MT /O2 /Wall /WX /Qspectre /Feshell_images.exe /link /MANIFEST:EMBED
rem cl.exe ../src/png_to_ico.cpp /MT /Zi /Wall /WX /Qspectre /Feshell_images.exe /link /DEBUG:FULL /MANIFEST:EMBED

cl.exe ../src/FilePane_Main.cpp /MT /O2 /Wall /WX /Qspectre /FeFilePanes.exe /link resource_filepanes.res /MANIFEST:EMBED
cl.exe ../src/FilePane_Main.cpp /MT /Zi /Wall /WX /Qspectre /FeFilePanes_DEBUG.exe /link /DEBUG:FULL resource_filepanes.res /MANIFEST:EMBED

