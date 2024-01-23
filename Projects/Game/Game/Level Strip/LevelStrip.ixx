module;

#include <chrono>
#include <unordered_map>
#include <typeindex>
#include <iostream>
#include <gsl/pointers>
#include <concepts>
#include <functional>
#include <iterator>
#include <algorithm>
#include <utility>

export module LevelStrip;
export import LevelStrip.Event.Base;
export import LevelStrip.Event.EnemySpawn;
import xk.Math.Matrix;
import SDL2pp;

import DeluEngine;
import xk.FunctionPointers;

namespace Delu
{
	using namespace xk::Math::Aliases;

	using seconds = std::chrono::duration<float>;
	export struct PlayField
	{
		uVector2 intRatio;
		float size;

		SDL2pp::FRect ToRect() const { return SDL2pp::FRect{ .w = intRatio.X() * size, .h = intRatio.Y() * size }; }
	};

	export struct LevelStripData
	{
		PlayField playField;
		std::vector<std::unique_ptr<Data<Event>>> events;
	};


	using EventConstructorFunc = xk::FuncPtr<std::unique_ptr<Event>(gsl::not_null<LevelStrip*>, std::unique_ptr<Data<Event>>)>;

	template<std::derived_from<Data<Event>> Ty>
	std::pair<std::type_index, EventConstructorFunc> MakeConstructorPair()
	{
		return 
		{ 
			typeid(Ty), 
			+[](gsl::not_null<LevelStrip*> strip, std::unique_ptr<Data<Event>> data) -> std::unique_ptr<Event> 
			{ 
				return std::make_unique<Ty::event_type>(strip, std::move(data)); 
			} 
		};
	}

	const std::unordered_map<std::type_index, EventConstructorFunc> eventConstructors
	{
		MakeConstructorPair<Data<EnemySpawnEvent>>()
	};

	export struct LevelStrip : public DeluEngine::SceneSystem
	{
		std::vector<std::unique_ptr<Event>> events;

		size_t lastExecutedEvent = 0;
		seconds elapsedTime{};

		LevelStrip(const gsl::not_null<ECS::Scene*> scene, LevelStripData data) :
			DeluEngine::SceneSystem(scene)
		{
			for (auto& eventData : data.events)
			{
				events.push_back(eventConstructors.at(typeid(*eventData))(this, std::move(eventData)));
			}

			std::sort(events.begin(), events.end(), [](const std::unique_ptr<Event>& lh, const std::unique_ptr<Event>& rh)
			{
				return lh->TimeToExecute() < rh->TimeToExecute();
			});
		}

		void Update(float deltaTime)
		{
			elapsedTime += seconds{ deltaTime };
			for (; lastExecutedEvent < events.size() && ShouldExecute(*events[lastExecutedEvent]); lastExecutedEvent++)
			{
				events[lastExecutedEvent]->Execute();
			}
		}

	private:
		bool ShouldExecute(const Event& event) const noexcept
		{
			return elapsedTime >= event.TimeToExecute();
		}
	};

	seconds Event::TimeToExecute() const noexcept
	{
		return m_data->timeToExecute;
	}
};