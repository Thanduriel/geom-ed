#include "simsystem.hpp"
#include<spdlog/spdlog.h>
#include <chrono>

namespace game{
namespace systems {
	SimSystem::SimSystem(double _cellSize, double _dt)
		: m_integrator(_cellSize, _dt, std::thread::hardware_concurrency() / 2), m_totalTime(0), m_expectedTime(0)
	{}

	void SimSystem::update(Components _comps, float _dt, sim::SimpleCubicMesh& _mesh)
	{
		m_expectedTime += _dt;
	//	while (static_cast<float>(m_totalTime) < m_expectedTime)
		{
			const auto start = std::chrono::high_resolution_clock::now();
			m_integrator.step(_mesh);
			const auto end = std::chrono::high_resolution_clock::now();
			spdlog::info("compute time: {}", std::chrono::duration<float>(end - start).count());
			m_totalTime += m_integrator.dt();
		}

		_comps.execute([&, _dt](components::Charge& charge
			, components::Velocity& velocity
			, const components::Position& pos)
			{
				const glm::dvec3 p = pos.value;
				const auto Etemp = _mesh.smoothE(p.x, p.y, p.z);
				const glm::vec3 E(Etemp.x, Etemp.y, Etemp.z);
				const auto Btemp = _mesh.smoothB(p.x, p.y, p.z);
				const glm::vec3 B(Btemp.x, Btemp.y, Btemp.z);

				const glm::vec3 F = charge.q * E + glm::cross(velocity.value * charge.q, B);
				velocity.value += F * _dt / charge.mass;
			});

		_comps.execute([_dt](const components::Velocity& velocity, components::Position& pos) 
			{
				pos.value += velocity.value * _dt;
			});

		_comps.execute([&](Entity ent, const components::Velocity&)
			{
				getComp<components::TransformNeedsUpdate>(_comps).insert(ent);
			});
	}
}}