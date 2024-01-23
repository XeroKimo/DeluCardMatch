module;

#include <vector>
#include <memory>
#include <gsl/pointers>
#include <functional>
#include <cassert>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <any>
#include <utility>
#include <stdexcept>
#include <iostream>
#include <type_traits>
#include <optional>

export module ECS;
export import xk.Math.Matrix;
export import xk.Math.Angles;
import xk.ScopeGuard;
import xk.FunctionPointers;

////Object template
//class Ty : public ECS::Object
//{
//public:
//	/*
//	* optional, 
//	* must be placed after all relevant variables
//	* list out variables that the library will auto call a SceneDestructor() function for you
//	* the variable must have a member function SceneDestructor()
//	*/
//	//using SceneDestructorMemberList = ECS::SceneDestructorMemberList<&Ty::someVar, &Ty::someVar2>;
//  /* if the above is not used define the following*/
//	//private: using Base::SceneDestructorMemberList;
//
//	/*
//	* Not optional, 
//	* list out any base classes that would contain a SceneDestructorList
//	* classes which inherit from ECS::Object are mandatory
//	*/
//	using ObjectBaseClasses = ECS::ObjectBaseClassesHelper<ECS::GameObject>;
//
//public:
//	Ty(const ObjectInitializer& initializer) :
//		//SceneAware{ initializer.scene } //Will be re-enabled when we can use virtual inheritence again
//		ECS::GameObjectClass(initializer)
//	{
//
//	}
// 
// 
//	/* optional */
//	void SceneLifetimeBegin()
//	
//	/* optional */
//	//void SceneDestructor();
//	/* if the above will not be used define the following */
//	//private: using Base::SceneDestructor();
//	
//	
//};

namespace ECS
{
	export class Object;
	export class GameObject;
	export class ObjectAllocator;
	export class Scene;

	export enum ReparentLogic
	{
		KeepWorldTransform,
		KeepLocalTransform
	};

	struct Transform
	{
		xk::Math::Vector<float, 2> position;
		xk::Math::Degree<float> rotation;

		Transform& operator+=(const Transform& rh)
		{
			position += rh.position;
			rotation += rh.rotation;
			return *this;
		}

		Transform& operator-=(const Transform& rh)
		{
			position -= rh.position;
			rotation -= rh.rotation;
			return *this;
		}

		friend Transform operator+(Transform lh, const Transform& rh)
		{
			return lh += rh;
		}

		friend Transform operator-(Transform lh, const Transform& rh)
		{
			return lh -= rh;
		}
	};

	class AnyPtr
	{
	private:
		void* m_ptr = nullptr;
		const std::type_info* m_type = nullptr;

	public:
		AnyPtr() noexcept = default;
		template<class Ty>
		AnyPtr(Ty* ptr) noexcept :
			m_ptr{ ptr },
			m_type{ ptr ? &typeid(Ty) : nullptr }
		{

		}

		template<class Ty>
		AnyPtr& operator=(Ty* ptr) noexcept
		{
			m_ptr = ptr;
			m_type = ptr ? &typeid(Ty) : nullptr;
			return *this;
		}

		template<class Ty>
		bool Is() const noexcept
		{
			return m_type && typeid(Ty) == *m_type;
		}

		template<class Ty>
		Ty* Get() const noexcept
		{
			if (Is<Ty>())
			{
				return reinterpret_cast<Ty*>(m_ptr);
			}
			return nullptr;
		}
	};

	export class TransformNode
	{
	private:
		TransformNode* m_parent = nullptr;
		std::vector<TransformNode*> m_children;
		Transform m_localTransform;

		mutable bool m_worldTransformIsDirty = true;
		mutable Transform m_cachedWorldTransform;
	public:
		AnyPtr userData;

	public:
		~TransformNode()
		{
			while (!m_children.empty())
			{
				m_children.back()->SetParent(m_parent, ReparentLogic::KeepLocalTransform);
			}
			SetParent(nullptr, ReparentLogic::KeepLocalTransform);
		}

	public:
		xk::Math::Vector<float, 2> GetLocalPosition() const noexcept { return m_localTransform.position; }
		xk::Math::Degree<float> GetLocalRotation() const noexcept { return m_localTransform.rotation; }
		Transform GetLocalTransform() const noexcept { return m_localTransform; }

		void SetLocalPosition(xk::Math::Vector<float, 2> position) noexcept 
		{ 
			m_localTransform.position = position; 
			m_worldTransformIsDirty = true;
			SetChildrenCachedWorldTransformDirty();
		}
		void SetLocalRotation(xk::Math::Degree<float> rotation) noexcept 
		{ 
			m_localTransform.rotation = rotation;
			m_worldTransformIsDirty = true;
			SetChildrenCachedWorldTransformDirty();
		}
		void SetLocalTransform(Transform transform) noexcept 
		{
			m_localTransform = transform;
			m_worldTransformIsDirty = true;
			SetChildrenCachedWorldTransformDirty();
		}

		xk::Math::Vector<float, 2> GetWorldPosition() const noexcept 
		{ 
			RecalculateWorldTransform();
			return m_cachedWorldTransform.position;
		}
		xk::Math::Degree<float> GetWorldRotation() const noexcept 
		{
			RecalculateWorldTransform();
			return m_cachedWorldTransform.rotation;
		}
		Transform GetWorldTransform() const noexcept 
		{
			RecalculateWorldTransform();
			return m_cachedWorldTransform;
		}

		void SetWorldPosition(xk::Math::Vector<float, 2> position) noexcept 
		{ 
			SetLocalPosition(position - GetParentWorldPosition());
		}
		void SetWorldRotation(xk::Math::Degree<float> rotation) noexcept 
		{ 
			SetLocalRotation(rotation - GetParentWorldRotation());
		}
		void SetWorldTransform(Transform transform) noexcept 
		{ 
			SetLocalTransform(transform - GetParentWorldTransform());
		}

		void SetParent(TransformNode* newParent, ReparentLogic logic = ReparentLogic::KeepWorldTransform)
		{
			DetectSelfParenting(newParent);
			DetectCycle(newParent);

			auto Reparent = [newParent, this]
				{
					if (m_parent)
						std::erase(m_parent->m_children, this);

					auto m_parentProxy = xk::RollbackOnFailure(m_parent, newParent, [this](auto, auto) { if (m_parent) m_parent->m_children.push_back(this); });

					if (m_parentProxy.value)
						m_parentProxy.value->m_children.push_back(this);
				};

			switch (logic)
			{
			case ReparentLogic::KeepWorldTransform:
			{
				auto oldWorldTransform = GetWorldTransform();
				Reparent();
				SetWorldTransform(oldWorldTransform);
			}
				break;
			case ReparentLogic::KeepLocalTransform:
				Reparent();
				m_worldTransformIsDirty = true;
				SetChildrenCachedWorldTransformDirty();
				break;
			default:
				throw std::runtime_error("Unknown reparenting logic");
			}
		}
		TransformNode* GetParent() const noexcept { return m_parent; }
		std::vector<TransformNode*> GetChildren() const noexcept { return m_children; }

	private:
		xk::Math::Vector<float, 2> GetParentWorldPosition() const noexcept
		{
			return GetParentWorldTransform().position;
		}

		xk::Math::Degree<float> GetParentWorldRotation() const noexcept
		{
			return GetParentWorldTransform().rotation;
		}

		Transform GetParentWorldTransform() const noexcept
		{
			return m_parent ? m_parent->GetWorldTransform() : Transform{};
		}

		void DetectCycle(TransformNode* newParent)
		{
			for (TransformNode* p = newParent; p; p = p->m_parent)
			{
				if (p == this)
					throw std::runtime_error("Cyclic parenting detected and is not allowed");
			}
		}

		void DetectSelfParenting(TransformNode* newParent)
		{
			if(newParent == this)
				throw std::runtime_error("Cannot parent to yourself");
		}

		void SetChildrenCachedWorldTransformDirty()
		{
			for (TransformNode* child : m_children)
			{
				if (child->m_worldTransformIsDirty)
					continue;

				child->m_worldTransformIsDirty = true;
				child->SetChildrenCachedWorldTransformDirty();
			}
		}

		void RecalculateWorldTransform() const
		{
			if (m_worldTransformIsDirty)
			{
				m_cachedWorldTransform = GetParentWorldTransform() + GetLocalTransform();
				m_worldTransformIsDirty = false;
			}
		}
	};

	export class WorldTransformProxy
	{
	private:
		class PositionProxy
		{
		private:
			TransformNode& m_transform;

		public:
			PositionProxy(TransformNode& transform) :
				m_transform(transform)
			{

			}

			PositionProxy& operator=(const xk::Math::Vector<float, 2>& position) { m_transform.SetWorldPosition(position); return *this; }
			PositionProxy& operator=(const PositionProxy& position) { return operator=(position.GetValue()); }

			PositionProxy& operator+=(const xk::Math::Vector<float, 2>& position) { return operator=(GetValue() + position); }
			PositionProxy& operator+=(const PositionProxy& position) { return operator+=(position.GetValue()); }

			PositionProxy& operator-=(const xk::Math::Vector<float, 2>& position) { return operator=(GetValue() - position); }
			PositionProxy& operator-=(const PositionProxy& position) { return operator-=(position.GetValue()); }

			xk::Math::Vector<float, 2> GetValue() const { return m_transform.GetWorldPosition(); }
			operator xk::Math::Vector<float, 2>() const { return GetValue(); }
		};

		class RotationProxy
		{
		private:
			TransformNode& m_transform;

		public:
			RotationProxy(TransformNode& transform) :
				m_transform(transform)
			{

			}

			RotationProxy& operator=(const xk::Math::Degree<float>& angle) { m_transform.SetWorldRotation(angle); return *this; }
			RotationProxy& operator=(const RotationProxy& angle) { return operator=(angle.GetValue()); }

			RotationProxy& operator+=(const xk::Math::Degree<float>& angle) { return operator=(GetValue() + angle); }
			RotationProxy& operator+=(const RotationProxy& angle) { return operator+=(angle.GetValue()); }

			RotationProxy& operator-=(const xk::Math::Degree<float>& angle) { return operator=(GetValue() - angle); }
			RotationProxy& operator-=(const RotationProxy& angle) { return operator-=(angle.GetValue()); }

			xk::Math::Degree<float> GetValue() const { return m_transform.GetWorldRotation(); }
			operator xk::Math::Degree<float>() const { return GetValue(); }
		};

	private:
		TransformNode& m_transform;

	public:
		WorldTransformProxy(TransformNode& transform) :
			m_transform(transform)
		{

		}

		PositionProxy Position() { return { m_transform }; }
		RotationProxy Rotation() { return { m_transform }; }
		Transform GetValue() const { return m_transform.GetWorldTransform(); }
	};

	export class LocalTransformProxy
	{
	private:
		class PositionProxy
		{
		private:
			TransformNode& m_transform;

		public:
			PositionProxy(TransformNode& transform) :
				m_transform(transform)
			{

			}

			PositionProxy& operator=(const xk::Math::Vector<float, 2>& position) { m_transform.SetLocalPosition(position); return *this; }
			PositionProxy& operator=(const PositionProxy& position) { return operator=(position.GetValue()); }

			PositionProxy& operator+=(const xk::Math::Vector<float, 2>& position) { return operator=(GetValue() + position); }
			PositionProxy& operator+=(const PositionProxy& position) { return operator+=(position.GetValue()); }

			PositionProxy& operator-=(const xk::Math::Vector<float, 2>& position) { return operator=(GetValue() - position); }
			PositionProxy& operator-=(const PositionProxy& position) { return operator-=(position.GetValue()); }

			xk::Math::Vector<float, 2> GetValue() const { return m_transform.GetLocalPosition(); }
			operator xk::Math::Vector<float, 2>() const { return GetValue(); }
		};

		class RotationProxy
		{
		private:
			TransformNode& m_transform;

		public:
			RotationProxy(TransformNode& transform) :
				m_transform(transform)
			{

			}

			RotationProxy& operator=(const xk::Math::Degree<float>& angle) { m_transform.SetLocalRotation(angle); return *this; }
			RotationProxy& operator=(const RotationProxy& angle) { return operator=(angle.GetValue()); }

			RotationProxy& operator+=(const xk::Math::Degree<float>& angle) { return operator=(GetValue() + angle); }
			RotationProxy& operator+=(const RotationProxy& angle) { return operator+=(angle.GetValue()); }

			RotationProxy& operator-=(const xk::Math::Degree<float>& angle) { return operator=(GetValue() - angle); }
			RotationProxy& operator-=(const RotationProxy& angle) { return operator-=(angle.GetValue()); }

			xk::Math::Degree<float> GetValue() const { return m_transform.GetLocalRotation(); }
			operator xk::Math::Degree<float>() const { return GetValue(); }
		};

	private:
		TransformNode& m_transform;

	public:
		LocalTransformProxy(TransformNode& transform) :
			m_transform(transform)
		{

		}

		PositionProxy Position() { return { m_transform }; }
		RotationProxy Rotation() { return { m_transform }; }
		Transform GetValue() const { return m_transform.GetLocalTransform(); }
	};

	export template<std::derived_from<Object> Ty>
	class UniqueObject
	{
		template<std::derived_from<Object> Ty2>
		friend class UniqueObject;

	private:
		std::shared_ptr<Ty> m_objectClass;

	public:
		UniqueObject() noexcept = default;
		UniqueObject(std::nullptr_t) noexcept {}

		template<std::derived_from<Object> Ty2>
		UniqueObject(std::weak_ptr<Ty2> objectClass) noexcept :
			m_objectClass(objectClass.lock())
		{
			RemoveRoot();
		}

		template<std::derived_from<Object> Ty2>
		UniqueObject(std::shared_ptr<Ty2> objectClass) noexcept :
			m_objectClass(std::move(objectClass))
		{
			RemoveRoot();
		}

		UniqueObject(const UniqueObject&) = delete;
		UniqueObject(UniqueObject&& other) noexcept :
			m_objectClass(std::move(other.m_objectClass))
		{
			other.m_objectClass = nullptr;
		}

		template<std::derived_from<Object> Ty2>
		UniqueObject(const UniqueObject<Ty2>&) = delete;

		template<std::derived_from<Object> Ty2>
		UniqueObject(UniqueObject<Ty2>&& other) noexcept :
			m_objectClass(std::move(other.m_objectClass))
		{
			other.m_objectClass = nullptr;
		}

		UniqueObject& operator=(const UniqueObject&) = delete;
		UniqueObject& operator=(UniqueObject&& other) noexcept
		{
			UniqueObject temp = std::move(other);
			std::swap(m_objectClass, temp.m_objectClass);
			temp.AddRoot();
			return *this;
		}

		bool operator==(std::nullptr_t) const noexcept { return m_objectClass == nullptr; }
		operator bool() const noexcept { return m_objectClass; }

		Ty* operator->() const { return Get(); }
		Ty& operator*() const { return *Get(); }

		Ty* Get() const { return m_objectClass.get(); }

		void SceneDestructor()
		{
			if(m_objectClass)
				m_objectClass->DeleteObject(m_objectClass);
		}

	private:
		void RemoveRoot();
		void AddRoot();
	};

	class SceneAware
	{
	private:
		gsl::not_null<Scene*> m_scene;
	public:
		SceneAware(gsl::not_null<Scene*> scene) :
			m_scene{ scene }
		{

		}

		Scene& GetScene() const { return *m_scene; }

		template<std::derived_from<Object> Ty, class... ConstructorParams>
		std::weak_ptr<Ty> NewObject(ConstructorParams&&... params);

		//void DeleteObject(Object* object);
		void DeleteObject(std::weak_ptr<Object> object);
	};
	
	template<class Ty>
	concept HasSceneDestructorMemberList = requires(Ty obj)
	{
		typename Ty::SceneDestructorMemberList;
	};
	
	template<class Ty>
	concept HasBaseClasses = requires(Ty obj)
	{
		typename Ty::ObjectBaseClasses;
	};

	template<class Ty>
	concept Range = requires(Ty obj)
	{
		{ obj.begin() };
		{ obj.end() };
	};

	template<class Ty>
	concept HasSceneConstructor = requires(Ty obj)
	{
		{ obj.SceneConstructor() };
	};

	template<class Ty>
	concept HasSceneDestructor = requires(Ty obj)
	{
		{ obj.SceneDestructor() };
	};

	export template<auto member>
	struct MemberObjectHolder
	{
		static constexpr auto mem = member;
	};

	export template<class Ty>
	struct IdentityHolder
	{
		using identity = Ty;
	};

	export template<auto... members>
	using SceneDestructorMemberListHelper = std::tuple<MemberObjectHolder<members>...>;

	export template<class... Ty>
	using ObjectBaseClassesHelper = std::tuple<IdentityHolder<Ty>...>;

	export struct TransformLocalSpace
	{
		Transform transform;
	};

	export struct TransformWorldSpace
	{
		Transform transform;
	};

	export struct ObjectInitializer
	{
		gsl::not_null<Scene*> scene;
		xk::FuncPtr<void(Object&)> sceneExtendedLifetimeDestructor;
	};

	export struct UserGameObjectInitializer
	{
		std::optional<std::variant<TransformLocalSpace, TransformWorldSpace>> initialTransform;
		GameObject* optionalParent = nullptr;
	};

	enum class ObjectLifetimeStage : unsigned char
	{
		Not_Started,
		Starting,
		Started
	};

	class Object : public SceneAware
	{
		friend ObjectAllocator;
		friend Scene;

	private:
		ObjectLifetimeStage m_lifetimeStage = ObjectLifetimeStage::Not_Started;
		bool m_pendingDestruction = false;
		xk::FuncPtr<void(Object&)> m_sceneExtendedLifetimeDestructor;
	public:
		std::string name;

	public:
		Object(const ObjectInitializer& initializer) :
			SceneAware{ initializer.scene },
			m_sceneExtendedLifetimeDestructor{ initializer.sceneExtendedLifetimeDestructor }
		{

		}
		virtual ~Object() = default;
	protected:
		void SceneConstructor() {}
		void SceneDestructor() {}
	};

	template<class DerivedSelf>
	class SceneTransform
	{
	private:
		TransformNode m_transform;

	public:
		SceneTransform(const UserGameObjectInitializer& initializer);

	public:
		virtual LocalTransformProxy LocalTransform() { return m_transform; }
		virtual WorldTransformProxy Transform() { return m_transform; }

		virtual void SetParent(DerivedSelf* parent)
		{
			m_transform.SetParent(parent ? &parent->m_transform : nullptr);
		}

		DerivedSelf* GetParent() const
		{
			return (m_transform.GetParent()) ? m_transform.GetParent()->userData.Get<DerivedSelf>() : nullptr;
		}
	};

	class GameObject : public Object, public SceneTransform<GameObject>
	{
	public:
		using ObjectBaseClasses = ObjectBaseClassesHelper<Object>;

	private:

	public:
		GameObject(const ObjectInitializer& objInitializer, const UserGameObjectInitializer& initializer) :
			//SceneAware{ initializer.scene },
			Object{ objInitializer },
			SceneTransform<GameObject>{ initializer }
		{
		}
	};

	template<class DerivedSelf>
	SceneTransform<DerivedSelf>::SceneTransform(const UserGameObjectInitializer& initializer)
	{
		m_transform.userData = static_cast<DerivedSelf*>(this);
		if (initializer.optionalParent)
			m_transform.SetParent(&initializer.optionalParent->m_transform, ECS::KeepLocalTransform);

		if (initializer.initialTransform)
		{
			if (std::holds_alternative<TransformLocalSpace>(initializer.initialTransform.value()))
				m_transform.SetLocalTransform(std::get<TransformLocalSpace>(initializer.initialTransform.value()).transform);
			else if (std::holds_alternative<TransformWorldSpace>(initializer.initialTransform.value()))
				m_transform.SetWorldTransform(std::get<TransformWorldSpace>(initializer.initialTransform.value()).transform);
		}
	}

	template<std::size_t I = 0, class Func, class... Tuple>
	void for_each_tuplement(Func&& func, std::tuple<Tuple...> elms)
	{
		if constexpr (I == sizeof...(Tuple))
			return;
		else
		{
			func(std::get<I>(elms));
			for_each_tuplement<I + 1>(func, elms);
		}
	}

	template<std::size_t I, class Func, class... Tuple>
	void for_each_tuplement_reverse_impl(Func&& func, std::tuple<Tuple...> elms)
	{
		func(std::get<I>(elms));
		if constexpr (I > 0)
			for_each_tuplement_reverse_impl<I - 1>(func, elms);
	}

	template<class Func, class... Tuple>
	void for_each_tuplement_reverse(Func&& func, std::tuple<Tuple...> elms)
	{
		if constexpr(sizeof...(Tuple) == 0)
			return;
		for_each_tuplement_reverse_impl<sizeof...(Tuple) - 1>(std::forward<Func>(func), std::move(elms));
	}

	int globalTabCount = 0;

	struct Tab
	{
		friend std::ostream& operator<<(std::ostream&, const Tab&)
		{
			for (int i = 0; i < globalTabCount; i++)
			{
				std::cout << "\t";
			}
			return std::cout;
		}
	};

	template<class Ty>
	struct SceneExtendedDestruction
	{
		static void DestructorForward(Object& obj)
		{
			globalTabCount += 1;
			std::cout << Tab() << "Scene lifetime ending for: " << obj.name << "\n";
			DestructorImpl(static_cast<Ty&>(obj));
			std::cout << "\n";
			globalTabCount -= 1;
		}

		static void DestructorImpl(Ty& obj)
		{
			std::cout << Tab() << "Destructor Type: " << typeid(Ty).name() << "\n";
			if constexpr (HasSceneDestructor<Ty>)
			{
				std::cout << Tab() << "Calling user scene destructor\n";
				obj.Ty::SceneDestructor();
			}

			if constexpr (HasSceneDestructorMemberList<Ty>)
			{
				globalTabCount += 1;
				std::cout << Tab() << "Begin Member Destructors\n";
				globalTabCount += 1;
				for_each_tuplement_reverse([&obj](auto ptr) mutable
				{
					using helper_type = decltype(ptr);
					using type = decltype(std::invoke(ptr.mem, obj));

					std::cout << "\0" << Tab() << "Member Type: " << typeid(type).name() << "\n";
					if constexpr (Range<type>)
					{
						for (auto& val : std::invoke(ptr.mem, obj))
						{
							static_assert(HasSceneDestructor<decltype(val)>);
							SceneExtendedDestruction<std::remove_reference_t<decltype(val)>>::DestructorImpl(val);
						}
					}
					else
					{
						static_assert(HasSceneDestructor<type>);
						SceneExtendedDestruction<std::remove_reference_t<type>>::DestructorImpl(std::invoke(ptr.mem, obj));
					}
				}, typename Ty::SceneDestructorMemberList{});

				globalTabCount -= 1;
				std::cout << "\0" << Tab() << "End Member Destructors\n";
				globalTabCount -= 1;
			}

			if constexpr (HasBaseClasses<Ty>)
			{
				for_each_tuplement_reverse([&obj](auto ptr) mutable
					{
						using helper_type = decltype(ptr);

						static_assert(std::derived_from<Ty, typename helper_type::identity>);
						SceneExtendedDestruction<typename helper_type::identity>::DestructorImpl(obj);
					}, typename Ty::ObjectBaseClasses{});
			}

		}
	};

	export class ObjectAllocator
	{
		struct DeferAddObjectEvent
		{
			std::shared_ptr<Object> gameObject;

			void operator()(ObjectAllocator& allocator) 
			{
				allocator.m_objects.push_back(gameObject);
			}
		};

		struct DeferRemoveObjectEvent
		{
			std::shared_ptr<Object> gameObject;
			void operator()(ObjectAllocator& allocator)
			{
				allocator.DeleteObject(gameObject);
			}
		};

		using Events = std::variant<DeferAddObjectEvent, DeferRemoveObjectEvent>;

	private:
		std::vector<std::shared_ptr<Object>> m_objects;
		std::vector<Events> m_events;

	public:
		void ExecuteEvents()
		{
			for (Events& event : m_events)
			{
				std::visit([this](auto func) { func(*this); }, event);
			}

			m_events.clear();
		}

		template<std::derived_from<Object> Ty, class... ConstructorParams>
		std::shared_ptr<Ty> DeferCreate(ConstructorParams&&... params)
		{
			std::shared_ptr<Ty> gameObject = std::make_shared<Ty>(std::forward<ConstructorParams>(params)...);
			m_events.push_back(DeferAddObjectEvent{ gameObject });
			return gameObject;
		}

		template<std::derived_from<Object> Ty, class... ConstructorParams>
		std::shared_ptr<Ty> Create(ConstructorParams&&... params)
		{
			std::shared_ptr<Ty> gameObject = std::make_shared<Ty>(std::forward<ConstructorParams>(params)...);
			m_objects.push_back(gameObject);
			return gameObject;
		}

		void DeferDeleteObject(std::shared_ptr<Object> gameObject)
		{
			gameObject->m_pendingDestruction = true;
			m_events.push_back(DeferRemoveObjectEvent{ gameObject });
		}

		void DeleteObject(std::shared_ptr<Object> gameObject)
		{
			auto iterator = std::find(m_objects.begin(), m_objects.end(), gameObject);

			if (iterator != m_objects.end())
			{
				std::iter_swap(iterator, m_objects.end() - 1);
				m_objects.pop_back();
			}
		}

		const std::vector<std::shared_ptr<Object>>& GetObjects() const noexcept { return m_objects; }
	};

	export class SceneSystem
	{
	private:
		gsl::not_null<Scene*> m_scene;

	public:
		SceneSystem(const gsl::not_null<Scene*> scene) :
			m_scene(scene)
		{

		}

		virtual void Update(float deltaTime) {}

		Scene& GetScene() const noexcept { return *m_scene; }
	};

	export class Scene
	{
		template<std::derived_from<Object> Ty>
		friend class UniqueObject;

		struct DeferredLifetimeEvent
		{
			std::shared_ptr<Object> gameObject;

			DeferredLifetimeEvent() = default;
			DeferredLifetimeEvent(std::shared_ptr<Object> gameObject) : gameObject(std::move(gameObject)) {}
			virtual ~DeferredLifetimeEvent() = default;
			virtual void Execute(Scene& scene) = 0;
			void operator()(Scene& scene) { Execute(scene); }
		};

		template<class Ty>
		struct DeferBeginLifetimeEvent : DeferredLifetimeEvent
		{
			DeferBeginLifetimeEvent(std::shared_ptr<Object> gameObject) : DeferredLifetimeEvent(std::move(gameObject)) {}
			void Execute(Scene& scene) override 
			{ 
				if (gameObject->m_pendingDestruction)
					return;

				gameObject->m_lifetimeStage = ObjectLifetimeStage::Starting;

				if constexpr (HasSceneConstructor<Ty>)
				{
					static_cast<Ty*>(gameObject.get())->SceneConstructor();
				}

				gameObject->m_lifetimeStage = ObjectLifetimeStage::Started;
			};
		};

		struct DeferEndLifetimeEvent : DeferredLifetimeEvent
		{
			DeferEndLifetimeEvent(std::shared_ptr<Object> gameObject) : DeferredLifetimeEvent(std::move(gameObject)) {}
			void Execute(Scene& scene) override 
			{
				gameObject->m_sceneExtendedLifetimeDestructor(*gameObject);
				scene.m_allocator.DeleteObject(gameObject);
			};
		};

		enum class InitPhase
		{
			LoadObjectsToMemory,
			BeginObjectLifetimes,
			Complete
		};

	private:
		struct EmptyConstructorTag {};

	private:
		std::any m_externalSystem;
		InitPhase m_phase = InitPhase::LoadObjectsToMemory;
		ObjectAllocator m_allocator;
		std::vector<std::unique_ptr<SceneSystem>> m_systems;
		std::unordered_set<std::shared_ptr<Object>> m_rootObjects;
		std::vector<std::unique_ptr<DeferredLifetimeEvent>> m_deferredEvents;

	public:
		template<std::invocable<Scene&> InitFunc>
		Scene(InitFunc&& func, std::any externalSystem) :
			m_externalSystem(std::move(externalSystem))
		{
			func(*this);
			BeginObjectLifetimes();
			FinishInit();
		}
		
		~Scene()
		{
			for (auto gameObject : m_rootObjects)
			{
				DestroyObject(std::move(gameObject));
			}
		}

		template<std::derived_from<Object> Ty, class... ConstructorParams>
		std::weak_ptr<Ty> NewObject(ConstructorParams&&... constructorParams)
		{
			if (m_phase == InitPhase::LoadObjectsToMemory)
			{
				std::shared_ptr<Ty> gameObject = m_allocator.Create<Ty>(ObjectInitializer{ this, &SceneExtendedDestruction<Ty>::DestructorForward }, std::forward<ConstructorParams>(constructorParams)...);
				m_deferredEvents.push_back(std::make_unique<DeferBeginLifetimeEvent<Ty>>(gameObject));
				m_rootObjects.insert(gameObject);
				return gameObject;
			}
			else if (m_phase == InitPhase::BeginObjectLifetimes)
			{
				std::shared_ptr<Ty> gameObject = m_allocator.Create<Ty>(ObjectInitializer{ this, &SceneExtendedDestruction<Ty>::DestructorForward }, std::forward<ConstructorParams>(constructorParams)...);
				m_rootObjects.insert(gameObject);
				if constexpr (HasSceneConstructor<Ty>)
				{
					gameObject->SceneConstructor();
				}

				return gameObject;
			}
			else
			{
				std::shared_ptr<Ty> gameObject = m_allocator.DeferCreate<Ty>(ObjectInitializer{ this, &SceneExtendedDestruction<Ty>::DestructorForward }, std::forward<ConstructorParams>(constructorParams)...);
				m_rootObjects.insert(gameObject);
				if constexpr (HasSceneConstructor<Ty>)
				{
					gameObject->SceneConstructor();
				}
				return gameObject;
			}
		}

		void DestroyObject(std::shared_ptr<Object> gameObject)
		{
			if (!gameObject)
				return;

			if (gameObject->m_pendingDestruction)
				return;

			gameObject->m_pendingDestruction = true;

			if (m_phase == InitPhase::LoadObjectsToMemory)
			{
				auto it = std::find_if(m_deferredEvents.begin(), m_deferredEvents.end(), [gameObject](auto& lifetime) { return lifetime->gameObject == gameObject; });
				{
					auto temp = std::move(*it);
					std::iter_swap(it, m_deferredEvents.end() - 1);
					m_deferredEvents.pop_back();
				}
				gameObject->m_sceneExtendedLifetimeDestructor(*gameObject);
				m_allocator.DeleteObject(gameObject);
			}
			else if (m_phase == InitPhase::BeginObjectLifetimes)
			{
				if (gameObject->m_lifetimeStage == ObjectLifetimeStage::Not_Started)
				{
					gameObject->m_sceneExtendedLifetimeDestructor(*gameObject);
					m_allocator.DeleteObject(gameObject);
				}
				else if(gameObject->m_lifetimeStage == ObjectLifetimeStage::Starting)
				{
					m_deferredEvents.push_back(std::make_unique<DeferEndLifetimeEvent>(gameObject));
				}
				else if (gameObject->m_lifetimeStage == ObjectLifetimeStage::Started)
				{
					gameObject->m_sceneExtendedLifetimeDestructor(*gameObject);
					m_allocator.DeleteObject(gameObject);
				}
			}
			else
			{
				gameObject->m_sceneExtendedLifetimeDestructor(*gameObject);
				m_allocator.DeferDeleteObject(gameObject);
			}
		}

		template<std::invocable<Object&> Func>
		void ForEachGameObject(Func&& func)
		{
			for (std::shared_ptr<Object> gameObject : m_allocator.GetObjects())
			{
				func(*gameObject);
			}
		}

		template<std::invocable<SceneSystem&> Func>
		void ForEachSceneSystem(Func&& func)
		{
			for (std::unique_ptr<SceneSystem>& sceneSystem : m_systems)
			{
				func(*sceneSystem);
			}
		}

		template<std::derived_from<SceneSystem> Ty, class... ConstructorParams>
		Ty& CreateSystem(ConstructorParams&&... params)
		{
			m_systems.push_back(std::make_unique<Ty>(this, std::forward<ConstructorParams>(params)...));
			return static_cast<Ty&>(*m_systems.back());
		}

		size_t GetObjectCount() const noexcept { return m_allocator.GetObjects().size(); }

		ObjectAllocator& GetAllocator() noexcept { return m_allocator; }
		const ObjectAllocator& GetAllocator() const noexcept { return m_allocator; }

		const std::any& GetExternalSystem() const { return m_externalSystem; }

	private:
		void BeginObjectLifetimes()
		{
			m_phase = InitPhase::BeginObjectLifetimes;
			ExecuteEvents();
		}
		void FinishInit()
		{
			m_phase = InitPhase::Complete;
			ExecuteEvents();
		}

		void ExecuteEvents()
		{
			auto events = std::move(m_deferredEvents);
			for (std::unique_ptr<DeferredLifetimeEvent>& event : events)
			{
				(*event)(*this);
			}
		}
	};

	template<std::derived_from<Object> Ty, class... ConstructorParams>
	std::weak_ptr<Ty> SceneAware::NewObject(ConstructorParams&&... params)
	{
		return GetScene().NewObject<Ty>(std::forward<ConstructorParams>(params)...);
	}

	void SceneAware::DeleteObject(std::weak_ptr<Object> object)
	{
		GetScene().DestroyObject(object.lock());
	}

	template<std::derived_from<Object> Ty>
	void UniqueObject<Ty>::RemoveRoot()
	{
		if(m_objectClass)
			m_objectClass->GetScene().m_rootObjects.erase(m_objectClass);
	}

	template<std::derived_from<Object> Ty>
	void UniqueObject<Ty>::AddRoot()
	{
		if (m_objectClass)
			m_objectClass->GetScene().m_rootObjects.insert(m_objectClass);
	}
}