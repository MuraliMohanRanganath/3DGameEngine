--[[
	This file lists every asset that must be built by the AssetBuildSystem
]]

return
{
	-- The top-level table is a dictionary whose keys
	-- correspond to the asset types (inherited from cbAssetTypeInfo)
	-- that you have defined in AssetBuildSystem.lua

	meshes =
	{
		-- The actual assets can be defined simply as relative path strings (the common case)
		"Meshes/ceiling.mesh",
		"Meshes/floor.mesh",
		"Meshes/metal.mesh",
		"Meshes/props.mesh",
		"Meshes/railing.mesh",
		"Meshes/walls.mesh",
		"Meshes/player.mesh",
		"Meshes/flag.mesh",
	},
	materials =
	{
		"Materials/ceiling.material",
		"Materials/floor.material",
		"Materials/metal.material",
		"Materials/props.material",
		"Materials/railing.material",
		"Materials/walls.material",
		"Materials/debugshape.material",
		"Materials/sprite.material",
		"Materials/red.material",
		"Materials/blue.material",
	},
	textures = 
	{
		"Textures/default.tga",
	},
	collisionData =
	{
		"CollisionData/Scene.cdata",
	},
	octreeData =
	{
		"Scene.octree",
	},
	sounds =
	{
		"Sounds/starwarstheme.wav",
		"Sounds/enemyhascapturedyourflag.wav",
		"Sounds/theenemyhastakenyourflag.wav",
		"Sounds/victory.wav",
		"Sounds/youhavetheflag.wav",
		"Sounds/capturetheenemyflag.wav",
		"Sounds/running-hard-surface.wav",
		"Sounds/running-gravel-or-dry-leaves-loop.wav",
	},
}