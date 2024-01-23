module;

#include <memory>

export module LevelStrip.Event.EnemySpawn;
export import LevelStrip.Event.Base;
import DeluEngine;
import xk.FunctionPointers;

namespace Delu
{
	export class EnemySpawnEvent : public Event
	{

	public:
		EnemySpawnEvent(gsl::not_null<LevelStrip*> levelStrip, std::unique_ptr<Data<Event>> data) :
			Event{ levelStrip, std::move(data) }
		{

		}

		void Execute() override;

	private:
		const Data<EnemySpawnEvent>& GetData() const noexcept { return Event::GetData<EnemySpawnEvent>(); }
	};

	template<>
	struct Data<EnemySpawnEvent> : public Data<Event>
	{
		using event_type = EnemySpawnEvent;
		std::shared_ptr<DeluEngine::SpriteData> sprite;

		Data(seconds timeToExecute, std::shared_ptr<DeluEngine::SpriteData> sprite) :
			Data<Event>(timeToExecute),
			sprite(sprite)
		{
		}

		~Data() override = default;
	};
}