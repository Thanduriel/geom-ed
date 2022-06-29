#pragma once

#include <memory>
#include <random>
#include <cassert>
#include <iostream>
#include <execution>
#include <ranges>
#include <algorithm>

namespace sim {

	template<typename It, typename Fn>
	void runMultiThreaded(It _begin, It _end, Fn _fn, int _numThreads = 1)
	{
		if (_numThreads == 1)
			_fn(_begin, _end);
		else
		{
			std::vector<std::thread> threads;
			threads.reserve(_numThreads - 1);

			using DistanceType = decltype(_end - _begin);
			const DistanceType n = static_cast<DistanceType>(_numThreads);
			const DistanceType rows = (_end - _begin) / n;
			for (DistanceType i = 0; i < n - 1; ++i)
				threads.emplace_back(_fn, i * rows, (i + 1) * rows);
			_fn((n - 1) * rows, _end);

			for (auto& thread : threads)
				thread.join();
		}
	}

	template<typename T>
	struct Vec3 
	{
		Vec3& operator=(const Vec3& _rhs) { x = _rhs.x; y = _rhs.y; z = _rhs.z; return *this; }

		T x, y, z;
	};

	// Cubic mesh with periodic boundary
	template<typename T>
	class CubicMesh
	{
	public:
		using FloatT = T;
		using Vec = Vec3<T>;
		using Index = int64_t;
		using FlatIndex = std::size_t;
		using SizeVec = Vec3<Index>;

		CubicMesh(const SizeVec& _gridSize, const Vec& _cellSize)
			: m_size{ _gridSize }
			, m_cellSize{_cellSize}
			, m_numElem(static_cast<FlatIndex>(_gridSize.x* _gridSize.y* _gridSize.z))
			, m_E(new Vec[m_numElem]{})
			, m_B(new Vec[m_numElem]{})
			, m_J(new Vec[m_numElem]{})
		{
			std::default_random_engine rng(0xA4FE1099);
			std::uniform_real_distribution<T> dist(0.0, 1.0);
			auto gen = [&]() { return Vec(dist(rng), dist(rng), dist(rng)); };

		//	std::generate_n(m_E.get(), m_numElem, gen);
		//	std::generate_n(m_B.get(), m_numElem, gen);
		//	std::generate_n(m_J.get(), m_numElem, gen);
		//	J(_gridSize.x / 2, _gridSize.y / 2, _gridSize.z / 2) = Vec{0.0,0.0,-10.0};
			E(_gridSize.x / 2, _gridSize.y / 2, _gridSize.z / 2) = Vec{ 1.0,0.0,0.0 };
			E(_gridSize.x / 4, _gridSize.y / 4, _gridSize.z / 2) = Vec{ -1.0,0.0,0.0 };
		//	E(_gridSize.x / 2+1, _gridSize.y / 2, _gridSize.z / 2) = Vec{ 0.0,2.0,1.0 };
		}

		const Vec& E(Index _x, Index _y, Index _z) const { return m_E[validFlatIndex({ _x,_y,_z })]; }
		Vec& E(Index _x, Index _y, Index _z) { return m_E[validFlatIndex({ _x,_y,_z })]; }
		const Vec& B(Index _x, Index _y, Index _z) const { return m_B[validFlatIndex({ _x,_y,_z })]; }
		Vec& B(Index _x, Index _y, Index _z) { return m_B[validFlatIndex({ _x,_y,_z })]; }
		// cells and dual cells have the same size and all edges are spacelike
		// *B == H
		const Vec& H(Index _x, Index _y, Index _z) { return m_B[validFlatIndex({ _x,_y,_z })]; }
		const Vec& J(Index _x, Index _y, Index _z) const { return m_J[validFlatIndex({ _x,_y,_z })]; }
		Vec& J(Index _x, Index _y, Index _z) { return m_J[validFlatIndex({ _x,_y,_z })]; }
		
		// raw access to the fields
		const Vec* E() const { return m_E.get(); }
		const Vec* B() const { return m_B.get(); }

		// Evaluate the charge density on the grid cell.
		// This should be conserved.
		T p(Index _x, Index _y, Index _z) const 
		{ 
			const Vec& E_ = E(_x, _y, _z);
			return (E_.x - E(_x - 1, _y, _z).x) / m_cellSize.x
				+ (E_.y - E(_x, _y - 1, _z).y) / m_cellSize.y
				+ (E_.z - E(_x, _y, _z - 1).z) / m_cellSize.z;
		}

		// Evaluate the divergence of B on a grid cell.
		// B should be divergence free.
		T divB(Index _x, Index _y, Index _z) const
		{
			const Vec& B_ = B(_x, _y, _z);
			return (B(_x + 1, _y, _z).x - B_.x) / m_cellSize.x
				+ (B(_x, _y+1, _z).y - B_.y) / m_cellSize.y
				+ (B(_x, _y, _z+1).z - B_.z) / m_cellSize.z;
		}

		SizeVec validIndex(const SizeVec& _ind) const
		{
			SizeVec v{ _ind.x % m_size.x, _ind.y % m_size.y, _ind.z % m_size.z };
			if (v.x < 0) v.x += m_size.x;
			if (v.y < 0) v.y += m_size.y;
			if (v.z < 0) v.z += m_size.z;
			return v;
		}

		FlatIndex flatIndex(const SizeVec& _ind) const
		{
			return static_cast<FlatIndex>(_ind.x + _ind.y * m_size.x + _ind.z * m_size.x * m_size.y);
		}

		FlatIndex validFlatIndex(const SizeVec& _ind) const
		{
			const FlatIndex flatIdx = flatIndex(validIndex(_ind));
			assert(flatIdx < m_numElem);
			return flatIdx;
		}

		const SizeVec size() const { return m_size; }
		const Vec& cellSize() const { return m_cellSize; }
		FlatIndex numElem() const { return m_numElem; }
	private:
		SizeVec m_size;
		Vec m_cellSize;
		FlatIndex m_numElem;
		std::unique_ptr<Vec[]> m_E;
		std::unique_ptr<Vec[]> m_B;
		std::unique_ptr<Vec[]> m_J;
	};

	template<typename T>
	class CubicIntegrator
	{
	public:
		CubicIntegrator(T _maxCellSize, T _dt = 0, int _numThreads = 1)
			: m_dt(_dt), m_numThreads(_numThreads)
		{
			constexpr T sqrt3 = 1.7320508075688772;

			//const Vec3<T>& cell = _mesh.cellSize();
			//const T maxCellSize = std::max(std::max(cell.x, cell.y), cell.z);
			const T cfl = _maxCellSize / sqrt3;
			if (m_dt == 0)
				m_dt = 0.99 * cfl;
			else if (m_dt >= _maxCellSize / sqrt3)
				std::cout << "[Warning] The step-size is to large, the simulation will be unstable.\n";
		}

		void step(CubicMesh<T>& _mesh)
		{
			CubicMesh<T>& mesh = _mesh;
			using Index = CubicMesh<T>::Index;
			const auto& size = mesh.size();
			const Vec3<T>& cellSize = mesh.cellSize();

			// dF = 0
			runMultiThreaded(int64_t(0), size.z, [&] (Index zStart, Index zEnd)
				{
					for (Index iz = zStart; iz < zEnd; ++iz)
						for (Index iy = 0; iy < size.y; ++iy)
							for (Index ix = 0; ix < size.x; ++ix)
							{
								Vec3<T>& B = mesh.B(ix, iy, iz);
								const Vec3<T>& E = mesh.E(ix, iy, iz);
								B.x += m_dt * ((mesh.E(ix, iy, iz + 1).y - E.y) / cellSize.z
									- (mesh.E(ix, iy + 1, iz).z - E.z) / cellSize.y);
								B.y += m_dt * ((mesh.E(ix + 1, iy, iz).z - E.z) / cellSize.x
									- (mesh.E(ix, iy, iz + 1).x - E.x) / cellSize.z);
								B.z += m_dt * ((mesh.E(ix, iy + 1, iz).x - E.x) / cellSize.y
									- (mesh.E(ix + 1, iy, iz).y - E.y) / cellSize.x);
							}
				}, m_numThreads);
			
			// d*F = J'
			runMultiThreaded(int64_t(0), size.z, [&](Index zStart, Index zEnd)
				{
					for (Index iz = zStart; iz < zEnd; ++iz)
						for (Index iy = 0; iy < size.y; ++iy)
							for (Index ix = 0; ix < size.x; ++ix)
							{
								constexpr T κE = 1.0;
								const Vec3<T>& H = mesh.H(ix, iy, iz);
								const Vec3<T>& J = mesh.J(ix, iy, iz);
								Vec3<T>& E = mesh.E(ix, iy, iz);
								E.x = κE * E.x + m_dt * ((H.z - mesh.H(ix, iy - 1, iz).z) / cellSize.y
									- (H.y - mesh.H(ix, iy, iz - 1).y) / cellSize.z
									- J.x);
								E.y = κE * E.y + m_dt * ((H.x - mesh.H(ix, iy, iz - 1).x) / cellSize.z
									- (H.z - mesh.H(ix - 1, iy, iz).z) / cellSize.x
									- J.y);
								E.z = κE * E.z + m_dt * ((H.y - mesh.H(ix - 1, iy, iz).y) / cellSize.x
									- (H.x - mesh.H(ix, iy - 1, iz).x) / cellSize.y
									- J.z);
							}
				}, m_numThreads);

		}

		T dt() const { return m_dt; }
	private:
		T m_dt;
		int m_numThreads;
	};

	using SimpleCubicMesh = CubicMesh<double>;
	using SimpleCubicIntegrator = CubicIntegrator<double>;
}