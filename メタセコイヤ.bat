set folder_path=sample_folder
set height=20
set offset_y=0

set source_file=%folder_path%\*_thickness*outline.bmp
for %%a in (%source_file%) do (
  OutputMQO.exe %height% %offset_y% %%a
)
del  %source_file%
