#pragma once

#include <HMP/Meshing/types.hpp>

namespace HMP::Gui::Utils
{

	class Transform final
	{

	public:

		static bool isNull(Real _value, Real epsilon = 1e-6);
		static bool isNull(const Vec& _vec, Real epsilon = 1e-6);
		static Real wrapAngle(Real _angleDeg);
		static Vec wrapAngles(const Vec& _anglesDeg);
		static Vec rotationMatToVec(const Mat3& _mat);
		static Vec toDeg(const Vec& _rad);
		static Vec toRad(const Vec& _deg);
		static Mat3 rotationXMat(Real _angleDeg);
		static Mat3 rotationYMat(Real _angleDeg);
		static Mat3 rotationZMat(Real _angleDeg);
		static Mat3 rotationMat(const Vec& _axis, Real _angleDeg);
		static Mat3 rotationMat(const Vec& _eulerAnglesDeg);
		static Mat3 scaleMat(const Vec& _scale);
		static Mat4 translationMat(const Vec& _translation);
		static Mat4 homogeneous(const Mat3& _mat);
		static Vec2 dir(const Vec2& _from, const Vec2& _to, const Vec2& _else = { 1.0, 0.0 });
		static Real angle(const Vec2& _from, const Vec2& _to);

		Vec origin{};
		Vec translation{};
		Vec scale{ 1.0 };
		Vec rotation{};

		Mat4 matrix() const;

		Real avgScale() const;

		bool isIdentity(Real epsilon = 1e-6) const;

	};

}