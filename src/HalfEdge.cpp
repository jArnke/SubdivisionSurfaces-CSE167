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


void HalfEdgeMesh::init(const char* filename) {
	std::vector< glm::vec3 > temp_vertices, vertices;
	std::vector< glm::vec3 > temp_normals, normals;
	std::vector< unsigned int > temp_vertexIndices, indices;
	std::vector< unsigned int > temp_normalIndices;

	// load obj file
	FILE* file = fopen(filename, "r");
	if (file == NULL) {
		std::cerr << "Cannot open file: " << filename << std::endl;
		exit(-1);
	}
	std::cout << "Loading " << filename << "...";
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
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			//std::string vertex1, vertex2, vertex3;
			if (temp_normals.size() == 0) {
				unsigned int vertexIndex[3];
				fscanf(file, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
				temp_vertexIndices.push_back(vertexIndex[0]);
				temp_vertexIndices.push_back(vertexIndex[1]);
				temp_vertexIndices.push_back(vertexIndex[2]);

			}
			else {
				unsigned int vertexIndex[3], normalIndex[3];
				fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
				temp_vertexIndices.push_back(vertexIndex[0]);
				temp_vertexIndices.push_back(vertexIndex[1]);
				temp_vertexIndices.push_back(vertexIndex[2]);
				temp_normalIndices.push_back(normalIndex[0]);
				temp_normalIndices.push_back(normalIndex[1]);
				temp_normalIndices.push_back(normalIndex[2]);
			}
		}
	}
	this->init(temp_vertices, temp_vertexIndices);
	std::cout << "done." << std::endl;


	/*
	// post processing
	std::cout << "Processing data...";
	unsigned int n = temp_vertexIndices.size(); // #(triangles)*3
	vertices.resize(n);
	normals.resize(n);
	indices.resize(n);
	for (unsigned int i = 0; i < n; i++) {
		indices[i] = i;
		vertices[i] = temp_vertices[temp_vertexIndices[i] - 1];
		normals[i] = temp_normals[temp_normalIndices[i] - 1];
	}

	this->init(vertices, indices);
	std::cout << "done." << std::endl;
	*/

}
bool vecComp(const glm::vec3 lhs, const glm::vec3 rhs)
{
	return lhs.x < rhs.x ||
		lhs.x == rhs.x && (lhs.y < rhs.y || lhs.y == rhs.y && lhs.z < rhs.z);
}

void HalfEdgeMesh::init(std::vector<glm::vec3> vertices, std::vector<GLuint> indices) {
	this->use_face_norm = true;
	//Create all points:
	int numVertices = vertices.size();
	for (int i = 0; i < numVertices; i++) {
		Point* point = (Point*)malloc(sizeof(Point));
		point->he = nullptr;
		point->pos = vertices[i];
		point->isBoundary = false;
		pts.push_back(point);
	}

	//Fill out Faces and HE's
	int len = indices.size();
	int numVerts = vertices.size();


	std::map< std::pair<int, int>, HalfEdge*> Edges;

	for (int it = 0; it < len; it += 3) {
		//get indices:
		int i = indices[it];
		int j = indices[it + 1];
		int k = indices[it + 2];
		i--;
		j--;
		k--;
		/*
		auto ijFlip = Edges.find(std::pair<int, int>(i, j));
		auto jkFlip = Edges.find(std::pair<int, int>(j, k));
		auto kiFlip = Edges.find(std::pair<int, int>(k, i));
		if (ijFlip != Edges.end()) {
			int temp = i;
			i = j;
			j = temp;
		}
		else if (jkFlip != Edges.end()) {
			int temp = k;
			k = j;
			j = temp;
		}
		else if (kiFlip != Edges.end()) {
			int temp = k;
			k = i;
			i = temp;
		}*/

		HalfEdge* ij = (HalfEdge*)malloc(sizeof(HalfEdge));
		HalfEdge* jk = (HalfEdge*)malloc(sizeof(HalfEdge));
		HalfEdge* ki = (HalfEdge*)malloc(sizeof(HalfEdge));

		ij->next = jk;
		jk->next = ki;
		ki->next = ij;

		ij->src = this->pts[i];
		jk->src = this->pts[j];
		ki->src = this->pts[k];

		//check if points have he
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

		glm::vec3 pos1 = face->he->src->pos;
		glm::vec3 pos2 = face->he->next->src->pos;
		glm::vec3 pos3 = face->he->next->next->src->pos;
		glm::vec3 v1 = pos2 - pos1;
		glm::vec3 v2 = pos3 - pos1;
		glm::vec3 norm = glm::normalize(glm::cross(v1, v2));

		face->norm = norm;

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
	this->labelBoundaries();
	this->deleteDanglingPts();
}

void HalfEdgeMesh::deleteDanglingPts() {
	int len = this->pts.size();
	std::vector<Point*> newPoints;
	for (int i = 0; i < len; i++) {
		if (this->pts[i]->he == nullptr) {
			continue;
		}
		newPoints.push_back(this->pts[i]);
	}
	this->pts = newPoints;


}
void HalfEdgeMesh::labelBoundaries() {
	std::cout << "labeling boundary points\n";
	int countPos = 0;
	int countNeg = 0;
	for (auto edge : this->hes) {
		if (edge->flip == nullptr) {
			edge->isBoundary = true;
			if (!edge->src->isBoundary) {
				edge->src->isBoundary = true;
				countPos++;
				std::cout << "\tBoundary Point at: " << edge->src->pos.x << ", " << edge->src->pos.y << "\n";
			}
		}
		else {
			edge->isBoundary = false;
		}
	}

	std::cout << "Num Boundary: " << countPos << "\n";


	//Print out data structure

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
	std::cout << "Preparring to process, " << this->pts.size() << ", points...\n";
	std::vector<Point*> newPoints;
	std::map<Point*, Point*> updatedPointPos;
	//update old point positions:
	for (auto pointIter : this->pts) {

		Point* pt = pointIter;
		if (pt->isBoundary) { // at crease position at pt = a(1/8) ---------- pt (3/4) -------------- b(1/8)
			std::cout << "processing boundary point\n";
			Point* a;
			Point* b;
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

			Point* newPt = (Point*)malloc(sizeof(Point));
			newPt->pos = (a->pos * .125f) + (b->pos * .125f) + (pt->pos * .75f);
			newPt->isBoundary = false;
			newPoints.push_back(newPt);
			updatedPointPos[pt] = newPt;
		}
		else {  //Otherwise calculate B given k neighbors:
			//traverse all neighbor points:
			std::vector<Point*> neighbors;
			HalfEdge* he0 = pt->he->next->next;
			HalfEdge* he = he0;
			do {
				neighbors.push_back(he->src);
				he = he->flip->next->next;
			} while (he != he0);
			int k = neighbors.size();
			float beta = this->calcBeta(k);

			glm::vec3 sum = glm::vec3(0, 0, 0);
			for (auto neighbor : neighbors) {
				sum += (neighbor->pos * beta);
			}

			sum += ((1 - (k * beta)) * pt->pos);

			Point* newPt = (Point*)malloc(sizeof(Point));
			newPt->pos = sum;
			newPt->isBoundary = false;
			newPoints.push_back(newPt);
			updatedPointPos[pt] = newPt;

		}
	}

	std::map<std::pair<Point*, Point*>, std::pair<HalfEdge*, Point*>> visitedEdges;
	std::vector<HalfEdge*> newEdges;
	std::vector<Face*> newFaces;
	std::map<std::pair<Point*, Point*>, HalfEdge*> edges;

	std::cout << "Number of processed pts: " << updatedPointPos.size() << "\n";
	//Connect new mesh -- gen new faces
	for (auto face : this->faces) {
		//get edges of current face:
		HalfEdge* hes[3];
		hes[0] = face->he;
		hes[1] = hes[0]->next;
		hes[2] = hes[1]->next;


		Point* pts[3];
		//Generate new points from face:
		for (int i = 0; i < 3; i++) {
			HalfEdge* he = hes[i];
			Point* a = he->src;
			Point* b = he->next->src;
			if (he->isBoundary) {
				//calculate new point position
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

		//generate new faces from new points:

		//gen middle face
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

		//add all edges and face to vectors:
		newEdges.push_back(ij);
		newEdges.push_back(jk);
		newEdges.push_back(ki);

		newFaces.push_back(face);

		//add HalfEdges to map:
		edges[std::pair<Point*, Point*>(pts[0], pts[1])] = ij;
		edges[std::pair<Point*, Point*>(pts[1], pts[2])] = jk;
		edges[std::pair<Point*, Point*>(pts[2], pts[0])] = ki;

		//gen outer faces
		for (int it = 0; it < 3; it++) {

			Point* i = updatedPointPos[(hes[it]->src)];
			Point* j = pts[it];
			Point* k = pts[(it + 2) % 3];

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

			//add all edges and face to vectors:
			newEdges.push_back(ij);
			newEdges.push_back(jk);
			newEdges.push_back(ki);

			newFaces.push_back(face);

			//add HalfEdges to map:
			edges[std::pair<Point*, Point*>(i, j)] = ij;
			edges[std::pair<Point*, Point*>(j, k)] = jk;
			edges[std::pair<Point*, Point*>(k, i)] = ki;

		}
	}


	this->clearData();

	this->pts = newPoints;
	this->hes = newEdges;
	this->faces = newFaces;

	this->labelBoundaries();

	this->buildVAO();



}

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

void HalfEdgeMesh::buildVAO() {
	std::vector<glm::vec3> vertices;
	std::vector<unsigned int> indices;
	std::vector<glm::vec3> normals;

	unsigned int n = faces.size() * 3;
	vertices.resize(n);
	indices.resize(n);
	normals.resize(n);
	for (unsigned int i = 0; i < n; i++) {
		indices[i] = i;
		Face* face = faces[i / 3];
		HalfEdge* he = face->he;
		for (int j = 0; j < i % 3; j++) {
			he = he->next;
		}
		vertices[i] = he->src->pos;

		//Calculate Normal:
		if (use_face_norm)
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
	std::cout << "Setting up buffers...";
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
	std::cout << "done." << std::endl;

}

void HalfEdgeMesh::clearData() {
	return;
}