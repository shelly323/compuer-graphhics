#include "TriangleMesh.h"

// Desc: Constructor of a triangle mesh.
TriangleMesh::TriangleMesh()
{
	numVertices = 0;
	numTriangles = 0;
	objCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	vboId = 0;
	iboId = 0;
}

// Desc: Destructor of a triangle mesh.
TriangleMesh::~TriangleMesh()
{
	vertices.clear();
	vertexIndices.clear();
	glDeleteBuffers(1, &vboId);
	glDeleteBuffers(1, &iboId);
}

// Desc: Load the geometry data of the model from file and normalize it.
bool TriangleMesh::LoadFromFile(const std::string& filePath, const bool normalized)
{	
	ifstream file(filePath, ios::in);

	if (!file.is_open()) {
		cout << "Failed to open file" << endl;
        return false;
	}

    vector<vec3> temp_positions;
    vector<vec2> temp_texcoords;
    vector<vec3> temp_normals;
    unordered_map<string, int> hash;

    if (file) {
        
        std::string line; 
        int k = 0;
        while (getline(file, line)) {
            istringstream iss(line);
            string prefix;

            iss >> prefix;

            if (prefix == "v") {
                vec3 v;
                if (iss >> v.x >> v.y >> v.z) {
                    temp_positions.push_back(v);
                }
            }
            else if (prefix == "vt") {
                vec2 vt;
                if (iss >> vt.x >> vt.y) {
                    temp_texcoords.push_back(vt);
                }
            }
            else if (prefix == "vn") {
                vec3 vn;
                if (iss >> vn.x >> vn.y >> vn.z) {
                    temp_normals.push_back(vn);
                }
            }
            else if (prefix == "f") {
                string combo_string;
                vector<int> v_indices, uv_indices, n_indices;
                
                while (iss >> combo_string) {
                    int vID, uvID, nID;
                    sscanf_s(combo_string.c_str(), "%d/%d/%d", &vID, &uvID, &nID);
                    v_indices.push_back(vID);
                    uv_indices.push_back(uvID);
                    n_indices.push_back(nID);
                    
                }
                
                if (v_indices.size() >= 3) {
                    int v1 = 0;
                    
                    for (int i = 1; i < v_indices.size() - 1; i++) {
                        int combo[3][3] = {
                            {v_indices[v1],uv_indices[v1],n_indices[v1]},
                            {v_indices[i],uv_indices[i],n_indices[i]},
                            {v_indices[i + 1],uv_indices[i + 1],n_indices[i + 1]}
                        };
                        
                        for (int j = 0; j < 3; j++) {

                            int pos_index = combo[j][0]-1;
                            int uv_index = combo[j][1]-1;
                            int normal_index = combo[j][2]-1;
                            
                            VertexPTN temp;
                            temp.position = temp_positions[pos_index];
                            temp.texcoord = temp_texcoords[uv_index];
                            temp.normal = temp_normals[normal_index];
                            
                            string a = to_string(pos_index) + to_string(uv_index) + to_string(normal_index);                            
                            auto iter = hash.find(a);
                            if (iter == hash.end( )) {
                                hash.insert({a,k });
                                vertices.push_back(temp);
                                vertexIndices.push_back(k);
                                numVertices++;
                                k++;
                            }
                            else {
                                vertexIndices.push_back(iter->second);                                
                            }                            
                        }
                        numTriangles++;
                    }
                }


            }
        }
    }   
    

	if (normalized) {
        vec3 Min = vertices[0].position, Max = vertices[0].position;

        for (int i = 1; i < numVertices; i++) {
            if (vertices[i].position.x > Max.x)  //maximum x
                Max.x = std::max(vertices[i].position.x, Max.x);
            if (vertices[i].position.y > Max.y)  //maximum y
                Max.y = std::max(vertices[i].position.y, Max.y);
            if (vertices[i].position.z > Max.z)  //maximum z
                Max.z = std::max(vertices[i].position.z, Max.z);
            if (vertices[i].position.x < Min.x)  //minimum x
                Min.x = std::min(vertices[i].position.x, Min.x);
            if (vertices[i].position.y < Min.y)  //minimum y
                Min.y= std::min(vertices[i].position.y, Min.y);
            if (vertices[i].position.z < Min.z)  //minimum z
                Min.z = std::min(vertices[i].position.z, Min.z);
        }

        objCenter = { (Min.x + Max.x) / 2,(Min.y + Max.y) / 2,(Min.z + Max.z) / 2 };
        vec3 offset = { 0 - objCenter.x, 0 - objCenter.y,0 - objCenter.z };
        
        float maximalExten = std::max(Max.x - Min.x, Max.y - Min.y);
        maximalExten = std::max(maximalExten, Max.z - Min.z);
        const float scaleFactor = 1.0 / maximalExten;

        for (int i = 0; i < numVertices; i++) {
            vertices[i].position.x = (vertices[i].position.x + offset.x) * scaleFactor;
            vertices[i].position.y = (vertices[i].position.y + offset.y) * scaleFactor;
            vertices[i].position.z = (vertices[i].position.z + offset.z) * scaleFactor;

        }
	}

	PrintMeshInfo();
	return true;
}

// Desc: Create vertex buffer and index buffer.
void TriangleMesh::CreateBuffers()
{
    glGenBuffers(1, &vboId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexPTN), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), (void*)0);

    glGenBuffers(1, &iboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexIndices.size() * sizeof(unsigned int), vertexIndices.data(), GL_STATIC_DRAW);
    
}

// Desc: Apply transformation to all vertices (DON'T NEED TO TOUCH)
void TriangleMesh::ApplyTransformCPU(const glm::mat4x4& mvpMatrix)
{
	for (int i = 0 ; i < numVertices; ++i) {
        glm::vec4 p = mvpMatrix * glm::vec4(vertices[i].position, 1.0f);
        if (p.w != 0.0f) {
            float inv = 1.0f / p.w; 
            vertices[i].position.x = p.x * inv;
            vertices[i].position.y = p.y * inv;
            vertices[i].position.z = p.z * inv;
        }
    }
}

// Desc: Print mesh information.
void TriangleMesh::PrintMeshInfo() const
{
	std::cout << "[*] Mesh Information: " << std::endl;
	std::cout << "# Vertices: " << numVertices << std::endl;
	std::cout << "# Triangles: " << numTriangles << std::endl;
	std::cout << "Center: (" << objCenter.x << " , " << objCenter.y << " , " << objCenter.z << ")" << std::endl;

    cout << endl;
    cout << "Press Esc to exit, Press tab to render next file" << endl;
}
