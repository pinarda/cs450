#include "include/parser.h"

// Parses an obj file to get vertex and normal data for each face
// defined in the file. Takes a path to the obj file, and two vectors
// to store the resulting vertex and normal lists.
bool obj_parser(const char* path,
                std::vector<vec4> &out_vertices,
                std::vector<vec4> &out_normals,
                vec3 &max){

	std::vector<unsigned int> vertexIndices, normalIndices;
	std::vector<vec4> temp_vertices;
	std::vector<vec4> temp_normals;

	// Open file
	FILE* file = fopen(path, "r");
	if(file == NULL){
		printf("File open error. Check obj filename: %s\n", path);
		return false;
	}
	
	while(1){
		char lineHeader[256];
		// Scan the next line of the file
		int res = fscanf(file, "%s", lineHeader);
		// End if the end of the file is reached
		if (res == EOF)
			break;
		// If line is a comment, skip it
		if(strcmp(lineHeader, "#") == 0){
			fscanf(file, "%[^\n]\n", lineHeader);
		}
		// If line contains vertex data, store the vertex information
		else if(strcmp(lineHeader, "v") == 0){
			vec4 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			vertex.w = 1.0;
			if(vertex.x > max.x){
				max.x = vertex.x;
			}
			if(vertex.y > max.y){
				max.y = vertex.y;
			}
			if(vertex.z > max.z){
				max.z = vertex.z;
			}
			temp_vertices.push_back(vertex);
		}
		// If line contains normal data, store the normal data
		else if(strcmp(lineHeader, "vn") == 0){
			vec4 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			normal.w = 0.0;
			temp_normals.push_back(normal);
		}
		// Execute if line contains face data
		else if(strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], normalIndex[3];
			// Get the vertex and normal indices that make up the face
			int matches = fscanf(file, "%d//%d %d//%d %d//%d\n",
			                     &vertexIndex[0], &normalIndex[0],
			                     &vertexIndex[1], &normalIndex[1],
			                     &vertexIndex[2], &normalIndex[2]);
			// Check that each vertex and normal was found and stored
			if(matches != 6){
				printf("Faces parse error\n");
				return false;
			}

			// Store the vertex and normal indices making up the face 
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
	}

	// For each vertex index, fetch the corresponding vertex data and
	// send that to the vertex vector
	int count = 0;
	for(unsigned int i = 0; i < vertexIndices.size(); i++){
		unsigned int vertexIndex = vertexIndices[i];
		vec4 vertex = temp_vertices[vertexIndex - 1];
		out_vertices.push_back(vertex);
	}

	// For each normal index, fetch the corresponding normal data and
	// send that to the vertex normal
	for(unsigned int i = 0; i < normalIndices.size(); i++){
		unsigned int normalIndex = normalIndices[i];
		vec4 normals = temp_normals[normalIndex - 1];
		out_normals.push_back(normals);
	}

	return true;
}

bool scene_parser(const char* path,
                  std::vector<std::string> &obj_files,
                  vec3 &eye,
                  vec3 &at,
                  vec3 &up,
                  char *proj,
                  std::vector<float> &proj_params){
	// Open file
	FILE* file = fopen(path, "r");
	if(file == NULL){
		printf("Scene open error. Specify a scene file as a command line argument\n");
		return false;
	}

	int matches;
	while(1){
		char lineHeader[256];
		// Scan the next line of the file
		int res = fscanf(file, "%s", lineHeader);
		// End if the end of the file is reached
		if (res == EOF)
			break;
		// If line is a comment, skip it
		if(strcmp(lineHeader, "#") == 0){
			matches = fscanf(file, "%[^\n]\n", lineHeader);
		}
		// If line contains obj file name, store file name
		else if(strcmp(lineHeader, "obj") == 0){
			std::string obj_file;
			matches = fscanf(file, "%s\n", &obj_file[0]);
			if(matches != 1){
				printf("obj file not specified\n");
				return false;
			}
			obj_files.push_back(obj_file);
		}
		// If line contains eye coords, store eye coords
 		else if(strcmp(lineHeader, "eye") == 0){
			matches = fscanf(file, "%f %f %f\n", &eye.x, &eye.y, &eye.z);
			if(matches != 3){
				printf("%d eye params found. Make sure there are no commas between values.\n", matches);
			}
		}
		// If line contains at coords, store at coords
		else if(strcmp(lineHeader, "at") == 0){
			matches = fscanf(file, "%f %f %f\n", &at.x, &at.y, &at.z);
			if(matches != 3){
				printf("%d at params found. Make sure there are no commas between values.\n", matches);
			}
		}
		// If line contains up coords, store up coords
		else if(strcmp(lineHeader, "up") == 0){
			matches = fscanf(file, "%f %f %f\n", &up.x, &up.y, &up.z);
			if(matches != 3){
				printf("%d up params found. Make sure there are no commas between values\n", matches);
			}
		}
		// If line contains projection type, store projection type
		else if(strcmp(lineHeader, "proj") == 0){
			matches = fscanf(file, "%s\n", proj);
			if(strcmp(proj, "Orthographic") && strcmp(proj, "Perspective")){
				printf("Invalid projection type: %s\n", proj);
				return false;
			}
		}
		// If line contains projection params, store params
		else if(strcmp(lineHeader, "params") == 0){
			float param;
			matches = fscanf(file, "%f", &param);
			while(matches > 0){
				proj_params.push_back(param);		
				matches = fscanf(file, "%f", &param);
			}
		}
	}
	return true;
}
