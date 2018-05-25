// Header Files
//=============

#include "cMayaOctreeExporter.h"

#include <algorithm>
#include <climits>
#include <cstdint>
#include <fstream>
#include <map>
#include <maya/MDagPath.h>
#include <maya/MFloatVector.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFnMesh.h>
#include <maya/MGlobal.h>
#include <maya/MIntArray.h>
#include <maya/MItDag.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItSelectionList.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MSelectionList.h>
#include <sstream>
#include <string>
#include <vector>
#include<cfloat>
#include"../../Engine/Math/cVector.h"

// Vertex Definition
//==================

namespace
{
	// This is the vertex struct that will hold the Maya data.
	// Note that this is independent of the "sVertex" struct
	// used at build-time and run-time to deal with binary mesh data.
	// This struct is used to store Maya-specific vertex information
	// and then output it to the human-readable source mesh file.
	struct sVertex_maya
	{
		// Position
		float x, y, z;
		// Normal
		float nx, ny, nz;
		// Tangent
		float tx, ty, tz;
		// Bitangent
		float btx, bty, btz;
		// Texture coordinates
		float u, v;
		// Color
		float r, g, b, a;

		sVertex_maya(const MPoint& i_position, const MFloatVector& i_normal,
			const MFloatVector& i_tangent, const MFloatVector& i_bitangent,
			const float i_texcoordU, const float i_texcoordV,
			const MColor& i_vertexColor)
			:
			x(static_cast<float>(i_position.x)), y(static_cast<float>(i_position.y)), z(static_cast<float>(i_position.z)),
			nx(i_normal.x), ny(i_normal.y), nz(i_normal.z),
			tx(i_tangent.x), ty(i_tangent.y), tz(i_tangent.z),
			btx(i_bitangent.x), bty(i_bitangent.y), btz(i_bitangent.z),
			u(i_texcoordU), v(i_texcoordV),
			r(i_vertexColor.r), g(i_vertexColor.g), b(i_vertexColor.b), a(i_vertexColor.a)
		{

		}
	};

	// This stores any information associated with a vertex that is necessary to export the mesh.
	// Most of the information is in the sVertex_maya itself,
	// but there are a few other things needed during processing that shouldn't be exported.
	struct sVertexInfo
	{
		sVertex_maya vertex;

		// A Maya "shading group" is similar to what we call a "material" in our class
		size_t shadingGroup;
		// This unique key is calculated in order to decide whether a new vertex should be created or not,
		// and that calculated key is assigned to the vertex so that it can be sorted uniquely
		const std::string uniqueKey;

		sVertexInfo(const MPoint& i_position, const MFloatVector& i_normal,
			const MFloatVector& i_tangent, const MFloatVector& i_bitangent,
			const float i_texcoordU, const float i_texcoordV,
			const MColor& i_vertexColor,
			const size_t i_shadingGroup,
			const std::string& i_uniqueKey)
			:
			vertex(i_position, i_normal, i_tangent, i_bitangent, i_texcoordU, i_texcoordV, i_vertexColor),
			shadingGroup(i_shadingGroup),
			uniqueKey(i_uniqueKey)
		{

		}
	};

}

// Static Data Initialization
//===========================

namespace
{
	const size_t s_vertexCountPerTriangle = 3;

	struct sTriangle
	{
		std::string vertexKeys[s_vertexCountPerTriangle];
		size_t shadingGroup;

		static bool CompareTriangles(const sTriangle& i_lhs, const sTriangle& i_rhs)
		{
			// Sort the triangles by shading group
			// (so that a single draw call can work with a single contiguous block of vertex and index data)
			if (i_lhs.shadingGroup != i_rhs.shadingGroup)
			{
				return i_lhs.shadingGroup < i_rhs.shadingGroup;
			}
			else
			{
				// If two triangles use the same shading group the order doesn't matter,
				// but it's nice to have the exported files be deterministic
				for (size_t i = 0; i < s_vertexCountPerTriangle; ++i)
				{
					if (i_lhs.vertexKeys[i] != i_rhs.vertexKeys[i])
					{
						return i_lhs.vertexKeys[i] < i_rhs.vertexKeys[i];
					}
				}
			}
			// If there are two identical triangles it means that i_lhs isn't less than i_rhs
			return false;
		}
	};

	/* this struct is used to store position information for the oct tree*/

	struct sVector {
	public:
		sVector() :x(0), y(0), z(0) {

		}
		sVector(float i_x, float i_y, float i_z) :x(i_x), y(i_y), z(i_z) {

		}

		float x, y, z;

	};
	/* this is the Node struct we will use to specify a node in the oct tree
	this is independent of the node that is created in the physics libraries*/

	struct sCube {
		sVector m_center;
		float m_width;
		void SetCenter(const sVector& i_center) {
			m_center.x = i_center.x;
			m_center.y = i_center.y;
			m_center.z = i_center.z;
		}
	};


	const size_t s_octantCountPerIndex = 2;
	struct sNode {
	public:
		~sNode() {
			for (size_t i = 0; i < s_octantCountPerIndex; ++i) {
				for (size_t j = 0; j < s_octantCountPerIndex; ++j) {
					for (size_t k = 0; k < s_octantCountPerIndex; ++k) {
						if (m_children[i][j][k] != NULL)
							delete m_children[i][j][k];
					}
				}
			}
		}
		sCube m_cube;
		size_t m_depth;
		sNode* m_children[s_octantCountPerIndex][s_octantCountPerIndex][s_octantCountPerIndex];
		std::vector<uint16_t> m_triangles;
	};

	struct sMaterialInfo
	{
		// As an example, the material node's name (which is useless) is currently stored
		MString nodeName;

		// Keep track of the the range of vertices and indices that use this material
		// (the stored indices are for the final vertex buffer and index buffer vectors)
		struct
		{
			size_t first, last;
		} vertexRange;
		struct
		{
			size_t first, last;
		} indexRange;

		sMaterialInfo()
		{
			vertexRange.first = indexRange.first = SIZE_MAX;
			vertexRange.last = indexRange.last = 0;
		}
	};
}

// Helper Function Declarations
//=============================

namespace
{
	std::string CreateUniqueVertexKey(const int i_positionIndex, const int i_normalIndex, const int i_tangentIndex,
		const int i_texcoordIndex, const int i_vertexColorIndex, const size_t i_shadingGroupIndex, const char* i_transformName);
	MStatus FillVertexAndIndexBuffer(const std::map<std::string, sVertexInfo>& i_uniqueVertices, const std::vector<MObject>& i_shadingGroups,
		std::vector<sTriangle>& io_triangles,
		std::vector<sVertexInfo>& o_vertexBuffer, std::vector<size_t>& o_indexBuffer,
		std::vector<sMaterialInfo>& o_materialInfo);
	MStatus ProcessAllMeshes(std::map<std::string, sVertexInfo>& o_uniqueVertices, std::vector<sTriangle>& o_triangles,
		std::vector<MObject>& o_shadingGroups);
	MStatus ProcessSelectedMeshes(std::map<std::string, sVertexInfo>& o_uniqueVertices, std::vector<sTriangle>& o_triangles,
		std::vector<MObject>& o_shadingGroups);
	MStatus ProcessSingleDagNode(const MDagPath& i_dagPath,
		std::map<std::string, sVertexInfo>& io_uniqueVertices, std::vector<sTriangle>& io_triangles,
		std::vector<MObject>& io_shadingGroups, std::map<std::string, size_t>& io_map_shadingGroupNamesToIndices);
	void CreateOctTreeNode(sNode** o_node, sVector& center, float i_width, size_t i_depth = 0);
	void AddTriangleToNode(sNode* i_node, const sVector& a, const sVector& b, const sVector& c, uint16_t tri_index);
	bool triBoxOverlap(float boxcenter[3], const float boxhalfsize, float triverts[3][3]);
	void GetBiggestVertexPointVal(std::vector<sVertexInfo>& vertexBuffer, float & MaxV, float &MinV);
	void WriteOutNode(sNode& i_node, std::ofstream& fout);
	MStatus WriteOctreeToFile(const MString& i_fileName, sNode* root);
}

// Inherited Interface
//====================

MStatus eae6320::cMayaOctTreeExporter::writer(const MFileObject& i_file, const MString& i_options, FileAccessMode i_mode)
{
	MStatus status;

	// Gather the vertex and index buffer information
	std::map<std::string, sVertexInfo> uniqueVertices;
	std::vector<sTriangle> triangles;
	std::vector<MObject> shadingGroups;
	{
		// The user decides whether to export the entire scene or just a selection
		if (i_mode == MPxFileTranslator::kExportAccessMode)
		{
			status = ProcessAllMeshes(uniqueVertices, triangles, shadingGroups);
			if (!status)
			{
				return status;
			}
		}
		else if (i_mode == MPxFileTranslator::kExportActiveAccessMode)
		{
			status = ProcessSelectedMeshes(uniqueVertices, triangles, shadingGroups);
			if (!status)
			{
				return status;
			}
		}
		else
		{
			MGlobal::displayError("Unexpected file access mode");
			return MStatus::kFailure;
		}
	}


	// Convert the mesh information to vertex and index buffers
	std::vector<sVertexInfo> vertexBuffer;
	std::vector<size_t> indexBuffer;
	std::vector<sMaterialInfo> materialInfo;
	{
		status = FillVertexAndIndexBuffer(uniqueVertices, shadingGroups, triangles, vertexBuffer, indexBuffer, materialInfo);
		if (!status)
		{
			return status;
		}
	}

	//from the vertex and index buffers above we will make a collision triangle list

	//initialise an oct tree
	sNode* root = NULL;
	{

		float MaxV, MinV;
		GetBiggestVertexPointVal(vertexBuffer, MaxV, MinV);
		float width = std::abs(MaxV) + std::abs(MinV);
		//this should create an octree of depth 3
		CreateOctTreeNode(&root, sVector(0, 0, 0), width);
		//if we are here we were able to initialise the octree successfully.


		//now we have to build an octtree
		uint32_t tri_index = 0;
		for (size_t i = 0; i < indexBuffer.size(); i += 3, ++tri_index) {
			const sVector a(vertexBuffer[i].vertex.x, vertexBuffer[i].vertex.y, vertexBuffer[i].vertex.z);
			const sVector b(vertexBuffer[i].vertex.x, vertexBuffer[i].vertex.y, vertexBuffer[i].vertex.z);
			const sVector c(vertexBuffer[i].vertex.x, vertexBuffer[i].vertex.y, vertexBuffer[i].vertex.z);
			AddTriangleToNode(root, a, b, c, tri_index);
		}

	}





	// Write the octree to the requested file
	{
		const MString filePath = i_file.fullName();
		return WriteOctreeToFile(filePath, root);
	}
	//by deleting the root node we will call a destructor on every child
	//hence clearing ourself from memory
	if (root != NULL)
		delete root;
}

// Helper Function Definitions
//============================

namespace
{
	std::string CreateUniqueVertexKey(const int i_positionIndex, const int i_normalIndex, const int i_tangentIndex,
		const int i_texcoordIndex, const int i_vertexColorIndex, const size_t i_shadingGroupIndex, const char* i_transformName)
	{
		std::ostringstream vertexKey;
		vertexKey << i_positionIndex << "_" << i_normalIndex << "_" << i_tangentIndex
			<< "_" << i_texcoordIndex << "_" << i_vertexColorIndex << "_" << i_shadingGroupIndex;
		if (i_transformName)
		{
			vertexKey << "_" << i_transformName;
		}
		return vertexKey.str();
	}

	MStatus FillVertexAndIndexBuffer(const std::map<std::string, sVertexInfo>& i_uniqueVertices, const std::vector<MObject>& i_shadingGroups,
		std::vector<sTriangle>& io_triangles,
		std::vector<sVertexInfo>& o_vertexBuffer, std::vector<size_t>& o_indexBuffer,
		std::vector<sMaterialInfo>& o_materialInfo)
	{
		MStatus status;

		// Fill in the material info
		{
			const size_t shadingGroupCount = i_shadingGroups.size();
			o_materialInfo.resize(shadingGroupCount);
			for (size_t i = 0; i < shadingGroupCount; ++i)
			{
				const MObject& shadingGroup = i_shadingGroups[i];
				MPlug surfaceShaderPlug = MFnDependencyNode(shadingGroup).findPlug("surfaceShader", &status);
				if (status)
				{
					MPlugArray connections;
					{
						const bool getConnectionsWithThisAsDestination = true;
						const bool dontGetConnectionsWithThisAsSource = false;
						surfaceShaderPlug.connectedTo(connections, getConnectionsWithThisAsDestination, dontGetConnectionsWithThisAsSource, &status);
						if (!status)
						{
							MGlobal::displayError(status.errorString());
							return status;
						}
					}
					if (connections.length() == 1)
					{
						// This is where you would put code to extract relevant information from the material
						sMaterialInfo& o_material = o_materialInfo[i];

						// For now this just gets the material node's name (which is useless),
						// but this could be made more sophisticated
						MFnDependencyNode materialNode(connections[0].node());
						o_material.nodeName = materialNode.name();
					}
					else if (connections.length() == 0)
					{
						// This can happen if a material was assigned to a mesh,
						// but then the material was deleted (while the shading group remained).
						// This example code will still work with a missing material,
						// but if you make the material handling more sophisticated
						// you should make sure to handle this case.
						o_materialInfo[i].nodeName = "UNASSIGNED";
					}
					else
					{
						MGlobal::displayError(MString("A shading group's surface shader had ") + connections.length() + " connections");
						return MStatus::kFailure;
					}
				}
				else
				{
					MGlobal::displayError(status.errorString());
					return status;
				}
			}
		}

		// Fill the vertex buffer with the vertices
		// and create a map from the unique key to the assigned index in the vertex buffer
		std::map<std::string, size_t> vertexKeyToIndexMap;
		{
			// Create a reverse map with a custom sorting order for the vertices
			struct CompareVertices
			{
				bool operator()(const sVertexInfo& i_lhs, const sVertexInfo& i_rhs)
				{
					// Sort the vertices by shading group
					// (so that a single draw call can work with a single contiguous block of vertex data)
					if (i_lhs.shadingGroup != i_rhs.shadingGroup)
					{
						return i_lhs.shadingGroup < i_rhs.shadingGroup;
					}
					else
					{
						// If two vertices use the same shading group the order doesn't matter
						return i_lhs.uniqueKey < i_rhs.uniqueKey;
					}
				}
			};
			std::map<sVertexInfo, std::string, CompareVertices> sortedVertices;
			for (std::map<std::string, sVertexInfo>::const_iterator i = i_uniqueVertices.begin(); i != i_uniqueVertices.end(); ++i)
			{
				sortedVertices.insert(std::make_pair(i->second, i->first));
			}
			// Assign the sorted vertices to the buffer
			size_t vertexIndex = 0;
			for (std::map<sVertexInfo, std::string>::iterator i = sortedVertices.begin(); i != sortedVertices.end(); ++i, ++vertexIndex)
			{
				const sVertexInfo& vertex = i->first;
				o_vertexBuffer.push_back(vertex);
				vertexKeyToIndexMap.insert(std::make_pair(i->second, vertexIndex));
				// Update the vertex range for the shading group that this material uses
				if (vertex.shadingGroup < o_materialInfo.size())
				{
					sMaterialInfo& materialInfo = o_materialInfo[vertex.shadingGroup];
					materialInfo.vertexRange.first = std::min(vertexIndex, materialInfo.vertexRange.first);
					materialInfo.vertexRange.last = std::max(vertexIndex, materialInfo.vertexRange.last);
				}
			}
		}

		// Fill the index buffer with the indices
		{
			// Sort the triangles by shading group
			std::sort(io_triangles.begin(), io_triangles.end(), sTriangle::CompareTriangles);
			// Assign the triangle indices to the index buffer
			const size_t triangleCount = io_triangles.size();
			const size_t indexCount = triangleCount * s_vertexCountPerTriangle;
			o_indexBuffer.resize(indexCount);
			for (size_t i = 0; i < triangleCount; ++i)
			{
				const sTriangle& triangle = io_triangles[i];
				for (size_t j = 0; j < s_vertexCountPerTriangle; ++j)
				{
					const std::string& vertexKey = triangle.vertexKeys[j];
					const size_t triangleIndex = vertexKeyToIndexMap.find(vertexKey)->second;
					const size_t indexBufferIndex = (i * s_vertexCountPerTriangle) + j;
					o_indexBuffer[indexBufferIndex] = triangleIndex;
					// Update the index range for the shading group that this material uses
					if (triangle.shadingGroup < o_materialInfo.size())
					{
						sMaterialInfo& materialInfo = o_materialInfo[triangle.shadingGroup];
						materialInfo.indexRange.first = std::min(indexBufferIndex, materialInfo.indexRange.first);
						materialInfo.indexRange.last = std::max(indexBufferIndex, materialInfo.indexRange.last);
					}
				}
			}
		}

		return MStatus::kSuccess;
	}

	MStatus ProcessAllMeshes(std::map<std::string, sVertexInfo>& o_uniqueVertices, std::vector<sTriangle>& o_triangles,
		std::vector<MObject>& o_shadingGroups)
	{
		std::map<std::string, size_t> map_shadingGroupNamesToIndices;
		for (MItDag i(MItDag::kDepthFirst, MFn::kMesh); !i.isDone(); i.next())
		{
			MDagPath dagPath;
			i.getPath(dagPath);
			if (!ProcessSingleDagNode(dagPath, o_uniqueVertices, o_triangles, o_shadingGroups, map_shadingGroupNamesToIndices))
			{
				return MStatus::kFailure;
			}
		}

		return MStatus::kSuccess;
	}

	MStatus ProcessSelectedMeshes(std::map<std::string, sVertexInfo>& o_uniqueVertices, std::vector<sTriangle>& o_triangles,
		std::vector<MObject>& o_shadingGroups)
	{
		// Iterate through each selected mesh
		MSelectionList selectionList;
		MStatus status = MGlobal::getActiveSelectionList(selectionList);
		if (status)
		{
			std::map<std::string, size_t> map_shadingGroupNamesToIndices;
			for (MItSelectionList i(selectionList, MFn::kMesh); !i.isDone(); i.next())
			{
				MDagPath dagPath;
				i.getDagPath(dagPath);
				if (!ProcessSingleDagNode(dagPath, o_uniqueVertices, o_triangles, o_shadingGroups, map_shadingGroupNamesToIndices))
				{
					return MStatus::kFailure;
				}
			}
		}
		else
		{
			MGlobal::displayError(MString("Failed to get active selection list: ") + status.errorString());
			return MStatus::kFailure;
		}

		return MStatus::kSuccess;
	}

	MStatus ProcessSingleDagNode(const MDagPath& i_dagPath,
		std::map<std::string, sVertexInfo>& io_uniqueVertices, std::vector<sTriangle>& io_triangles,
		std::vector<MObject>& io_shadingGroups, std::map<std::string, size_t>& io_map_shadingGroupNamesToIndices)
	{
		MStatus status;

		// Get the mesh from the DAG path
		MFnMesh mesh(i_dagPath);
		if (mesh.isIntermediateObject())
		{
			return MStatus::kSuccess;
		}

		// Get a list of the positions
		MPointArray positions;
		{
			status = mesh.getPoints(positions, MSpace::kWorld);
			if (!status)
			{
				MGlobal::displayError(status.errorString());
				return status;
			}
		}

		// Get a list of the normals
		MFloatVectorArray normals;
		{
			status = mesh.getNormals(normals, MSpace::kWorld);
			if (!status)
			{
				MGlobal::displayError(status.errorString());
				return status;
			}
		}

		// Get a list of tangents
		MFloatVectorArray tangents;
		{
			status = mesh.getTangents(tangents, MSpace::kWorld);
			if (!status)
			{
				MGlobal::displayError(status.errorString());
				return status;
			}
		}

		// Get a list of bitangents
		MFloatVectorArray bitangents;
		{
			status = mesh.getBinormals(bitangents, MSpace::kWorld);
			if (!status)
			{
				MGlobal::displayError(status.errorString());
				return status;
			}
		}

		// Get a list of the texture coordinates
		MFloatArray texcoordUs, texcoordVs;
		{
			status = mesh.getUVs(texcoordUs, texcoordVs);
			if (!status)
			{
				MGlobal::displayError(status.errorString());
				return status;
			}
		}

		// Get a list of the vertex colors
		MColorArray vertexColors;
		{
			int colorSetCount = mesh.numColorSets();
			if (colorSetCount > 0)
			{
				MString* useDefaultColorSet = NULL;	// If more than one color set exists this code will only get the "default" one (as chosen by Maya)
				MColor defaultColor(1.0f, 1.0f, 1.0f, 1.0f);
				status = mesh.getColors(vertexColors, useDefaultColorSet, &defaultColor);
				if (!status)
				{
					MGlobal::displayError(status.errorString());
					return status;
				}
			}
		}

		// A single mesh (i.e. geometric data)
		// can be used by multiple DAG nodes in a Maya scene.
		// (For example, a single sphere mesh could be instanced many times
		// but at different positions, with different orientations, scales, and materials.)
		// An instance ID identifies the specific node that should be processed by this function.
		unsigned int instanceId = 0;
		if (i_dagPath.isInstanced())
		{
			instanceId = i_dagPath.instanceNumber(&status);
			if (!status)
			{
				MGlobal::displayError(MString("Failed to get the DAG path's instance number: ") + status.errorString());
				return MStatus::kFailure;
			}
		}

		// Get a list of the shading groups (i.e. materials)
		std::vector<size_t> polygonShadingGroupIndices;
		{
			MObjectArray shadingGroups;
			MIntArray localIndices;
			status = mesh.getConnectedShaders(instanceId, shadingGroups, localIndices);
			if (status)
			{
				// Remap each local shading group index (i.e. that applies to the array returned by getConnectedShaders())
				// to an index into our static list
				std::vector<size_t> shadingGroupIndices;
				{
					shadingGroupIndices.resize(shadingGroups.length());
					for (unsigned int i = 0; i < shadingGroups.length(); ++i)
					{
						size_t shadingGroupIndex;
						{
							MObject shadingGroup = shadingGroups[i];
							std::string shadingGroupName = MFnDependencyNode(shadingGroup).name().asChar();
							std::map<std::string, size_t>::iterator mapLookUp = io_map_shadingGroupNamesToIndices.find(shadingGroupName);
							if (mapLookUp != io_map_shadingGroupNamesToIndices.end())
							{
								shadingGroupIndex = mapLookUp->second;
							}
							else
							{
								const size_t newIndex = io_shadingGroups.size();
								io_shadingGroups.push_back(shadingGroup);
								io_map_shadingGroupNamesToIndices.insert(std::make_pair(shadingGroupName, newIndex));
								shadingGroupIndex = newIndex;
							}
						}
						shadingGroupIndices[i] = shadingGroupIndex;
					}
				}
				// Convert each polygon shading group index
				{
					const unsigned int polygonCount = localIndices.length();
					if (polygonCount == mesh.numPolygons())
					{
						polygonShadingGroupIndices.resize(polygonCount);
						for (unsigned int i = 0; i < polygonCount; ++i)
						{
							const int localIndex = localIndices[i];
							if (localIndex >= 0)
							{
								polygonShadingGroupIndices[i] = shadingGroupIndices[static_cast<size_t>(localIndex)];
							}
							else
							{
								// If a polygon doesn't have a shading group the index will be -1
								polygonShadingGroupIndices[i] = static_cast<size_t>(localIndex);
							}
						}
					}
					else
					{
						MGlobal::displayError(MString("mesh.numPolygons() returned ") + mesh.numPolygons()
							+ " but mesh.getConnectedShaders() returned " + polygonCount
							+ " indices! According to my understanding of the Maya API this should never happen");
					}
				}
			}
			else
			{
				MGlobal::displayError(status.errorString());
				return status;
			}
		}

		// Gather vertex and triangle information
		{
			// Use the name of the transform to ensure uniqueness
			// (This is necessary because uniqueness is otherwise determined by indices within a given mesh.
			// If the actual data (like the position coordinates) was used instead then this could be ignored
			// and two identical vertices from two completely different meshes could be saved as a single one.
			// This should happen rarely in practice, but a production-quality exporter
			// should probably be more strict about testing equivalence to try and save as much memory as possible.)
			const char* transformName = NULL;
			{
				transformName = MFnDependencyNode(mesh.parent(instanceId)).name().asChar();
			}

			MPointArray trianglePositions;
			MIntArray positionIndices;
			size_t polygonIndex = 0;
			for (MItMeshPolygon i(mesh.object()); !i.isDone(); i.next(), ++polygonIndex)
			{
				if (i.hasValidTriangulation())
				{
					const size_t shadingGroup = polygonShadingGroupIndices[polygonIndex];

					// Store information for each vertex in the polygon
					std::map<int, const std::string> indexToKeyMap;
					{
						MIntArray vertices;
						status = i.getVertices(vertices);
						if (status)
						{
							for (unsigned int j = 0; j < vertices.length(); ++j)
							{
								const int positionIndex = vertices[j];
								const int normalIndex = i.normalIndex(j);
								const int tangentIndex = i.tangentIndex(j);
								int texcoordIndex;
								{
									status = i.getUVIndex(j, texcoordIndex);
									if (!status)
									{
										MGlobal::displayError(status.errorString());
										return status;
									}
								}
								int vertexColorIndex = -1;
								MColor vertexColor(1.0f, 1.0f, 1.0f, 1.0f);
								{
									int colorSetCount = mesh.numColorSets();
									if (colorSetCount > 0)
									{
										status = i.getColorIndex(j, vertexColorIndex);
										if (status)
										{
											if (vertexColorIndex >= 0)
											{
												vertexColor = vertexColors[vertexColorIndex];
											}
										}
										else
										{
											MGlobal::displayError(status.errorString());
											return status;
										}
									}
								}
								const std::string vertexKey = CreateUniqueVertexKey(positionIndex, normalIndex, tangentIndex,
									texcoordIndex, vertexColorIndex, shadingGroup, transformName);
								indexToKeyMap.insert(std::make_pair(positionIndex, vertexKey));
								io_uniqueVertices.insert(std::make_pair(vertexKey,
									sVertexInfo(positions[positionIndex], normals[normalIndex],
										tangents[tangentIndex], bitangents[tangentIndex],
										texcoordUs[texcoordIndex], texcoordVs[texcoordIndex],
										vertexColor,
										shadingGroup, vertexKey)
									));
							}
						}
						else
						{
							MGlobal::displayError(status.errorString());
							return status;
						}
					}
					// Store information for each individual triangle in the polygon
					{
						int triangleCount = 0;
						i.numTriangles(triangleCount);
						for (int j = 0; j < triangleCount; ++j)
						{
							i.getTriangle(j, trianglePositions, positionIndices);
							if (static_cast<size_t>(positionIndices.length()) == s_vertexCountPerTriangle)
							{
								sTriangle triangle;
								for (unsigned int k = 0; k < static_cast<unsigned int>(s_vertexCountPerTriangle); ++k)
								{
									const int positionIndex = positionIndices[k];
									std::map<int, const std::string>::iterator mapLookUp = indexToKeyMap.find(positionIndex);
									if (mapLookUp != indexToKeyMap.end())
									{
										triangle.vertexKeys[k] = mapLookUp->second;
									}
									else
									{
										MGlobal::displayError("A triangle gave a different vertex index than the polygon gave");
										return MStatus::kFailure;
									}
								}
								triangle.shadingGroup = shadingGroup;
								io_triangles.push_back(triangle);
							}
							else
							{
								MGlobal::displayError(MString("Triangle #") + j + " reports that it has " +
									positionIndices.length() + "! According to my understanding of Maya this should never happen");
								return MStatus::kFailure;
							}
						}
					}
				}
				else
				{
					MGlobal::displayError("This mesh has an invalid triangulation");
					return MStatus::kFailure;
				}
			}
		}

		return MStatus::kSuccess;
	}

	void CreateOctTreeNode(sNode** o_node, sVector& center, float i_width, size_t i_depth)
	{
		const size_t MAXDEPTH = 3;
		if (i_depth > MAXDEPTH)
			return;

		*o_node = new sNode();
		(*o_node)->m_cube.SetCenter(center);
		(*o_node)->m_cube.m_width = i_width;
		(*o_node)->m_depth = i_depth;

		sVector vertices[s_octantCountPerIndex][s_octantCountPerIndex][s_octantCountPerIndex];

		{
			float w1 = i_width / 2;
			vertices[0][0][0].x = center.x - w1;
			vertices[0][0][0].y = center.y - w1;
			vertices[0][0][0].z = center.z + w1;


			vertices[0][0][1].x = center.x - w1;
			vertices[0][0][1].y = center.y + w1;
			vertices[0][0][1].z = center.z + w1;


			vertices[0][1][0].x = center.x + w1;
			vertices[0][1][0].y = center.y + w1;
			vertices[0][1][0].z = center.z + w1;


			vertices[0][1][1].x = center.x + w1;
			vertices[0][1][1].y = center.y - w1;
			vertices[0][1][1].z = center.z + w1;

			//2nd set

			vertices[1][0][0].x = center.x - w1;
			vertices[1][0][0].y = center.y - w1;
			vertices[1][0][0].z = center.z - w1;


			vertices[1][0][1].x = center.x - w1;
			vertices[1][0][1].y = center.y + w1;
			vertices[1][0][1].z = center.z - w1;


			vertices[1][1][0].x = center.x + w1;
			vertices[1][1][0].y = center.y + w1;
			vertices[1][1][0].z = center.z - w1;


			vertices[1][1][1].x = center.x + w1;
			vertices[1][1][1].y = center.y - w1;
			vertices[1][1][1].z = center.z - w1;


		}
		for (size_t i = 0; i < s_octantCountPerIndex; ++i) {
			for (size_t j = 0; j < s_octantCountPerIndex; ++j) {
				for (size_t k = 0; k < s_octantCountPerIndex; ++k) {
					//calculate the center of the new child
					float childCenterX = ((vertices[i][j][k].x + center.x) / 2.0f);
					float childCenterY = ((vertices[i][j][k].y + center.y) / 2.0f);
					float childCenterZ = ((vertices[i][j][k].z + center.z) / 2.0f);
					sVector child_center(childCenterX, childCenterY, childCenterZ);

					//calculate the width of the new child
					CreateOctTreeNode(&((*o_node)->m_children[i][j][k]), child_center, i_width / 2.0f, i_depth + 1);
				}
			}
		}

	}

	void AddTriangleToNode(sNode * i_node, const sVector & a, const sVector & b, const sVector & c, uint16_t tri_index)
	{
		float boxcenter[3];
		boxcenter[0] = i_node->m_cube.m_center.x;
		boxcenter[1] = i_node->m_cube.m_center.y;
		boxcenter[2] = i_node->m_cube.m_center.z;
		float trivertex[3][3];

		trivertex[0][0] = a.x;
		trivertex[0][1] = a.y;
		trivertex[0][2] = a.z;

		trivertex[1][0] = b.x;
		trivertex[1][1] = b.y;
		trivertex[1][2] = b.z;

		trivertex[2][0] = c.x;
		trivertex[2][1] = c.y;
		trivertex[2][2] = c.z;

		if (triBoxOverlap(boxcenter, i_node->m_cube.m_width / 2.0f, trivertex)) {
			const float MAX_DEPTH = 3;
			if (i_node->m_depth == MAX_DEPTH) {
				i_node->m_triangles.push_back(tri_index);

			}
			else {
				for (size_t i = 0; i < s_octantCountPerIndex; ++i) {
					for (size_t j = 0; j < s_octantCountPerIndex; ++j) {
						for (size_t k = 0; k < s_octantCountPerIndex; ++k) {
							AddTriangleToNode((i_node->m_children[i][j][k]), a, b, c, tri_index);
						}
					}
				}
			}

		}
	}

	void GetBiggestVertexPointVal(std::vector<sVertexInfo>& vertexBuffer, float & biggestVetexVal, float & smallestVertexVal)
	{
		biggestVetexVal = FLT_MIN;
		smallestVertexVal = FLT_MAX;
		for (auto vertex : vertexBuffer) {
			if (vertex.vertex.x > biggestVetexVal) {
				biggestVetexVal = vertex.vertex.x;
			}
			else if (vertex.vertex.x < smallestVertexVal) {
				smallestVertexVal = vertex.vertex.x;
			}
			if (vertex.vertex.y > biggestVetexVal) {
				biggestVetexVal = vertex.vertex.y;
			}
			else if (vertex.vertex.y < smallestVertexVal) {
				smallestVertexVal = vertex.vertex.y;
			}
			if (vertex.vertex.z > biggestVetexVal) {
				biggestVetexVal = vertex.vertex.z;
			}
			else if (vertex.vertex.z < smallestVertexVal) {
				smallestVertexVal = vertex.vertex.z;
			}

		}
	}

	void WriteOutNode(sNode & i_node, std::ofstream & fout)
	{
		const size_t MAX_DEPTH = 3;
		if (i_node.m_depth > MAX_DEPTH)
			return;
		fout << "\n{\n";
		fout << "cube={\n";
		{

			fout << std::fixed;
			fout << "center={";
			fout << i_node.m_cube.m_center.x << "," << i_node.m_cube.m_center.y << "," << i_node.m_cube.m_center.z;
			fout << "},\n";
			fout << "width=";
			fout << i_node.m_cube.m_width << "\n";

		}
		fout << "},\n";
		{
			fout << std::fixed;
			fout << "depth=";
			fout << i_node.m_depth << "," << "\n";
		}
		fout << "triangles={\n";
		{
			for (auto index : i_node.m_triangles) {
				fout << index << ",";
			}
		}
		fout << "},\n";
		fout << "},\n";
		if (i_node.m_depth == MAX_DEPTH)
			return;
		for (size_t i = 0; i < s_octantCountPerIndex; ++i) {
			for (size_t j = 0; j < s_octantCountPerIndex; ++j) {
				for (size_t k = 0; k < s_octantCountPerIndex; ++k) {
					WriteOutNode(*i_node.m_children[i][j][k], fout);
				}
			}

		}
	}

	MStatus WriteOctreeToFile(const MString& i_fileName, sNode* root)
	{

		std::ofstream fout(i_fileName.asChar());
		if (fout.is_open())
		{
			fout.precision(4);
			fout << "return\n"
				"{\n";
			{
				fout << "Nodes=\n{";
				WriteOutNode(*root, fout);
				fout << "},\n";
			}
			fout << "}\n";
			fout.close();
			return MStatus::kSuccess;
		}
		else
		{
			MGlobal::displayError(MString("Couldn't open ") + i_fileName + " for writing");
			return MStatus::kFailure;
		}

		return MStatus::kSuccess;
	}
}

#define X 0
#define Y 1
#define Z 2
#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

inline void SUB(float* dest, const float v1[3], const float v2[3]) {
	dest[0] = v1[0] - v2[0];
	dest[1] = v1[1] - v2[1];
	dest[2] = v1[2] - v2[2];
}

inline void CROSS(float *dest, const float v1[3], const float v2[3]) {
	dest[0] = v1[1] * v2[2] - v1[2] * v2[1];
	dest[1] = v1[2] * v2[0] - v1[0] * v2[2];
	dest[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

inline void FINDMINMAX(const float i_x0, const float i_x1, const float i_x2, float*o_min, float * o_max) {
	*o_min = *o_max = i_x0;
	if (i_x1 < *o_min)
		*o_min = i_x1;
	if (i_x1 > *o_max)
		*o_max = i_x1;
	if (i_x2 < *o_min)
		*o_min = i_x2;
	if (i_x2 > *o_max)
		*o_max = i_x2;
}

int planeBoxOverlap(float normal[3], float vert[3], float maxbox)
{
	int q;
	float vmin[3], vmax[3], v;
	for (q = X; q <= Z; q++)
	{
		v = vert[q];
		if (normal[q]>0.0f)
		{
			vmin[q] = -maxbox - v;
			vmax[q] = maxbox - v;
		}
		else
		{
			vmin[q] = maxbox - v;	
			vmax[q] = -maxbox - v;	
		}
	}
	if (DOT(normal, vmin)>0.0f) return 0;	
	if (DOT(normal, vmax) >= 0.0f) return 1;	
	return 0;

}

inline int  AXISTEST_X01(const float a, const float  b, const float fa, const float  fb,
	const float v0[3], const float v1[3], const float v2[3], const float box_half_size,
	float & min, float & max, float & p0, float & p2, float & rad) {
	p0 = a*v0[Y] - b*v0[Z];
	p2 = a*v2[Y] - b*v2[Z];
	if (p0 < p2) { min = p0; max = p2; }
	else { min = p2; max = p0; }
	rad = fa * box_half_size + fb * box_half_size;
	if (min > rad || max < -rad) return 0;
	return 1;
}

inline int AXISTEST_X2(const float a, const float  b, const float fa, const float  fb,
	const float v0[3], const float v1[3], const float v2[3], const float box_half_size,
	float & min, float & max, float & p0, float & p1, float & rad) {
	p0 = a*v0[Y] - b*v0[Z];
	p1 = a*v1[Y] - b*v1[Z];
	if (p0 < p1) { min = p0; max = p1; }
	else { min = p1; max = p0; }
	rad = fa * box_half_size + fb * box_half_size;
	if (min > rad || max < -rad) return 0;
	return 1;
}

inline int AXISTEST_Y02(const float a, const float  b, const float fa, const float  fb,
	const float v0[3], const float v1[3], const float v2[3], const float box_half_size,
	float & min, float & max, float & p0, float & p2, float & rad) {
	p0 = -a*v0[X] + b*v0[Z];
	p2 = -a*v2[X] + b*v2[Z];
	if (p0 < p2) { min = p0; max = p2; }
	else { min = p2; max = p0; }
	rad = fa * box_half_size + fb * box_half_size;
	if (min > rad || max < -rad) return 0;
	return 1;
}

inline int AXISTEST_Y1(const float a, const float  b, const float fa, const float  fb,
	const float v0[3], const float v1[3], const float v2[3], const float box_half_size,
	float & min, float & max, float & p0, float & p1, float & rad) {
	p0 = -a*v0[X] + b*v0[Z];
	p1 = -a*v1[X] + b*v1[Z];
	if (p0 < p1) { min = p0; max = p1; }
	else { min = p1; max = p0; }
	rad = fa * box_half_size + fb * box_half_size;
	if (min > rad || max < -rad) return 0;
	return 1;
}

inline int AXISTEST_Z12(const float a, const float  b, const float fa, const float  fb,
	const float v0[3], const float v1[3], const float v2[3], const float box_half_size,
	float & min, float & max, float & p1, float & p2, float & rad) {
	p1 = a*v1[X] - b*v1[Y];
	p2 = a*v2[X] - b*v2[Y];
	if (p2 < p1) { min = p2; max = p1; }
	else { min = p1; max = p2; }
	rad = fa * box_half_size + fb * box_half_size;
	if (min > rad || max < -rad) return 0;
	return 1;
}

inline int AXISTEST_Z0(const float a, const float  b, const float fa, const float  fb,
	const float v0[3], const float v1[3], const float v2[3], const float box_half_size,
	float & min, float & max, float & p0, float & p1, float & rad) {
	p0 = a*v0[X] - b*v0[Y];
	p1 = a*v1[X] - b*v1[Y];
	if (p0 < p1) { min = p0; max = p1; }
	else { min = p1; max = p0; }
	rad = fa * box_half_size + fb *box_half_size;
	if (min > rad || max < -rad) return 0;
	return 1;
}

namespace {
	bool triBoxOverlap(float boxcenter[3], const float boxhalfsize, float triverts[3][3])
	{
		float v0[3], v1[3], v2[3];
		float min, max, p0, p1, p2, rad, fex, fey, fez;	
		float normal[3], e0[3], e1[3], e2[3];
		SUB(v0, triverts[0], boxcenter);
		SUB(v1, triverts[1], boxcenter);
		SUB(v2, triverts[2], boxcenter);
		SUB(e0, v1, v0);  
		SUB(e1, v2, v1);  
		SUB(e2, v0, v2);  

		fex = fabsf(e0[X]);
		fey = fabsf(e0[Y]);
		fez = fabsf(e0[Z]);
		AXISTEST_X01(e0[Z], e0[Y], fez, fey, v0, v1, v2, boxhalfsize, min, max, p0, p2, rad);
		AXISTEST_Y02(e0[Z], e0[X], fez, fex, v0, v1, v2, boxhalfsize, min, max, p0, p2, rad);
		AXISTEST_Z12(e0[Y], e0[X], fey, fex, v0, v1, v2, boxhalfsize, min, max, p1, p2, rad);
		
		fex = fabsf(e1[X]);
		fey = fabsf(e1[Y]);
		fez = fabsf(e1[Z]);
		AXISTEST_X01(e1[Z], e1[Y], fez, fey, v0, v1, v2, boxhalfsize, min, max, p0, p2, rad);
		AXISTEST_Y02(e1[Z], e1[X], fez, fex, v0, v1, v2, boxhalfsize, min, max, p0, p2, rad);
		AXISTEST_Z0(e1[Y], e1[X], fey, fex, v0, v1, v2, boxhalfsize, min, max, p0, p1, rad);
		fex = fabsf(e2[X]);
		fey = fabsf(e2[Y]);
		fez = fabsf(e2[Z]);
		AXISTEST_X2(e2[Z], e2[Y], fez, fey, v0, v1, v2, boxhalfsize, min, max, p0, p1, rad);
		AXISTEST_Y1(e2[Z], e2[X], fez, fex, v0, v1, v2, boxhalfsize, min, max, p0, p1, rad);
		AXISTEST_Z12(e2[Y], e2[X], fey, fex, v0, v1, v2, boxhalfsize, min, max, p1, p2, rad);
		FINDMINMAX(v0[X], v1[X], v2[X], &min, &max);
		if (min > boxhalfsize || max < -boxhalfsize) return 0;
		FINDMINMAX(v0[Y], v1[Y], v2[Y], &min, &max);
		if (min > boxhalfsize || max < -boxhalfsize) return 0;
		FINDMINMAX(v0[Z], v1[Z], v2[Z], &min, &max);
		if (min > boxhalfsize || max < -boxhalfsize) return 0;
		CROSS(normal, e0, e1);
		if (!planeBoxOverlap(normal, v0, boxhalfsize)) return 0;
		return 1; 
	}
}