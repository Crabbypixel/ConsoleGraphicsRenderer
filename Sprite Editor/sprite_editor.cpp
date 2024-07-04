#include <iostream>
#include "cfGeometryLib.h"

int main()
{
	using namespace cf::geom2d;

	cf::vec_2d vPoint = { 10.0f, 10.0f };

	auto c = circle<float>({20.0f, 20.0f}, 15.0f);

	if (contains(c, vPoint))
	{
		std::cout << "contains!\n";
	}

	return 0;
}