#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iostream>
#include <stack>
#include <filesystem>

#include "math.h"
#include "load_model.h"

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

void load_meshes_and_textures(std::vector<Mesh>& meshes, std::vector<Texture>& textures, std::vector<std::string>& debug_nodes, std::string file_path) {
	/*
	Using Assimp to load all the meshes and textures
	*/

	bool debug_using_triangulate = true;

	// get folder path
	std::filesystem::path file_path_ = std::filesystem::path(file_path);
	std::filesystem::path folder_path = file_path_.parent_path();

	// load the file
	Assimp::Importer importer;
	const aiScene* scene;
	if (debug_using_triangulate) {
		scene = importer.ReadFile(file_path, aiProcess_Triangulate);
	}
	else {
		scene = importer.ReadFile(file_path, 0);
	}
	if (scene == NULL) throw std::runtime_error("Failed to load the file");

	// prepare to loop through all nodes
	std::stack<aiNode*> nodes_stack;
	std::stack<aiMatrix4x4> transforms_stack;
	nodes_stack.push(scene->mRootNode);
	transforms_stack.push(aiMatrix4x4());
	int i_offset = 0;
	int v_offset = 0;

	// use a hash map to check for loaded texture
	std::unordered_map<std::string, int> texture_index;

	// use a hash map to check for loaded debug nodes
	std::unordered_set<std::string> debug_nodes_index;

	// loop through all the meshes
	while (!nodes_stack.empty()) {

		// current node
		aiNode* node = nodes_stack.top();
		nodes_stack.pop();

		// current transform
		aiMatrix4x4 transform = transforms_stack.top();
		transforms_stack.pop();
		transform = transform * node->mTransformation;

		// use this flag to only add one node to the debug node list
		bool debug_added = false;

		// for each mesh in the node
		for (int i = 0; i < node->mNumMeshes; i++) {
			
			// mesh node
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

			// TODO: handle meshes without diffuse color texture
			// check if this mesh has a texture
			aiString texture_path;
			scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texture_path);
			if (texture_path.length > 0) {
				if (texture_index.find(texture_path.C_Str()) == texture_index.end()) {
					texture_index[texture_path.C_Str()] = textures.size();
					std::filesystem::path correct_texture_path = std::filesystem::path(folder_path).append(texture_path.C_Str());
					textures.emplace_back(correct_texture_path.string());
				}
			} else {
				continue;
			}

			// add the node to the debug list
			if (!debug_added) {
				debug_added = true;
				if (debug_nodes_index.find(std::string(node->mName.C_Str())) == debug_nodes_index.end()) {
					debug_nodes_index.insert(node->mName.C_Str());
					debug_nodes.push_back(node->mName.C_Str());
				}
			}

			// instantiate a Mesh
			Mesh loaded_mesh = Mesh();
			loaded_mesh.index_offset = i_offset;
			loaded_mesh.vertex_offset = v_offset;
			loaded_mesh.vertices.reserve(mesh->mNumVertices);
			loaded_mesh.texture_index = texture_index[texture_path.C_Str()];
			loaded_mesh.debug_node_name = node->mName.C_Str();
			v_offset += mesh->mNumVertices;

			// read vertices
			for (int j = 0; j < mesh->mNumVertices; j++) {
				aiVector3D position = mesh->mVertices[j];
				aiVector3D normal = mesh->mNormals[j];
				aiVector3D uv = mesh->mTextureCoords[0][j]; // TODO: deal with multiple texture coordinates if applicable
				loaded_mesh.vertices.emplace_back(*reinterpret_cast<glm::vec3*>(&position), *reinterpret_cast<glm::vec3*>(&normal), *reinterpret_cast<glm::vec2*>(&uv));
			}

			// read indices
			for (int j = 0; j < mesh->mNumFaces; j++) {
				aiFace face = mesh->mFaces[j];
				if (face.mNumIndices != 3) continue;
				loaded_mesh.indices.insert(loaded_mesh.indices.end(), face.mIndices, face.mIndices + 3);
			}

			// update the index offset
			i_offset += loaded_mesh.indices.size();

			// transform
			loaded_mesh.init_transform = glm::mat4(
				transform.a1, transform.b1, transform.c1, transform.d1,
				transform.a2, transform.b2, transform.c2, transform.d2,
				transform.a3, transform.b3, transform.c3, transform.d3,
				transform.a4, transform.b4, transform.c4, transform.d4
			);
			
			// done reading this mesh
			meshes.push_back(loaded_mesh);
		}

		// traverse the children
		for (int i = node->mNumChildren - 1; i >= 0; i--) {
			nodes_stack.push(node->mChildren[i]);
			transforms_stack.push(transform);
		}
	}
}

void print_node_structure(std::string file_path) {
	
	// load the file
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(file_path, aiProcess_Triangulate);
	if (scene == NULL) throw std::runtime_error("Failed to load the file");

	std::cout << scene->mRootNode->mNumChildren << std::endl;
}