#ifndef EAE6320_OCTREE_H
#define EAE6320_OCTREE_H

#include"../Math/cVector.h"
#include <cstdint>

namespace eae6320 
{
	namespace Physics 
	{
		namespace Octree 
		{
			struct NodeData 
			{
				struct CubeData 
				{
					struct sPos 
					{
						float x[3];
					};
					sPos m_center;
					float m_width;
				};
				CubeData m_cube;
				uint8_t m_depth;
				uint16_t m_no_tris;
				uint16_t* m_tris;
			};
			bool Initialise();
			bool Load(const char* const i_fileName);
			void Compute();
			void displayOctree();
			NodeData GetNodeFromPoint(const eae6320::Math::cVector& point);
			void CleanUp();
		}
	}
}
#endif // EAE6320_OCTREE_H