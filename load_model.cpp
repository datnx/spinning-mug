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

void print_node_structure(std::string file_path) {
	
	// load the file
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(file_path, aiProcess_Triangulate);
	if (scene == NULL) throw std::runtime_error("Failed to load the file");

	std::cout << scene->mRootNode->mNumChildren << std::endl;
}

glm::vec3 parse_coordinates(std::string line, int start) {
	std::vector<std::string> coordinates = split(line.substr(start), ' ');
	return glm::vec3(std::stof(coordinates[0]), std::stof(coordinates[1]), std::stof(coordinates[2]));
}

glm::vec2 parse_uv(std::string line) {
	std::vector<std::string> values = split(line.substr(3), ' ');
	return glm::vec2(std::stof(values[0]), std::stof(values[1]));
}

void load_meshes_and_textures_obj(
	std::vector<Mesh>& meshes,
	std::vector<Texture>& textures,
	std::vector<std::string>& debug_nodes,
	std::string obj_path,
	std::string mtl_path
) {

	// load the file
	std::ifstream file;
	file.open(obj_path);
	if (file.fail()) {
		std::string fail_message = "failed to open " + obj_path;
		throw std::runtime_error(fail_message);
	}

	std::vector<glm::vec3> coordinates;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uv;
	std::vector<std::vector<std::string>> faces;
	std::string object;
	std::string material;
	std::vector<std::string> materials;
	std::vector<std::string> temp;

	// read line by line
	while (!file.eof()) {
		std::string line;
		std::getline(file, line);
		if (line[0] == 'v' && line[1] == ' ') coordinates.push_back(parse_coordinates(line, 2));
		if (line[0] == 'v' && line[1] == 'n') normals.push_back(parse_coordinates(line, 3));
		if (line[0] == 'v' && line[1] == 't') uv.push_back(parse_uv(line));
		if (line[0] == 'o') {
			if (!temp.empty()) {
				faces.push_back(temp);
				temp.clear();
				debug_nodes.push_back(object + "_" + material);
				materials.push_back(material);
			}
			object = line.substr(2);
		}
		if (line[0] == 'u') {
			if (!temp.empty()) {
				faces.push_back(temp);
				temp.clear();
				debug_nodes.push_back(object + "_" + material);
				materials.push_back(material);
			}
			material = line.substr(7);
		}
		if (line[0] == 'f') temp.push_back(line.substr(2));
	}
	file.close();

	// get folder path
	std::filesystem::path file_path_ = std::filesystem::path(obj_path);
	std::filesystem::path folder_path = file_path_.parent_path();

	// open mtl file
	file.open(mtl_path);
	if (file.fail()) {
		std::string fail_message = "failed to open " + mtl_path;
		throw std::runtime_error(fail_message);
	}

	// load the textures
	std::unordered_map<std::string, int> material_mapping;
	std::string diffuse_texture_file;
	std::string material_name;
	while (!file.eof()) {
		std::string line;
		std::getline(file, line);
		if (line[0] == 'n') {
			if (!diffuse_texture_file.empty()) {
				material_mapping[material_name] = textures.size();
				std::filesystem::path correct_texture_path =
					std::filesystem::path(folder_path).append(diffuse_texture_file);
				textures.emplace_back(correct_texture_path.string());
			}
			material_name = line.substr(7);
			diffuse_texture_file.clear();
		}
		if (line.substr(0, 6) == "map_Kd") diffuse_texture_file = line.substr(7);
	}
	file.close();

	// create meshes
	int vertex_offset = 0;
	int index_offset = 0;
	for (int i = 0; i < faces.size(); i++) {

		// skip the meshes without a texture
		if (material_mapping.find(materials[i]) == material_mapping.end()) {
			continue;
		}

		// skip if the mesh is not group135
		//if (debug_nodes[i] != "group135_materials") continue;

		// instantiate the mesh
		Mesh mesh = Mesh();
		mesh.init_transform = glm::mat4(1.0f);
		mesh.index_offset = index_offset;
		mesh.vertex_offset = vertex_offset;
		mesh.texture_index = material_mapping[materials[i]];
		mesh.debug_node_name = debug_nodes[i];

		// load vertices and indices
		int index = 0;
		std::unordered_map<std::string, int> unique_vertices;
		for (int j = 0; j < faces[i].size(); j++) {
			std::vector<std::string> face_vertices = split(faces[i][j], ' ');
			std::vector<uint32_t> indices;
			for (int k = 0; k < face_vertices.size(); k++) {
				if (unique_vertices.find(face_vertices[k]) == unique_vertices.end()) {
					unique_vertices[face_vertices[k]] = index;
					indices.push_back(index);
					std::vector<std::string> pos_uv_nor_indices = split(face_vertices[k], '/');
					mesh.vertices.emplace_back(
						coordinates[std::stoi(pos_uv_nor_indices[0]) - 1],
						normals[std::stoi(pos_uv_nor_indices[2]) - 1],
						uv[std::stoi(pos_uv_nor_indices[1]) - 1]
					);
					index++;
				} else indices.push_back(unique_vertices[face_vertices[k]]);
			}
			if (indices.size() == 4) {
				std::vector<uint32_t> triangles = { indices[0], indices[1], indices[2], indices[2], indices[3], indices[0] };
				mesh.indices.insert(mesh.indices.end(), triangles.begin(), triangles.end());
			} else if (indices.size() == 3) {
				mesh.indices.insert(mesh.indices.end(), indices.begin(), indices.end());
			}
		}

		// update offsets
		vertex_offset += mesh.vertices.size();
		index_offset += mesh.indices.size();

		// done loading this mesh
		meshes.push_back(mesh);
	}
}