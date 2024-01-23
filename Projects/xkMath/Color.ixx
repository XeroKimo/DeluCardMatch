export module xk.Math.Color;
import xk.Math.Matrix;

namespace xk::Math
{
	using namespace Aliases;

	export struct Color : public u8Vector4
	{
		using u8Vector4::Vector;
	};
};