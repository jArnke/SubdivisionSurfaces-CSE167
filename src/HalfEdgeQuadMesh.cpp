#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif

#include <map>
#include <unordered_map>
#include "HalfEdge.h"

/*
* Loads an obj file, converts its data into a half edge mesh.  Works only on triangle or quad meshes.
* @todo convert general polygolan faces to multiple trianges
*
*/
void HalfEdgeQuadMesh::init(const char* filename) {
	std::vector< glm::vec3 > vertices;
	std::vector< unsigned int > indices;

	//whether or not the mesh contains normals
	bool contains_normals = false;

	// Open obj file
	FILE* file = fopen(filename, "r");
	if (file == NULL) { //Failed to open file
		std::cerr << "Cannot open file: " << filename << std::endl;
		exit(-1);
	}

	std::cout << "Loading " << filename << "...";
	//traverse file line by line until reach eof.
	while (!feof(file)) {
		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader
		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			vertices.push_back(vertex);
		}

		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			contains_normals = true;
		}

		else if (strcmp(lineHeader, "f") == 0) {
			//std::string vertex1, vertex2, vertex3;
			if (!contains_normals) {
				unsigned int vertexIndex[4];
				fscanf(file, "%d %d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2], &vertexIndex[3]);
				indices.push_back(vertexIndex[0]);
				indices.push_back(vertexIndex[1]);
				indices.push_back(vertexIndex[2]);
				indices.push_back(vertexIndex[3]);

			}
			else {
				unsigned int vertexIndex[4], normalIndex[4];
				fscanf(file, "%d//%d %d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2], &vertexIndex[3], &normalIndex[3]);
				indices.push_back(vertexIndex[0]);
				indices.push_back(vertexIndex[1]);
				indices.push_back(vertexIndex[2]);
				indices.push_back(vertexIndex[3]);
			}
		}
	}

	//build mesh from vertices and indices:
	this->init(vertices, indices);
	std::cout << "done." << std::endl;
}


/*
Builds HalfEdge datastructure from faces described by vertices and indices arrays
*/
void HalfEdgeQuadMesh::init(std::vector<glm::vec3> vertices, std::vector<GLuint> indices) {

	this->mode = GL_QUADS;
	//Create "Points" from vertice array.
	int numVertices = vertices.size();
	for (int i = 0; i < numVertices; i++) {
		Point* point = (Point*)malloc(sizeof(Point));
		point->he = nullptr;
		point->pos = vertices[i];
		point->isBoundary = false;
		pts.push_back(point);
	}

	//Fill out Faces and HE's
	int numIndices = indices.size();

	//Map assaings a pair of indices a half edge.  The half edge point from vertex a to vertex b can be found at Edges[pair(a, b)]
	//Given edge from a, b. If that edge's flip has already been inserted, it can be found at Edges[pair(b, a)];
	std::map< std::pair<int, int>, HalfEdge*> Edges;

	for (int it = 0; it < numIndices; it += 4) { //traverse 4 indices at a time, inserting a new face for the 4 given vertices

		//get indices:
		int i = indices[it];
		int j = indices[it + 1];
		int k = indices[it + 2];
		int l = indices[it + 3];

		//the indices array in a .obj file starts at 1 rather than 0.  Thuse we need to subtract 1 to get the actual index into the vertice array.
		i--;
		j--;
		k--;
		l--;

		/* Create new halfEdges for the current face */  //@Todo move this to a function?
		HalfEdge* ij = (HalfEdge*)malloc(sizeof(HalfEdge));
		HalfEdge* jk = (HalfEdge*)malloc(sizeof(HalfEdge));
		HalfEdge* kl = (HalfEdge*)malloc(sizeof(HalfEdge));
		HalfEdge* li = (HalfEdge*)malloc(sizeof(HalfEdge));

		ij->next = jk;
		jk->next = kl;
		kl->next = li;
		li->next = ij;

		ij->src = this->pts[i];
		jk->src = this->pts[j];
		kl->src = this->pts[k];
		li->src = this->pts[l];

		//update points to reference the new HalfEdge starting at that point
		ij->src->he = ij;
		jk->src->he = jk;
		kl->src->he = kl;
		li->src->he = li;

		ij->isBoundary = false;
		jk->isBoundary = false;
		kl->isBoundary = false;
		li->isBoundary = false;


		//check for flip edges:
		auto ji = Edges.find(std::pair<int, int>(j, i));
		if (ji != Edges.end()) {
			ij->flip = (*ji).second;
			(*ji).second->flip = ij;
		}
		else {
			ij->flip = nullptr;
		}

		auto kj = Edges.find(std::pair<int, int>(k, j));
		if (kj != Edges.end()) {
			jk->flip = (*kj).second;
			(*kj).second->flip = jk;
		}
		else {
			jk->flip = nullptr;
		}

		auto lk = Edges.find(std::pair<int, int>(l, k));
		if (lk != Edges.end()) {
			kl->flip = (*lk).second;
			(*lk).second->flip = kl;
		}
		else {
			kl->flip = nullptr;
		}

		auto il = Edges.find(std::pair<int, int>(i, l));
		if (il != Edges.end()) {
			li->flip = (*il).second;
			(*il).second->flip = li;
		}
		else {
			li->flip = nullptr;
		}

		//Generate face:
		Face* face = (Face*)malloc(sizeof(Face));
		face->he = ij;

		//Calculate Normal Vector for a given face 
		glm::vec3 pos1 = face->he->src->pos;
		glm::vec3 pos2 = face->he->next->src->pos;
		glm::vec3 pos3 = face->he->next->next->src->pos;
		glm::vec3 v1 = pos2 - pos1;
		glm::vec3 v2 = pos3 - pos1;
		glm::vec3 norm = glm::normalize(glm::cross(v1, v2));

		face->norm = norm;

		//Attach reference to face to each of its HalfEdges:
		ij->face = face;
		jk->face = face;
		kl->face = face;
		li->face = face;

		//add all edges and face to vectors:
		this->hes.push_back(ij);
		this->hes.push_back(jk);
		this->hes.push_back(kl);
		this->hes.push_back(li);

		this->faces.push_back(face);

		//add HalfEdges to map:
		Edges[std::pair<int, int>(i, j)] = ij;
		Edges[std::pair<int, int>(j, k)] = jk;
		Edges[std::pair<int, int>(k, l)] = kl;
		Edges[std::pair<int, int>(l, i)] = li;
	}
	//Clean up mesh structure and label boundary points.
	this->labelBoundaries();
	this->deleteDanglingPts();
}

/* Builds the Vertex Array Object for the mesh so that it can be rendered by the GPU */
/* If we choose to display more information: such as highlighting boundary edges edit shader and make sure to fill in uniforms here */
void HalfEdgeQuadMesh::buildVAO() {

	std::vector<glm::vec3> vertices;
	std::vector<unsigned int> indices;
	std::vector<glm::vec3> normals;

	unsigned int n = faces.size() * 4;
	vertices.resize(n);
	indices.resize(n);
	normals.resize(n);
	for (unsigned int i = 0; i < n; i++) { //@todo This code could probably be a bit cleaner if you traverse through each face rather than looping 3 times over a given face.  This is inefficient and hard to read...
		indices[i] = i;
		Face* face = faces[i / 4];
		HalfEdge* he = face->he;
		for (int j = 0; j < i % 4; j++) {
			he = he->next;
		}
		vertices[i] = he->src->pos;

		//Calculate Normal:
		if (true)
			normals[i] = face->norm;
		else {
			//get neighbors:
			Point* curPt = he->src;
			HalfEdge* he0 = he;
			std::vector<Face*> neighbors;
			do {
				neighbors.push_back(he->face);
				if (he->flip == nullptr)
					break;
				he = he->flip->next;
			} while (he != he0);

			unsigned int numNeighbors = neighbors.size();
			glm::vec3 sum = glm::vec3(0, 0, 0);
			for (unsigned int i = 0; i < numNeighbors; i++) {
				sum += neighbors[i]->norm;
			}
			glm::vec3 norm = glm::normalize(sum);
			normals[i] = norm;
		}
	}

	// setting up buffers
	glGenVertexArrays(1, &vao);
	buffers.resize(3);
	glGenBuffers(3, buffers.data());
	glBindVertexArray(vao);

	// 0th attribute: position
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, n * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// 1st attribute: normal
	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, n * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);


	// indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, n * sizeof(indices[0]), &indices[0], GL_STATIC_DRAW);

	count = n;
	glBindVertexArray(0);

}

void HalfEdgeQuadMesh::subdivide() {
	//for each face add a new face point, with position as average of vertices of face:
	std::vector<Point*> facePoints;
	std::map<Face*, Point*> faceToFacePoint;
	
	for (auto face : faces) {

		//get average value of all faces original vertices
		HalfEdge* he = face->he;
		HalfEdge* he0 = face->he;
		glm::vec3 sum = glm::vec3(0.0f, 0.0f, 0.0f);
		do {
			sum += he->src->pos;
			he = he->next;
		} while (he != he0);
		glm::vec3 average = sum * .25f;
		
		//allocate new Point
		Point* newPt = (Point*)malloc(sizeof(Point*));
		newPt->pos = average;

		//stash ptr
		facePoints.push_back(newPt);
		faceToFacePoint[face] = newPt;
	}
	
	//for each edge add a new edge point
	std::vector<Point*> edgePoints;
	std::map<HalfEdge*, Point*> edgeToEdgePoint;
	for (auto he : hes) {
		if (he->isBoundary) {
			std::cout << "Whoops, we didnt handle boundary points...";
		}
		else {
			//check if flip edge was already handled
			auto pt = edgeToEdgePoint.find(he->flip);
			if (pt != edgeToEdgePoint.end()) {
				edgeToEdgePoint[he] = pt->second;
				continue;
			}
			//otherwise create new point:
			glm::vec3 midPoint = (he->src->pos + he->next->src->pos) * .5f;
			glm::vec3 AverageFacePoint = (faceToFacePoint[he->face]->pos + faceToFacePoint[he->flip->face]->pos) * .5f;
			
			Point* newPt = (Point*)malloc(sizeof(Point));
			newPt->pos = (midPoint + AverageFacePoint) * .5f;
			edgePoints.push_back(newPt);
			edgeToEdgePoint[he] = newPt;
		}
	}

	//update original point positions
	std::vector<glm::vec3> updatedPositions;
	for (auto point : pts) {
		glm::vec3 F = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 R = glm::vec3(0.0f, 0.0f, 0.0f);
		int n = 0;
		//get neighbors:
		HalfEdge* he = point->he;
		HalfEdge* he0 = he;
		do {
			F = F + faceToFacePoint[he->face]->pos;
			R = R + ((he->src->pos + he->next->src->pos) * .5f);
			n++;
			if (he->flip == nullptr)
				break;
			he = he->flip->next;
		} while (he != he0);
		F = F * (1.0f / n);
		R = R * (1.0f / n);

		glm::vec3 pos = F + (2.0f * R) + ((n - 3.0f) * point->pos);
		pos = pos * (1.0f/n);
		updatedPositions.push_back(pos);
	}

	//update original position of original points
	for (int i = 0; i < updatedPositions.size(); i++) {
		pts[i]->pos = updatedPositions[i];
	}

	//subdivide each face into 4 new faces:
	std::vector<Face*> newFaces;
	std::vector<HalfEdge*> newHes;
	std::map<std::pair<Point*, Point*>, HalfEdge*> edges;
	for (auto face : faces) {
		for (int iter = 0; iter < 4; iter++) {
			HalfEdge* he = face->he;
			for (int j = 0; j < iter; j++) {
				he = he->next;
			}

			Point* i = he->src;
			Point* j = edgeToEdgePoint[he];
			Point* k = faceToFacePoint[face];
			Point* l = edgeToEdgePoint[he->next->next->next];

			/* Create new halfEdges for the current face */  //@Todo move this to a function?
			HalfEdge* ij = (HalfEdge*)malloc(sizeof(HalfEdge));
			HalfEdge* jk = (HalfEdge*)malloc(sizeof(HalfEdge));
			HalfEdge* kl = (HalfEdge*)malloc(sizeof(HalfEdge));
			HalfEdge* li = (HalfEdge*)malloc(sizeof(HalfEdge));

			ij->next = jk;
			jk->next = kl;
			kl->next = li;
			li->next = ij;

			ij->src = i;
			jk->src = j;
			kl->src = k;
			li->src = l;

			//update points to reference the new HalfEdge starting at that point
			ij->src->he = ij;
			jk->src->he = jk;
			kl->src->he = kl;
			li->src->he = li;

			ij->isBoundary = false;
			jk->isBoundary = false;
			kl->isBoundary = false;
			li->isBoundary = false;


			//check for flip edges:
			auto ji = edges.find(std::pair<Point*, Point*>(j, i));
			if (ji != edges.end()) {
				ij->flip = (*ji).second;
				(*ji).second->flip = ij;
			}
			else {
				ij->flip = nullptr;
			}

			auto kj = edges.find(std::pair<Point*, Point*>(k, j));
			if (kj != edges.end()) {
				jk->flip = (*kj).second;
				(*kj).second->flip = jk;
			}
			else {
				jk->flip = nullptr;
			}

			auto lk = edges.find(std::pair<Point*, Point*>(l, k));
			if (lk != edges.end()) {
				kl->flip = (*lk).second;
				(*lk).second->flip = kl;
			}
			else {
				kl->flip = nullptr;
			}

			auto il = edges.find(std::pair<Point*, Point*>(i, l));
			if (il != edges.end()) {
				li->flip = (*il).second;
				(*il).second->flip = li;
			}
			else {
				li->flip = nullptr;
			}

			//Generate face:
			Face* face = (Face*)malloc(sizeof(Face));
			face->he = ij;

			//Calculate Normal Vector for a given face 
			glm::vec3 pos1 = face->he->src->pos;
			glm::vec3 pos2 = face->he->next->src->pos;
			glm::vec3 pos3 = face->he->next->next->src->pos;
			glm::vec3 v1 = pos2 - pos1;
			glm::vec3 v2 = pos3 - pos1;
			glm::vec3 norm = glm::normalize(glm::cross(v1, v2));

			face->norm = norm;

			//Attach reference to face to each of its HalfEdges:
			ij->face = face;
			jk->face = face;
			kl->face = face;
			li->face = face;

			//add all edges and face to vectors:
			newHes.push_back(ij);
			newHes.push_back(jk);
			newHes.push_back(kl);
			newHes.push_back(li);

			newFaces.push_back(face);

			//add HalfEdges to map:
			edges[std::pair<Point*, Point*>(i, j)] = ij;
			edges[std::pair<Point*, Point*>(j, k)] = jk;
			edges[std::pair<Point*, Point*>(k, l)] = kl;
			edges[std::pair<Point*, Point*>(l, i)] = li;
		}
	}

	this->faces = newFaces;
	this->hes = newHes;

	for (auto pt : facePoints)
		this->pts.push_back(pt);
	for (auto pt : edgePoints)
		this->pts.push_back(pt);


}