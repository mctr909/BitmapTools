set folder_path=sample_folder
set thickness=2

set source_file=%folder_path%\*.bmp
set thickened_file=%folder_path%\*_thickness%thickness%.bmp
set rename_src_name=_thickness%thickness%_layer1
set rename_dst_name=

for %%a in (%source_file%) do (
  outline.exe %thickness% %%a
)
for %%a in (%thickened_file%) do (
  outline.exe 1 %%a
)
del %thickened_file%

cd %~dp0\%folder_path%
for %%F in ( * ) do call :sub "%%F"
exit /b
:sub
  set FILE_NAME=%1
  call set FILE_NAME=%%FILE_NAME:%rename_src_name%=%rename_dst_name%%%
  ren %1 %FILE_NAME%
  goto :EOF
