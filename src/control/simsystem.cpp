#include "simsystem.hpp"
#include<engine/math/geometrictypes.hpp>
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

			using Index = sim::SimpleCubicMesh::Index;
			const auto size = _mesh.size();
			double pTotal = 0.0;
			double divBTotal = 0.0;
			for (Index iz = 0; iz < size.z; ++iz)
				for (Index iy = 0; iy < size.y; ++iy)
					for (Index ix = 0; ix < size.x; ++ix)
					{
						pTotal += std::abs(_mesh.p(ix, iy, iz));
						divBTotal += std::abs(_mesh.divB(ix, iy, iz));
					}

			spdlog::info("compute time: {:.4f}, divB: {:.6e}, divE: {:.6e}"
				, std::chrono::duration<float>(end - start).count()
				, divBTotal, pTotal);
			m_totalTime += m_integrator.dt();
		}

		_comps.execute([&, _dt](components::Charge& charge
			, components::Velocity& velocity
			, const components::Position& pos)
			{
				const glm::vec3 p = pos.value;
				const auto Etemp = _mesh.smoothE(p.x, p.y, p.z);
				const glm::vec3 E(Etemp.x, Etemp.y, Etemp.z);
				const auto Btemp = _mesh.smoothB(p.x, p.y, p.z);
				const glm::vec3 B(Btemp.x, Btemp.y, Btemp.z);

				const glm::vec3 F = charge.q * E + glm::cross(velocity.value * charge.q, B);
				velocity.value += F * _dt / charge.mass;
			});

		const auto s = _mesh.size();
		const math::Box meshBound(glm::vec3(0.f), glm::vec3(s.x, s.y, s.z));

		_comps.execute([_dt,&_mesh, &meshBound](components::PreviousPosition& prevPos, const components::Velocity& velocity, components::Position& pos)
			{
				const glm::vec3 p = pos.value + velocity.value * _dt;
				if (!meshBound.isIn(p)) 
				{
					// correction at boundary
					const auto pValid = _mesh.validPosition(p.x, p.y, p.z);
					pos.value = { pValid.x, pValid.y, pValid.z };
					// previous position should not include the jump
					prevPos.value = pos.value;
				}
				else
				{
					// regular previous pos update happens in TailDrawing
					pos.value = p;
				}
			});

		_comps.execute([&](Entity ent, const components::Velocity&)
			{
				getComp<components::TransformNeedsUpdate>(_comps).insert(ent);
			});
	}
}}