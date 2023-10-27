#pragma once

#include <vector>
#include <string>
#include <assimp/scene.h>

#include "vertex.h"
#include "scene.h"

// load meshes and textures from file
void load_meshes_and_textures(std::vector<Mesh>& meshes, std::vector<Texture>& textures, std::string file_path);

void load_meshes(std::vector<Mesh>& meshes, std::string file_path);