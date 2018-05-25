#include "Octree.h"
#include <vector>
#include <math.h>
#include "../Platform/Platform.h"
#include "../Graphics/DebugObject.h"

namespace {
	uint16_t s_noOfNodes = 0;
	std::vector<eae6320::Physics::Octree::NodeData> s_nodeData;
	eae6320::Graphics::DebugObject debugBox[585];
}

bool eae6320::Physics::Octree::Initialise()
{
	return false;
}

bool eae6320::Physics::Octree::Load(const char* const i_path)
{
	eae6320::Platform::sDataFromFile data;
	std::string errorMessage;

	if (eae6320::Platform::LoadBinaryFile(i_path, data, &errorMessage)) {
		uint8_t* octreeData;
		octreeData = reinterpret_cast<uint8_t*>(data.data);
		const uint8_t* eof = octreeData + data.size;
		{
			s_noOfNodes = *reinterpret_cast<uint16_t*>(octreeData);
		}
		octreeData += sizeof(s_noOfNodes);
		while (octreeData != eof)
		{
			eae6320::Physics::Octree::NodeData node;
			size_t node_size = *reinterpret_cast<size_t*>(octreeData);
			octreeData += sizeof(node_size);
			{
				node.m_cube = *reinterpret_cast<eae6320::Physics::Octree::NodeData::CubeData*>(octreeData);
				octreeData += sizeof(node.m_cube);
			}
			{
				node.m_depth = *reinterpret_cast<uint8_t*>(octreeData);
				octreeData += sizeof(node.m_depth);
			}
			{
				node.m_no_tris = *reinterpret_cast<uint8_t*>(octreeData);
				octreeData += sizeof(node.m_no_tris);
			}
			if (node.m_no_tris > 0) {
				size_t size = sizeof(eae6320::Physics::Octree::NodeData::m_no_tris)*node.m_no_tris;
				node.m_tris = static_cast<uint16_t*>(malloc(size));
				memcpy(node.m_tris, octreeData, size);
				octreeData += size;
			}
			else {
				node.m_tris = NULL;
			}
			s_nodeData.push_back(node);
		}
		data.Free();
	}
	return true;
}

void eae6320::Physics::Octree::Compute()
{
	Graphics::cMaterial * material = new Graphics::cMaterial();
	material->Load("data/materials/debugshape.material");
	int i = 0;
	for (auto temp : s_nodeData) {
		eae6320::Math::cVector center(temp.m_cube.m_center.x[0], temp.m_cube.m_center.x[1], temp.m_cube.m_center.x[2]);
		uint8_t r, g, b;
		if (temp.m_depth == 0) {
			r = 255;
			g = 0;
			b = 0;
		}
		else if (temp.m_depth == 1) {
			r = 0;
			g = 255;
			b = 0;
		}
		else if (temp.m_depth == 2) {
			r = 0;
			g = 0;
			b = 255;
		}
		else {
			r = 255;
			g = 255;
			b = 0;
		}
		debugBox[i].drawBoxDebugObject(material, center, Math::cVector(), temp.m_cube.m_width, temp.m_cube.m_width, temp.m_cube.m_width, r, g, b, 1);
		i++;
	}
}

void eae6320::Physics::Octree::displayOctree()
{
	for (int i = 0; i < 585; i++) {
		Graphics::SetMesh(debugBox[i].meshObject);
	}
}

eae6320::Physics::Octree::NodeData eae6320::Physics::Octree::GetNodeFromPoint(const eae6320::Math::cVector& i_point) {
	const uint8_t MAX_DEPTH = 3;
	for (auto node : s_nodeData) {
		if (node.m_depth = MAX_DEPTH && node.m_no_tris>0) {
			const float half_width = node.m_cube.m_width / 2.0f;
			const float px = i_point.x;
			const float py = i_point.y;
			const float pz = i_point.z;
			const eae6320::Math::cVector center(node.m_cube.m_center.x[0], node.m_cube.m_center.x[1], node.m_cube.m_center.x[2]);
			const float dist_x = fabs(px - center.x);
			const float dist_y = fabs(py - center.y);
			const float dist_z = fabs(pz - center.z);

			if (dist_x <= half_width || dist_y <= half_width || dist_z <= half_width)
				return node;
		}

	}
	return s_nodeData[0];
}
void eae6320::Physics::Octree::CleanUp()
{
	for (auto temp : s_nodeData) {
		if (temp.m_tris != NULL) {
			free(temp.m_tris);
		}
	}
	s_nodeData.clear();

	for (int i = 0; i < 585; i++) {
		debugBox[i].cleanUp();
	}
}