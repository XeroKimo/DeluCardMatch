export module xk.Math.Color;
import xk.Math.Matrix;

namespace xk::Math
{
	export struct Color 
	{
		xk::Math::Aliases::u8Vector4 value;

		auto& R() { return value.X(); }
		auto& G() { return value.Y(); }
		auto& B() { return value.Z(); }
		auto& A() { return value.W(); }

		const auto& R() const { return value.X(); }
		const auto& G() const { return value.Y(); }
		const auto& B() const { return value.Z(); }
		const auto& A() const { return value.W(); }
		//using u8Vector4::Vector;
	};
};