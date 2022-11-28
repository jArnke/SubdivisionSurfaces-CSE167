/**************************************************
Half Edge is a subclass of Geometry which takes an object
and converts it into the half edge structure for subdivision
*****************************************************/
#include <glm/glm.hpp>
#include "Geometry.h"
#ifndef __HALFEDGE_H__
#define __HALFEDGE_H__

struct HalfEdge;
struct Point;
struct Face;

struct HalfEdge {
	HalfEdge* next;
	HalfEdge* flip;
	Point* src;
	Face* face;
	bool isBoundary;
};

struct Point {
	HalfEdge* he;
	glm::vec3 pos;
	bool isBoundary;
};



struct Face {
	HalfEdge* he;
	glm::vec3 norm;
};

class HalfEdgeMesh : public Geometry {
public:

	std::vector<Point*> pts;
	std::vector<HalfEdge*> hes;
	std::vector<Face*> faces;
	bool use_face_norm;

	void init(const char* filePath);
	void init(std::vector<glm::vec3> vertices, std::vector<GLuint> indices);
	void subdivide();
	void buildVAO();

private:
	float calcBeta(int k);
	void labelBoundaries();
	void clearData();
	void deleteDanglingPts();
};

#endif
