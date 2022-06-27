#include "simsystem.hpp"

namespace systems {
	SimSystem::SimSystem(double _cellSize, double _dt)
		: m_integrator(_cellSize, _dt), m_totalTime(0), m_expectedTime(0)
	{}

	void SimSystem::update(float _dt, sim::SimpleCubicMesh& _mesh)
	{
		m_expectedTime += _dt;
		while (static_cast<float>(m_totalTime) < m_expectedTime)
		{
			m_integrator.step(_mesh);
			m_totalTime += m_integrator.dt();
		}
	}
}