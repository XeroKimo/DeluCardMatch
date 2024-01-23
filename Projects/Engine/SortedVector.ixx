export module SortedVector;



//struct Identity
//{
//	template<class Ty>
//	constexpr Ty&& operator()(Ty&& val) const noexcept { return std::forward<Ty>(val); }
//};
//template<class Ty, std::invocable<Ty> Projection>
//using ProjectionReturn_t = decltype(Projection(std::declval<Ty>()));
//
//template<class Ty,
//	std::invocable<Ty> Projection = Identity,
//	std::invocable<ProjectionReturn_t<Ty, Projection>, ProjectionReturn_t<Ty, Projection>> Pred = std::less<ProjectionReturn_t<Ty, Projection>>>
///*std::invocable<decltype(std::declval<std::vector<Ty>>().begin()), decltype(std::declval<std::vector<Ty>>().end()), Pred> SortFunc = std::sort<decltype(std::declval<std::vector<Ty>>().begin()), Pred>>*/
//class SortedVector
//{
//	std::vector<Ty> m_vector;
//
//public:
//};