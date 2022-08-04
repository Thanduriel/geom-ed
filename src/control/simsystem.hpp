#pragma once

#include "components.hpp"
#include "simulation/cubicmesh.hpp"
#include <engine/game/components/simpleComponents.hpp>
#include <engine/game/core/componentaccess.hpp>

namespace game {
namespace systems {

	class SimSystem 
	{
	public:
		SimSystem(double _cellSize, double _dt = 0.0);

		using Components = ComponentTuple<
			WriteAccess<components::Position>
			, WriteAccess<components::Velocity>
			, ReadAccess<components::Charge>
			, ReadAccess<components::PreviousPosition>
			, WriteAccess<components::TransformNeedsUpdate>>;
		void update(Components _comps, float _dt, sim::SimpleCubicMesh& _mesh);
	private:
		sim::SimpleCubicIntegrator m_integrator;
		sim::SimpleCubicMesh::FloatT m_totalTime;
		float m_expectedTime;
	};
}}