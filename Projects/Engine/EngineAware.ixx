module;

#include <gsl/pointers>

export module DeluEngine:EngineAware;

namespace DeluEngine
{
	export struct Engine;

	export class EngineAware
	{
	private:
		gsl::not_null<Engine*> m_engine;

	public:
		EngineAware(gsl::not_null<Engine*> engine) :
			m_engine(engine)
		{

		}

		Engine& GetEngine() const noexcept { return *m_engine; }
	};
}