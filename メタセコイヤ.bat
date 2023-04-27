set folder_path=sample_folder
set height_1=10
set height_2=10
set height_3=17

set /a offset_1 = 0
set /a offset_2 = offset_1 + height_1
set /a offset_3 = offset_2 + height_2

set layer1_file=%folder_path%\*_layer1.bmp
set layer2_file=%folder_path%\*_layer2.bmp
set layer3_file=%folder_path%\*_layer3.bmp

for %%a in (%layer1_file%) do (
  OutputMQO.exe %height_1% %offset_1% "layer1" %%a
)
for %%a in (%layer2_file%) do (
  OutputMQO.exe %height_2% %offset_2% "layer2" %%a *_layer1.mqo
)
for %%a in (%layer3_file%) do (
  OutputMQO.exe %height_3% %offset_3% "layer3" %%a *_layer1.mqo
)

del %layer1_file%
del %layer2_file%
del %layer3_file%

set rename_src_name=_layer1.mqo
set rename_dst_name=.mqo
cd %~dp0\%folder_path%
for %%F in ( * ) do call :sub "%%F"
exit /b
:sub
  set FILE_NAME=%1
  call set FILE_NAME=%%FILE_NAME:%rename_src_name%=%rename_dst_name%%%
  ren %1 %FILE_NAME%
  goto :EOF
