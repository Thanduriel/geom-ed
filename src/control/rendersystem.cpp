#include "rendersystem.hpp"
#include <engine/graphics/core/texture.hpp>

namespace game{
namespace systems{

	using FaceData = utils::MeshData::FaceData;

	// simple quad spanning the xy plane
	static const utils::MeshData meshData{
		.positions = {glm::vec3(0.f,0.f,0.f), glm::vec3(1.f,0.f,0.f), glm::vec3(0.f,1.f,0.f), glm::vec3(1.f,1.f,0.f)},
		.textureCoordinates = {glm::vec2(0.f,0.f), glm::vec2(0.f, 1.f), glm::vec2(1.f,0.f), glm::vec2(1.f,1.f)},
		.normals = { glm::vec3(0.f,0.f,1.f) },
		.faces = {FaceData{FaceData::VertexIndices{0,0,0}, FaceData::VertexIndices{1,1,0}, FaceData::VertexIndices{2,2,0}},
				FaceData{FaceData::VertexIndices{2,2,0}, FaceData::VertexIndices{1,1,0}, FaceData::VertexIndices{3,3,0}}}
	};

	static constexpr std::array<glm::vec3, 11> COLOR_MAP = {
		glm::vec3(54 / 255.f, 75 / 255.f, 154 / 255.f)
		, glm::vec3(74 / 255.f, 123 / 255.f, 183 / 255.f)
		, glm::vec3(110 / 255.f, 166 / 255.f, 205 / 255.f)
		, glm::vec3(152 / 255.f, 202 / 255.f, 225 / 255.f)
		, glm::vec3(194 / 255.f, 228 / 255.f, 239 / 255.f)
		, glm::vec3(234 / 255.f, 236 / 255.f, 204 / 255.f)
		, glm::vec3(254 / 255.f, 218 / 255.f, 139 / 255.f)
		, glm::vec3(253 / 255.f, 179 / 255.f, 102 / 255.f)
		, glm::vec3(246 / 255.f, 126 / 255.f, 75 / 255.f)
		, glm::vec3(221 / 255.f, 61 / 255.f, 45 / 255.f)
		, glm::vec3(165 / 255.f, 0 / 255.f, 38 / 255.f) };

	// maps s in [0,1] to a color
	static glm::vec3 sampleColorMap(float s)
	{
		if (s <= 0.f) return COLOR_MAP.front();

		const float f = s * COLOR_MAP.size();
		const size_t idx = static_cast<int64_t>(f);
		const float r = f - static_cast<float>(idx);
		if (idx >= COLOR_MAP.size() - 1)
			return COLOR_MAP.back();

		return (1.f - r) * COLOR_MAP[idx] + r * COLOR_MAP[idx+1];
	}

	RenderSystem::RenderSystem()
		: m_maxLen(1.f),
		m_sampler(graphics::Sampler::Filter::LINEAR,
			graphics::Sampler::Filter::LINEAR, 
			graphics::Sampler::Filter::LINEAR, 
			graphics::Sampler::Border::REPEAT),
		m_mesh(meshData)
	{
	}

	void RenderSystem::update(Components _comps, const sim::CubicMesh<double>& _mesh)
	{
		using namespace components;
		// init new slices
		_comps.execute([&](Entity ent, Slice& slice)
			{
				if (slice.texture)
					return;

				glm::quat rot = glm::identity<glm::quat>();//glm::quatLookAt(glm::vec3(0.f, -1.f, 0.f), glm::vec3(1.f, 0.f, 0.f));
				getComp<Rotation>(_comps).insert(ent, rot);
				getComp<Position>(_comps).insert(ent, glm::vec3(-_mesh.size().x / 2.f, -_mesh.size().y / 2.f, 0.f));
				getComp<Scale>(_comps).insert(ent, glm::vec3(_mesh.size().x, _mesh.size().y, 1.f));
				getComp<Transform>(_comps).insert(ent);
				slice.texture.reset(graphics::Texture2D::create(
					static_cast<int>(_mesh.size().x), static_cast<int>(_mesh.size().y), graphics::TexFormat::RGBA32F, m_sampler));
				getComp<Model>(_comps).insert(ent, m_mesh, *slice.texture);
				getComp<TransformNeedsUpdate>(_comps).insert(ent);
			});

		using Index = sim::SimpleCubicMesh::Index;
		const auto& size = _mesh.size();

		m_textureBuffer.resize(_mesh.size().x * _mesh.size().y * 4);
		std::vector<float> magnitude;
		_comps.execute([&](Entity ent, const Slice& slice, Position& pos)
			{
				float maxLen = 0.f;
				pos.value.z = static_cast<float>(slice.layer);
				for (Index iy = 0; iy < size.y; ++iy)
				{
					const Index yFlat = iy * size.x * 4;
					for (Index ix = 0; ix < size.x; ++ix)
					{
						const Index xFlat = ix * 4;
						const auto& field = _mesh.E(ix, iy, slice.layer);
						const float len = static_cast<float>(std::sqrt(field.x * field.x + field.y * field.y + field.z * field.z));
						maxLen = std::max(len, maxLen);
						const float s = static_cast<float>(len) / m_maxLen;
						// quadratic -1.f * s * s * s * s + 2*s
						const float s1 = 1.f - s;
						const float sq = s1 * s1;
						const glm::vec3 color = sampleColorMap(1.f - sq * sq * sq);

						m_textureBuffer[xFlat + yFlat] = color.r;
						m_textureBuffer[xFlat + yFlat + 1] = color.g;
						m_textureBuffer[xFlat + yFlat + 2] = color.b;
						m_textureBuffer[xFlat + yFlat + 3] = 1.f;
					}
				}
				m_maxLen = maxLen;
				std::cout << m_maxLen << "\n";
				slice.texture->fillMipMap(0, reinterpret_cast<uint8_t*>(m_textureBuffer.data()), false, graphics::PixelDataType::FLOAT);
			});
	}

}}