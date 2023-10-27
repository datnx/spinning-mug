#include <fstream>
#include <unordered_map>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iostream>
#include <stack>

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

void load_meshes_and_textures(std::vector<Mesh>& meshes, std::vector<Texture>& textures, std::string file_path) {
	/*
	Using Assimp to load all the meshes and textures
	*/

	// load the file
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(file_path, aiProcess_Triangulate);
	if (scene == NULL) throw std::runtime_error("Failed to load the file");

	// prepare to loop through all nodes
	std::stack<aiNode*> nodes_stack;
	std::stack<aiMatrix4x4> transforms_stack;
	nodes_stack.push(scene->mRootNode);
	transforms_stack.push(aiMatrix4x4());
	int i_offset = 0;
	int v_offset = 0;

	// set the vertex color to white for now
	glm::vec4 white = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	// loop
	while (!nodes_stack.empty()) {

		// current node
		aiNode* node = nodes_stack.top();
		nodes_stack.pop();

		// current transform
		aiMatrix4x4 transform = transforms_stack.top();
		transforms_stack.pop();
		transform = transform * node->mTransformation;

		// for each mesh in the node
		for (int i = 0; i < node->mNumMeshes; i++) {
			
			// mesh node
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			
			// instantiate a Mesh
			Mesh loaded_mesh = Mesh();
			loaded_mesh.index_offset = i_offset;
			loaded_mesh.vertex_offset = v_offset;
			loaded_mesh.vertices.reserve(mesh->mNumVertices);
			loaded_mesh.indices.reserve(mesh->mNumFaces * 3);
			i_offset += mesh->mNumFaces * 3;
			v_offset += mesh->mNumVertices;

			// read vertices
			for (int j = 0; j < mesh->mNumVertices; j++) {
				aiVector3D position = mesh->mVertices[j];
				aiVector3D normal = mesh->mNormals[j];
				aiVector3D uv = mesh->mTextureCoords[0][j]; // TODO: deal with multiple texture coordinates if applicable
				loaded_mesh.vertices.emplace_back(*reinterpret_cast<glm::vec3*>(&position), *reinterpret_cast<glm::vec3*>(&normal), white, *reinterpret_cast<glm::vec2*>(&uv));
			}

			// read indices
			for (int j = 0; j < mesh->mNumFaces; j++) {
				aiFace face = mesh->mFaces[j];
				if (face.mNumIndices != 3) throw std::runtime_error("This is not a triangle");
				loaded_mesh.indices.insert(loaded_mesh.indices.end(), face.mIndices, face.mIndices + 3);
			}

			// transform
			loaded_mesh.init_transform = glm::mat4(
				transform.a1, transform.b1, transform.c1, transform.d1,
				transform.a2, transform.b2, transform.c2, transform.d2,
				transform.a3, transform.b3, transform.c3, transform.d3,
				transform.a4, transform.b4, transform.c4, transform.d4
			);

			// material name and debug, will be removed in the future
			loaded_mesh.material_name = scene->mMaterials[mesh->mMaterialIndex]->GetName().C_Str();
			loaded_mesh.debug = node->mName.C_Str();
			
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