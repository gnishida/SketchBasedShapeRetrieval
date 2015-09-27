#include "OBJLoader.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <QString>
#include <QTextStream>
#include <QStringList>
#include <QFile>

/**
 * Load vertices data from a OBJ file.
 */
void OBJLoader::load(const std::string& filename, std::vector<Vertex>& vertices) {
	QFile file(filename.c_str());
	if (!file.open(QIODevice::ReadOnly)) {
		return;
	}

	QTextStream in(&file);
 
	std::vector<glm::vec3> raw_vertices;
	std::vector<glm::vec3> raw_normals;
	std::vector<glm::vec2> raw_texCoords;
	std::vector<std::vector<OBJ_Vertex> > faces;

	int numTriangles = 0;

	while (!in.atEnd()) {
		QString line = in.readLine();
		if (line.startsWith("v ")) {
			int index1 = line.indexOf(QRegExp("[0-9\\.\\-]"), 2);
			int index2 = line.lastIndexOf(QRegExp("[0-9]"));
			QStringList values = line.mid(index1, index2 - index1 + 1).split(" ");
			raw_vertices.push_back(glm::vec3(values[0].toFloat(), values[1].toFloat(), values[2].toFloat()));
		} else if (line.startsWith("vn ")) {
			int index1 = line.indexOf(QRegExp("[0-9\\.\\-]"), 2);
			int index2 = line.lastIndexOf(QRegExp("[0-9]"));
			if (index1 >= 0 && index2 >= 0) {
				QStringList values = line.mid(index1, index2 - index1 + 1).split(" ");
				raw_normals.push_back(glm::vec3(values[0].toFloat(), values[1].toFloat(), values[2].toFloat()));
			}
		} else if (line.startsWith("vt ")) {
			int index1 = line.indexOf(QRegExp("[0-9\\.\\-]"), 2);
			int index2 = line.lastIndexOf(QRegExp("[0-9]"));
			if (index1 >= 0 && index2 >= 0) {
				QStringList values = line.mid(index1, index2 - index1 + 1).split(" ");
				raw_texCoords.push_back(glm::vec2(values[0].toFloat(), values[1].toFloat()));
			}
		} else if (line.startsWith("f ")) {
			int index1 = line.indexOf(QRegExp("[0-9]"), 2);
			int index2 = line.lastIndexOf(QRegExp("[0-9]"));
			QStringList values = line.mid(index1, index2 - index1 + 1).split(" ");
			std::vector<OBJ_Vertex> face(values.size());
			for (int i = 0; i < values.size(); ++i) {
				face[i].v_index = values[i].split("/")[0].toUInt() - 1;
				if (values[i].split("/").size() >= 2 && values[i].split("/")[1].size() > 0) {
					face[i].t_index = values[i].split("/")[1].toUInt() - 1;
				}
				if (values[i].split("/").size() >= 3 && values[i].split("/")[2].size() > 0) {
					face[i].n_index = values[i].split("/")[2].toUInt() - 1;
				}
			}
			faces.push_back(face);
			numTriangles += face.size() - 2;
		} else if (line.size() == 0) {
			/* ignore empty line */
		} else if (line.startsWith("#")) {
			/* ignore comment line */
		} else {
			/* ignore this line */
		}
	}

	vertices.resize(numTriangles * 3);
	int count = 0;
	for (int i = 0; i < faces.size(); ++i) {
		glm::vec3 normal = glm::cross(raw_vertices[faces[i][1].v_index] - raw_vertices[faces[i][0].v_index], raw_vertices[faces[i][2].v_index] - raw_vertices[faces[i][0].v_index]);
		normal = glm::normalize(normal);

		for (int j = 0; j < faces[i].size() - 2; ++j) {
			{
				vertices[count].position = raw_vertices[faces[i][0].v_index];
				vertices[count].color = glm::vec4(1, 1, 1, 1);
				if (faces[i][0].n_index >= 0 && faces[i][0].n_index < raw_normals.size()) {
					vertices[count].normal = raw_normals[faces[i][0].n_index];
				} else {
					vertices[count].normal = normal;
				}
				if (faces[i][0].t_index >= 0 && faces[i][0].t_index < raw_texCoords.size()) {
					vertices[count].texCoord = raw_texCoords[faces[i][0].t_index];
				}
				count++;
			}

			{
				vertices[count].position = raw_vertices[faces[i][j+1].v_index];
				vertices[count].color = glm::vec4(1, 1, 1, 1);
				if (faces[i][j+1].n_index >= 0 && faces[i][j+1].n_index < raw_normals.size()) {
					vertices[count].normal = raw_normals[faces[i][j+1].n_index];
				} else {
					vertices[count].normal = normal;
				}
				if (faces[i][j+1].t_index >= 0 && faces[i][j+1].t_index < raw_texCoords.size()) {
					vertices[count].texCoord = raw_texCoords[faces[i][j+1].t_index];
				}
				count++;
			}

			{
				vertices[count].position = raw_vertices[faces[i][j+2].v_index];
				vertices[count].color = glm::vec4(1, 1, 1, 1);
				if (faces[i][j+2].n_index >= 0 && faces[i][j+2].n_index < raw_normals.size()) {
					vertices[count].normal = raw_normals[faces[i][j+2].n_index];
				} else {
					vertices[count].normal = normal;
				}
				if (faces[i][j+2].t_index >= 0 && faces[i][j+2].t_index < raw_texCoords.size()) {
					vertices[count].texCoord = raw_texCoords[faces[i][j+2].t_index];
				}
				count++;
			}
		}
	}
}

/**
 * Load vertices data from a OBJ file.
 */
bool OBJLoader::load(const char* filename, std::vector<std::vector<glm::vec3> >& points, std::vector<std::vector<glm::vec3> >& normals, std::vector<std::vector<glm::vec2> >& texCoords) {
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)) {
		return false;
	}

	QTextStream in(&file);
 
	std::vector<glm::vec3> raw_vertices;
	std::vector<glm::vec3> raw_normals;
	std::vector<glm::vec2> raw_texCoords;
	std::vector<std::vector<unsigned int> > v_elements;
	std::vector<std::vector<unsigned int> > t_elements;
	std::vector<std::vector<unsigned int> > n_elements;

	while (!in.atEnd()) {
		QString line = in.readLine();
		if (line.startsWith("v ")) {
			int index1 = line.indexOf(QRegExp("[0-9\\.\\-]"), 2);
			int index2 = line.lastIndexOf(QRegExp("[0-9]"));
			QStringList values = line.mid(index1, index2 - index1 + 1).split(" ");
			raw_vertices.push_back(glm::vec3(values[0].toFloat(), values[1].toFloat(), values[2].toFloat()));
		} else if (line.startsWith("vn ")) {
			int index1 = line.indexOf(QRegExp("[0-9\\.\\-]"), 2);
			int index2 = line.lastIndexOf(QRegExp("[0-9]"));
			QStringList values = line.mid(index1, index2 - index1 + 1).split(" ");
			raw_normals.push_back(glm::vec3(values[0].toFloat(), values[1].toFloat(), values[2].toFloat()));
		} else if (line.startsWith("vt ")) {
			int index1 = line.indexOf(QRegExp("[0-9\\.\\-]"), 2);
			int index2 = line.lastIndexOf(QRegExp("[0-9]"));
			QStringList values = line.mid(index1, index2 - index1 + 1).split(" ");
			raw_texCoords.push_back(glm::vec2(values[0].toFloat(), values[1].toFloat()));
		} else if (line.startsWith("f ")) {
			int index1 = line.indexOf(QRegExp("[0-9]"), 2);
			int index2 = line.lastIndexOf(QRegExp("[0-9]"));
			QStringList values = line.mid(index1, index2 - index1 + 1).split(" ");

			std::vector<unsigned int> v_ele;
			std::vector<unsigned int> n_ele;
			std::vector<unsigned int> t_ele;
			for (int i = 0; i < values.size(); ++i) {
				v_ele.push_back(values[i].split("/")[0].toUInt() - 1);
				if (values[0].split("/").size() >= 2 && values[0].split("/")[1].size() > 0) {
					t_ele.push_back(values[i].split("/")[1].toUInt() - 1);
				}
				if (values[0].split("/").size() >= 3 && values[0].split("/")[2].size() > 0) {
					n_ele.push_back(values[i].split("/")[2].toUInt() - 1);
				}
			}

			v_elements.push_back(v_ele);
			if (t_ele.size() > 0) {
				t_elements.push_back(t_ele);
			}
			if (n_ele.size() > 0) {
				n_elements.push_back(n_ele);
			}
		} else if (line.size() == 0) {
			/* ignore empty line */
		} else if (line.startsWith("#")) {
			/* ignore comment line */
		} else {
			/* ignore this line */
		}
	}

	points.resize(v_elements.size());
	normals.resize(v_elements.size());
	if (t_elements.size() > 0 && t_elements.size() == v_elements.size()) {
		texCoords.resize(v_elements.size());
	}
	for (int i = 0; i < v_elements.size(); ++i) {
		points[i].resize(v_elements[i].size());
		normals[i].resize(v_elements[i].size());
		if (texCoords.size() > 0) {
			texCoords[i].resize(v_elements[i].size());
		}

		for (int j = 0; j < v_elements[i].size(); ++j) {
			unsigned int idx = v_elements[i][j];
			points[i][j] = raw_vertices[idx];
		}

		glm::vec3 normal = glm::cross(points[i][1] - points[i][0], points[i][2] - points[i][0]);
		normal = glm::normalize(normal);

		if (n_elements.size() > 0 && n_elements[i].size() > 0) {
			for (int j = 0; j < n_elements[i].size(); ++j) {
				unsigned int idx = n_elements[i][j];
				if (idx < raw_normals.size()) {
					normals[i][j] = raw_normals[idx];
				} else {
					normals[i][j] = normal;
				}
			}
		} else {
			for (int j = 0; j < v_elements[i].size(); ++j) {
				normals[i][j] = normal;
			}
		}

		if (texCoords.size() > 0 && t_elements[i].size() > 0) {
			for (int j = 0; j < t_elements[i].size(); ++j) {
				unsigned int idx = t_elements[i][j];
				texCoords[i][j] = raw_texCoords[idx];
			}
		}
	}

	return true;
}
