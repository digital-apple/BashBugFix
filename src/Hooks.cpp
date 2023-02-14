#include "Hooks.h"

namespace Hooks::Draw
{
	void func(){}

	void Install()
	{
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(38048, 39004), OFFSET(0x10F, 0x14F) };

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_func, std::uintptr_t a_addr)
			{
				Xbyak::Label funcLabel;
				Xbyak::Label skipLabel;
				Xbyak::Label retnLabel;

#ifdef SKYRIM_AE
				const int actorState1{ 0x0C8 }; // 0x0C0 for 1.6.353, 0x0C8 for 1.6.640
				const int actorState2{ 0x0CC }; // 0x0C4 for 1.6.353, 0x0CC for 1.6.640
#endif

#ifdef SKYRIM_SE
				const int actorState1{ 0x0C0 };
				const int actorState2{ 0x0C4 };
#endif
				
				mov(edx, ptr[rcx + 0x28]);
			
#ifdef SKYRIM_AE
				cmp(dword[rdi + actorState1], 0x60000000);
#endif

#ifdef SKYRIM_SE
				cmp(dword[rbx + actorState1], 0x60000000);
#endif

				jne(skipLabel);
				mov(edx, 2);

#ifdef SKYRIM_AE
				cmp(dword[rdi + actorState2], 0x1160);
#endif

#ifdef SKYRIM_SE
				cmp(dword[rbx + actorState2], 0x1160);
#endif

				jne(skipLabel);
				mov(edx, 2);
				
				L(skipLabel);
				shr(edx, 1);
				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(a_addr + 0x5);

				L(funcLabel);
				dq(a_func);
			}
		};

		Patch patch(reinterpret_cast<uintptr_t>(func), target.address());
		patch.ready();

		auto& trampoline = SKSE::GetTrampoline();
		SKSE::AllocTrampoline(75);

		trampoline.write_branch<5>(target.address(), trampoline.allocate(patch));

		logger::info("Hook    :    Hook \"Draw\" was installed.");
	}
};

namespace Hooks::Reset
{
	void func(RE::Actor* a_actor)
	{
		a_actor->actorState1.meleeAttackState = RE::ATTACK_STATE_ENUM::kBash;

		logger::info("Hook    :    Currently resetting attack flags for {}", a_actor->GetName());
		const auto address = reinterpret_cast<uintptr_t>(&a_actor->actorState2);

		std::jthread([=]() {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			while (a_actor->GetAttackState() == RE::ATTACK_STATE_ENUM::kBash) {
				auto value = *reinterpret_cast<uint16_t*>(address);
				if (value != 4192 && value != 4448 || a_actor->GetLifeState() == RE::ACTOR_LIFE_STATE::kBleedout) {
					SKSE::GetTaskInterface()->AddTask([=]() {
						a_actor->actorState1.meleeAttackState = RE::ATTACK_STATE_ENUM::kNone;
					});
				}
			}
		}).detach();
		logger::info("Hook    :    Done resetting attack flags for {}\n", a_actor->GetName());
	}

	void Install()
	{
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(41733, 42814), 0x1A };

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_func)
			{
				Xbyak::Label funcLabel;
				Xbyak::Label skipLabel;
				Xbyak::Label retnLabel;

#ifdef SKYRIM_AE
				const int actorState1{ 0x0C8 };	// 0x0C0 for 1.6.353, 0x0C8 for 1.6.640
				const int actorState2{ 0x0CC };	// 0x0C4 for 1.6.353, 0x0CC for 1.6.640
#endif

#ifdef SKYRIM_SE
				const int actorState1{ 0x0C0 };
				const int actorState2{ 0x0C4 };
#endif

				cmp(dword[rbx + actorState2], 0x1160);
				je(skipLabel);

				and_(dword[rbx + actorState1], 0x0FFFFFFF);
				jmp(retnLabel);

				L(skipLabel);
				mov(rcx, rbx);
				call(ptr[rip + funcLabel]);
				jmp(retnLabel);

				L(retnLabel);
				mov(al, 1);
				add(rsp, 0x20);
				pop(rbx);
				ret();

				L(funcLabel);
				dq(a_func);
			}
		};

		Patch patch(reinterpret_cast<uintptr_t>(func));
		patch.ready();

		auto& trampoline = SKSE::GetTrampoline();
		SKSE::AllocTrampoline(65);

		trampoline.write_branch<5>(target.address(), trampoline.allocate(patch));

		logger::info("Hook    :    Hook \"Reset\" was installed.");
	}
}

namespace Hooks
{
	void Install()
	{
		logger::info("Hooks    :    Installing hooks.");
		Draw::Install();
		Reset::Install();
	}
}
