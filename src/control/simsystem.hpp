#pragma once

#include "simulation/cubicmesh.hpp"

namespace systems {

	class SimSystem 
	{
	public:
		SimSystem(double _cellSize, double _dt = 0.0);
		void update(float _dt, sim::SimpleCubicMesh& _mesh);
	private:
		sim::SimpleCubicIntegrator m_integrator;
		sim::SimpleCubicMesh::FloatT m_totalTime;
		float m_expectedTime;
	};
}