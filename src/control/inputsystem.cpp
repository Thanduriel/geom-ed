#include "inputsystem.hpp"

namespace game {
namespace systems {

	void InputSystem::update(Components _comps, game::EntityCreator& _creator)
	{
		if (!_creator.get(m_activeSlice))
		{
			Entity ent = _creator.create();
			m_activeSlice = _creator.ref(ent);
			getComp<components::Slice>(_comps).insert(ent, 16);
		}
	}
}}