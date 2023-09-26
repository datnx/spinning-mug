#include <fstream>
#include <unordered_map>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iostream>
#include <stack>

#include "fbx_utils.h"
#include "math.h"
#include "load_model.h"
#include "string_utils.h"

void print_matrix(aiMatrix4x4 m) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			std::cout << m[i][j] << " ";
		}
		std::cout << std::endl;
	}
}

void print_color(aiColor3D c) {
	std::cout << "(" << c.r << ", " << c.g << ", " << c.b << ")";
}

model load_model_fbx(std::string file) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate);

	std::cout << "num textures: " << scene->mNumTextures << std::endl;

	if (scene == NULL) throw std::runtime_error("Failed to load the file");
	if (scene->mNumAnimations != 1) throw std::runtime_error("For now, only 1 animation is supported");

	aiNode* root = scene->mRootNode;

	model loaded_model;
	loadMesh(loaded_model, root, aiMatrix4x4(), scene);

	return loaded_model;
}

void loadMesh(model& loaded_model, aiNode* node, aiMatrix4x4 transform, const aiScene* scene) {

	aiAnimation* animation = scene->mAnimations[0];
	bool animated = false;
	if (!animated) transform = transform * node->mTransformation;
	
	if (node->mNumMeshes > 0) {
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			uint16_t offset = (uint16_t)loaded_model.vertices.size();
			std::cout << node->mName.C_Str() << std::endl;
			print_matrix(transform);
			for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
				aiVector3D position = mesh->mVertices[j];
				aiVector3D normal = mesh->mNormals[j];
				glm::vec3 pos = mul(transform, position);
				glm::vec3 nor = mul(
					aiMatrix3x3(
						transform.a1, transform.a2, transform.a3,
						transform.b1, transform.b2, transform.b3,
						transform.c1, transform.c2, transform.c3
					).Inverse().Transpose(),
					normal
				);
				glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				loaded_model.vertices.push_back(Vertex(pos, nor, color, glm::vec2()));
			}
			for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
				aiFace face = mesh->mFaces[j];
				if (face.mNumIndices != 3) throw std::runtime_error("This is not a triangle");
				uint16_t first_index = face.mIndices[0] + offset;
				uint16_t second_index = face.mIndices[1] + offset;
				uint16_t third_index = face.mIndices[2] + offset;
				std::vector<uint16_t> triangle = {first_index, second_index, third_index};
				loaded_model.indices.insert(loaded_model.indices.end(), triangle.begin(), triangle.end());
			}
		}
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		loadMesh(loaded_model, node->mChildren[i], transform, scene);
	}
}

glm::vec3 parse_coordinates(std::string line, int start) {
	std::vector<std::string> coordinates = split(line.substr(start), ' ');
	return glm::vec3(std::stof(coordinates[0]), std::stof(coordinates[1]), std::stof(coordinates[2]));
}

std::string parse_index_block(std::string block) {
	std::string vertex;
	bool read = true;
	for (int i = 0; i < block.length(); i++) {
		if (read) vertex.push_back(block[i]);
		if (block[i] == '/') read = !read;
	}
	return vertex;
}

std::vector<std::string> parse_face(std::string line) {
	std::vector<std::string> all_vertices = split(line.substr(2), ' ');
	for (int i = 0; i < all_vertices.size(); i++) all_vertices[i] = parse_index_block(all_vertices[i]);
	return all_vertices;
}

model load_model(std::string file_path) {
	std::ifstream file;

	file.open(file_path);
	
	if (file.fail()) {
		std::string fail_message = "failed to open " + file_path;
		throw std::runtime_error("failed to open file");
	}

	std::vector<glm::vec3> coordinates;
	std::vector<glm::vec3> normals;
	std::vector<std::string> faces;
	while (!file.eof()) {
		std::string line;
		std::getline(file, line);
		if (line[0] == 'v' && line[1] == ' ') coordinates.push_back(parse_coordinates(line, 2));
		if (line[0] == 'v' && line[1] == 'n') normals.push_back(parse_coordinates(line, 3));
		if (line[0] == 'f') faces.push_back(line);
	}
	file.close();

	model loaded_model;
	int index = 0;
	std::unordered_map<std::string, int> unique_vertices;
	for (int i = 0; i < faces.size(); i++) {
		std::vector<std::string> face_vertices = parse_face(faces[i]);
		std::vector<uint16_t> indices;
		for (int j = 0; j < face_vertices.size(); j++) {
			if (unique_vertices.find(face_vertices[j]) == unique_vertices.end()) {
				unique_vertices[face_vertices[j]] = index;
				indices.push_back(index);
				std::vector<std::string> pos_nor_indices = split(face_vertices[j], '/');
				glm::vec3 pos = coordinates[std::stoi(pos_nor_indices[0]) - 1];
				glm::vec3 normal = normals[std::stoi(pos_nor_indices[1]) - 1];
				glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				loaded_model.vertices.push_back(Vertex(pos, normal, color, glm::vec2()));
				index++;
			}
			else indices.push_back(unique_vertices[face_vertices[j]]);
		}
		if (indices.size() == 4) {
			std::vector<uint16_t> triangles = {indices[0], indices[1], indices[2], indices[2], indices[3], indices[0]};
			loaded_model.indices.insert(loaded_model.indices.end(), triangles.begin(), triangles.end());
		}
	}

	return loaded_model;
}

void load_meshes(std::vector<Mesh>& meshes, std::string file_path) {
	/*
	Using Assimp to load all the meshes from file_path.
	*/

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(file_path, aiProcess_Triangulate);
	if (scene == NULL) throw std::runtime_error("Failed to load the file");
	if (scene->mNumAnimations != 1) throw std::runtime_error("For now, only 1 animation is supported");
	std::stack<aiNode*> nodes_stack;
	std::stack<aiMatrix4x4> transforms_stack;
	nodes_stack.push(scene->mRootNode);
	transforms_stack.push(aiMatrix4x4());
	glm::vec4 white = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	int i_offset = 0;
	int v_offset = 0;
	while (!nodes_stack.empty()) {
		aiNode* node = nodes_stack.top();
		nodes_stack.pop();
		aiMatrix4x4 transform = transforms_stack.top();
		transforms_stack.pop();
		transform = transform * node->mTransformation;
		for (int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			Mesh loaded_mesh = Mesh();
			loaded_mesh.index_offset = i_offset;
			loaded_mesh.vertex_offset = v_offset;
			loaded_mesh.vertices.reserve(mesh->mNumVertices);
			loaded_mesh.indices.reserve(mesh->mNumFaces * 3);
			i_offset += mesh->mNumFaces * 3;
			v_offset += mesh->mNumVertices;
			for (int j = 0; j < mesh->mNumVertices; j++) {
				aiVector3D position = mesh->mVertices[j];
				aiVector3D normal = mesh->mNormals[j];
				aiVector3D uv = mesh->mTextureCoords[0][j];
				loaded_mesh.vertices.emplace_back(*reinterpret_cast<glm::vec3*>(&position), *reinterpret_cast<glm::vec3*>(&normal), white, *reinterpret_cast<glm::vec2*>(&uv));
			}
			for (int j = 0; j < mesh->mNumFaces; j++) {
				aiFace face = mesh->mFaces[j];
				if (face.mNumIndices != 3) throw std::runtime_error("This is not a triangle");
				loaded_mesh.indices.insert(loaded_mesh.indices.end(), face.mIndices, face.mIndices + 3);
			}
			loaded_mesh.init_transform = glm::mat4(
				transform.a1, transform.b1, transform.c1, transform.d1,
				transform.a2, transform.b2, transform.c2, transform.d2,
				transform.a3, transform.b3, transform.c3, transform.d3,
				transform.a4, transform.b4, transform.c4, transform.d4
			);
			loaded_mesh.material_name = scene->mMaterials[mesh->mMaterialIndex]->GetName().C_Str();
			loaded_mesh.debug = node->mName.C_Str();
			meshes.push_back(loaded_mesh);
		}
		for (int i = node->mNumChildren - 1; i >= 0; i--) {
			nodes_stack.push(node->mChildren[i]);
			transforms_stack.push(transform);
		}
	}
}