set folder_path=sample_folder
set thickness=2

set source_file=%folder_path%\*.bmp
set thickened_file=%folder_path%\*_thickness*.bmp
set delete_file=%folder_path%\*_thickness%thickness%.bmp
for %%a in (%source_file%) do (
  outline.exe %thickness% %%a
)
for %%a in (%thickened_file%) do (
  outline.exe 1 %%a
)
del  %delete_file%
