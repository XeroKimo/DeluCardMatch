module;

#include <vector>
#include <cmath>
#include <concepts>
#include <utility>
#include <iostream>
#include <span>

export module xk.Math.CatmullRomSpline;
import xk.Math.Matrix;

namespace xk::Math
{
	using namespace Aliases;
	export struct CatmullSplineTraits
	{
		static constexpr auto characteristicMatrix = Matrix<double, 4, 4>
		{
			0, 2, 0, 0,
			-1, 0, 1, 0,
			2, -5, 4, -1,
			-1, 3, -3, 1
		} / 2;

		template<class Ty>
		static constexpr Vector<Ty, 2> InterpolateCached(const Vector<Vector<Ty, 2>, 4>& points, float t)
		{
			return Matrix<double, 1, 4>{ 1, t, t* t, t* t* t } * points;
		}

		template<class Ty>
		static constexpr Vector<Ty, 2> InterpolateCached(const Vector<Ty, 2>& p0, const Vector<Ty, 2>& p1, const Vector<Ty, 2>& p2, const Vector<Ty, 2>& p3, float t)
		{
			return InterpolateCached(Vector<Vector<Ty, 2>, 4>{ p0, p1, p2, p3 }, t);
		}

		template<class Ty>
		static constexpr Vector<Ty, 2> Interpolate(const Vector<Vector<Ty, 2>, 4>& points, float t)
		{
			return InterpolateCached(Vector<Vector<Ty, 2>, 4>{characteristicMatrix* points}, t);
		}

		template<class Ty>
		static constexpr Vector<Ty, 2> Interpolate(const Vector<Ty, 2>& p0, const Vector<Ty, 2>& p1, const Vector<Ty, 2>& p2, const Vector<Ty, 2>& p3, float t)
		{
			return Interpolate({ p0, p1, p2, p3 }, t);
		}

		template<class Ty>
		static constexpr Vector<Ty, 2> Interpolate(Vector<Ty, 2> p0, Vector<Ty, 2> p1, float t)
		{
			return Interpolate(-p1, p0, p1, (p1 - p0) + p1, t);
		}
	};

	export template<class Ty>
	struct CatmullRomSegment
	{
		using vector_type = Vector<Ty, 2>;
		Vector<vector_type, 4> points;

		vector_type Interpolate(float t) const
		{
			return CatmullSplineTraits::Interpolate(points, t);
		}

		vector_type BeginPoint() const { return points[1]; }
		vector_type EndPoint() const { return points[2]; }

		Ty ApproximateLength() const noexcept
		{
			return Magnitude(points[1] - points[0]);
		}
	};

	using namespace Aliases;

	template<class Ty>
	Ty Lerp(const Ty& a, const Ty& b, float t)
	{
		return (1 - t) * a + t * b;
	}

	template<class Ty>
	float ReverseLerp(const Ty& a, const Ty& b, const Ty& v)
	{
		return static_cast<float>((v - a) / (b - a));
	}

	template<class Ty1, class Ty2>
	Ty2 Map(const Ty1& value, std::pair<Ty1, Ty1> inRange, std::pair<Ty2, Ty2> outRange)
	{
		return Lerp(outRange.first, outRange.second, ReverseLerp(inRange.first, inRange.second, value));
	}

	export template<class Ty>
	struct CatmullRomSpline
	{
		using vector_type = Vector<Ty, 2>;

		std::vector<vector_type> points;

		vector_type& operator[](size_t index) { return points[index]; }
		const vector_type& operator[](size_t index) const { return points[index]; }

		void AddPoint(vector_type point)
		{
			points.push_back(point);
		}

		void RemoveEndPoint()
		{
			points.pop_back();
		}

		void RemovePoint(size_t index)
		{
			points.erase(points.begin() + index);
		}

		void InsertPoint(vector_type point, size_t index)
		{
			points.insert(points.begin() + index, point);
		}

		vector_type Interpolate(float t) const
		{
			t *= SegmentCount();
			std::make_signed_t<size_t> wholeValue = static_cast<std::make_signed_t<size_t>>(std::floor(t));
			return InterpolateSegment(wholeValue, wholeValue < SegmentCount() ? t - wholeValue : 1.f);
		}

		vector_type InterpolateSegment(std::make_signed_t<size_t> segment, float t) const
		{
			return GetSegment(segment).Interpolate(t);
		}

		std::vector<double> GenerateDistanceTable(size_t samples)
		{
			std::vector<double> samplePoints;
			samplePoints.reserve(samples);
			samplePoints.push_back(0);
			for (size_t i = 1; i < samplePoints.capacity(); i++)
			{
				vector_type previousPoint = Interpolate(static_cast<float>(i - 1) / (samplePoints.capacity()));
				vector_type point = Interpolate(static_cast<float>(i) / (samplePoints.capacity()));
				samplePoints.push_back(samplePoints.back() + Magnitude(previousPoint - point));
			}

			return samplePoints;
		}

		vector_type InterpolateDistance(double distance, std::span<double> distanceTable)
		{
			for (int i = 0; i < distanceTable.size() - 1; i++)
			{
				if (distance >= distanceTable[i] && distance < distanceTable[i + 1])
				{
					float t = Map<double, float>(distance, { distanceTable[i], distanceTable[i + 1] }, { static_cast<float>(i) / (distanceTable.size() - 1), static_cast<float>(i + 1) / (distanceTable.size() - 1) });
					return Interpolate(t);
				}
			}

			return Interpolate(1.0);
		}

		vector_type InterpolateDistance(double distance)
		{
			size_t subdivisions = 4;
			std::vector<double> distanceTable = GenerateDistanceTable(PointCount() * subdivisions);
			return InterpolateDistance(distance, distanceTable);
		}
		
		vector_type& BeginPoint()
		{
			return points.front();
		}
		
		vector_type& EndPoint()
		{
			return points.back();
		}

		vector_type GhostBeginPoint() const
		{
			return -points[1];
		}

		vector_type GhostEndPoint() const 
		{
			const auto& p0 = *(points.end() - 2);
			const auto& p1 = *(points.end() - 1);
			return (p1 - p0) + p1;
		}

		CatmullRomSegment<Ty> GetSegment(std::make_signed_t<size_t> segment) const noexcept
		{
			if (SegmentCount() == 1)
			{
				return { { GhostBeginPoint(), points[0], points[1], GhostEndPoint() } };
			}
			if (segment >= SegmentCount())
			{
				return { { *(points.end() - 3), *(points.end() - 2), *(points.end() - 1), GhostEndPoint() } };
			}

			if (segment == 0)
			{
				return { { GhostBeginPoint(), points[0], points[1], points[2] } };
			}
			else if (segment == SegmentCount() - 1)
			{
				return { { points[segment - 1], points[segment], points[segment + 1], GhostEndPoint() } };
			}
			else
			{
				return { { points[segment - 1], points[segment], points[segment + 1], points[segment + 2] } };
			}
		}

		CatmullRomSegment<Ty> GetSegmentChecked(size_t segment) const
		{
			if (segment >= SegmentCount())
				throw std::out_of_range{"Index out of range\n"};

			return GetSegment(segment);
		}

		size_t PointCount() const noexcept { return points.size(); }
		std::make_signed_t<size_t> SegmentCount() const noexcept { return static_cast<std::make_signed_t<size_t>>(points.size()) - 1; }

		Ty ApproximateLength() const noexcept
		{
			Ty size = 0;
			for (size_t i = 0; i < SegmentCount(); i++)
			{
				size += GetSegment(i).ApproximateLength();
			}
			return size;
		}
	};

	export template<class Ty>
	Ty Lerp(const Ty& p1, const Ty& p2, double t)
	{
		return p1 * (1 - t) + p2 * t;
	}

	export template<class Ty>
	struct LinearSegment
	{
		using vector_type = Vector<Ty, 2>;
		vector_type begin;
		vector_type end;

		vector_type Interpolate(double t) const
		{
			return Lerp(begin, end, t);
		}

		vector_type BeginPoint() const { return begin; }
		vector_type EndPoint() const { return end; }

		vector_type Direction() const noexcept { return Normalize(end - begin); }

		Ty Length() const noexcept
		{
			return Magnitude(end - begin);
		}
	};

	export template<class Ty>
	struct LinearSpline
	{
		using vector_type = Vector<Ty, 2>;

		std::vector<vector_type> points;

		vector_type& operator[](size_t index) { return points[index]; }
		const vector_type& operator[](size_t index) const { return points[index]; }

		void AddPoint(vector_type point)
		{
			points.push_back(point);
		}

		void RemoveEndPoint()
		{
			points.pop_back();
		}

		void RemovePoint(size_t index)
		{
			points.erase(points.begin() + index);
		}

		void InsertPoint(vector_type point, size_t index)
		{
			points.insert(points.begin() + index, point);
		}

		vector_type& BeginPoint()
		{
			return points.front();
		}

		vector_type& EndPoint()
		{
			return points.back();
		}

		vector_type Interpolate(double t) const
		{
			t *= SegmentCount();
			return InterpolateSegment(t, t < SegmentCount() ? t - std::floor(t) : 1.0);
		}

		vector_type InterpolateSegment(size_t segment, double t) const
		{
			return GetSegment(segment).Interpolate(t);
		}

		vector_type InterpolateDistance(double distance) const
		{
			for (size_t i = 0; i < SegmentCount(); i++)
			{
				LinearSegment segment = GetSegment(i);
				if (distance - segment.Length() < 0)
				{
					return segment.BeginPoint() + segment.Direction() * distance;
				}
				distance -= segment.Length();
			}

			return points.back();
		}

		LinearSegment<Ty> GetSegment(size_t segment) const
		{
			return { points[segment], points[segment + 1] };
		}

		LinearSegment<Ty> GetSegmentChecked(size_t segment) const
		{
			if (segment >= SegmentCount())
				throw std::out_of_range{ "Index out of range\n" };

			return GetSegment(segment);
		}

		size_t PointCount() const noexcept { return points.size(); }
		std::make_signed_t<size_t> SegmentCount() const noexcept { return static_cast<std::make_signed_t<size_t>>(points.size()) - 1; }

		Ty Length() const noexcept 
		{
			Ty size = 0;
			for (size_t i = 0; i < SegmentCount(); i++)
			{
				size += GetSegment(i).Length();
			}
			return size;
		}
	};

	export template<class Ty>
	LinearSpline<Ty> ConvertSpline(CatmullRomSpline<Ty> spline, size_t subdivisionsPerSegment)
	{
		LinearSpline<Ty> linearSpline;
		size_t totalDivisions = subdivisionsPerSegment + 1;
		for (size_t i = 0; i < spline.SegmentCount(); i++)
		{
			CatmullRomSegment segment = spline.GetSegment(i);
			//Adds points from begin point and (end point - 1) of a segment.
			//End points of a segment is skipped since segment.EndPoint() == (segment + 1).BeginPoint();
			for (size_t division = 0; division < subdivisionsPerSegment; division++)
			{
				linearSpline.AddPoint(segment.Interpolate(static_cast<double>(division) / totalDivisions));
			}
		}
		
		linearSpline.AddPoint(spline.EndPoint());
		return linearSpline;
	}
}