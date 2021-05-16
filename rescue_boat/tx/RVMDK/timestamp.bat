echo off
::echo %date%_%time%
set date_num=%date:~0,10%
set date_num=%date_num:-=%
set date_num=%date_num:/=%
set time_num=%time:~0,8%
set time_num=%time_num::=%
if "%time_num:~0,1%"==" " set "time_num=0%time_num:~1%"
echo on
copy .\Output\tx.hex .\tx_%date_num%_%time_num%.hex