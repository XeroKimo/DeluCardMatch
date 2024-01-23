module;

#include <functional>
#include <vector>
#include <memory>
#include <chrono>

export module DeluGame;

import DeluEngine;
import LevelStrip;
import DeluGame.Player;
import DeluGame.PlayerState;
import DeluGame.PowerUpPoint;
import Enemy;
import LevelStrip.Event.EnemySpawn;
import DeluGame.ControllerContextConstants;

using namespace xk::Math::Aliases;
export auto TestSceneMain()
{
	return [](ECS::Scene& s)
		{
		};

}

export std::function<void(ECS::Scene&)> GameMain(DeluEngine::Engine& engine)
{
	return TestSceneMain();
}