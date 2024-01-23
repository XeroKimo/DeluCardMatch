module DeluEngine:Physics;
import DeluEngine;

namespace DeluEngine
{
	template<>
	b2World& PhysicsContainer<PhysicsGameObject>::GetPhysicsWorld()
	{
		return Self().GetEngine().physicsWorld;
	}
}