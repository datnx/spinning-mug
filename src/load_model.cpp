#include <fstream>
#include <unordered_map>

#include "load_models.h"

int numTabs = 0;

std::vector<std::string> split(std::string s, char delimiter) {
	std::vector<std::string> splits;
	std::string substring = "";
	for (char c : s) {
		if (c == delimiter) {
			splits.push_back(substring);
			substring = "";
		} else substring.push_back(c);
	}
	splits.push_back(substring);
	return splits;
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
				Vertex vertex;
				std::vector<std::string> pos_nor_indices = split(face_vertices[j], '/');
				vertex.pos = coordinates[std::stoi(pos_nor_indices[0]) - 1];
				vertex.normal = normals[std::stoi(pos_nor_indices[1]) - 1];
				vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);
				loaded_model.vertices.push_back(vertex);
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