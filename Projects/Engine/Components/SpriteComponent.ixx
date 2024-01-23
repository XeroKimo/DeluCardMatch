module;

#include <memory>

export module DeluEngine:SpriteComponent;
import :Engine;
export import :ECS;
export import :Renderer;

namespace DeluEngine
{
	export class SpriteObject : public GameObject
	{
	public:
		struct ConstructorParams
		{
			std::shared_ptr<SpriteData> sprite = nullptr;
		};

	private:
		SpriteHandle m_spriteHandle;

	public:
		using ObjectBaseClasses = ECS::ObjectBaseClassesHelper<DeluEngine::GameObject>;

	public:
		SpriteObject(const ECS::ObjectInitializer& initializer, const ECS::UserGameObjectInitializer& goInitializer, ConstructorParams params = {}) :
			//SceneAware(initializer.scene),
			GameObject(initializer, goInitializer),
			m_spriteHandle(GetEngine().renderer.CreateSprite(params.sprite))
		{

		}

	public:
		void Update(float deltaTime) override
		{
			m_spriteHandle->position = Transform().Position();
			m_spriteHandle->angle = static_cast<xk::Math::Degree<float>>(Transform().Rotation());
		}
	};
}