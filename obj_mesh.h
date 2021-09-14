#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
struct VertexData {
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

// data class for obj data
struct ObjData {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	GLulong numFaces;
	GLuint vaoId;

	std::string baseDir;
	std::vector<tinyobj::material_t> materials;
	std::map<std::string, GLuint> textures;
};

std::string GetBaseDir(const std::string& filepath) {
	if (filepath.find_last_of("/\\") != std::string::npos)
		return filepath.substr(0, filepath.find_last_of("/\\"));
	return "";
}

bool FileExists(const std::string& absFilename) {
	struct stat buffer;
	return (stat(absFilename.c_str(), &buffer) == 0);
}

void LoadTexInMem(ObjData* objData, std::string& texname, std::string& baseDir, int& width, int& height);

void LoadTextureData(ObjData* objData) {
	int width, height;
	std::string baseDir = objData->baseDir;
	std::cout << "Material size: " << objData->materials.size() << std::endl;
	for (size_t m = 0; m < objData->materials.size(); m++) {
		tinyobj::material_t* mp = &objData->materials[m];
		if (mp->diffuse_texname.length() > 0) {
			std::string texname = mp->diffuse_texname;
			LoadTexInMem(objData, texname, baseDir, width, height);
		}
		if (mp->bump_texname.length() > 0) {
			std::string texname = mp->bump_texname;
			LoadTexInMem(objData, texname, baseDir, width, height);
		}
	}
}

void LoadTexInMem(ObjData* objData, std::string& texname, std::string& baseDir, int& width, int& height)
{
	if (objData->textures.find(texname) == objData->textures.end()) {
		GLuint textureId;
		int comp;

		std::string textureFileName = baseDir + "/" + texname;
		if (!FileExists(textureFileName)) {
			std::cerr << "Unable to find file: " << textureFileName << std::endl;
			exit(1);
		}
		unsigned char* image = stbi_load(
			textureFileName.c_str(),
			&width,
			&height,
			&comp,
			STBI_default
		);

		if (!image) {
			std::cerr << "Unable to load texture: " << textureFileName << std::endl;
			exit(1);
		}

		std::cout << "Loaded texture: " << textureFileName << std::endl;

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		GLenum format = GL_RGBA;
		if (comp == 3) {
			format = GL_RGB;
		}
		else if (comp == 4) {
			format = GL_RGBA;
		}

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			format,
			width,
			height,
			0,
			format,
			GL_UNSIGNED_BYTE,
			image);
		//clamping the borders of a texture 
		/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);*/
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(image);
		objData->textures.insert(std::make_pair(texname, textureId));
	}
}


// function used mainly for loading obj models
void LoadObjFile(ObjData* objData, std::string filename) {
	std::string warn;
	std::string err;

	std::string basepath = "Assets/";
	std::string inputfile = basepath + filename;

	std::string baseDir = GetBaseDir(inputfile);
	objData->baseDir = baseDir;

	bool isSuccess = tinyobj::LoadObj(&objData->attrib,
		&objData->shapes,
		&objData->materials,
		&warn,
		&err,
		inputfile.c_str(),
		baseDir.c_str()
	);
	if (!err.empty()) {
		std::cerr << err << std::endl;
	}

	std::cout << "Loaded " << filename << std::endl;
	std::cout << "with # of shapes " << objData->shapes.size() << std::endl;
	std::cout << "with # of vertices " << objData->attrib.vertices.size() << std::endl;
	std::cout << "-------------------" << std::endl;
}



void LoadObjToMemory(ObjData* objData, GLfloat scaleFactor, GLfloat tOffset[]) {

	std::vector<glm::vec3> vertices;
	std::vector<GLuint> indices;
	std::vector<VertexData> vertexList;

	/*for (int i = 0; i < objData->attrib.vertices.size() / 3; i++) {

		VertexData vertexData;
		vertexData.position = {
			objData->attrib.vertices[i * 3] * scaleFactor + tOffset[0],// x
			objData->attrib.vertices[i * 3 + 1] * scaleFactor + tOffset[1],// y
			objData->attrib.vertices[i * 3 + 2] * scaleFactor + tOffset[2]// z
		};


	}*/

	for (int i = 0; i < objData->shapes.size(); i++) {
		tinyobj::shape_t shape = objData->shapes[i];
		for (int j = 0; j < shape.mesh.indices.size(); j++) {
			tinyobj::index_t idx = shape.mesh.indices[j];

			VertexData vertexData;

			vertexData.position = {
				objData->attrib.vertices[size_t(idx.vertex_index) * 3] * scaleFactor + tOffset[0],// x
				objData->attrib.vertices[size_t(idx.vertex_index) * 3 + 1] * scaleFactor + tOffset[1],// y
				objData->attrib.vertices[size_t(idx.vertex_index) * 3 + 2] * scaleFactor + tOffset[2]// z
			};

			if (idx.texcoord_index >= 0) {
				vertexData.uv = {
					objData->attrib.texcoords[size_t(idx.texcoord_index) * 2 + 0],
					objData->attrib.texcoords[size_t(idx.texcoord_index) * 2 + 1]
				};
			}

			if (idx.normal_index >= 0) {
				vertexData.normal = {
					objData->attrib.normals[size_t(idx.normal_index) * 3],
					objData->attrib.normals[size_t(idx.normal_index) * 3 + 1],
					objData->attrib.normals[size_t(idx.normal_index) * 3 + 2]
				};
			}
			vertexList.push_back(vertexData);
			indices.push_back(indices.size());
		}
	}

	objData->numFaces = indices.size();

	//Generate normals if no normal data exists
	if (objData->attrib.normals.size() == 0) {
		for (int i = 0; i < vertexList.size() / 3; i++) {
			int idx = i * 3;
			glm::vec3 normal = glm::normalize(glm::cross(vertexList[idx + 1].position - vertexList[idx].position,
				vertexList[idx + 2].position - vertexList[idx].position));
			vertexList[idx].normal = normal;
			vertexList[idx + 1].normal = normal;
			vertexList[idx + 2].normal = normal;
		}
	}

	if (objData->attrib.texcoords.size()) {
		for (int i = 0; i < vertexList.size() / 3; i++) {
			int idx = i * 3;

			glm::vec3 e1 = vertexList[idx + 1].position - vertexList[idx].position;
			glm::vec3 e2 = vertexList[idx + 2].position - vertexList[idx + 1].position;

			glm::vec2 deltaUv1 = vertexList[idx + 1].uv - vertexList[idx].uv;
			glm::vec2 deltaUv2 = vertexList[idx + 2].uv - vertexList[idx + 1].uv;

			float f = 1.0f / (deltaUv1.x * deltaUv2.y - deltaUv2.x * deltaUv1.y);

			glm::vec3 tangent;
			tangent.x = f * (deltaUv2.y * e1.x - deltaUv1.y * e2.x);
			tangent.y = f * (deltaUv2.y * e1.y - deltaUv1.y * e2.y);
			tangent.z = f * (deltaUv2.y * e1.z - deltaUv1.y * e2.z);

			glm::vec3 bitangent;
			bitangent.x = f * (-deltaUv2.x * e2.x + deltaUv1.x * e1.x);
			bitangent.y = f * (-deltaUv2.x * e2.y + deltaUv1.x * e1.y);
			bitangent.z = f * (-deltaUv2.x * e2.z + deltaUv1.x * e1.z);

			tangent = glm::normalize(tangent);
			bitangent = glm::normalize(bitangent);

			for (int j = 0; j < 3; j++) {
				int idx2 = idx + j;

				glm::vec3 normal = vertexList[idx2].normal;
				glm::vec3 adjustedT = glm::normalize(tangent - glm::dot(tangent, normal) * normal);
				vertexList[idx2].tangent = adjustedT;
				vertexList[idx2].bitangent = glm::cross(normal, adjustedT);
			}
		}
	}
	 
	// generate VAO
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	std::cout << "Vertex Array ID: " << VAO << std::endl;

	// generate VBO (vertices)
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	std::cout << "Vertex Buffer ID: " << VBO << std::endl;
	glBufferData(GL_ARRAY_BUFFER, vertexList.size() * sizeof(VertexData), &vertexList[0], GL_STATIC_DRAW);

	//set attributes for vertice buffer
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(VertexData),  //stride, we can give 0 to let opengl handle it, we can also give 3 * sizeof(GLfloat).
		(void*)0
	);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(VertexData),
		(void*)offsetof(VertexData, uv)
	);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(VertexData),
		(void*)offsetof(VertexData, normal)
	);

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(
		3,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(VertexData),
		(void*)offsetof(VertexData, tangent)
	);

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(
		4,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(VertexData),
		(void*)offsetof(VertexData, bitangent)
	);

	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		indices.size() * sizeof(GLuint),
		&indices[0],
		GL_STATIC_DRAW
	);

	LoadTextureData(objData);

	objData->vaoId = VAO;
}
