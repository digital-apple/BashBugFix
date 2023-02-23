#include "Hooks.h"

namespace Hooks::Draw
{
	std::uint8_t func(RE::Actor* a_actor)
	{
		bool bIsBashing = false;
		a_actor->GetGraphVariableBool("IsBashing", bIsBashing);

		if (bIsBashing) {
			return 2u;
		}

		return 1u;
	}

	void Install()
	{
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(38048, 39004), OFFSET(0x10F, 0x14F) };

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_func, std::uintptr_t a_addr)
			{
				Xbyak::Label funcLabel;
				Xbyak::Label retnLabel;

				push(r10);
				push(r9);
				push(r8);
				push(rdi);
				push(rsi);
				push(rbx);

#ifdef SKYRIM_AE
				mov(rcx, rdi);
#endif

#ifdef SKYRIM_SE
				mov(rcx, rbx);
#endif

				sub(rsp, 0x20);
				call(ptr[rip + funcLabel]);
				add(rsp, 0x20);
				mov(edx, eax);
				shr(edx, 1);

				pop(rbx);
				pop(rsi);
				pop(rdi);
				pop(r8);
				pop(r9);
				pop(r10);

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
		SKSE::AllocTrampoline(200);

		trampoline.write_branch<5>(target.address(), trampoline.allocate(patch));

		logger::info("Hook    :    Hook \"Draw\" was installed.");
	}
};

namespace Hooks::NextAttack
{
	std::uint8_t func(RE::Actor* a_actor)
	{
		bool bIsBashing = false;
		a_actor->GetGraphVariableBool("IsBashing", bIsBashing);

		if (bIsBashing) {
			return 6u;
		}

		return 4u;
	}

	void Install()
	{
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(41731, 42812), OFFSET(0x6, 0x7) };

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_func, std::uintptr_t a_addr)
			{
				Xbyak::Label funcLabel;
				Xbyak::Label retnLabel;

				push(r11);
				push(r10);
				push(r9);
				push(r8);
				push(rdx);

				mov(rcx, rdx);
				sub(rsp, 0x20);
				call(ptr[rip + funcLabel]);
				add(rsp, 0x20);
				mov(ecx, eax);

				pop(rdx);
				pop(r8);
				pop(r9);
				pop(r10);
				pop(r11);

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
		SKSE::AllocTrampoline(200);

		trampoline.write_branch<5>(target.address(), trampoline.allocate(patch));

		logger::info("Hook    :    Hook \"NextAttack\" was installed.");
	}
}

namespace Hooks::Reset
{
	void func(RE::Actor* a_actor)
	{
		bool bIsBashing = false;
		a_actor->GetGraphVariableBool("IsBashing", bIsBashing);

		if (bIsBashing) {
			a_actor->actorState1.meleeAttackState = RE::ATTACK_STATE_ENUM::kBash;
			logger::info("Hook    :    Currently resetting attack flags for {}", a_actor->GetName());
			std::jthread([=]() {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				while (a_actor->GetAttackState() == RE::ATTACK_STATE_ENUM::kBash) {
					bool bIsBashing = false;
					a_actor->GetGraphVariableBool("IsBashing", bIsBashing);
					bool bIsAttacking = false;
					a_actor->GetGraphVariableBool("IsAttacking", bIsAttacking);
					if (!bIsBashing && !bIsAttacking || a_actor->GetLifeState() == RE::ACTOR_LIFE_STATE::kBleedout) {
						SKSE::GetTaskInterface()->AddTask([=]() {
							a_actor->actorState1.meleeAttackState = RE::ATTACK_STATE_ENUM::kNone;
						});
					}
				}
			}).detach();
			logger::info("Hook    :    Done resetting attack flags for {}\n", a_actor->GetName());
		} else {
			a_actor->actorState1.meleeAttackState = RE::ATTACK_STATE_ENUM::kNone;
		}
	}

	void Install()
	{
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(41733, 42814), 0x1A };

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_func, std::uintptr_t a_addr)
			{
				Xbyak::Label funcLabel;
				Xbyak::Label retnLabel;

				push(r11);
				push(r10);
				push(r9);
				push(r8);
				push(rdx);
				push(rcx);
				push(rax);

				mov(rcx, rdx);
				sub(rsp, 0x28);
				call(ptr[rip + funcLabel]);
				add(rsp, 0x28);

				pop(rax);
				pop(rcx);
				pop(rdx);
				pop(r8);
				pop(r9);
				pop(r10);
				pop(r11);

				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(a_addr + 0xA);

				L(funcLabel);
				dq(a_func);
			}
		};

		Patch patch(reinterpret_cast<uintptr_t>(func), target.address());
		patch.ready();

		auto& trampoline = SKSE::GetTrampoline();
		SKSE::AllocTrampoline(200);

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
		NextAttack::Install();
		Reset::Install();
	}
}
