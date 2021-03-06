#ifndef EAE6320_MAYA_OCTREE_EXPORTER_H
#define EAE6320_MAYA_OCTREE_EXPORTER_H

#include <maya/MPxFileTranslator.h>
namespace eae6320
{
	class cMayaOctTreeExporter : public MPxFileTranslator
	{
		// Inherited Interface
		//====================

	public:

		// The writer method is what exports the file
		virtual bool haveWriteMethod() const { return true; }
		virtual MStatus writer(const MFileObject& i_file, const MString& i_options, FileAccessMode i_mode);

		// We won't implement a Maya importer in our class.
		// If you wanted to be able to import your mesh files into Maya
		// you would have to return true from haveReadMethod()
		// and then implement reader().

		// You can choose what the default file extension of an exported mesh is
		virtual MString defaultExtension() const { return "octree"; }

		// Interface
		//==========

	public:

		// This function is used by Maya to create an instance of the exporter (see registerFileTranslator() in EntryPoint.cpp)
		static void* Create()
		{
			return new cMayaOctTreeExporter;
		}
	};
}
#endif // !EAE6320_MAYA_OCTREE_EXPORTER_H