#ifndef EAE6320_TRIANGLE_DATA_H
#define EAE6320_TRIANGLE_DATA_H

#include "../Math/cVector.h"

namespace eae6320 {
	namespace Physics {
		struct sTriangle {
			eae6320::Math::cVector A;
			eae6320::Math::cVector B;
			eae6320::Math::cVector C;
		};
	}
}
#endif // EAE6320_TRIANGLE_DATA_H