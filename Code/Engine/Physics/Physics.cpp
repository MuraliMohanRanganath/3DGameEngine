#include "Physics.h"
#include "../Graphics/VertexData.h"
#include "../Platform/Platform.h"
#include "TriangleData.h"
#include "../Time/Time.h"

namespace {
	uint32_t noOfTris = 0;
	eae6320::Physics::sTriangle* triangles = NULL;
}

void eae6320::Physics::CheckCollision(Graphics::GameObject* gameObject)
{
	Math::cVector gravity(0.0f, -100, 0.0f);
	Math::cVector p = gameObject->transform.getPosition();
	float deltaTime = Time::GetElapsedSecondCount_duringPreviousFrame();
	{
		Math::cVector q = (gameObject->transform.getPosition()) - Math::cVector(0, gameObject->rigidBody.height, 0);
		bool hasIntersected = false;
		for (size_t i = 0; i < noOfTris; ++i) {
			float u, v, w, t;
			if (IntersectSegmentTriangle(p, q, triangles[i].A, triangles[i].B, triangles[i].C, &u, &v, &w, &t)) {
				hasIntersected = true;
				const float tri_center_y = (triangles[i].A.y + triangles[i].B.y + triangles[i].C.y) / 3.0f;
				const float displacement_y = fabs(tri_center_y - q.y);
				Math::cVector position = gameObject->transform.getPosition();
				if (displacement_y > 1)
					gameObject->Move(Math::cVector(position.x, position.y + displacement_y*0.1f, position.z));
				else {
					gameObject->Move(Math::cVector(position.x, position.y + displacement_y, position.z));
				}
			}
		}
		//if (!hasIntersected) {
		//	gameObject->rigidBody.acceleration += gravity;
		//	gameObject->Move(gameObject->transform.getPosition() + ((gameObject->rigidBody.velocity * deltaTime) + (gameObject->rigidBody.acceleration * (0.5f * deltaTime * deltaTime))));
		//}
	}
	{
		Math::cVector q = (gameObject->transform.getPosition()) - Math::cVector(0, 0, gameObject->rigidBody.width);
		for (size_t i = 0; i < noOfTris; ++i) {
			float u, v, w, t;
			if (IntersectSegmentTriangle(p, q, triangles[i].A, triangles[i].B, triangles[i].C, &u, &v, &w, &t)) {
				const float tri_center_z = (triangles[i].A.z + triangles[i].B.z + triangles[i].C.z) / 3.0f;
				const float displacement_z = fabs(tri_center_z - q.z);
				Math::cVector position = gameObject->transform.getPosition();
				gameObject->Move(Math::cVector(position.x + displacement_z*0.1f, position.y, position.z + displacement_z));
			}
		}
	}
	{
		Math::cVector q = (gameObject->transform.getPosition()) + Math::cVector(0, 0, gameObject->rigidBody.width);

		for (size_t i = 0; i < noOfTris; ++i) {
			float u, v, w, t;
			if (IntersectSegmentTriangle(p, q, triangles[i].A, triangles[i].B, triangles[i].C, &u, &v, &w, &t)) {
				const float tri_center_z = (triangles[i].A.z + triangles[i].B.z + triangles[i].C.z) / 3.0f;
				const float displacement_z = fabs(tri_center_z - q.z);
				Math::cVector position = gameObject->transform.getPosition();
				gameObject->Move(Math::cVector(position.x - displacement_z*0.1f, position.y, position.z - displacement_z));
			}
		}
	}
	{
		Math::cVector q = (gameObject->transform.getPosition()) - Math::cVector(gameObject->rigidBody.length, 0, 0);
		for (size_t i = 0; i < noOfTris; ++i) {
			float u, v, w, t;
			if (IntersectSegmentTriangle(p, q, triangles[i].A, triangles[i].B, triangles[i].C, &u, &v, &w, &t)) {
				const float tri_center_x = (triangles[i].A.x + triangles[i].B.x + triangles[i].C.x) / 3.0f;
				const float displacement_x = fabs(tri_center_x - q.x);
				Math::cVector position = gameObject->transform.getPosition();
				gameObject->Move(Math::cVector(position.x + displacement_x, position.y, position.z - displacement_x*0.1f));
			}
		}
	}
	{
		Math::cVector q = (gameObject->transform.getPosition()) + Math::cVector(gameObject->rigidBody.length, 0, 0);
		for (size_t i = 0; i < noOfTris; ++i) {
			float u, v, w, t;
			if (IntersectSegmentTriangle(p, q, triangles[i].A, triangles[i].B, triangles[i].C, &u, &v, &w, &t)) {
				const float tri_center_x = (triangles[i].A.x + triangles[i].B.x + triangles[i].C.x) / 3.0f;
				const float displacement_x = fabs(tri_center_x - q.x);
				Math::cVector position = gameObject->transform.getPosition();
				gameObject->Move(Math::cVector(position.x - displacement_x, position.y, position.z + displacement_x*0.1f));
			}
		}
	}
}

int eae6320::Physics::IntersectSegmentTriangle(const Math::cVector& p, const Math::cVector& q, const Math::cVector& a, const Math::cVector& b, const Math::cVector& c, float *o_u, float *o_v, float *o_w, float *o_t)
{
	eae6320::Math::cVector ab = b - a;
	eae6320::Math::cVector ac = c - a;
	eae6320::Math::cVector qp = p - q;

	// Compute triangle normal. Can be precalculated or cached if
	// intersecting multiple segments against the same triangle
	eae6320::Math::cVector n = Cross(ab, ac);
	// Compute denominator d. If d <= 0, segment is parallel to or points
	// away from triangle, so exit early
	float d = Dot(qp, n);
	if (d <= 0.0f) return 0;

	// Compute intersection t value of pq with plane of triangle. A ray
	// intersects if 0 <= t. Segment intersects if 0 <= t <= 1. Delay
	// dividing by d until intersection has been found to pierce triangle
	eae6320::Math::cVector ap = p - a;
	*o_t = Dot(ap, n);
	if (*o_t < 0.0f) return 0;
	if (*o_t > d) return 0; // For segment; exclude this code line for a ray test

						 // Compute barycentric coordinate components and test if within bounds
	eae6320::Math::cVector e = Cross(qp, ap);
	*o_v = Dot(ac, e);
	if (*o_v < 0.0f || *o_v > d) return 0;
	*o_w = -Dot(ab, e);
	if (*o_w < 0.0f || *o_v + *o_w > d) return 0;

	// Segment/ray intersects triangle. Perform delayed division and
	// compute the last barycentric coordinate component
	float ood = 1.0f / d;
	*o_t *= ood;
	*o_v *= ood;
	*o_w *= ood;
	*o_u = 1.0f - *o_v - *o_w;
	return 1;
}

bool eae6320::Physics::Load(const char* const i_path)
{
	eae6320::Platform::sDataFromFile data;
	std::string errorMessage;
	if (eae6320::Platform::LoadBinaryFile(i_path, data, &errorMessage))
	{
		uint8_t* meshdata = reinterpret_cast<uint8_t*>(data.data);
		//Triangles
		{
			noOfTris = *reinterpret_cast<uint32_t*>(meshdata);
			meshdata += sizeof(noOfTris);
			size_t size = sizeof(eae6320::Physics::sTriangle)*noOfTris;
			triangles = static_cast<eae6320::Physics::sTriangle*>(malloc(size));
			memcpy(triangles, meshdata, size);
		}
		return true;
	}
	else {
		return false;
	}
}