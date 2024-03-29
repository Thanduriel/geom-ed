#include "inputsystem.hpp"
#include <engine/input/keyboardInterface.hpp>
#include <engine/utils/config.hpp>
#include <engine/graphics/core/device.hpp>
#include <engine/utils/meshLoader.hpp>
#include <engine/graphics/resources.hpp>
#include <engine/math/random.hpp>
#include <GLFW/glfw3.h>
#include <numbers>


namespace game {
namespace systems {

	using namespace input;

	enum struct Actions {
		SHOW_E,
		SHOW_B,
		LAYER_UP,
		LAYER_DOWN,
		CAMERA_FORWARD,
		CAMERA_BACKWARD,
		CAMERA_ROTATE,
		SPAWN_PARTICLE,
		SPAWN_MOD
	};

	InputSystem::InputSystem()
		: m_inputs(new KeyboardInterface(utils::Config::get()["inputs"]["keyboard"],
			{ {"showE", Key::E},
			{"showB", Key::B},
			{"layerUp", Key::UP},
			{"layerDown", Key::DOWN},
			{"cameraForward", MouseButton::LEFT}, // not in use
			{"cameraBackward", Key::DOWN}, // not in use
			{"rotateCamera", MouseButton::LEFT},
			{"spawnParticle", MouseButton::RIGHT},
			{"spawnMod", Key::LEFT_SHIFT} },
			{})),
		m_cameraDistance(60.f),
		m_rotationPhi(0.f),
		m_rotationTheta(0.f),
		m_cursorPos(0.f),
		m_mesh(*utils::MeshLoader::get("models/sphere.obj")),
		m_sampler(graphics::Sampler::Filter::LINEAR,
			graphics::Sampler::Filter::LINEAR,
			graphics::Sampler::Filter::LINEAR,
			graphics::Sampler::Border::REPEAT)
	{
	}


	void InputSystem::update(Components _comps, game::EntityCreator& _creator
		, graphics::Camera& _camera
		, const sim::SimpleCubicMesh& _mesh)
	{
		static const graphics::Texture2D& texture = *graphics::Texture2DManager::get(
			"textures/planet1.png", m_sampler);

		Entity ent = _creator.get(m_activeSlice);
		if (!ent)
		{
			ent = _creator.create();
			m_activeSlice = _creator.ref(ent);
			getComp<components::Slice>(_comps).insert(ent, 16);
		}

		// switch displayed field
		components::Slice& slice = getComp<components::Slice>(_comps).getUnsafe(ent);
		if (m_inputs->isKeyPressed(Actions::SHOW_E))
			slice.field = components::Slice::Field::E;
		if (m_inputs->isKeyPressed(Actions::SHOW_B))
			slice.field = components::Slice::Field::B;

		// slice selection
		if (m_inputs->isKeyPressed(Actions::LAYER_UP))
		{
			++slice.layer;
			if (slice.layer >= _mesh.size().z) 
				slice.layer -= static_cast<int>(_mesh.size().z);
		}
		if (m_inputs->isKeyPressed(Actions::LAYER_DOWN))
		{
			--slice.layer;
			if (slice.layer < 0)
				slice.layer += static_cast<int>(_mesh.size().z);
		}

		// camera movement
		if (const float scroll = InputManager::getScroll().y)
			m_cameraDistance -= scroll;

		if (m_inputs->isKeyPressed(Actions::CAMERA_ROTATE))
		{
			constexpr float rotSpeed = 0.02f;
			const glm::vec2 cursorDiff = (m_cursorPos - InputManager::getCursorPos()) * rotSpeed;
			m_rotationPhi -= cursorDiff.x;
			m_rotationTheta = std::clamp(m_rotationTheta + cursorDiff.y, 0.f, std::numbers::pi_v<float>);
		}
		m_cursorPos = InputManager::getCursorPos();

		const float cosTheta = std::cos(m_rotationTheta);
		const float sinTheta = std::sin(m_rotationTheta);
		const float cosPhi = std::cos(m_rotationPhi);
		const float sinPhi = std::sin(m_rotationPhi);
		const glm::vec3 cameraPos = glm::vec3(cosPhi * sinTheta, sinPhi * sinTheta, cosTheta);

		const glm::vec3 right = cameraPos.z < 0.99f ? glm::cross(cameraPos, glm::vec3(0.f, 0.f, 1.f))
			: glm::cross(glm::vec3(cosPhi, sinPhi, 0.f), cameraPos);
		const glm::vec3 up = -glm::cross(cameraPos, right);

		const glm::vec3 meshCenter(_mesh.size().x / 2.f, _mesh.size().y / 2.f, _mesh.size().z / 2.f);
		const glm::vec3 cp = m_cameraDistance * cameraPos + meshCenter;

		_camera.setView(glm::lookAt(m_cameraDistance * cameraPos + meshCenter, meshCenter, up));

		// spawn particles
		if (m_inputs->getKeyState(Actions::SPAWN_PARTICLE) == ActionState::PRESSED
			|| (m_inputs->isKeyPressed(Actions::SPAWN_PARTICLE) && m_inputs->isKeyPressed(Actions::SPAWN_MOD)))
		{
			const auto s = _mesh.size();
			const math::Box meshBound(glm::vec3(0.f), glm::vec3(s.x, s.y, s.z));

			const math::Ray3D ray = _camera.getRay(m_cursorPos);
			const float t = (static_cast<float>(slice.layer /** _mesh.cellSize().z*/) - ray.origin.z) 
				/ ray.direction.z;
			const glm::vec3 pos = ray.origin + t * ray.direction;
			if (meshBound.isIn(pos))
			{
				CreateComponents(_comps, _creator.create())
					.add<components::Position>(pos)
					.add<components::Rotation>(math::random::rotation())
					.add<components::Velocity>(glm::vec3(0.f))
					.add<components::Transform>(glm::identity<glm::mat4>())
					.add<components::Model>(m_mesh, texture, glm::identity<glm::mat4>())
					.add<components::TransformNeedsUpdate>()
					.add<components::Charge>(0.1f, 10.f)
					.add<components::PreviousPosition>(pos);
			}
		}
	}
}}