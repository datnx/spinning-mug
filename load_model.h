#pragma once

#include <vector>
#include <string>
#include <assimp/scene.h>

#include "vertex.h"
#include "scene.h"

struct model {
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
};

model load_model_fbx(std::string file);

void loadMesh(model& loaded_model, aiNode* node, aiMatrix4x4 transform, const aiScene* scene);

model load_model(std::string file);

glm::vec3 parse_coordinates(std::string line, int start);

std::vector<std::string> parse_face(std::string line);

std::string parse_index_block(std::string block);

void load_meshes(std::vector<Mesh>& meshes, std::string file_path);