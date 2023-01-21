set folder_path=sample_folder
set height_1=20
set height_2=20
set height_3=20
set offset_1=0
set offset_2=20
set offset_3=40

set layer1_file=%folder_path%\*_layer1.bmp
set layer2_file=%folder_path%\*_layer2.bmp
set layer3_file=%folder_path%\*_layer3.bmp

for %%a in (%layer1_file%) do (
  OutputMQO.exe %height_1% %offset_1% %%a
)
for %%a in (%layer2_file%) do (
  OutputMQO.exe %height_2% %offset_2% %%a
)
for %%a in (%layer3_file%) do (
  OutputMQO.exe %height_3% %offset_3% %%a
)

del  %layer1_file%
del  %layer2_file%
del  %layer3_file%