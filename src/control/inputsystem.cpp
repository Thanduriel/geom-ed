#include "inputsystem.hpp"
#include <engine/input/keyboardInterface.hpp>
#include <engine/utils/config.hpp>
#include <engine/graphics/core/device.hpp>
#include <GLFW/glfw3.h>

namespace game {
namespace systems {

	using namespace input;

	enum struct Actions {
/*		MOVE_LEFT,
		MOVE_RIGHT,
		MOVE_UP,
		MOVE_DOWN,
		MOVE_FORWARD,
		MOVE_BACKWARD*/
		SHOW_E,
		SHOW_B,
		LAYER_UP,
		LAYER_DOWN,
		CAMERA_FORWARD,
		CAMERA_BACKWARD
	};

	InputSystem::InputSystem()
		: m_inputs(new KeyboardInterface(utils::Config::get()["inputs"]["keyboard"],
			{ {"showE", Key::E},
			{"showB", Key::B},
			{"layerUp", Key::UP},
			{"layerDown", Key::DOWN},
			{"cameraForward", input::MouseButton::LEFT},
			{"cameraBackward", Key::DOWN} },
			{})),
		m_cameraDistance(60.f),
		m_rotationPhi(0.f),
		m_rotationTheta(0.f),
		m_cursorPos(0.f)
	{
	}


	void InputSystem::update(Components _comps, game::EntityCreator& _creator, graphics::Camera& _camera)
	{
		Entity ent = _creator.get(m_activeSlice);
		if (!ent)
		{
			ent = _creator.create();
			m_activeSlice = _creator.ref(ent);
			getComp<components::Slice>(_comps).insert(ent, 16);
		}

		components::Slice& slice = getComp<components::Slice>(_comps).getUnsafe(ent);
		if (m_inputs->isKeyPressed(Actions::SHOW_E))
			slice.field = components::Slice::Field::E;
		if (m_inputs->isKeyPressed(Actions::SHOW_B))
			slice.field = components::Slice::Field::B;

		if (m_inputs->isKeyPressed(Actions::LAYER_UP))
			++slice.layer;
		if (m_inputs->isKeyPressed(Actions::LAYER_DOWN))
			--slice.layer;

		if (const float scroll = InputManager::getScroll().y)
			m_cameraDistance -= scroll;

		if (InputManager::isButtonPressed(MouseButton::LEFT))
		{
			constexpr float rotSpeed = 0.02f;
			const glm::vec2 cursorDiff = (m_cursorPos - InputManager::getCursorPos()) * rotSpeed;
			m_rotationPhi -= cursorDiff.x;
			m_rotationTheta = std::clamp(m_rotationTheta - cursorDiff.y, 0.f, 3.1415f);
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

		_camera.setView(glm::lookAt(m_cameraDistance * cameraPos, glm::vec3(0.f, 0.f, 0.f), up));
	}
}}