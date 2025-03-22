#ifndef EXTENDED_KS_INCLUDED
#define EXTENDED_KS_INCLUDED

#include <cstring>
#include "Cutscene.h"
#include "SM64DS_PI.h"

using Any = KuppaScriptImpl::CharID_Type<0xff>;

namespace KuppaScriptImpl {

template<std::size_t scriptSize = 0, class... Initializers>
class ExtendedScriptCompiler : public BaseScriptCompiler<ExtendedScriptCompiler, scriptSize, Initializers...>
{
	using Base = BaseScriptCompiler<ExtendedScriptCompiler, scriptSize, Initializers...>;

public:
	template<uint8_t subID, class... NewInitializers>
	consteval auto CamInstruction(const auto&... args)
	{
		return static_cast<Base&>(*this).template CamInstruction<subID, NewInitializers...>(args...);
	}

	template<CharID Char, uint8_t subID, class... NewInitializers>
	consteval auto PlayerInstruction(const auto&... args)
	{
		return static_cast<Base&>(*this).template PlayerInstruction<Char, subID, NewInitializers...>(args...);
	}

	/* -------- -------- Custom player instructions -------- -------- */

	template<CharID Char = Any>
	consteval auto PlayWindSound()
	{
		return PlayerInstruction<Char, 15>();
	}

	consteval auto SetEntranceMode(int8_t entranceMode)
	{
		return PlayerInstruction<Any, 16>(entranceMode);
	}

	/* -------- -------- Custom camera instructions -------- -------- */

	template<class F, CharID Char = Any>
	consteval auto Call(F)
	{
		static constexpr bool noParams      = std::is_invocable_v<F>;
		static constexpr bool paramIsCam    = std::is_invocable_v<F, Camera&>;
		static constexpr bool paramIsPlayer = std::is_invocable_v<F, Player&>;

		static_assert(paramIsCam + paramIsPlayer + noParams == 1);
		static_assert(!paramIsPlayer || std::same_as<Char, Any>);

		static constexpr auto funcPtr = +[](ActorBase& actor) [[gnu::flatten]]
		{
			if constexpr (noParams)      F{}();
			if constexpr (paramIsCam)    F{}(static_cast<Camera&>(actor));
			if constexpr (paramIsPlayer) F{}(static_cast<Player&>(actor));
		};

		using Initializer = decltype([](char* scriptStart)
		{
			char* addr = scriptStart + scriptSize + 7;

			std::memcpy(addr, &funcPtr, sizeof(funcPtr));
		});

		if constexpr (paramIsPlayer)
			return PlayerInstruction<Char, 14, Initializer>(0);
		else
			return CamInstruction<39, Initializer>(0);
	}

	template<unsigned length>
	consteval auto Print(const char (&string)[length])
	{
		return CamInstruction<45>(std::to_array<const char, length>(string));
	}

	consteval auto Nop()
	{
		return CamInstruction<0xff>(); // no implementation needed
	}
};

template<> struct DefaultScriptCompiler<{}>
{
	using Type = ExtendedScriptCompiler<>;
};

template<> struct DefaultCharImpl<ExtendedScriptCompiler>
{
	using Type = Any;
};

} // namespace KuppaScriptImpl

#endif