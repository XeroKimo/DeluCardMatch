
export module xk.FunctionPointers;

namespace xk
{
	template<class Ty>
	struct FuncPtrHelper;

	template<class Ty1, class Ty2>
	struct MemFuncPtrHelper;

	template<class RetType, class... Params>
	struct FuncPtrHelper<RetType(Params...)>
	{
		using type = RetType(*)(Params...);
	};

	template<class ObjType, class RetType, class... Params>
	struct MemFuncPtrHelper<ObjType, RetType(Params...)>
	{
		using type = RetType(ObjType::*)(Params...);
	};

	export template<class FuncSig>
	using FuncPtr = FuncPtrHelper<FuncSig>::type;

	export template<class ObjType, class FuncSig>
	using MemFuncPtr = MemFuncPtrHelper<ObjType, FuncSig>::type;
}