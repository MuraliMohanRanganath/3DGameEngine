#include "DebugObject.h"

void eae6320::Graphics::DebugObject::Move(Math::cVector i_position)
{
	transform.Move(i_position);
	meshObject.position = i_position;
}

void eae6320::Graphics::DebugObject::Rotate(Math::cVector i_rotation)
{
	transform.Rotate(i_rotation);
	meshObject.rotation = transform.getOrientation();
}

void eae6320::Graphics::DebugObject::initializeBoxDebugObject(Math::cVector initPosition, Math::cVector initRotation, float width, float height, float depth, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
	meshObject.mesh = new Graphics::Mesh();
	meshObject.mesh->DrawCube(width, height, depth, red, green, blue, alpha);
	meshObject.material = new Graphics::cMaterial();
	meshObject.material->Load(debugshapeMaterialPath);
	Move(initPosition);
	Rotate(initRotation);
}

void eae6320::Graphics::DebugObject::drawBoxDebugObject(Graphics::cMaterial* material,Math::cVector initPosition, Math::cVector initRotation, float width, float height, float depth, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
	meshObject.mesh = new Graphics::Mesh();
	meshObject.mesh->DrawCube(width, height, depth, red, green, blue, alpha);
	meshObject.material = material;
	Move(initPosition);
	Rotate(initRotation);
}




void eae6320::Graphics::DebugObject::initializeSphereDebugObject(Math::cVector initPosition, Math::cVector initRotation, float radius, int sliceCount, int stackCount, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
	meshObject.mesh = new Graphics::Mesh();
	meshObject.mesh->DrawSphere(radius, sliceCount, stackCount, red, green, blue, alpha);
	meshObject.material = new Graphics::cMaterial();
	meshObject.material->Load(debugshapeMaterialPath);
	Move(initPosition);
	Rotate(initRotation);
}

void eae6320::Graphics::DebugObject::initializeCylinderDebugObject(Math::cVector initPosition, Math::cVector initRotation, float bottomRadius, float topRadius, float height, int sliceCount, int stackCount, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
	meshObject.mesh = new Graphics::Mesh();
	meshObject.mesh->DrawCylinder(bottomRadius, topRadius, height, sliceCount, stackCount, red, green, blue, alpha);
	meshObject.material = new Graphics::cMaterial();
	meshObject.material->Load(debugshapeMaterialPath);
	Move(initPosition);
	Rotate(initRotation);
}

void eae6320::Graphics::DebugObject::initializeLineDebugObject(Math::cVector initPosition, Math::cVector initRotation, eae6320::Math::cVector start, eae6320::Math::cVector end, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
	meshObject.mesh = new Graphics::Mesh();
	meshObject.mesh->DrawLine(start, end, red, green, blue, alpha);
	meshObject.material = new Graphics::cMaterial();
	meshObject.material->Load(debugshapeMaterialPath);
	Move(initPosition);
	Rotate(initRotation);
}

void eae6320::Graphics::DebugObject::updateLine(eae6320::Math::cVector start, eae6320::Math::cVector end, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
	meshObject.mesh->DrawLine(start, end, red, green, blue, alpha);
}

bool eae6320::Graphics::DebugObject::cleanUp()
{
	if (meshObject.mesh)
		meshObject.mesh->CleanUp();
	if (meshObject.material)
		meshObject.material->CleanUp();
	return true;
}