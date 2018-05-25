/*
	The two functions in this file are how Maya will interact with the plug-in
*/

// Header Files
//=============

#include "cMayaMeshExporter.h"
#include "cMayaCollisionDataExporter.h"
#include "cMayaOctreeExporter.h"
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include <maya/MObject.h>
#include <maya/MStatus.h>

// Static Data Initialization
//===========================

namespace
{
	// This will be displayed in Maya's dropdown list of available export formats
	const char* s_pluginName = "ranganath_murali's EAE6320 Mesh Format";
	const char* s_pluginName2 = "ranganath_murali's EAE6320 Collision Data Format";
	const char* s_pluginName3 = "ranganath_murali's EAE6320 Octree Data Format";
}

// Entry Point
//============

__declspec(dllexport) MStatus initializePlugin( MObject io_object )
{
	// Create a plugin function set
	MFnPlugin plugin( io_object );

	// Register the exporter
	MStatus status;
	{
		char* noIcon = "none";
		status = plugin.registerFileTranslator( s_pluginName, noIcon,
			// This function is what Maya should call to create a new instance of the mesh exporter
			eae6320::cMayaMeshExporter::Create );
		if ( !status )
		{
			MGlobal::displayError( MString( "Failed to register mesh exporter: " ) + status.errorString() );
		}
	}

	MStatus status2;
	{
		char* noIcon = "none";
		status2 = plugin.registerFileTranslator(s_pluginName2, noIcon, eae6320::cMayaCollisionDataExporter::Create);
		if (!status2)
		{
			MGlobal::displayError(MString("Failed to register Collision Data exporter: ") + status2.errorString());
		}
	}
	MStatus status3;
	{
		char* noIcon = "none";
		status3 = plugin.registerFileTranslator(s_pluginName3, noIcon, eae6320::cMayaOctTreeExporter::Create);
		if (!status3)
		{
			MGlobal::displayError(MString("Failed to register Octree Data exporter: ") + status3.errorString());
		}
	}
    return status3;
}

__declspec(dllexport) MStatus uninitializePlugin( MObject io_object )
{
	// Create a plugin function set
	MFnPlugin plugin( io_object );

	// Register the exporter
	MStatus status;
	{
		status = plugin.deregisterFileTranslator( s_pluginName );
		if ( !status )
		{
			MGlobal::displayError( MString( "Failed to deregister mesh exporter: " ) + status.errorString() );
		}
	}

	MStatus status2;
	{
		status2 = plugin.deregisterFileTranslator(s_pluginName2);
		if (!status2)
		{
			MGlobal::displayError(MString("Failed to deregister Collision Data exporter: ") + status2.errorString());
		}
	}
	MStatus status3;
	{
		status3 = plugin.deregisterFileTranslator(s_pluginName3);
		if (!status3)
		{
			MGlobal::displayError(MString("Failed to deregister Octree Data exporter: ") + status3.errorString());
		}
	}
    return status3;
}
