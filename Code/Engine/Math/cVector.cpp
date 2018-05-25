// Header Files
//=============

#include "cVector.h"
#include "cQuaternion.h"
#include <cmath>
#include "../Asserts/Asserts.h"

// Static Data Initialization
//===========================

namespace
{
	const float s_epsilon = 1.0e-9f;
}

// Interface
//==========

// Addition
eae6320::Math::cVector eae6320::Math::cVector::operator +( const cVector& i_rhs ) const
{
	return cVector( x + i_rhs.x, y + i_rhs.y, z + i_rhs.z );
}
eae6320::Math::cVector& eae6320::Math::cVector::operator +=( const cVector& i_rhs )
{
	x += i_rhs.x;
	y += i_rhs.y;
	z += i_rhs.z;
	return *this;
}

// Subtraction / Negation
eae6320::Math::cVector eae6320::Math::cVector::operator -( const cVector& i_rhs ) const
{
	return cVector( x - i_rhs.x, y - i_rhs.y, z - i_rhs.z );
}
eae6320::Math::cVector& eae6320::Math::cVector::operator -=( const cVector& i_rhs )
{
	x -= i_rhs.x;
	y -= i_rhs.y;
	z -= i_rhs.z;
	return *this;
}
eae6320::Math::cVector eae6320::Math::cVector::operator -() const
{
	return cVector( -x, -y, -z );
}

// Multiplication
eae6320::Math::cVector eae6320::Math::cVector::operator *( const float i_rhs ) const
{
	return cVector( x * i_rhs, y * i_rhs, z * i_rhs );
}
eae6320::Math::cVector& eae6320::Math::cVector::operator *=( const float i_rhs )
{
	x *= i_rhs;
	y *= i_rhs;
	z *= i_rhs;
	return *this;
}
eae6320::Math::cVector operator *( const float i_lhs, const eae6320::Math::cVector& i_rhs )
{
	return i_rhs * i_lhs;
}

// Division
eae6320::Math::cVector eae6320::Math::cVector::operator /( const float i_rhs ) const
{
	EAE6320_ASSERTF( std::abs( i_rhs ) > s_epsilon, "Can't divide by zero" );
	const float rhs_reciprocal = 1.0f / i_rhs;
	return cVector( x * rhs_reciprocal, y * rhs_reciprocal, z * rhs_reciprocal );
}
eae6320::Math::cVector& eae6320::Math::cVector::operator /=( const float i_rhs )
{
	EAE6320_ASSERTF( std::abs( i_rhs ) > s_epsilon, "Can't divide by zero" );
	const float rhs_reciprocal = 1.0f / i_rhs;
	x *= rhs_reciprocal;
	y *= rhs_reciprocal;
	z *= rhs_reciprocal;
	return *this;
}

// Length / Normalization
float eae6320::Math::cVector::GetLength() const
{
	return std::sqrt( ( x * x ) + ( y * y ) + ( z * z ) );
}
float eae6320::Math::cVector::Normalize()
{
	const float length = GetLength();
	EAE6320_ASSERTF( length > s_epsilon, "Can't divide by zero" );
	operator /=( length );
	return length;
}
eae6320::Math::cVector eae6320::Math::cVector::CreateNormalized() const
{
	const float length = GetLength();
	EAE6320_ASSERTF( length > s_epsilon, "Can't divide by zero" );
	const float length_reciprocal = 1.0f / length;
	return cVector( x * length_reciprocal, y * length_reciprocal, z * length_reciprocal );
}

// Products
float eae6320::Math::Dot( const cVector& i_lhs, const cVector& i_rhs )
{
	return ( i_lhs.x * i_rhs.x ) + ( i_lhs.y * i_rhs.y ) + ( i_lhs.z * i_rhs.z );
}
eae6320::Math::cVector eae6320::Math::Cross( const cVector& i_lhs, const cVector& i_rhs )
{
	return cVector(
		( i_lhs.y * i_rhs.z ) - ( i_lhs.z * i_rhs.y ),
		( i_lhs.z * i_rhs.x ) - ( i_lhs.x * i_rhs.z ),
		( i_lhs.x * i_rhs.y ) - ( i_lhs.y * i_rhs.x ) );
}

// Comparison
bool eae6320::Math::cVector::operator ==( const cVector& i_rhs ) const
{
	// Use & rather than && to prevent branches (all three comparisons will be evaluated)
	return ( x == i_rhs.x ) & ( y == i_rhs.y ) & ( z == i_rhs.z );
}
bool eae6320::Math::cVector::operator !=( const cVector& i_rhs ) const
{
	// Use | rather than || to prevent branches (all three comparisons will be evaluated)
	return ( x != i_rhs.x ) | ( y != i_rhs.y ) | ( z != i_rhs.z );
}

// Initialization / Shut Down
//---------------------------

eae6320::Math::cVector::cVector( const float i_x, const float i_y, const float i_z )
	:
	x( i_x ), y( i_y ), z( i_z )
{

}

eae6320::Math::cVector eae6320::Math::QuatVector(const cQuaternion& quat, const cVector& vec)
{
	float num = quat.m_x * 2.0f;
	float num2 = quat.m_y * 2.0f;
	float num3 = quat.m_z * 2.0f;
	float num4 = quat.m_x * num;
	float num5 = quat.m_y * num2;
	float num6 = quat.m_z * num3;
	float num7 = quat.m_x * num2;
	float num8 = quat.m_x * num3;
	float num9 = quat.m_y * num3;
	float num10 = quat.m_w * num;
	float num11 = quat.m_w * num2;
	float num12 = quat.m_w * num3;
	cVector result;
	result.x = (1.0f - (num5 + num6)) * vec.x + (num7 - num12) * vec.y + (num8 + num11) * vec.z;
	result.y = (num7 + num12) * vec.x + (1.0f - (num4 + num6)) * vec.y + (num9 - num10) * vec.z;
	result.z = (num8 - num11) * vec.x + (num9 + num10) * vec.y + (1.0f - (num4 + num5)) * vec.z;
	return result;
}

float eae6320::Math::DistanceSq(const cVector& i_lhs, const cVector& i_rhs) {
	const float x = i_lhs.x - i_rhs.x;
	const float y = i_lhs.y - i_rhs.y;
	const float z = i_lhs.z - i_rhs.z;
	return x*x + y*y + z*z;
}