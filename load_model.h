#pragma once

#include <vector>
#include <string>
#include <assimp/scene.h>

#include "vertex.h"
#include "scene.h"

// load meshes and textures from file
void load_meshes_and_textures(std::vector<Mesh>& meshes, std::vector<Texture>& textures, std::vector<std::string>& debug_nodes, std::string file_path);

// print node structure for debugging
void print_node_structure(std::string file_path);

// debug load group275
void load_group275_debug(std::vector<Mesh>& meshes, std::vector<std::string>& debug_nodes, std::string file_path);

// load obj from scratch
void load_meshes_and_textures_obj(
	std::vector<Mesh>& meshes,
	std::vector<Texture>& textures,
	std::vector<std::string>& debug_nodes,
	std::string obj_path,
	std::string mtl_path
);