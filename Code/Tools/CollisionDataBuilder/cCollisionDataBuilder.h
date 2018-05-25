#ifndef EAE6320_COLLISION_DATA_BUILDER_H
#define EAE6320_COLLISION_DATA_BUILDER_H

#include"../AssetBuildLibrary/cbBuilder.h"

namespace eae6320
{
	namespace AssetBuild
	{
		class cCollisionDataBuilder : public cbBuilder
		{
			// Inherited Implementation
			//=========================

		private:

			// Build
			//------

			virtual bool Build(const std::vector<std::string>& i_arguments);
		};
	}
}
#endif // EAE6320_COLLISION_DATA_BUILDER_H