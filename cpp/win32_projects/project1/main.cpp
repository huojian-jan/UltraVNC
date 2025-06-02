#include <windows.h>

#pragma comment(lib,"User32.lib")


int main(HINSTANCE hInstance, HINSTANCE prevIns, LPSTR lpCmdLine, int nCmdShow)
{
	MessageBox(NULL, "MessageBox", "this is caption", MB_OK);
	return 0;
}