#pragma once

#include <vector>
#include <string>
#include <assimp/scene.h>

#include "vertex.h"
#include "scene.h"

// print node structure for debugging
void print_node_structure(std::string file_path);

// load obj from scratch
void load_meshes_and_textures_obj(
	std::vector<Mesh>& meshes,
	std::vector<Texture>& textures,
	std::vector<std::string>& debug_nodes,
	std::string obj_path,
	std::string mtl_path
);