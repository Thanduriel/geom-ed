#pragma once

#include "simulation/cubicmesh.hpp"
#include <engine/game/components/simpleComponents.hpp>
#include <engine/game/core/componentaccess.hpp>

namespace game {
namespace systems {

	struct Charge 
	{
		Charge(float _mass, float _charge) : mass(_mass), charge(_charge) {}
		float mass;
		float charge;
	};

	class SimSystem 
	{
	public:
		SimSystem(double _cellSize, double _dt = 0.0);

		using Components = ComponentTuple<
			WriteAccess<components::Position>
			, WriteAccess<components::Velocity>
			, WriteAccess<components::TransformNeedsUpdate>>;
		void update(float _dt, sim::SimpleCubicMesh& _mesh);
	private:
		sim::SimpleCubicIntegrator m_integrator;
		sim::SimpleCubicMesh::FloatT m_totalTime;
		float m_expectedTime;
	};}
}