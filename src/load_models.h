#pragma once

#include <vector>
#include <string>

#include "vertex.h"

struct model {
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
};

model load_model(std::string file);

glm::vec3 parse_coordinates(std::string line, int start);

std::vector<std::string> parse_face(std::string line);

std::string parse_index_block(std::string block);

std::vector<std::string> split(std::string s, char delimiter);