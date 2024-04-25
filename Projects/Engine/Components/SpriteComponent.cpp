module DeluEngine:SpriteComponent;
import :Engine;

namespace DeluEngine
{
	SpriteObject::SpriteObject(const ECS::ObjectInitializer& initializer, const ECS::UserGameObjectInitializer& goInitializer, ConstructorParams params) :
		//SceneAware(initializer.scene),
		GameObject(initializer, goInitializer),
		m_spriteHandle(GetEngine().renderer.CreateSprite(params.sprite))
	{

	}
}