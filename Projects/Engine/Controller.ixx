module;

#include <array>
#include <bitset>
#include <cstddef>
#include <span>
#include <type_traits>
#include <algorithm>
#include <numeric>
#include <SDL2/SDL.h>
#include <functional>
#include <vector>
#include <variant>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <unordered_map>
#include <gsl/pointers>
#include <string_view>
#include <iostream>

export module DeluEngine:Controller;
import SDL2pp;
export import xk.Math.Matrix;

namespace DeluEngine
{
	export enum class KeyState
	{
		Pressed,
		Released,
		Held
	};

	export enum class Key : std::underlying_type_t<std::byte>
	{
		Temp_None,

		One,
		Two,
		Three,
		Four,
		Five,
		Six,
		Seven,
		Eight,
		Nine,
		Zero,

		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,

		Left,
		Right,
		Up,
		Down
	};

	std::pair<Key, KeyState> MapKey(const SDL_KeyboardEvent& event)
	{
		Key key;
		KeyState keyState;

		switch (event.keysym.sym)
		{
			using enum Key;
		case SDLK_0:
			key = Zero;
			break;
		case SDLK_1:
			key = One;
			break;
		case SDLK_2:
			key = Two;
			break;
		case SDLK_3:
			key = Three;
			break;
		case SDLK_4:
			key = Four;
			break;
		case SDLK_5:
			key = Five;
			break;
		case SDLK_6:
			key = Six;
			break;
		case SDLK_7:
			key = Seven;
			break;
		case SDLK_8:
			key = Eight;
			break;
		case SDLK_9:
			key = Nine;
			break;

		case SDLK_a:
			key = A;
			break;
		case SDLK_b:
			key = B;
			break;
		case SDLK_c:
			key = C;
			break;
		case SDLK_d:
			key = D;
			break;
		case SDLK_e:
			key = E;
			break;
		case SDLK_f:
			key = F;
			break;
		case SDLK_g:
			key = G;
			break;
		case SDLK_h:
			key = H;
			break;
		case SDLK_i:
			key = I;
			break;
		case SDLK_j:
			key = J;
			break;
		case SDLK_k:
			key = K;
			break;
		case SDLK_l:
			key = L;
			break;
		case SDLK_m:
			key = M;
			break;
		case SDLK_n:
			key = N;
			break;
		case SDLK_o:
			key = O;
			break;
		case SDLK_p:
			key = P;
			break;
		case SDLK_q:
			key = Q;
			break;
		case SDLK_r:
			key = R;
			break;
		case SDLK_s:
			key = S;
			break;
		case SDLK_t:
			key = T;
			break;
		case SDLK_u:
			key = U;
			break;
		case SDLK_v:
			key = V;
			break;
		case SDLK_w:
			key = W;
			break;
		case SDLK_x:
			key = X;
			break;
		case SDLK_y:
			key = Y;
			break;
		case SDLK_z:
			key = Z;
			break;

		default:
			key = Temp_None;
			break;
		}


		switch (event.state)
		{
		case SDL_PRESSED:
			keyState = KeyState::Pressed;
			break;
		case SDL_RELEASED:
			keyState = KeyState::Released;
			break;
		}

		return { key, keyState };
	}

	export class Controller
	{
	private:
		std::bitset<256> m_previousState;
		std::bitset<256> m_currentState;
		const std::array<Key, 256> allKeys = []() 
			{
				std::array<Key, 256> keys;
				std::generate(keys.begin(), keys.end(), [i = 0]() mutable { return static_cast<Key>(i++); });
				return keys;
			}();

	public:
		void SetState(Key key)
		{
			m_currentState.set(static_cast<size_t>(key));
		}

		void ResetState(Key key)
		{
			m_currentState.set(static_cast<size_t>(key), false);
		}

		void SwapBuffers()
		{
			m_previousState = m_currentState;
		}

		bool Pressed(Key key) const noexcept
		{
			return m_currentState.test(static_cast<size_t>(key)) && !m_previousState.test(static_cast<size_t>(key));
		}

		bool Released(Key key) const noexcept
		{
			return !m_currentState.test(static_cast<size_t>(key)) && m_previousState.test(static_cast<size_t>(key));
		}

		bool Held(Key key) const noexcept
		{
			return m_currentState.test(static_cast<size_t>(key)) && m_previousState.test(static_cast<size_t>(key));
		}

		bool AnyPressed(std::span<const Key> keys) const noexcept
		{
			return std::any_of(keys.begin(), keys.end(), [this](Key key) { return Pressed(key);  });
		}

		bool AnyReleased(std::span<const Key> keys) const noexcept
		{
			return std::any_of(keys.begin(), keys.end(), [this](Key key) { return Released(key);  });
		}

		bool AnyHeld(std::span<const Key> keys) const noexcept
		{
			return std::any_of(keys.begin(), keys.end(), [this](Key key) { return Held(key);  });
		}

		bool AnyPressed() const noexcept
		{
			return AnyPressed(allKeys);
		}

		bool AnyReleased() const noexcept
		{
			return AnyReleased(allKeys);
		}

		bool AnyHeld() const noexcept
		{
			return AnyHeld(allKeys);
		}

		bool AllPressed(std::span<Key> keys) const noexcept
		{
			return std::all_of(keys.begin(), keys.end(), [this](Key key) { return Pressed(key);  });
		}

		bool AllReleased(std::span<Key> keys) const noexcept
		{
			return std::all_of(keys.begin(), keys.end(), [this](Key key) { return Released(key);  });
		}

		bool AllHeld(std::span<Key> keys) const noexcept
		{
			return std::all_of(keys.begin(), keys.end(), [this](Key key) { return Held(key);  });
		}
	};

	void UpdateController(Controller& controller, const SDL_KeyboardEvent& event)
	{
		auto [key, state] = MapKey(event);

		if (state == KeyState::Pressed)
			controller.SetState(key);
		else
			controller.ResetState(key);
	}

	namespace Input
	{
		export Controller* defaultController;

		export bool Pressed(Key key) noexcept
		{
			return defaultController->Pressed(key);
		}

		export bool Released(Key key) noexcept
		{
			return defaultController->Released(key);
		}

		export bool Held(Key key) noexcept
		{
			return defaultController->Held(key);
		}

		export bool AnyPressed(std::span<const Key> keys) noexcept
		{
			return defaultController->AnyPressed(keys);
		}

		export bool AnyReleased(std::span<const Key> keys) noexcept
		{
			return defaultController->AnyReleased(keys);
		}

		export bool AnyHeld(std::span<const Key> keys) noexcept
		{
			return defaultController->AnyHeld(keys);
		}

		export bool AnyPressed() noexcept
		{
			return defaultController->AnyPressed();
		}

		export bool AnyReleased() noexcept
		{
			return defaultController->AnyReleased();
		}

		export bool AnyHeld() noexcept
		{
			return defaultController->AnyHeld();
		}

		export bool AllPressed(std::span<Key> keys) noexcept
		{
			return defaultController->AllPressed(keys);
		}

		export bool AllReleased(std::span<Key> keys) noexcept
		{
			return defaultController->AllReleased(keys);
		}

		export bool AllHeld(std::span<Key> keys) noexcept
		{
			return defaultController->AllHeld(keys);
		}
	}

	namespace Experimental
	{
		export struct Button
		{
			template<class Ty>
			using AllOf = std::vector<Ty>;
			using Input = std::variant<Key>;

			std::vector<Input> inputs;
			std::function<void(bool)> action;

			void TryInvoke(const Controller& controller, KeyState keyStateToCheck) const
			{
				bool keyDown = false;
				auto keyExecuted = [&](const Input& input)
					{
						if (keyStateToCheck == KeyState::Pressed)
						{
							keyDown = true;
							return controller.Pressed(std::get<0>(input));
						}
						else if (keyStateToCheck == KeyState::Released)
						{
							return controller.Released(std::get<0>(input));
						}
						else if (keyStateToCheck == KeyState::Held)
						{
							keyDown = true;
							return controller.Held(std::get<0>(input));
						}

						throw std::logic_error("Unknown key state");
					};

				if (std::any_of(inputs.begin(), inputs.end(), keyExecuted))
					action(keyDown);
			}
		};

		export struct Axis2D
		{
			template<class Ty>
			using AllOf = std::vector<Ty>;
			using Input = std::variant<Key>;

			std::vector<std::pair<Input, xk::Math::Vector<float, 2>>> inputs;
			std::function<void(xk::Math::Vector<float, 2>)> action;

			void TryInvoke(const Controller& controller, KeyState keyStateToCheck) const
			{
				auto keyExecuted = [&](const std::pair<Input, xk::Math::Vector<float, 2>>& input)
				{
					if (keyStateToCheck == KeyState::Pressed)
					{
						return controller.Pressed(std::get<0>(input.first));
					}
					else if (keyStateToCheck == KeyState::Released)
					{
						return controller.Released(std::get<0>(input.first));
					}
					else if (keyStateToCheck == KeyState::Held)
					{
						return controller.Held(std::get<0>(input.first));
					}

					throw std::logic_error("Unknown key state");
				};

				auto gatherState = [&]() -> std::optional<xk::Math::Vector<float, 2>>
				{
					if (keyStateToCheck == KeyState::Held)
					{
						xk::Math::Vector<float, 2> state;
						bool inputFound = false;
						for (auto [input, stateOutput] : inputs)
						{
							if (controller.Held(std::get<0>(input)) || controller.Pressed(std::get<0>(input)))
							{
								inputFound = true;
								state += stateOutput;
							}
							else if (controller.Released(std::get<0>(input)))
							{
								inputFound = true;
							}
						}

						return (inputFound) ? std::optional<xk::Math::Vector<float, 2>>{ state } : std::nullopt;
					}
					else
					{
						xk::Math::Vector<float, 2> state;
						bool inputFound = false;
						for (auto [input, stateOutput] : inputs | std::views::filter(keyExecuted))
						{
							inputFound = true;
							state += stateOutput;
						}

						return (inputFound) ? std::optional<xk::Math::Vector<float, 2>>{ state } : std::nullopt;
					}
				};

				auto axis = gatherState();
				if (axis.has_value())
				{
					if (xk::Math::MagnitudeSquared(axis.value()) != 0)
						action(xk::Math::Normalize(axis.value()));
					else
						action({});
				}
			}
		};

		export struct ControllerAction
		{
			std::string name;
			KeyState invocationState;
			std::variant<Button, Axis2D> action;

			void TryInvoke(const Controller& controller) const
			{
				try
				{
					std::visit([&](const auto& actionType) { actionType.TryInvoke(controller, invocationState); }, action);
				}
				catch(const std::bad_function_call&)
				{
					std::cout << name << " action has no bounded callback\n";
				}
			}

			void BindButton(std::function<void(bool)> callback)
			{
				std::get<Button>(action).action = std::move(callback);
			}

			void BindAxis2D(std::function<void(xk::Math::Vector<float, 2>)> callback)
			{
				std::get<Axis2D>(action).action = std::move(callback);
			}
		};

		export struct ControllerContext
		{
			std::vector<ControllerAction> actions;

			void Execute(const Controller& controller) const
			{
				for (const ControllerAction& action : actions)
				{
					action.TryInvoke(controller);
				}
			}

			ControllerAction& FindAction(std::string_view name)
			{
				auto it = std::find_if(actions.begin(), actions.end(), [name](const ControllerAction& action) { return action.name == name; });
				if (it == actions.end())
					throw std::logic_error("Could not find matching action: " + std::string{ name });

				return *it;
			}
		};

		export class ControllerContextManager
		{
		private:
			std::unordered_map<std::string, ControllerContext> m_registeredContexts;
			std::vector<gsl::not_null<ControllerContext*>> m_contextStack;

		public:
			void RegisterContext(std::string contextName, ControllerContext context)
			{
				auto [it, added] = m_registeredContexts.insert({ contextName, std::move(context) });

				if (!added)
					throw std::logic_error{ "Controller context name conflict: " + contextName };
			}

			void RegisterContext(std::string_view contextName, ControllerContext context)
			{
				RegisterContext(std::string{ contextName }, std::move(context));
			}

			ControllerContext& FindContext(std::string contextName)
			{
				auto it = m_registeredContexts.find(contextName);
				if (it == m_registeredContexts.end())
					throw std::logic_error{ "Could not find matching action: " + contextName };

				return it->second;
			}

			ControllerContext& FindContext(std::string_view contextName)
			{
				return FindContext(std::string{ contextName });
			}

			void PushContext(std::string contextName)
			{
				m_contextStack.push_back(&m_registeredContexts.at(contextName));
			}

			void PushContext(std::string_view contextName)
			{
				PushContext(std::string{ contextName });
			}

			void PopContext()
			{
				m_contextStack.pop_back();
			}

			void ClearContextStack()
			{
				m_contextStack.clear();
			}

			void Execute(const Controller& controller) const
			{
				if (m_contextStack.size() == 0)
					return;

				m_contextStack.back()->Execute(controller);
			}

			ControllerContext& GetCurrentContext()
			{
				if (m_contextStack.size() == 0)
					throw std::logic_error{ "No context on the stack" };

				return *m_contextStack.back();
			}
		};
	}
};