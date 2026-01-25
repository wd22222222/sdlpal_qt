

建立此项目的是以开源项目sdlpal为基础，编制一款全新的综合游戏数据编辑软件，

通过对数据修改，可生成全新的游戏，实现全新的游戏体验，

一、功能描述：

1.本项目基于QT开发，开发工具为vs2022 或 vs2026,支持跨平台项目

2.原SdlPal部分由C改为用C++重构。

3.SDL改用OPENGL的GLSL进行渲染。

4.表格编辑部分除可对游戏数据进行修改外，还开发了对游戏地图等图像编辑功能

5.实现了游戏脚本编辑功能。

6.同时适应对SDLPAL两大基本原生版本的支持。

7.实现了游戏调试和运行功能

8.添加鼠标控制功能

二、环境搭建 win10 或 win11系统

第一步、将语言支持改为支持 UTF-8 设置->时间和语言->将bate版使用Unicode UTF-8 提供全球语言支持选项打开。这一步非常重要，否则可能无法通过编译

第二步、下载并安装 vs 2022 或 vs2026

第三步、下载安装vcpkg

进入cmd 命令行程序

在准备安装vcpkg 上一目录下如 d:/drv 或 d:/ 运行

注意电脑上要有50g 以上的空间

git clone https://github.com/microsoft/vcpkg.git

进入vcpkg子目录

cd vcpkg

生成VCPKG.EXE,输入命令

bootstrap-vcpkg.bat

安装 qt5-base 库

vcpkg install qt5-base

安装 qt5-base 静态库

vcpkg install qt5-base:x64-windows-static

或安装qt6 qt6库

安装 qt6-base 库

vcpkg install qtbase

安装 qt6-base 静态库

vcpkg install qtbase:x64-windows-static

如果同时安装两个库，将优先使用QT6

安装sdl2 库

vcpkg install sdl2

安装 sdl2 静态库

vcpkg install sdl2:x64-windows-static

如果使用sdl3 安装 sdl3

vcpkg install sdl3

安装 sdl3 静态库

vcpkg install sdl3:x64-windows-static

sdl2 sdl3 可同时在一个目录下安装 不冲突

安装 glad 库

vcpkg install glad

安装 glad 静态库

vcpkg install glad:x64-windows-static

安装 freetype 库

vcpkg install freetype

安装 freetype 静态库

vcpkg install freetype:x64-windows-static

安装 ogg 库

vcpkg install libogg:x64-windows

vcpkg install libogg:x64-windows-static

vcpkg install libvorbis

vcpkg install libvorbis:x64-windows-static

最后，启动vcpkg

vcpkg integrate install

第四步

下载并安装qt插件

Qt VS Tools for Visual Studio 2022

至此，全部环境搭建完成。如无意外，将可以编译运行成功了。

几点说明：

1.建议使用cmake 进行编译，和sin方式相比，cmake可以充分利用cpu 的多核，编译速度加快很多。配置起来也相对简单，不容易出问题

2.建议使用静态编译，结果，只需单一的exe 文件即可运行，免去对DLL文件的依赖。

3.cmake 方式，可以在QT集成环境中得到很好的支持，但在由于QT5下的minGW版本过低，对C++20支持差 无法通过编译，使用MSVC没有任何问题。qt6的集成环境支持minGw编译。

4.使用Ogg 音乐效果比其他方式好。

5.目前，同时支持qt5和qr6，今后后续版本中，还将继续支持，直到有了qt6的后继版本

6,同样，目前支持sdl2和sdl3 今后的后续版本中还将继续支持，直到有了sdl3的后续版本。


