#include <HMP/Gui/Utils/Transform.hpp>

#include <cmath>

namespace HMP::Gui::Utils
{

	bool Transform::isNull(Real _value, Real epsilon)
	{
		return _value >= -epsilon && _value <= epsilon;
	}

	bool Transform::isNull(const Vec& _vec, Real epsilon)
	{
		return isNull(_vec.x(), epsilon) &&
			isNull(_vec.y(), epsilon) &&
			isNull(_vec.z(), epsilon);
	}

	Real Transform::wrapAngle(Real _angleDeg)
	{
		if (_angleDeg < 0)
		{
			return 360.0f - static_cast<Real>(std::fmod(-_angleDeg, 360.0f));
		}
		else
		{
			return static_cast<Real>(std::fmod(_angleDeg, 360.0f));
		}
	}

	Vec Transform::wrapAngles(const Vec& _anglesDeg)
	{
		return {
			wrapAngle(_anglesDeg.x()),
			wrapAngle(_anglesDeg.y()),
			wrapAngle(_anglesDeg.z())
		};
	}

	Vec Transform::toDeg(const Vec& _rad)
	{
		return {
			cinolib::to_deg(_rad.x()),
			cinolib::to_deg(_rad.y()),
			cinolib::to_deg(_rad.z())
		};
	}

	Vec Transform::toRad(const Vec& _deg)
	{
		return {
			cinolib::to_rad(_deg.x()),
			cinolib::to_rad(_deg.y()),
			cinolib::to_rad(_deg.z())
		};
	}

	Vec Transform::rotationMatToVec(const Mat3& _mat)
	{
		Real sy = std::sqrt(_mat(0, 0) * _mat(0, 0) + _mat(1, 0) * _mat(1, 0));
		const bool singular{ isNull(sy) };
		Vec rad;
		if (!singular)
		{
			rad.x() = std::atan2(_mat(2, 1), _mat(2, 2));
			rad.y() = std::atan2(-_mat(2, 0), sy);
			rad.z() = std::atan2(_mat(1, 0), _mat(0, 0));
		}
		else
		{
			rad.x() = std::atan2(-_mat(1, 2), _mat(1, 1));
			rad.y() = std::atan2(-_mat(2, 0), sy);
			rad.z() = 0;
		}
		return toDeg(rad);
	}

	Mat3 Transform::rotationXMat(Real _angleDeg)
	{
		const Real r{ cinolib::to_rad(_angleDeg) };
		const Real c{ std::cos(r) };
		const Real s{ std::sin(r) };
		return Mat3{
			1,	0,	0,
			0,	c, -s,
			0,  s,	c
		};
	}

	Mat3 Transform::rotationYMat(Real _angleDeg)
	{
		const Real r{ cinolib::to_rad(_angleDeg) };
		const Real c{ std::cos(r) };
		const Real s{ std::sin(r) };
		return Mat3{
			c,	0, 	s,
			0,	1,	0,
		   -s,	0,	c
		};
	}

	Mat3 Transform::rotationZMat(Real _angleDeg)
	{
		const Real r{ cinolib::to_rad(_angleDeg) };
		const Real c{ std::cos(r) };
		const Real s{ std::sin(r) };
		return Mat3{
			c, -s,	0,
			s,	c,	0,
			0,	0,	1
		};
	}

	Mat3 Transform::rotationMat(const Vec& _axis, Real _angleDeg)
	{
		return Mat3::ROT_3D(_axis, cinolib::to_rad(_angleDeg));
	}

	Mat3 Transform::rotationMat(const Vec& _eulerAnglesDeg)
	{
		return
			rotationXMat(_eulerAnglesDeg.x()) *
			rotationYMat(_eulerAnglesDeg.y()) *
			rotationZMat(_eulerAnglesDeg.z());
	}

	Mat3 Transform::scaleMat(const Vec& _scale)
	{
		return Mat3::DIAG(_scale);
	}

	Mat4 Transform::translationMat(const Vec& _translation)
	{
		return Mat4::TRANS(_translation);
	}

	Mat4 Transform::homogeneous(const Mat3& _mat)
	{
		return Mat4{
			_mat(0,0),	_mat(0,1), 	_mat(0,2), 	0,
			_mat(1,0), 	_mat(1,1), 	_mat(1,2), 	0,
			_mat(2,0), 	_mat(2,1), 	_mat(2,2), 	0,
			0,			0,			0,			1
		};
	}

	Vec2 Transform::dir(const Vec2& _from, const Vec2& _to, const Vec2& _else)
	{
		Vec2 diff{ _to - _from };
		return diff.is_null() ? _else : diff.normalized();
	}

	Real Transform::angle(const Vec2& _from, const Vec2& _to)
	{
		return wrapAngle(cinolib::to_deg(std::atan2(_from.y(), _from.x()) - std::atan2(_to.y(), _to.x())));
	}

	Real Transform::avgScale() const
	{
		return (scale.x() + scale.y() + scale.z()) / 3;
	}

	Mat4 Transform::matrix() const
	{
		const Mat4 rotation{ homogeneous(rotationMat(this->rotation)) };
		const Mat4 scale{ homogeneous(scaleMat(this->scale)) };
		const Mat4 translation{ translationMat(this->translation + origin) };
		const Mat4 origin{ translationMat(-this->origin) };
		return translation * rotation * scale * origin;
	}

	bool Transform::isIdentity(Real epsilon) const
	{
		return
			isNull(translation, epsilon) &&
			isNull(scale - Vec{ 1 }, epsilon) &&
			isNull(wrapAngles(rotation + Vec{ 180 }) - Vec{ 180 }, epsilon);
	}

}