#pragma once

#include "rendersystem.hpp"
#include <engine/game/core/componentaccess.hpp>
#include <engine/game/core/lifetimeManager2.hpp>

namespace game{
namespace systems {

	class InputSystem
	{
	public:
		using Components = ComponentTuple<
			ReadAccess<components::Slice>>;
		void update(Components _comps, EntityCreator& _creator);
	private:
		game::EntityRef m_activeSlice;
	};
}}