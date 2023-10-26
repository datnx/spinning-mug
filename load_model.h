#pragma once

#include <vector>
#include <string>
#include <assimp/scene.h>

#include "vertex.h"
#include "scene.h"

void load_meshes(std::vector<Mesh>& meshes, std::string file_path);