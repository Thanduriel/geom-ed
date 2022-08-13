#pragma once

#include "components.hpp"
#include "simulation/cubicmesh.hpp"
#include <engine/game/core/componentaccess.hpp>
#include <engine/game/components/simpleComponents.hpp>
#include <engine/game/components/model.hpp>
#include <engine/graphics/renderer/mesh.hpp>
#include <engine/graphics/core/sampler.hpp>
#include <engine/graphics/core/texture.hpp>
#include <engine/graphics/renderer/linerenderer.hpp>

namespace game {

namespace components {
	struct Slice
	{
		Slice(int _layer) : layer(_layer), field(Field::E) {}

		enum struct Field {
			E,B
		};

		Field field;
		int layer;
		std::unique_ptr<graphics::Texture2D> texture;
	};
}
namespace systems {

	class RenderSystem
	{
	public:
		RenderSystem();

		using Components = ComponentTuple<
			WriteAccess<components::Position>
			, WriteAccess<components::Rotation>
			, WriteAccess<components::Velocity>
			, WriteAccess<components::Scale>
			, WriteAccess<components::Transform>
			, WriteAccess<components::Model>
			, WriteAccess<components::TransformNeedsUpdate>
			, ReadAccess<components::Slice>>;
		void update(Components _components, const sim::SimpleCubicMesh& _mesh);

	private:
		std::vector<float> m_textureBuffer;
		float m_maxLen;
		graphics::Mesh m_mesh;
		graphics::Sampler m_sampler;
		
	};

	class TailDrawing
	{
	public:
		using Components = ComponentTuple<
			ReadAccess<components::Position>
			, WriteAccess<components::PreviousPosition>>;
		void update(Components _components, const graphics::Camera& _camera);
	private:
		graphics::LineRenderer m_lineRenderer;
	};
}}