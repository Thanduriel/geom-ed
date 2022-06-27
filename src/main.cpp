#include "simulation/cubicmesh.hpp"
#include "control/simstate.hpp"
#include <engine/game/core/game.hpp>
#include <iostream>

constexpr double c = 1.0;

int main()
{
	game::Game game;
	game.run(std::make_unique<control::SimState>());

/*	using namespace sim;

	using FloatT = double;
	using Mesh = CubicMesh<FloatT>;
	using Index = Mesh::Index;

	constexpr FloatT ds = 1.0;
	constexpr FloatT dt = 0.57;

	constexpr int64_t n = 32;
	const Mesh::SizeVec size{ n,n,n };
	Mesh mesh(size, Mesh::Vec{ ds,ds,ds });

	using Integrator = CubicIntegrator<double>;
	Integrator integrator(mesh, dt);
	for (int i = 0; i < 4096; ++i) 
	{
		double pTotal = 0.0;
		double divBTotal = 0.0;
		for (Index iz = 0; iz < size.z; ++iz)
			for (Index iy = 0; iy < size.y; ++iy)
				for (Index ix = 0; ix < size.x; ++ix)
				{
					pTotal += std::abs(mesh.p(ix, iy, iz));
					divBTotal += std::abs(mesh.divB(ix, iy, iz));
				}
		std::cout << divBTotal << " | " << pTotal << "\n";

	/*	for (Index iy = 0; iy < size.y; ++iy)
		{
			for (Index ix = 0; ix < size.x; ++ix)
				std::cout << mesh.B(ix, 16, iy).x << " ";
			std::cout << "\n";
		}
	//	std::cout << mesh.E(16, 16, 16).x << "\n";
		integrator.step(mesh);
	}*/
}