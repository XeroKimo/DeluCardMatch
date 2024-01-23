module;

#include <concepts>
#include <exception>

export module xk.ScopeGuard;

namespace xk
{
	export template<std::invocable<> Ty>
	class ScopeExit
	{
		Ty m_invocable;

	public:
		ScopeExit(Ty invocable) :
			m_invocable{ std::move(invocable) }
		{

		}

		~ScopeExit()
		{
			m_invocable();
		}
	};

	export template<std::invocable<> Ty>
	class ScopeFailure
	{
		int m_uncaughtExceptions{ std::uncaught_exceptions() };
		Ty m_invocable;

	public:
		ScopeFailure(Ty invocable) :
			m_invocable{ std::move(invocable) }
		{

		}

		~ScopeFailure()
		{
			if(std::uncaught_exceptions() > m_uncaughtExceptions)
				m_invocable();
		}
	};

	export template<std::invocable<> Ty>
	class ScopeSuccess
	{
		int m_uncaughtExceptions{ std::uncaught_exceptions() };
		Ty m_invocable;

	public:
		ScopeSuccess(Ty invocable) :
			m_invocable{ std::move(invocable) }
		{

		}

		~ScopeSuccess()
		{
			if(std::uncaught_exceptions() == m_uncaughtExceptions)
				m_invocable();
		}
	};

	export template<class Ty, std::invocable<Ty&, Ty> Func>
	class RollbackOnFailureImpl
	{
	private:
		int m_uncaughtExceptions{ std::uncaught_exceptions() };
		Func m_func;

	public:
		Ty rollbackValue;
		Ty& value;

	public:
		RollbackOnFailureImpl(Ty& value, Func func) :
			m_func(func),
			rollbackValue(value),
			value(value)
		{

		}

		RollbackOnFailureImpl(Ty& value, Ty newValue, Func func) :
			m_func(func),
			rollbackValue(value),
			value(value)
		{
			value = newValue;
		}

		~RollbackOnFailureImpl()
		{
			if (std::uncaught_exceptions() > m_uncaughtExceptions)
			{
				Ty preRollbackValue = value;
				value = rollbackValue;
				m_func(value, preRollbackValue);
			}
		}
	};


	export template<class Ty>
	auto RollbackOnFailure(Ty& value)
	{
		return RollbackOnFailureImpl(value, [](Ty&, Ty) {});
	}

	export template<class Ty>
		auto RollbackOnFailure(Ty& value, Ty newValue)
	{
		return RollbackOnFailureImpl(value, newValue, [](Ty&, Ty) {});
	}

	export template<class Ty, std::invocable<Ty&, Ty> Func>
	RollbackOnFailureImpl<Ty, Func> RollbackOnFailure(Ty& value, Func&& func)
	{
		return RollbackOnFailureImpl(value, std::forward<Func>(func));
	}

	export template<class Ty, std::invocable<Ty&, Ty> Func>
	RollbackOnFailureImpl<Ty, Func> RollbackOnFailure(Ty& value, Ty newValue, Func&& func)
	{
		return RollbackOnFailureImpl(value, newValue, std::forward<Func>(func));
	}

	export template<class Ty, std::invocable<Ty&> Func>
	class CommitOnSuccessImpl
	{
	private:
		int m_uncaughtExceptions{ std::uncaught_exceptions() };
		Func m_func;

		Ty& valueRef;
	public:
		Ty value;

	public:
		CommitOnSuccessImpl(Ty& value, Func func) :
			m_func(func),
			valueRef(value),
			value(value)
		{

		}

		CommitOnSuccessImpl(Ty& value, Ty newValue, Func func) :
			m_func(func),
			valueRef(value),
			value(newValue)
		{

		}

		~CommitOnSuccessImpl()
		{
			if (std::uncaught_exceptions() == m_uncaughtExceptions)
			{
				valueRef = value;
				m_func(valueRef);
			}
		}
	};


	export template<class Ty>
	auto CommitOnSuccess(Ty& value)
	{
		return CommitOnSuccessImpl(value, [](Ty&) {});
	}

	export template<class Ty>
	auto CommitOnSuccess(Ty& value, Ty newValue)
	{
		return CommitOnSuccessImpl(value, newValue, [](Ty&) {});
	}

	export template<class Ty, std::invocable<Ty&> Func>
	CommitOnSuccessImpl<Ty, Func> CommitOnSuccess(Ty& value, Func func)
	{
		return CommitOnSuccessImpl(value, func);
	}

	export template<class Ty, std::invocable<Ty&> Func>
	CommitOnSuccessImpl<Ty, Func> CommitOnSuccess(Ty& value, Ty newValue, Func func)
	{
		return CommitOnSuccessImpl(value, newValue, func);
	}
}