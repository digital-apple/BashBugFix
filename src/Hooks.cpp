#include "Hooks.h"

namespace Hooks 
{
	struct CheckAttack 
	{
		static bool thunk(RE::Actor* a_actor)
		{
			if (a_actor->GetAttackState() == RE::ATTACK_STATE_ENUM::kNone) { return true; }
			return func(a_actor);
		}
		static inline REL::Relocation<decltype(thunk)> func;
	};

	void Install()
	{
		REL::Relocation<std::uintptr_t> check_attack{ RELOCATION_ID(42832, 44001), OFFSET(0x4D, 0x4F) };
		stl::write_thunk_call<CheckAttack>(check_attack.address());
		logger::info("Installed Hook!");
	}
};
