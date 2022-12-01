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
void HalfEdgeMesh::init(const char* filename) {
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
				unsigned int vertexIndex[3];
				fscanf(file, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
				indices.push_back(vertexIndex[0]);
				indices.push_back(vertexIndex[1]);
				indices.push_back(vertexIndex[2]);

			}
			else {
				unsigned int vertexIndex[3], normalIndex[3];
				fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
				indices.push_back(vertexIndex[0]);
				indices.push_back(vertexIndex[1]);
				indices.push_back(vertexIndex[2]);
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
void HalfEdgeMesh::init(std::vector<glm::vec3> vertices, std::vector<GLuint> indices) {
	
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

	for (int it = 0; it < numIndices; it += 3) { //traverse 3 indices at a time, inserting a new face for the 3 given vertices

		//get indices:
		int i = indices[it];
		int j = indices[it + 1];
		int k = indices[it + 2];
		
		//the indices array in a .obj file starts at 1 rather than 0.  Thuse we need to subtract 1 to get the actual index into the vertice array.
		i--;
		j--;
		k--;

		/* Create new halfEdges for the current face */  //@Todo move this to a function?
		HalfEdge* ij = (HalfEdge*)malloc(sizeof(HalfEdge));
		HalfEdge* jk = (HalfEdge*)malloc(sizeof(HalfEdge));
		HalfEdge* ki = (HalfEdge*)malloc(sizeof(HalfEdge));

		ij->next = jk;
		jk->next = ki;
		ki->next = ij;

		ij->src = this->pts[i];
		jk->src = this->pts[j];
		ki->src = this->pts[k];

		//update points to reference the new HalfEdge starting at that point
		ij->src->he = ij;
		jk->src->he = jk;
		ki->src->he = ki;

		ij->isBoundary = false;
		jk->isBoundary = false;
		ki->isBoundary = false;


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

		auto ik = Edges.find(std::pair<int, int>(i, k));
		if (ik != Edges.end()) {
			ki->flip = (*ik).second;
			(*ik).second->flip = ki;
		}
		else {
			ki->flip = nullptr;
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
		face->newFace = false;

		//Attach reference to face to each of its HalfEdges:
		ij->face = face;
		jk->face = face;
		ki->face = face;

		//add all edges and face to vectors:
		this->hes.push_back(ij);
		this->hes.push_back(jk);
		this->hes.push_back(ki);

		this->faces.push_back(face);

		//add HalfEdges to map:
		Edges[std::pair<int, int>(i, j)] = ij;
		Edges[std::pair<int, int>(j, k)] = jk;
		Edges[std::pair<int, int>(k, i)] = ki;
	}
	//Clean up mesh structure and label boundary points.
	this->labelBoundaries();
	this->deleteDanglingPts();
}

/* Removes Disconnected points from Mesh */
//@TODO fix memory leak:
void HalfEdgeMesh::deleteDanglingPts() {
	int len = this->pts.size();
	std::vector<Point*> newPoints;

	for (int i = 0; i < len; i++) {
		if (this->pts[i]->he == nullptr) {
			//TODO Free memory allocated for disconnected point
			continue;
		}
		newPoints.push_back(this->pts[i]);
	}

	this->pts = newPoints;
}

/* Traverses Edges Looking for Boundaries of Mesh s*/
void HalfEdgeMesh::labelBoundaries() {

	for (auto edge : this->hes) {
		if (edge->flip == nullptr) {
			edge->isBoundary = true;
			edge->src->isBoundary = true;
		}
		//no need to do anything in opposite case as isBoundary is set to false by default
	}
	

	//Print out data structure: Changed to true for debugging @TODO move to its own function...
	if (false) { // change to false when done testing
		std::cout << "\n\n[Printing resulting data structure]";
		for (int i = 0; i < faces.size(); i++) {
			std::cout << "\tFace " << i << ":\n";
			HalfEdge* he = faces[i]->he;
			HalfEdge* he0 = he;
			do {
				glm::vec3 pos = he->src->pos;
				std::cout << "\t\t" << pos.x << ", " << pos.y << ", " << pos.z << "\n";
				he = he->next;
			} while (he != he0);
		}

		for (int i = 0; i < hes.size(); i++) {
			std::cout << "\tEdge " << i << ":\n";
			HalfEdge* he = hes[i];
			if (he->flip == nullptr) {
				std::cout << "\t[Boundary Edge]";
				glm::vec3 pos1 = he->src->pos;
				glm::vec3 pos2 = he->next->src->pos;
				std::cout << pos1.x << ", " << pos1.y << ", " << pos1.z << " -> " << pos2.x << ", " << pos2.y << ", " << pos2.z << "\n";
			}
			else {
				glm::vec3 pos1 = he->src->pos;
				glm::vec3 pos2 = he->next->src->pos;
				std::cout << "\t\t" << pos1.x << ", " << pos1.y << ", " << pos1.z << " -> " << pos2.x << ", " << pos2.y << ", " << pos2.z << "\n";
			}

		}
	}
}


void HalfEdgeMesh::subdivide() {
	//vector of all new points to be added throughout algoritmn 
	//replaces old this->pts at the end
	std::vector<Point*> newPoints;

	//maps old references to old Point* to their new Point* with their updated position
	//used when splitting faces.
	std::map<Point*, Point*> OldToNewPoints;

	//traverses points and calculates their new position based on neighbors
	for (auto pointIter : this->pts) {

		//current point
		Point* pt = pointIter;

		if (pt->isBoundary) { // at crease position at pt = a(1/8) ---------- pt (3/4) -------------- b(1/8)
			Point* a;
			Point* b;
			//i don't remember how this works but if you draw it out I think it makes sense
			//at the end point a and point b will refer to the cw and ccw most neighbors of the current point
			//@Todo create a helper function which rotates cw or ccw from a given point to make this more readable.
			if (pt->he->flip == nullptr) {
				a = pt->he->next->src;
				if (pt->he->next->next->flip == nullptr) {
					b = pt->he->next->next->src;
				}
				else {
					b = pt->he->next->next->flip->next->next->src;
				}
			}
			else {
				a = pt->he->next->next->src;
				b = pt->he->flip->next->next->src;
			}

			//create a new point
			Point* newPt = (Point*)malloc(sizeof(Point));

			//calculate position of new point as a weighted average of the current point and its ccw / cw most neighbors
			newPt->pos = (a->pos * .125f) + (b->pos * .125f) + (pt->pos * .75f);
			newPt->isBoundary = false;
			newPoints.push_back(newPt);
			//map pointer from original point to new point
			OldToNewPoints[pt] = newPt;
		}
		else {  //If point is not a boundary location...

			//obtain a list of all neighbor points:
			std::vector<Point*> neighbors;
			HalfEdge* he0 = pt->he->next->next;
			HalfEdge* he = he0;
			do {
				neighbors.push_back(he->src);
				he = he->flip->next->next;
			} while (he != he0);

			//calculate beta given k neighbors...
			int k = neighbors.size();
			float beta = this->calcBeta(k);

			//calculate new position based on weighted average of neighbors
			glm::vec3 sum = glm::vec3(0, 0, 0);
			for (auto neighbor : neighbors) {
				sum += (neighbor->pos * beta);
			}
			sum += ((1 - (k * beta)) * pt->pos);

			//create new point based on new position:
			Point* newPt = (Point*)malloc(sizeof(Point));
			newPt->pos = sum;
			newPt->isBoundary = false;
			newPoints.push_back(newPt);
			OldToNewPoints[pt] = newPt; //map pointer to old point to new point
		}
	}
	//Done calculating updated positions for original vertices:
	
	//Now traverse each face, creating new points at the midway point of each edge, and then splitting the face into 4 subfaces:
	std::map<std::pair<Point*, Point*>, std::pair<HalfEdge*, Point*>> visitedEdges; //Maps two points to the Edge connecting the two points as well as the newly created midPoint of that edge
																					//Prevents creating a new midPoint at both the Halfs of a given edge
	std::vector<HalfEdge*> newEdges; //new list of edges to replace this->hes
	std::vector<Face*> newFaces;     //new list of faces to replace this->faces
	std::map<std::pair<Point*, Point*>, HalfEdge*> edges;	//Map of edges used like in init() to find flip edges

	//gen new faces
	for (auto face : this->faces) {
		//get edges of current face:
		HalfEdge* hes[3];
		hes[0] = face->he;
		hes[1] = hes[0]->next;
		hes[2] = hes[1]->next;


		Point* pts[3];
		//First generate new points from face at edge midpoints:
		for (int i = 0; i < 3; i++) {
			HalfEdge* he = hes[i];		//"he" is HalfEdge from Point* a to Point* b
			Point* a = he->src;
			Point* b = he->next->src;
			if (he->isBoundary) {
				//calculate new point position as average of points a and b
				Point* pt = (Point*)malloc(sizeof(Point));
				pt->pos = (a->pos + b->pos) * .5f;
				pt->isBoundary = false;

				//store point
				pts[i] = pt;
				newPoints.push_back(pt);

				//mark edge pair as visited and stash reference to new midpoint
				visitedEdges[std::pair<Point*, Point*>(a, b)] = std::pair<HalfEdge*, Point*>(he, pt);
			}
			else {
				//check if other half edge has already been visited:
				if (visitedEdges.find(std::pair<Point*, Point*>(b, a)) != visitedEdges.end()) { // if already placed a point at the opposite halfEdge
					//pull stashed midpoint:
					Point* pt = visitedEdges[std::pair<Point*, Point*>(b, a)].second;
					visitedEdges[std::pair<Point*, Point*>(a, b)] = std::pair<HalfEdge*, Point*>(he, pt); //probably dont need this line as both sides of the edge pair have been visisted at this point meaning we should never have to pull out this value again...
					pts[i] = pt;
					continue;
				}
				//calculate new point position
				Point* pt = (Point*)malloc(sizeof(Point));
				//neighbor points, a, b, c, d refer to locations in diagram from slides.  the edge goes from point a to b.  Two faces share those points, c and d refer to the outside points of each of those faces.
				Point* c = he->next->next->src;
				Point* d = he->flip->next->next->src;
				pt->pos = ((a->pos + b->pos) * .375f) + ((c->pos + d->pos) * .125f);
				pt->isBoundary = false;

				//store new point
				pts[i] = pt;
				newPoints.push_back(pt);

				//mark edge pair as visited and stash reference to new midpoint
				visitedEdges[std::pair<Point*, Point*>(a, b)] = std::pair<HalfEdge*, Point*>(he, pt);
			}
		}

		//generate new face from new points:
		//gen middle face:  Connects each of the newly generated midPoints into a new face.
		HalfEdge* ij = (HalfEdge*)malloc(sizeof(HalfEdge));
		HalfEdge* jk = (HalfEdge*)malloc(sizeof(HalfEdge));
		HalfEdge* ki = (HalfEdge*)malloc(sizeof(HalfEdge));

		ij->next = jk;
		jk->next = ki;
		ki->next = ij;

		ij->src = pts[0];
		jk->src = pts[1];
		ki->src = pts[2];

		ij->src->he = ij;
		jk->src->he = jk;
		ki->src->he = ki;

		ij->isBoundary = false;
		jk->isBoundary = false;
		ki->isBoundary = false;

		//Each edge has no flip yet as the other inside faces haven't been added yet.
		ij->flip = nullptr;
		jk->flip = nullptr;
		ki->flip = nullptr;

		Face* face = (Face*)malloc(sizeof(Face));
		face->he = ij;

		ij->face = face;
		jk->face = face;
		ki->face = face;

		glm::vec3 pos1 = pts[0]->pos;
		glm::vec3 pos2 = pts[1]->pos;
		glm::vec3 pos3 = pts[2]->pos;
		glm::vec3 v1 = pos2 - pos1;
		glm::vec3 v2 = pos3 - pos1;
		glm::vec3 norm = glm::normalize(glm::cross(v1, v2));
		face->norm = norm;
		face->newFace = true;

		//add all edges and face to vectors:
		newEdges.push_back(ij);
		newEdges.push_back(jk);
		newEdges.push_back(ki);

		newFaces.push_back(face);

		//add HalfEdges to map:
		edges[std::pair<Point*, Point*>(pts[0], pts[1])] = ij;
		edges[std::pair<Point*, Point*>(pts[1], pts[2])] = jk;
		edges[std::pair<Point*, Point*>(pts[2], pts[0])] = ki;

		//gen outer faces:
		for (int it = 0; it < 3; it++) {
			
			//each face contains one original vertex of the face and two of the newly generated midPoint vertices
			Point* i = OldToNewPoints[(hes[it]->src)];  //original point
			Point* j = pts[it];	//newly created point
			Point* k = pts[(it + 2) % 3];//opposite newly created point

			HalfEdge* ij = (HalfEdge*)malloc(sizeof(HalfEdge));
			HalfEdge* jk = (HalfEdge*)malloc(sizeof(HalfEdge));
			HalfEdge* ki = (HalfEdge*)malloc(sizeof(HalfEdge));

			ij->next = jk;
			jk->next = ki;
			ki->next = ij;

			ij->src = i;
			jk->src = j;
			ki->src = k;

			ij->src->he = ij;
			jk->src->he = jk;
			ki->src->he = ki;

			ij->isBoundary = false;
			jk->isBoundary = false;
			ki->isBoundary = false;

			//check for flip edges:
			auto ji = edges.find(std::pair<Point*, Point*>(j, i));
			if (ji != edges.end()) {
				ij->flip = (*ji).second;
				(*ji).second->flip = ij;
			}
			else
				ij->flip = nullptr;


			auto kj = edges.find(std::pair<Point*, Point*>(k, j));
			if (kj != edges.end()) {
				jk->flip = (*kj).second;
				(*kj).second->flip = jk;
			}
			else
				jk->flip = nullptr;

			auto ik = edges.find(std::pair<Point*, Point*>(i, k));
			if (ik != edges.end()) {
				ki->flip = (*ik).second;
				(*ik).second->flip = ki;
			}
			else
				ki->flip = nullptr;

			//allocate face
			Face* face = (Face*)malloc(sizeof(Face));
			face->he = ij;

			ij->face = face;
			jk->face = face;
			ki->face = face;

			glm::vec3 pos1 = i->pos;
			glm::vec3 pos2 = j->pos;
			glm::vec3 pos3 = k->pos;
			glm::vec3 v1 = pos2 - pos1;
			glm::vec3 v2 = pos3 - pos1;
			glm::vec3 norm = glm::normalize(glm::cross(v1, v2));
			face->norm = norm;
			face->newFace = false;

			//add all edges and face to vectors:
			newEdges.push_back(ij);
			newEdges.push_back(jk);
			newEdges.push_back(ki);

			newFaces.push_back(face);

			//add HalfEdges to map:
			edges[std::pair<Point*, Point*>(i, j)] = ij;
			edges[std::pair<Point*, Point*>(j, k)] = jk;
			edges[std::pair<Point*, Point*>(k, i)] = ki;

		}//Done adding outside faces
	}
	//done subdividing faces

	//at this point, newPoints should have all the newly created points and updated old points
	//newEdges should contain all the new edges
	//and newFaces should contain all the new faces
	
	//therefore we can safely free the memory for the old values
	this->clearData();

	//and override their vectors
	this->pts = newPoints;
	this->hes = newEdges;
	this->faces = newFaces;

	//finally we clean up by labeling the boundary points in the new mesh
	this->labelBoundaries();

}

/* Calculates Beta given k neighbors */
//Formula comes from slides...
float HalfEdgeMesh::calcBeta(int k) {
	float beta;
	if (k > 3) {
		beta = 3.0f / (8.0f * k);
	}
	else {
		beta = 0.1875f; //  3/16
	}
	return beta;
}

/* Builds the Vertex Array Object for the mesh so that it can be rendered by the GPU */
/* If we choose to display more information: such as highlighting boundary edges edit shader and make sure to fill in uniforms here */
void HalfEdgeMesh::buildVAO(bool use_face_norm, bool outline) {
	std::vector<glm::vec3> vertices;
	std::vector<unsigned int> indices;
	std::vector<glm::vec3> normals;

	unsigned int n = faces.size() * 3;
	vertices.resize(n);
	indices.resize(n);
	normals.resize(n);
	for (unsigned int i = 0; i < n; i++) { //@todo This code could probably be a bit cleaner if you traverse through each face rather than looping 3 times over a given face.  This is inefficient and hard to read...
		indices[i] = i;
		Face* face = faces[i / 3];
		HalfEdge* he = face->he;
		for (int j = 0; j < i % 3; j++) {
			he = he->next;
		}
		vertices[i] = he->src->pos;

		//Calculate Normal:
		if (outline) {
			if (face->newFace) {
				normals[i] = glm::vec3(0.0f, 0.0f, 1.0f);
			}
			else {
				normals[i] = glm::vec3(1.0f, 0.0f, 0.0f);
			}
		}
		else if (use_face_norm)
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

/* @TODO free memory for points halfEdges and faces in vectors */
void HalfEdgeMesh::clearData() {
	return;
}

