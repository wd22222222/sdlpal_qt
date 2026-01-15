#include "mainwindow.h"  
#include <QApplication.h>  

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec.h>
#endif

#define _CRTDBG_MAP_ALLOC
//#include <crtdbg.h>
#include <QDir.h>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#if _DEBUG
//#include <io.h>
void CreateDebugConsole() {
	
	AllocConsole();

	// 重定向标准输入输出
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
	//freopen_s(&fp, "CONIN$", "r", stdin);
	// 设置控制台编码
	SetConsoleOutputCP(CP_UTF8);
	printf("调试控制台已创建\n");
}
#else
void CreateDebugConsole() {};
#endif

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	auto argc = __argc;
	auto argv = __argv;
	CreateDebugConsole();
	printf("这是调试信息\n");
	std::cout << "C++流输出" << std::endl;
#else
int main(int argc, char* argv[])
{
#endif

	QDir::setCurrent("d:/pal");
	QApplication a(argc, argv);
	MainWindow w;
	// 设置编码为UTF-8  
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	// Qt5 设置编码
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#else
	// Qt6 默认使用 UTF-8，通常不需要额外设置
#endif
	OutputDebugStringA("调试窗口\n");
	w.show();
	return a.exec();
}
