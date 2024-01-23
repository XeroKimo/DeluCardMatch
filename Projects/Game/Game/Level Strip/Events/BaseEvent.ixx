module;

#include <chrono>
#include <gsl/pointers>
#include <memory>

export module LevelStrip.Event.Base;

namespace Delu
{
	export template<class Ty>
	struct Data;

	export struct LevelStrip;

	export class Event
	{
		using seconds = std::chrono::duration<float>;

		gsl::not_null<LevelStrip*> m_levelStrip;
		std::unique_ptr<Data<Event>> m_data;

	public:
		Event(gsl::not_null<LevelStrip*> levelStrip, std::unique_ptr<Data<Event>> data) :
			m_levelStrip{ levelStrip },
			m_data{ std::move(data) }
		{

		}
		virtual ~Event() = default;

	public:
		virtual void Execute() = 0;
		seconds TimeToExecute() const noexcept;

	protected:
		template<class Ty>
		const Data<Ty>& GetData() const noexcept { return static_cast<const Data<Ty>&>(*m_data); }

		LevelStrip& GetStrip() noexcept { return *m_levelStrip; }
		const LevelStrip& GetStrip() const noexcept { return *m_levelStrip; }
	};

	template<>
	struct Data<Event>
	{
		using seconds = std::chrono::duration<float>;
		seconds timeToExecute;

		Data(seconds timeToExecute) :
			timeToExecute(timeToExecute)
		{

		}
		virtual ~Data() = 0 {}
	};
}