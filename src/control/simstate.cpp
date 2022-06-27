#include "simstate.hpp"
#include "simsystem.hpp"
#include "inputsystem.hpp"
#include "rendersystem.hpp"
#include <engine/game/systems/transforms.hpp>
#include <engine/game/systems/meshdrawnig.hpp>

namespace control {

	SimState::SimState()
		: m_world(graphics::Camera(45.f, 0.01f, 200.f))
	{
		using FloatT = double;
		using Mesh = sim::CubicMesh<FloatT>;
		using Index = Mesh::Index;

		constexpr int64_t n = 32;
		constexpr FloatT ds = 1.0;
		constexpr FloatT dt = 1.0 / 60.0;

		m_world.getResource<graphics::Camera>().setView(
			glm::lookAt(glm::vec3(0.f, 0.f, 60.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f)));


		m_world.addResource<Mesh>(Mesh::SizeVec{ n,n,n }, Mesh::Vec{ ds,ds,ds });
		m_world.addSystem(std::make_unique<systems::SimSystem>(ds, dt), game::SystemGroup::Process);
		m_world.addSystem(std::make_unique<game::systems::InputSystem>(), game::SystemGroup::Process);
		m_world.addSystem(std::make_unique<game::systems::Transforms>(), game::SystemGroup::Process);

		auto renderSystem = m_world.addSystem(std::make_unique<game::systems::RenderSystem>(), game::SystemGroup::Draw);
		m_world.addSystem(std::make_unique<game::systems::MeshDrawing>(), game::SystemGroup::Draw, renderSystem);
	}

	void SimState::process(float _deltaTime)
	{
		m_world.process(game::SystemGroup::Process, _deltaTime);

		if (input::InputManager::isKeyPressed(input::Key::ESCAPE))
			finish();
	}

	void SimState::draw(float _deltaTime)
	{
		m_world.process(game::SystemGroup::Draw, _deltaTime);
	}
}