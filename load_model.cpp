#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iostream>
#include <stack>
#include <filesystem>
#include <utility>

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

void serialize(
	Scene* scene,
	std::string out_path
) {

	// open the file
	std::ofstream file(
		out_path.c_str(),
		std::ofstream::binary | std::ofstream::out | std::ofstream::trunc
	);

	// serialize meshes
	uint16_t num_meshes = scene->meshes.size();
	file.write(reinterpret_cast<char*>(&num_meshes), sizeof(uint16_t));
	for (int i = 0; i < num_meshes; i++) {
		scene->meshes[i].serialize(file);
	}

	// serialize meshes with normal map
	uint16_t num_meshes_with_normal_map = scene->meshes_with_normal_map.size();
	file.write(reinterpret_cast<char*>(&num_meshes_with_normal_map), sizeof(uint16_t));
	for (int i = 0; i < num_meshes_with_normal_map; i++) {
		scene->meshes_with_normal_map[i].serialize(file);
	}

	// serialize textures
	uint16_t num_textures = scene->textures.size();
	file.write(reinterpret_cast<char*>(&num_textures), sizeof(uint16_t));
	for (int i = 0; i < num_textures; i++) {
		scene->textures[i].serialize(file);
	}

	// serialize debug_nodes
	uint16_t num_debugs = scene->debug_node_names.size();
	file.write(reinterpret_cast<char*>(&num_debugs), sizeof(uint16_t));
	for (int i = 0; i < num_debugs; i++) {
		uint16_t str_size = scene->debug_node_names[i].size();
		file.write(reinterpret_cast<char*>(&str_size), sizeof(uint16_t));
		file.write(scene->debug_node_names[i].c_str(), str_size);
	}

	// close the file
	file.close();
}

void deserialize(
	Scene* scene,
	std::string bin_path
) {

	// open the file
	std::ifstream file(
		bin_path.c_str(),
		std::ifstream::in | std::ifstream::binary
	);

	// deserialize meshes
	uint16_t num_meshes;
	file.read(reinterpret_cast<char*>(&num_meshes), sizeof(uint16_t));
	scene->meshes.resize(num_meshes);
	for (int i = 0; i < num_meshes; i++) {
		scene->meshes[i].deserialize(file);
	}

	// deserialize meshes with normal map
	uint16_t num_meshes_with_normal_map;
	file.read(reinterpret_cast<char*>(&num_meshes_with_normal_map), sizeof(uint16_t));
	scene->meshes_with_normal_map.resize(num_meshes_with_normal_map);
	for (int i = 0; i < num_meshes_with_normal_map; i++) {
		scene->meshes_with_normal_map[i].deserialize(file);
	}

	// deserialize textures
	uint16_t num_textures;
	file.read(reinterpret_cast<char*>(&num_textures), sizeof(uint16_t));
	scene->textures.resize(num_textures);
	for (int i = 0; i < num_textures; i++) {
		scene->textures[i].deserialize(file);
	}

	// deserialize debug_nodes
	uint16_t num_debugs;
	file.read(reinterpret_cast<char*>(&num_debugs), sizeof(uint16_t));
	scene->debug_node_names.resize(num_debugs);
	for (int i = 0; i < num_debugs; i++) {
		uint16_t str_size;
		file.read(reinterpret_cast<char*>(&str_size), sizeof(uint16_t));
		scene->debug_node_names[i].resize(str_size);
		file.read(&scene->debug_node_names[i][0], str_size);
	}

	// close the file
	file.close();
}

void load_meshes_and_textures_obj(
	Scene* scene,
	std::string obj_path,
	std::string mtl_path
) {

	// check for serialized file
	std::string bin_path = obj_path.substr(0, obj_path.length() - 4) + ".bin";
	if (std::filesystem::exists(bin_path)) {
		deserialize(scene, bin_path);
		return;
	}

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
				scene->debug_node_names.push_back(object + "_" + material);
				materials.push_back(material);
			}
			object = line.substr(2);
		}
		if (line[0] == 'u') {
			if (!temp.empty()) {
				faces.push_back(temp);
				temp.clear();
				scene->debug_node_names.push_back(object + "_" + material);
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

	// load the textures and normal maps
	std::unordered_map<std::string, std::pair<int, int>> material_mapping;
	std::string diffuse_texture_file;
	std::string normal_map_file;
	std::string material_name;
	while (!file.eof()) {
		std::string line;
		std::getline(file, line);
		if (line[0] == 'n') {
			if (!material_name.empty()) {
				if (!diffuse_texture_file.empty()) {
					material_mapping[material_name].first = scene->textures.size();
					std::filesystem::path correct_texture_path =
						std::filesystem::path(folder_path).append(diffuse_texture_file);
					scene->textures.emplace_back(correct_texture_path.string());
				} else material_mapping[material_name].first = -1;
				if (!normal_map_file.empty()) {
					material_mapping[material_name].second = scene->normal_maps.size();
					std::filesystem::path correct_normal_map_path =
						std::filesystem::path(folder_path).append(normal_map_file);
					scene->normal_maps.emplace_back(correct_normal_map_path.string());
				} else material_mapping[material_name].second = -1;
				diffuse_texture_file.clear();
				normal_map_file.clear();
			}
			material_name = line.substr(7);
		}
		if (line.substr(0, 6) == "map_Kd") diffuse_texture_file = line.substr(7);
		if (line.substr(0, 8) == "map_Bump") normal_map_file = line.substr(9);
	}
	file.close();
	if (!diffuse_texture_file.empty()) {
		material_mapping[material_name].first = scene->textures.size();
		std::filesystem::path correct_texture_path =
			std::filesystem::path(folder_path).append(diffuse_texture_file);
		scene->textures.emplace_back(correct_texture_path.string());
	}
	else material_mapping[material_name].first = -1;
	if (!normal_map_file.empty()) {
		material_mapping[material_name].second = scene->normal_maps.size();
		std::filesystem::path correct_normal_map_path =
			std::filesystem::path(folder_path).append(normal_map_file);
		scene->normal_maps.emplace_back(correct_normal_map_path.string());
	}
	else material_mapping[material_name].second = -1;

	// create meshes
	int vertex_offset = 0;
	int index_offset = 0;
	int n_vertex_offset = 0;
	int n_index_offset = 0;
	for (int i = 0; i < faces.size(); i++) {

		// skip the meshes without a texture
		if (material_mapping.find(materials[i]) == material_mapping.end() ||
			material_mapping[materials[i]].first == -1) continue;

		// if this mesh doesn't have a normal map
		if (material_mapping[materials[i]].second == -1) {
			
			// instantiate the mesh
			Mesh mesh = Mesh();
			mesh.init_transform = glm::mat4(1.0f);
			mesh.index_offset = index_offset;
			mesh.vertex_offset = vertex_offset;
			mesh.texture_index = material_mapping[materials[i]].first;
			mesh.debug_node_name = scene->debug_node_names[i];

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
					}
					else indices.push_back(unique_vertices[face_vertices[k]]);
				}
				if (indices.size() == 4) {
					std::vector<uint32_t> triangles = { indices[0], indices[1], indices[2], indices[2], indices[3], indices[0] };
					mesh.indices.insert(mesh.indices.end(), triangles.begin(), triangles.end());
				}
				else if (indices.size() == 3) {
					mesh.indices.insert(mesh.indices.end(), indices.begin(), indices.end());
				}
			}

			// update offsets
			vertex_offset += mesh.vertices.size();
			index_offset += mesh.indices.size();

			// done loading this mesh
			scene->meshes.push_back(mesh);
		} else {
			
			// instantiate the mesh
			MeshWithNormalMap mesh = MeshWithNormalMap();
			mesh.init_transform = glm::mat4(1.0f);
			mesh.index_offset = n_index_offset;
			mesh.vertex_offset = n_vertex_offset;
			mesh.texture_index = material_mapping[materials[i]].first;
			mesh.normal_map_index = material_mapping[materials[i]].second;
			mesh.debug_node_name = scene->debug_node_names[i];

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
			n_vertex_offset += mesh.vertices.size();
			n_index_offset += mesh.indices.size();

			// done loading this mesh
			scene->meshes_with_normal_map.push_back(mesh);
		}

		// serialize the model for faster loading next time
		serialize(scene, bin_path);
	}
}