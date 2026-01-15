#include "../main/cscript.h"  

#ifdef _WIN32
#include <windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#else
int main(int argc, char* argv[])
{
#endif
   class CScript* a = new CScript;  
   delete a;  
   return 0;  
}

std::string p_Script(int)
{
	return "";
}
