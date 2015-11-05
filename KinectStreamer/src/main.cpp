#include "streamer.h"
#include <iostream>

void callback()
{
	std::cout << '.' << std::endl;
}

int main()
{
	Device device(callback);
	device.start();

	std::cin.get();

	device.stop();

	return 0;
}
