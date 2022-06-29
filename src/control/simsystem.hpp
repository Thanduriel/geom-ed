#pragma once

#include "simulation/cubicmesh.hpp"
#include <engine/game/components/simpleComponents.hpp>
#include <engine/game/core/componentaccess.hpp>

namespace game {
namespace components{
	struct Charge
	{
		Charge(float _mass, float _charge) : mass(_mass), q(_charge) {}
		float mass;
		float q; // charge of the particle
	};
}

namespace systems {

	class SimSystem 
	{
	public:
		SimSystem(double _cellSize, double _dt = 0.0);

		using Components = ComponentTuple<
			WriteAccess<components::Position>
			, WriteAccess<components::Velocity>
			, ReadAccess<components::Charge>
			, WriteAccess<components::TransformNeedsUpdate>>;
		void update(Components _comps, float _dt, sim::SimpleCubicMesh& _mesh);
	private:
		sim::SimpleCubicIntegrator m_integrator;
		sim::SimpleCubicMesh::FloatT m_totalTime;
		float m_expectedTime;
	};}
}