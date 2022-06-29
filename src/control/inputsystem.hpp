#pragma once

#include "rendersystem.hpp"
#include <engine/game/core/componentaccess.hpp>
#include <engine/game/core/lifetimeManager2.hpp>
#include <engine/input/inputmanager.hpp>
#include <engine/graphics/camera.hpp>

namespace game{
namespace systems {

	class InputSystem
	{
	public:
		InputSystem();
		using Components = ComponentTuple<
			ReadAccess<components::Slice>>;
		void update(Components _comps, EntityCreator& _creator, graphics::Camera& _camera);
	private:
		game::EntityRef m_activeSlice;
		std::unique_ptr<input::InputInterface> m_inputs;
		float m_cameraDistance;
		float m_rotationTheta;
		float m_rotationPhi;
		glm::vec2 m_cursorPos;
	};
}}