module;
#include <memory>

module LevelStrip.Event.EnemySpawn;
import LevelStrip;
import DeluEngine;
import Enemy;

namespace Delu
{
	void EnemySpawnEvent::Execute()
	{
		std::shared_ptr<Enemy> e = GetStrip().GetScene().NewObject<Enemy>(ECS::UserGameObjectInitializer{}, GetData()).lock();
	}
}