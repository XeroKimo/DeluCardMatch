module;
#include <string_view>

export module DeluGame.ControllerContextConstants;

export namespace Delu::ControllerContext
{
	constexpr std::string_view playerControlContext = "Player Control Context";

	namespace Actions
	{
		constexpr std::string_view playerMovement = "Player Movement";
		constexpr std::string_view playerFire = "Player Firing";
		constexpr std::string_view playerStopFire = "Player Stop Firing";
	}
};