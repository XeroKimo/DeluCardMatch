module;

#include <chrono>
#include <string_view>
#include <vector>
#include <string>
#include <cstdint>
#include <memory>
#include <queue>
#include <functional>
#include <unordered_map>
#include <gsl/pointers>

export module DeluEngine:Heart;

namespace DeluEngine
{

	export class Heart;

	export class PulseCallback;


	export class PulseGroup
	{
	private:
		std::int16_t priority;
		float timeMultiplier = 1;
		std::vector<std::unique_ptr<PulseGroup>> m_childGroups;
		std::string name;

		std::vector<gsl::not_null<PulseCallback*>> callbacks;

	public:
		PulseGroup(std::string name, int16_t priority) :
			priority{ priority },
			name{ name }
		{

		}

		void AddPulseCallback(gsl::not_null<PulseCallback*> callback)
		{
			callbacks.push_back(std::move(callback));
		}

		void RemovePulseCallback(gsl::not_null<PulseCallback*> callback)
		{
			std::erase(callbacks, callback);
		}

		void AddChildGroup(std::unique_ptr<PulseGroup> group)
		{
			m_childGroups.push_back(std::move(group));
			std::push_heap(m_childGroups.begin(), m_childGroups.end(), [](const auto& lh, const auto& rh) { return std::greater()(lh->GetPriority(), rh->GetPriority()); });
		}

		void ClearCallbacks()
		{
			callbacks.clear();
		}

		void Pulse(std::chrono::nanoseconds deltaTime);
		
		std::int16_t GetPriority() const noexcept { return priority; }
	};

	export class Heart
	{
	private:
		std::chrono::steady_clock::time_point previousTick = std::chrono::steady_clock::now();
		std::vector<std::unique_ptr<PulseGroup>> rootGroups;
		std::unordered_map<std::string, PulseGroup*> lookUpCache;

	public:
		void RegisterGroup(std::string_view name, std::int16_t priority)
		{
			auto index = name.rfind('.');
			if(index != std::string_view::npos)
			{
				auto group = std::make_unique<PulseGroup>(std::string{ name.substr(index + 1) }, priority);
				lookUpCache.insert({ std::string{ name }, group.get() });
				lookUpCache.at(std::string{ name.substr(0, index) })->AddChildGroup(std::move(group));
			}
			else
			{
				rootGroups.push_back(std::make_unique<PulseGroup>(std::string{ name }, priority));
				lookUpCache.insert({ std::string{ name }, rootGroups.back().get() });
				std::push_heap(rootGroups.begin(), rootGroups.end(), [](const auto& lh, const auto& rh) { return std::greater()(lh->GetPriority(), rh->GetPriority()); });
			}
		}

		void RegisterCallback(std::string_view name, gsl::not_null<PulseCallback*> callback)
		{
			lookUpCache.at(std::string{ name })->AddPulseCallback(callback);
		}

		void RemoveCallback(std::string_view name, gsl::not_null<PulseCallback*> callback)
		{
			lookUpCache.at(std::string{ name })->RemovePulseCallback(callback);
		}

		void ClearCallbacks(std::string_view name)
		{
			lookUpCache.at(std::string{ name })->ClearCallbacks();
		}

		void Pulse()
		{
			auto currentTick = std::chrono::steady_clock::now();
			auto delta = currentTick - previousTick;

			for(auto& pulseGroup : rootGroups)
			{
				pulseGroup->Pulse(delta);
			}

			previousTick = currentTick;
		}
	};

	export Heart gHeart;

	export class PulseCallback
	{
	private:
		Heart* m_heart;
		std::string m_groupName;

	public:
		PulseCallback(std::string groupName, Heart& heart = gHeart) :
			m_heart{ &heart },
			m_groupName{ groupName }
		{
			m_heart->RegisterCallback(m_groupName, this);
		}
		PulseCallback(const PulseCallback&) = delete;
		PulseCallback(PulseCallback&& other) noexcept :
			m_heart{ other.m_heart },
			m_groupName{ other.m_groupName }
		{
			m_heart->RegisterCallback(m_groupName, this);
		}

		virtual void Update(std::chrono::nanoseconds deltaTime) = 0;

	protected:
		PulseCallback& operator=(const PulseCallback&) = delete;
		PulseCallback& operator=(PulseCallback&&) noexcept = default;

		~PulseCallback()
		{
			m_heart->RemoveCallback(m_groupName, this);
		}
	};

	void PulseGroup::Pulse(std::chrono::nanoseconds deltaTime)
	{
		deltaTime *= timeMultiplier;

		std::size_t i = 0;
		for(; i < m_childGroups.size() && m_childGroups[i]->priority <= 0; i++)
		{
			m_childGroups[i]->Pulse(deltaTime);
		}

		for(auto& callback : callbacks)
		{
			callback->Update(deltaTime);
		}

		for(; i < m_childGroups.size(); i++)
		{
			m_childGroups[i]->Pulse(deltaTime);
		}
	}
}
