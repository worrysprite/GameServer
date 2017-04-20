#include "Math.h"
#include <chrono>

namespace ws
{
	namespace utils
	{
		std::mt19937 Math::randomGenerator((unsigned int)std::chrono::system_clock::now().time_since_epoch().count());

	}
}

