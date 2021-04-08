#pragma once

// STD
#include <algorithm>
#include <array>

// GLM
#include <glm/gtx/norm.hpp>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Noise/Noise.hpp>
#include <Engine/Noise/RangePermutation.hpp>
#include <Engine/Noise/PoissonDistribution.hpp>


namespace Engine {// TODO: move
	/**
	 * When inherited from, uses empty base optimization (ebo) to provide a member of type @p T.
	 * The dervied type may need to use #ENGINE_EMPTY_BASE for empty bases to be optimized.
	 * @see ENGINE_EMPTY_BASE
	 */
	template<class T>
	class ENGINE_EMPTY_BASE BaseMember {
		private:
			T value;

		public:
			BaseMember() {}

			template<class... Args>
			BaseMember(Args... args) : value(std::forward<Args>(args)...) {}

			[[nodiscard]] ENGINE_INLINE T& get() { return value; }
			[[nodiscard]] ENGINE_INLINE const T& get() const { return value; }
	};

	/** @see BaseMember */
	template<class T>
	requires std::is_empty_v<T>
	class ENGINE_EMPTY_BASE BaseMember<T> : public T {
		public:
			BaseMember() {}

			template<class... Args>
			BaseMember(Args... args) {}

			[[nodiscard]] ENGINE_INLINE T& get() { return *this; }
			[[nodiscard]] ENGINE_INLINE const T& get() const { return *this; }
	};
}

// TOOD: Look at "Implementation of Fast and Adaptive Procedural Cellular Noise" http://www.jcgt.org/published/0008/01/02/paper.pdf
namespace Engine::Noise {
	// TODO: move
	// TODO: name?
	template<int32 Value>
	class ConstantDistribution {
		public:
			constexpr int32 operator[](const int i) const {
				return Value;
			}
	};

	const static inline auto constant1 = ConstantDistribution<1>{};

	// TODO: Vectorify?
	// TODO: There seems to be some diag artifacts (s = 0.91) in the noise (existed pre RangePermutation)
	// TODO: For large step sizes (>10ish. very noticeable at 100) we can start to notice repetitions in the noise. I suspect this this correlates with the perm table size.
	// TODO: Do those artifacts show up with simplex as well? - They are. But only for whole numbers? If i do 500.02 instead of 500 they are almost imperceptible.
	// TODO: Version/setting for distance type (Euclidean, Manhattan, Chebyshev, Minkowski)
	// TODO: Multiple types. Some common: F1Squared, F1, F2, F2 - F1, F2 + F1
	template<class Perm, class Dist, class Metric>
	class ENGINE_EMPTY_BASE WorleyNoiseGeneric : protected BaseMember<Perm>, BaseMember<Dist>, BaseMember<Metric> {
		public:
			// For easy changing later
			// TODO: template params?
			using Float = float32;
			using Int = int32;
			using IVec = glm::vec<2, Int>;
			using FVec = glm::vec<2, Float>;

			class Result {
				public:
					/** The coordinate of the cell the point is in. */
					IVec cell;

					/** The number of the point in the cell. */
					Int n;

					// TODO: Update comment.
					/** The squared distance from the original input point. */
					Float value = std::numeric_limits<Float>::max();
			};

			template<class PermTuple, class DistTuple, class MetricTuple>
			WorleyNoiseGeneric(std::piecewise_construct_t, PermTuple&& permTuple, DistTuple&& distTuple, MetricTuple&& metricTuple)
				// This is fine, even for large objects, due to guaranteed copy elision. See C++ Standard [tuple.apply]
				: BaseMember<Perm>(std::make_from_tuple<BaseMember<Perm>>(std::forward<PermTuple>(permTuple)))
				, BaseMember<Dist>(std::make_from_tuple<BaseMember<Dist>>(std::forward<DistTuple>(distTuple)))
				, BaseMember<Metric>(std::make_from_tuple<BaseMember<Metric>>(std::forward<MetricTuple>(metricTuple))) {
			}
			
			// TODO: This code makes some assumptions about `Distribution`. We should probably note those or enforce those somewhere.
			WorleyNoiseGeneric(Perm perm, Dist dist, Metric metric)
				: BaseMember<Perm>(std::move(perm))
				, BaseMember<Dist>(std::move(dist))
				, BaseMember<Metric>(std::move(metric)) {
			}

			// TODO: doc
			// TODO: name? F1Squared would be more standard
			/**
			 * TODO: finish doc
			 * In this case Result::value is the squared distance to the nearest point.
			 */
			Result valueD2(Float x, Float y) const noexcept {
				Result result;

				evaluate({x, y}, [&](const FVec pos, const FVec point, const IVec cell, Int ci) ENGINE_INLINE {
					const auto m = metric()(pos, point);
					//const Float d2 = glm::length2(point - pos); // TODO: rm
					if (m < result.value) {
						result = {cell, ci, m};
					}
				});

				return result;
			}

			// TODO: doc
			// TODO: name
			/**
			 * TODO: finish doc
			 * TODO: typically F2-F1 is with the distances not the squared distances
			 * In this case Result::value is difference between the squared distance to the two nearest point.
			 */
			Result valueF2F1(Float x, Float y) const noexcept {
				Result result1;
				Result result2;

				evaluate({x, y}, [&](const FVec pos, const FVec point, const IVec cell, Int ci) ENGINE_INLINE {
					const auto m = metric()(pos, point);
					//const Float d2 = glm::length2(point - pos); // TODO: rm
					
					if (m < result1.value) {
						result2 = result1;
						result1 = {cell, ci, m};
					} else if (m < result2.value) {
						result2 = {cell, ci, m};
					}
				});

				result1.value = result2.value - result1.value;
				return result1;
			}

		protected:
			[[nodiscard]] ENGINE_INLINE decltype(auto) perm() { return BaseMember<Perm>::get(); }
			[[nodiscard]] ENGINE_INLINE decltype(auto) perm() const { return BaseMember<Perm>::get(); }

			[[nodiscard]] ENGINE_INLINE decltype(auto) dist() { return BaseMember<Dist>::get(); }
			[[nodiscard]] ENGINE_INLINE decltype(auto) dist() const { return BaseMember<Dist>::get(); }
			
			[[nodiscard]] ENGINE_INLINE decltype(auto) metric() { return BaseMember<Metric>::get(); }
			[[nodiscard]] ENGINE_INLINE decltype(auto) metric() const { return BaseMember<Metric>::get(); }

			// TODO: Doc
			template<class PointProcessor>
			void evaluate(const FVec pos, PointProcessor&& pp) const {
				// Figure out which base unit square we are in
				const IVec base = { floorTo<Int>(pos.x), floorTo<Int>(pos.y) };

				// TODO: check boundary cubes. Based on our closest point we can cull rows/cols
				for (IVec offset{-1}; offset.y < 2; ++offset.y) {
					for (offset.x = -1; offset.x < 2; ++offset.x) {
						// Position and points in this cell
						const IVec cell = base + offset;
						const int numPoints = dist()(perm()(cell.x, cell.y));

						// Find the best point in this cell
						for (int i = 0; i < numPoints; ++i) {
							const FVec poff = FVec{ perm()(cell.x, cell.y, +i), perm()(cell.x, cell.y, -i) } * (Float{1} / Float{255});
							const FVec point = FVec{cell} + poff;
							pp(pos, point, cell, i);
						}
					}
				}
			}
	};

	inline static const RangePermutation<256> TODO_rm_perm = 1234567890;
	inline static const auto TODO_rm_metric = [](auto&& pos, auto&& point) ENGINE_INLINE { return glm::length2(point - pos); };
	// TODO: Doc
	template<auto* Dist>
	class WorleyNoiseFrom : public WorleyNoiseGeneric<decltype(TODO_rm_perm), decltype(*Dist), decltype(TODO_rm_metric)> {
		public:
			WorleyNoiseFrom(int64 seed) : WorleyNoiseGeneric<decltype(TODO_rm_perm), decltype(*Dist), decltype(TODO_rm_metric)>{seed, *Dist, TODO_rm_metric} {
			}
	};

	// TODO: doc
	class WorleyNoise : public WorleyNoiseFrom<&poisson3> {
	};

}
