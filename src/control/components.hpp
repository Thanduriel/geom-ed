#pragma once

#include <glm/glm.hpp>

namespace game {
namespace components{

	struct Charge
	{
		Charge(float _mass, float _charge) : mass(_mass), q(_charge) {}
		float mass;
		float q; // charge of the particle
	};

	struct PreviousPosition
	{
		PreviousPosition(const glm::vec3& _pos) : value(_pos) {}
		glm::vec3 value;
	};
}}