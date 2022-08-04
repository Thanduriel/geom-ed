#pragma once

#include "rendersystem.hpp"
#include "simsystem.hpp"
#include <engine/game/core/componentaccess.hpp>
#include <engine/game/core/lifetimeManager2.hpp>
#include <engine/input/inputmanager.hpp>
#include <engine/graphics/camera.hpp>
#include <engine/graphics/renderer/mesh.hpp>

namespace game{
namespace systems {

	class InputSystem
	{
	public:
		InputSystem();
		using Components = ComponentTuple<
			WriteAccess<components::Model>,
			WriteAccess<components::Velocity>,
			WriteAccess<components::Rotation>,
			WriteAccess<components::Position>,
			WriteAccess<components::Transform>,
			WriteAccess<components::TransformNeedsUpdate>,
			WriteAccess<components::Charge>,
			WriteAccess<components::PreviousPosition>,
			ReadAccess<components::Slice>>;
		void update(Components _comps, EntityCreator& _creator
			, graphics::Camera& _camera
			, const sim::CubicMesh<double>& _mesh);
	private:
		game::EntityRef m_activeSlice;
		std::unique_ptr<input::InputInterface> m_inputs;
		float m_cameraDistance;
		float m_rotationTheta;
		float m_rotationPhi;
		glm::vec2 m_cursorPos;
		graphics::Mesh m_mesh;
		graphics::Sampler m_sampler;
	};
}}