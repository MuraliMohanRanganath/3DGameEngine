#include "Mesh.h"
#include "../Asserts/Asserts.h"
#include "../Platform/Platform.h"
#include "../Logging/Logging.h"
#include "../Math/Functions.h"
#include "../Math/cVector.h"

#include <vector>

bool eae6320::Graphics::Mesh::LoadBinaryFile(const char* const i_path) {
	bool wereThereErrors = false;
	{
		eae6320::Platform::sDataFromFile binaryFile;
		std::string errorMessage;

		if (!eae6320::Platform::LoadBinaryFile(i_path, binaryFile, &errorMessage))
		{
			wereThereErrors = true;
			EAE6320_ASSERTF(false, errorMessage.c_str());
			eae6320::Logging::OutputError("Failed to load binary file \"%s\": %s", i_path, errorMessage.c_str());
		}

		uint8_t* data = reinterpret_cast<uint8_t*>(binaryFile.data);

		verticesCount = *reinterpret_cast<uint32_t*>(data);
		indicesCount = *reinterpret_cast<uint32_t*>(data + sizeof(uint32_t));

		size_t size = sizeof(eae6320::Graphics::sVertex)*verticesCount;
		vertexData = static_cast<eae6320::Graphics::sVertex*>(malloc(size));
		memcpy(vertexData, data + sizeof(verticesCount), size);

		vertexData = reinterpret_cast<eae6320::Graphics::sVertex*>(data + sizeof(uint32_t) + sizeof(uint32_t));
		indices = reinterpret_cast<uint32_t*>(data + sizeof(uint32_t)*2 + sizeof(eae6320::Graphics::sVertex) * verticesCount);
		
		if (!Initialize()) {
			wereThereErrors = true;
		}

		//binaryFile.Free();
	}
	return !wereThereErrors;
}
#ifdef _DEBUG
bool eae6320::Graphics::Mesh::DrawLine(Math::cVector start, Math::cVector end, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	bool wereThereErrors = false;
	{
		verticesCount = 3;
		indicesCount = 3;
		vertexData = new sVertex[verticesCount];

		vertexData[0] = sVertex(start.x, start.y, start.z, r, g, b, a, 0, 0);
		vertexData[1] = sVertex(end.x, end.y, end.z, r, g, b, a, 0, 0);
		vertexData[2] = sVertex(start.x, start.y, start.z, r, g, b, a, 0, 0);

		indices = new uint32_t[indicesCount]{0,1,2};

		if (!Initialize()) {
			wereThereErrors = true;
		}
	}
	return !wereThereErrors;
}

bool eae6320::Graphics::Mesh::DrawCube(float width, float height, float depth, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	bool wereThereErrors = false;
	{
		verticesCount = 24;
		indicesCount = 36;
		vertexData = new sVertex[verticesCount];

		float w2 = 0.5f*width;
		float h2 = 0.5f*height;
		float d2 = 0.5f*depth;

		//front
		vertexData[0] = sVertex(-w2, -h2, -d2, r, g, b, a, 0, 1);
		vertexData[1] = sVertex(-w2, +h2, -d2, r, g, b, a, 0, 0);
		vertexData[2] = sVertex(+w2, +h2, -d2, r, g, b, a, 1, 0);
		vertexData[3] = sVertex(+w2, -h2, -d2, r, g, b, a, 1, 1);
		// back
		vertexData[4] = sVertex(-w2, -h2, +d2, r, g, b, a, 1, 1);
		vertexData[5] = sVertex(+w2, -h2, +d2, r, g, b, a, 0, 1);
		vertexData[6] = sVertex(+w2, +h2, +d2, r, g, b, a, 0, 0);
		vertexData[7] = sVertex(-w2, +h2, +d2, r, g, b, a, 1, 0);
		// top
		vertexData[8] = sVertex(-w2, +h2, -d2, r, g, b, a, 0, 1);
		vertexData[9] = sVertex(-w2, +h2, +d2, r, g, b, a, 0, 0);
		vertexData[10] = sVertex(+w2, +h2, +d2, r, g, b, a, 1, 0);
		vertexData[11] = sVertex(+w2, +h2, -d2, r, g, b, a, 1, 1);
		// bottom
		vertexData[12] = sVertex(-w2, -h2, -d2, r, g, b, a, 1, 1);
		vertexData[13] = sVertex(+w2, -h2, -d2, r, g, b, a, 0, 1);
		vertexData[14] = sVertex(+w2, -h2, +d2, r, g, b, a, 0, 0);
		vertexData[15] = sVertex(-w2, -h2, +d2, r, g, b, a, 1, 0);
		// left
		vertexData[16] = sVertex(-w2, -h2, +d2, r, g, b, a, 0, 1);
		vertexData[17] = sVertex(-w2, +h2, +d2, r, g, b, a, 0, 0);
		vertexData[18] = sVertex(-w2, +h2, -d2, r, g, b, a, 1, 0);
		vertexData[19] = sVertex(-w2, -h2, -d2, r, g, b, a, 1, 1);
		// right
		vertexData[20] = sVertex(+w2, -h2, -d2, r, g, b, a, 0, 1);
		vertexData[21] = sVertex(+w2, +h2, -d2, r, g, b, a, 0, 0);
		vertexData[22] = sVertex(+w2, +h2, +d2, r, g, b, a, 1, 0);

		vertexData[23] = sVertex(+w2, -h2, +d2, r, g, b, a, 1, 1);
		indices = new uint32_t[indicesCount]{
			0,1,2,0,2,3,
			4,5,6,4,6,7,
			8,9,10,8,10,11,
			12,13,14,12,14,15,
			16,17,18,16,18,19,
			20,21,22,20,22,23
		};

		if (!Initialize()) {
			wereThereErrors = true;
		}
	}
	return !wereThereErrors;
}

bool eae6320::Graphics::Mesh::DrawSphere(float radius, int sliceCount, int stackCount, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	bool wereThereErrors = false;
	{
		std::vector<sVertex> verticies;
		verticies.push_back(sVertex(0.0f, radius, 0.0f, r, g, b, a, 0, 0));

		float phiStep = Math::Pi / stackCount;
		float thetaStep = 2.0f*Math::Pi / sliceCount;

		for (int i = 1; i <= stackCount - 1; i++)
		{
			float phi = i*phiStep;
			for (int j = 0; j <= sliceCount; j++) {
				float theta = j*thetaStep;
				Math::cVector p = Math::cVector((radius*sin(phi)*cos(theta)), (radius*cos(phi)), (radius* sin(phi)*sin(theta)));
				verticies.push_back(sVertex(p.x, p.y, p.z, r, g, b, a, 0, 0));
			}
		}
		verticies.push_back(sVertex(0, -radius, 0, r, g, b, a, 0, 0));
		verticesCount = (uint32_t)verticies.size();
		vertexData = new sVertex[verticesCount];
		for (size_t i = 0; i < verticies.size(); i++)
		{
			vertexData[i] = verticies[i];
		}

		std::vector<uint32_t> indexList;
		for (int i = 1; i <= sliceCount; i++)
		{
			indexList.push_back(0);
			indexList.push_back(i + 1);
			indexList.push_back(i);
		}
		uint32_t baseIndex = 1;
		uint32_t ringVertexCount = sliceCount + 1;
		for (int i = 0; i < stackCount - 2; i++)
		{
			for (int j = 0; j < sliceCount; j++)
			{

				indexList.push_back(baseIndex + (i*ringVertexCount) + j);
				indexList.push_back(baseIndex + (i*ringVertexCount) + j + 1);
				indexList.push_back(baseIndex + ((i + 1)*ringVertexCount) + j);

				indexList.push_back(baseIndex + ((i + 1)*ringVertexCount) + j);
				indexList.push_back(baseIndex + (i*ringVertexCount) + j + 1);
				indexList.push_back(baseIndex + ((i + 1)*ringVertexCount) + j + 1);
			}
		}

		uint32_t southPoleIndex = verticesCount - 1;
		baseIndex = southPoleIndex - ringVertexCount;
		for (int i = 0; i < sliceCount; i++)
		{
			indexList.push_back(southPoleIndex);
			indexList.push_back(baseIndex + i);
			indexList.push_back(baseIndex + i + 1);
		}

		indicesCount = (uint32_t)indexList.size();
		indices = new uint32_t[indicesCount];

		for (size_t i = 0; i < indexList.size(); i++)
		{
			indices[i] = indexList[i];
		}

		if (!Initialize()) {
			wereThereErrors = true;
		}
	}
	return !wereThereErrors;
}

bool eae6320::Graphics::Mesh::DrawCylinder(float bottomRadius, float topRadius, float height, int sliceCount, int stackCount, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	bool wereThereErrors = false;
	{
		float stackHeight = height / stackCount;
		float radiusStep = (topRadius - bottomRadius) / stackCount;
		int ringCount = stackCount + 1;

		std::vector<sVertex> verticies;
		for (int i = 0; i < ringCount; i++)
		{
			float y = -0.5f*height + i*stackHeight;
			float radius = bottomRadius + i*radiusStep;
			float dTheta = 2.0f*Math::Pi / sliceCount;
			for (int j = 0; j <= sliceCount; j++)
			{
				float c = cos(j*dTheta);
				float s = sin(j*dTheta);
				Math::cVector v = Math::cVector(radius*c, y, radius*s);
				verticies.push_back(sVertex(v.x, v.y, v.z, r, g, b, a, 0, 0));
			}
		}

		int ringVertexCount = sliceCount + 1;

		std::vector<uint32_t> indicies;
		for (int i = 0; i < stackCount; i++) {
			for (int j = 0; j < sliceCount; j++) {
				indicies.push_back(i*ringVertexCount + j);
				indicies.push_back((i + 1)*ringVertexCount + j);
				indicies.push_back((i + 1)*ringVertexCount + j + 1);

				indicies.push_back(i*ringVertexCount + j);
				indicies.push_back((i + 1)*ringVertexCount + j + 1);
				indicies.push_back(i*ringVertexCount + j + 1);
			}
		}

		DrawCylinderTopCap(topRadius, height, sliceCount, verticies, indicies, r, g, b, a);
		DrawCylinderBottomCap(bottomRadius, height, sliceCount, verticies, indicies, r, g, b, a);

		verticesCount = (uint32_t)verticies.size();
		vertexData = new sVertex[verticesCount];
		for (size_t i = 0; i < verticies.size(); i++)
		{
			vertexData[i] = verticies[i];
		}
		indicesCount = (uint32_t)indicies.size();
		indices = new uint32_t[indicesCount];

		for (size_t i = 0; i < indicies.size(); i++)
		{
			indices[i] = indicies[i];
		}

		if (!Initialize()) {
			wereThereErrors = true;
		}
	}
	return !wereThereErrors;
}
void eae6320::Graphics::Mesh::DrawCylinderTopCap(float topRadius, float height, int sliceCount, std::vector<sVertex> &verticies, std::vector<uint32_t> &indicies, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
	int baseIndex = (int)verticies.size();

	float y = 0.5f*height;
	float dTheta = 2.0f*Math::Pi / sliceCount;

	for (int i = 0; i <= sliceCount; i++) {
		float x = topRadius*cos(i*dTheta);
		float z = topRadius*sin(i*dTheta);

		verticies.push_back(sVertex(x, y, z, red, green, blue, alpha, 0, 0));
	}
	verticies.push_back(sVertex(0, y, 0, red, green, blue, alpha, 0, 0));
	int centerIndex = (int)verticies.size() - 1;
	for (int i = 0; i < sliceCount; i++) {
		indicies.push_back(centerIndex);
		indicies.push_back(baseIndex + i + 1);
		indicies.push_back(baseIndex + i);
	}
}

void eae6320::Graphics::Mesh::DrawCylinderBottomCap(float bottomRadius, float height, int sliceCount, std::vector<sVertex> &verticies, std::vector<uint32_t> &indicies, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
	int baseIndex = (int)verticies.size();

	float y = -0.5f*height;
	float dTheta = 2.0f*Math::Pi / sliceCount;

	for (int i = 0; i <= sliceCount; i++) {
		float x = bottomRadius*cos(i*dTheta);
		float z = bottomRadius*sin(i*dTheta);

		verticies.push_back(sVertex(x, y, z, red, green, blue, alpha, 0, 0));
	}
	verticies.push_back(sVertex(0, y, 0, red, green, blue, alpha, 0, 0));
	int centerIndex = (int)verticies.size() - 1;
	for (int i = 0; i < sliceCount; i++) {
		indicies.push_back(centerIndex);
		indicies.push_back(baseIndex + i);
		indicies.push_back(baseIndex + i + 1);
	}
}
#endif