/**************************************************
Half Edge is a subclass of Geometry which takes an object
and converts it into the half edge structure for subdivision
*****************************************************/
#include <glm/glm.hpp>
#include "Geometry.h"
#ifndef __HALFEDGE_H__
#define __HALFEDGE_H__

/* Data Structs for HalfEdgeMesh */
/* Together they make up the connectivity/topology of the surface */
struct HalfEdge;
struct Point;
struct Face;

struct HalfEdge {
	HalfEdge* next;		//Pointer to next HalfEdge in triangle
	HalfEdge* flip;		//Pointer to HalfEdge on opposite side of edge.  Given HalfEdge, he, from point A to B: he.flip will point from B to A and be a part of the face on the other side of that edge
	Point* src;			//Pointer to point this HalfEdge originates from.  Target Point can be obtained through: he->next->src;
	Face* face;			//Pointer to triangle face this HalfEdge belongs to.
	bool isBoundary;	//Whether or not this HalfEdge is located on the boundary of the mesh.  In otherwords, true when he->flip == null,  otherwise false.
};

struct Point {
	HalfEdge* he;		//Pointer to an arbritrary HalfEdge originating from this point.  In implementation, points to the last half edge to be added to this point.  
	glm::vec3 pos;		//Where this point is located in space, realtive to the models coordinates.
	bool isBoundary;	//Whether or not his point is located on the boundary of the mesh.
};



struct Face {
	HalfEdge* he;		//Pointer to an arbritary HalfEdge that is part of the face.  
	glm::vec3 norm;		//Normal vector pointing outwards from this face.
	bool newFace;
};

class HalfEdgeMesh : public Geometry {
public:
	//Containers to give easy access to parts of the surface:
	std::vector<Point*> pts;
	std::vector<HalfEdge*> hes;
	std::vector<Face*> faces;


	//Loads the data from a .obj file and converts it into 2 arrays of vertices and indices.
	//Then calls init using those arrays
	void init(const char* filePath);
	//Builds the HalfEdgeMesh data structure using input values.
	//Creates necessary Points, HalfEdges, and Faces to represent mesh and populates member vectors of this class
	void init(std::vector<glm::vec3> vertices, std::vector<GLuint> indices);

	//Uses Loop Subdivision Algorithm to subdivide the surface once, then replaces the current values in the vectors of this class
	void subdivide();

	//Converts the values in the pts,hes, and faces, into a format passable to the GPU for rendering
	void buildVAO(bool, bool);

	//traverse the list of edges to find boundary edges, labeling them and their source point as boundary edges/points
	void labelBoundaries();

	//Looks for points disconnected from the mesh, and removes them.
	void deleteDanglingPts();
private:
	//Calculates the 
	float calcBeta(int k);


	//@TODO not yet implemented
	//frees the memory allocated for the meshes current points, halfedges, and faces. 
	//Should be called by the deconstructor of this class
	//Also called in the subdivide command before replacing the old lists with the updated values from subdividing.
	void clearData();

};


class HalfEdgeQuadMesh : public HalfEdgeMesh {
public:
	//Loads the data from a .obj file and converts it into 2 arrays of vertices and indices.
	//Then calls init using those arrays
	void init(const char* filePath);
	//Builds the HalfEdgeMesh data structure using input values.
	//Creates necessary Points, HalfEdges, and Faces to represent mesh and populates member vectors of this class
	void init(std::vector<glm::vec3> vertices, std::vector<GLuint> indices);

	//Uses Loop Subdivision Algorithm to subdivide the surface once, then replaces the current values in the vectors of this class
	void subdivide();

	//Converts the values in the pts,hes, and faces, into a format passable to the GPU for rendering
	void buildVAO();
};


#endif
