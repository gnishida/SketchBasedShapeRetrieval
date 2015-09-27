#pragma once

#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "Vertex.h"

struct OBJ_Vertex {
	int v_index;
	int n_index;
	int t_index;

	OBJ_Vertex() : v_index(-1), n_index(-1), t_index(-1) {}
};

class OBJLoader {
protected:
	OBJLoader() {}

public:
	static void load(const std::string& filename, std::vector<Vertex>& vertices);
	static bool load(const char* filename, std::vector<std::vector<glm::vec3> >& points, std::vector<std::vector<glm::vec3> >& normals, std::vector<std::vector<glm::vec2> >& texCoords);
};

