#include "simsystem.hpp"
#include<spdlog/spdlog.h>
#include <chrono>

namespace game{
namespace systems {
	SimSystem::SimSystem(double _cellSize, double _dt)
		: m_integrator(_cellSize, _dt, std::thread::hardware_concurrency() / 2), m_totalTime(0), m_expectedTime(0)
	{}

	void SimSystem::update(float _dt, sim::SimpleCubicMesh& _mesh)
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


	}
}}