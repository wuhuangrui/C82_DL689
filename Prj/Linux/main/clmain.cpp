#include "bios.h"
#include "FaAPI.h"

int main(int argc, char* argv[])
{
	system("/clou/ppp/script/ppp-off");
	OutBeepMs(100);
	MainThread(NULL);
	printf("program exit!\r\n");
	return 0;
}
