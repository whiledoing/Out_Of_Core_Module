@echo off
echo 将Doxygen输出文件编码从utf-8转换到gbk
set path=%path%;"d:\\Desktop\\doxybat"

echo 处理chm输入文件
call utf82gbk.bat index.hhp
call utf82gbk.bat index.hhc
call utf82gbk.bat index.hhk

