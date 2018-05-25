/*
	This is the fragment shader used to render meshes
*/

#if defined( EAE6320_PLATFORM_D3D )

// Constants
//==========

cbuffer constantBuffer_frame : register( b0 )
{
	float4x4 g_transform_worldToCamera;
	float4x4 g_transform_cameraToScreen;
	float g_elapsedSecondCount_total;
}

cbuffer constantBuffer_material : register( b2 ) 
{
	float4 g_color;	
}

Texture2D textureObject : register( t0 );
SamplerState samplerState : register( s0 );

// Entry Point
//============

void main(

	// Input
	//======

	// The GPU provides us with position
	in float4 i_position : SV_POSITION,

	// Whatever arbitrary data (i.e. everything excluding position) was output from the vertex shader
	// will be interpolated across the triangle and given as input to the fragment shader
	in float4 i_color : COLOR,
	in float2 i_textureCoordinates : TEXCOORD0,
	
	// Output
	//=======

	// Whatever color value is output from the fragment shader
	// will determine the color of the corresponding pixel on the screen
	out float4 o_color : SV_TARGET

	)
	
	{
		float4 sampledColor = textureObject.Sample( samplerState, i_textureCoordinates );
	
		// For now set the fragment as the interpolated color
		o_color = i_color * g_color * sampledColor;
	}

#elif defined( EAE6320_PLATFORM_GL )

// The version of GLSL to use must come first
#version 420

// Constants
//==========

layout( std140, binding = 0 ) uniform constantBuffer_frame
{
	mat4 g_transform_worldToCamera;
	mat4 g_transform_cameraToScreen;
	float g_elapsedSecondCount_total;
};

layout( std140, binding = 2) uniform constantBuffer_material
{
	vec4 g_color;
};

// Input
//======

// Whatever arbitrary data (i.e. everything excluding position) was output from the vertex shader
// will be interpolated across the triangle and given as input to the fragment shader

layout( location = 0 ) in vec4 i_color;
layout( location = 2 ) in vec2 i_textureCoordinates;
layout( binding = 0 ) uniform sampler2D textureObject;

// Output
//=======

// Whatever color value is output from the fragment shader
// will determine the color of the corresponding pixel on the screen
out vec4 o_color;

// Entry Point
//============

void main()
{
	vec4 sampledColor = texture2D(textureObject, i_textureCoordinates);
	
	// For now set the fragment as the interpolated color
	o_color = i_color * g_color * sampledColor;
}

#endif
