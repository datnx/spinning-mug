#include <fstream>
#include <iostream>
#include <glm/glm.hpp>

#include "light.h"
#include "string_utils.h"

light::light() {
	num_unattenuated_point_light = 0;
	num_directional_light = 0;
	num_point_light = 0;
	num_spot_light = 0;
}

void light::load_file(std::string file_path) {
	std::ifstream file;

	file.open(file_path);

	if (file.fail()) {
		std::string fail_message = "failed to open " + file_path;
		throw std::runtime_error("failed to open file");
	}

	while (!file.eof()) {
		std::string line;
		std::getline(file, line);
		if (line.substr(0, 3) == "pna") {
			num_unattenuated_point_light += 1;
			std::vector<std::string> values = split(line, ' ');
			int light_index = num_unattenuated_point_light - 1;
			unattenuated_point_light[light_index].pos.x = std::stof(values[1]);
			unattenuated_point_light[light_index].pos.y = std::stof(values[2]);
			unattenuated_point_light[light_index].pos.z = std::stof(values[3]);
			unattenuated_point_light[light_index].col.r = std::stof(values[4]);
			unattenuated_point_light[light_index].col.g = std::stof(values[5]);
			unattenuated_point_light[light_index].col.b = std::stof(values[6]);
		}
		if (line.substr(0, 3) == "dir") {
			num_directional_light += 1;
			std::vector<std::string> values = split(line, ' ');
			int light_index = num_directional_light - 1;
			directional_light[light_index].dir.x = std::stof(values[1]);
			directional_light[light_index].dir.y = std::stof(values[2]);
			directional_light[light_index].dir.z = std::stof(values[3]);
			directional_light[light_index].col.r = std::stof(values[4]);
			directional_light[light_index].col.g = std::stof(values[5]);
			directional_light[light_index].col.b = std::stof(values[6]);
		}
		if (line.substr(0, 3) == "pwa") {
			num_point_light += 1;
			std::vector<std::string> values = split(line, ' ');
			int light_index = num_point_light - 1;
			point_light[light_index].pos.x = std::stof(values[1]);
			point_light[light_index].pos.y = std::stof(values[2]);
			point_light[light_index].pos.z = std::stof(values[3]);
			point_light[light_index].col.r = std::stof(values[4]);
			point_light[light_index].col.g = std::stof(values[5]);
			point_light[light_index].col.b = std::stof(values[6]);
			point_light[light_index].falloff = std::stof(values[7]);
		}
		if (line.substr(0, 3) == "spo") {
			num_spot_light += 1;
			std::vector<std::string> values = split(line, ' ');
			int light_index = num_spot_light - 1;
			spot_light[light_index].pos.x = std::stof(values[1]);
			spot_light[light_index].pos.y = std::stof(values[2]);
			spot_light[light_index].pos.z = std::stof(values[3]);
			spot_light[light_index].dir.x = std::stof(values[4]) - spot_light[light_index].pos.x;
			spot_light[light_index].dir.y = std::stof(values[5]) - spot_light[light_index].pos.y;
			spot_light[light_index].dir.z = std::stof(values[6]) - spot_light[light_index].pos.z;
			spot_light[light_index].col.r = std::stof(values[7]);
			spot_light[light_index].col.g = std::stof(values[8]);
			spot_light[light_index].col.b = std::stof(values[9]);
			spot_light[light_index].cos_p = cos(glm::radians(std::stof(values[10])));
			spot_light[light_index].cos_u = cos(glm::radians(std::stof(values[11])));
			spot_light[light_index].falloff = std::stof(values[12]);
		}
	}
	file.close();
}