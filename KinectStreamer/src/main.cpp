#include "streamer.h"
#include <iostream>

void callback(uint16_t *_depth_buffer)
{
	std::cout << _depth_buffer[0] << std::endl;
}

int main()
{
	Device device(callback);
	device.start();

	std::cin.get();

	device.stop();

	return 0;
}
