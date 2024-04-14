#pragma once

#include <vector>
#include <string>

#include "vertex.h"
#include "scene.h"

// load obj from scratch
void load_meshes_and_textures_obj(
	Scene* scene,
	std::string obj_path,
	std::string mtl_path
);