// This file is part of Hermes3D
//
// Copyright (c) 2009 hp-FEM group at the University of Nevada, Reno (UNR).
// Email: hpfem-group@unr.edu, home page: http://hpfem.org/.
//
// Hermes3D is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
// Hermes3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hermes3D; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#ifndef _MESH_H_
#define _MESH_H_

#include "../../../hermes_common/judyarray.h"
#include "../../../hermes_common/judyarrayptr.h"
#include "../../../hermes_common/mapord.h"

typedef
	unsigned int uint;


// 2D element modes
enum ElementMode2D {
	HERMES_MODE_TRIANGLE = 0,
	HERMES_MODE_QUAD = 1
};

// 3D element modes
enum ElementMode3D {
	HERMES_MODE_TET = 0,
	HERMES_MODE_HEX = 1,
	HERMES_MODE_PRISM = 2
};


class Mesh3DLoader;

// Vertex /////////////////////////////////////////////////////////////////////

class Vertex {
public:
	static const int NUM_COORDS = 3;

	Vertex();
	Vertex(double _x, double _y, double _z);
	Vertex(const Vertex &o);

	double x, y, z;					// coordinates
};

// Facet //////////////////////////////////////////////////////////////////////

class Facet {
public:
	int left, right;
};

// Elements ///////////////////////////////////////////////////////////////////

class Element {
public:
	int id;
	virtual ElementMode3D get_mode() const = 0;
	virtual uint *get_vertices() = 0;
};

class Tetra : public Element {
public:
	static const int NUM_VERTICES = 4;

	Tetra();
	Tetra(uint v[]);
	Tetra(uint v1, uint v2, uint v3, uint v4);

	uint vtcs[NUM_VERTICES];

	virtual ElementMode3D get_mode() const { return HERMES_MODE_TET; }
	virtual uint *get_vertices() { return vtcs; }
};

class Hex : public Element {
public:
	static const int NUM_VERTICES = 8;

	Hex();
	Hex(uint v[]);
	Hex(uint v1, uint v2, uint v3, uint v4, uint v5, uint v6, uint v7, uint v8);

	uint vtcs[NUM_VERTICES];

	virtual ElementMode3D get_mode() const { return HERMES_MODE_HEX; }
	virtual uint *get_vertices() { return vtcs; }
};

class Prism : public Element {
public:
	static const int NUM_VERTICES = 6;

	Prism();
	Prism(uint v[]);
	Prism(uint v1, uint v2, uint v3, uint v4, uint v5, uint v6);

	uint vtcs[NUM_VERTICES];

	virtual ElementMode3D get_mode() const { return HERMES_MODE_PRISM; }
	virtual uint *get_vertices() { return vtcs; }
};

// 2D ///////////////////////////////////////////////////////////////////

class Quad {
public:
	static const int NUM_VERTICES = 4;
};

class Tri {
public:
	static const int NUM_VERTICES = 3;
};


/// Base class for boundaries of all types
///
///
class Boundary {
public:
	Boundary(int marker);
	Boundary(const Boundary &o);
	virtual ~Boundary();

	virtual int get_marker() const { return marker; }

	virtual ElementMode2D get_mode() const = 0;
	virtual uint *get_vertices() = 0;

	virtual Boundary *copy() = 0;

	int id;										// boundary id

protected:
	int marker;
};


/// Triangular boundary
///
///
class BoundaryTri : public Boundary {
public:
	static const int NUM_VERTICES = 3;
	static const int NUM_EDGES = 3;

	BoundaryTri(uint vtcs[], int marker);
	BoundaryTri(const BoundaryTri &o);
	virtual ~BoundaryTri();

	virtual Boundary *copy();
	virtual ElementMode2D get_mode() const { return HERMES_MODE_TRIANGLE; }
	virtual uint *get_vertices() { return vtcs; }

protected:
	uint vtcs[NUM_VERTICES];
};


/// Quadrilateral boundary
///
class BoundaryQuad : public Boundary {
public:
	static const int NUM_VERTICES = 4;
	static const int NUM_EDGES = 4;

	BoundaryQuad(uint vtcs[], int marker);
	BoundaryQuad(const BoundaryQuad &o);
	virtual ~BoundaryQuad();

	virtual Boundary *copy();
	virtual ElementMode2D get_mode() const { return HERMES_MODE_QUAD; }
	virtual uint *get_vertices() { return vtcs; }

protected:
	uint vtcs[NUM_VERTICES];
};


// Mesh ///////////////////////////////////////////////////////////////////////

class Mesh {
public:
	Mesh();

	/// Loads the mesh from a file. Aborts the program on error.
	/// @param filename [in] The name of the file.
	bool load(const char *file_name, Mesh3DLoader *loader);

	/// Adds an vertex
	int add_vertex(double x, double y, double z);
	/// Adds an element
	/// @param[in] e An element to add
	Tetra *add_tetra(uint vtcs[]);
	Hex *add_hex(uint vtcs[]);
	Prism *add_prism(uint vtcs[]);

	Boundary *add_tri_boundary(uint vtcs[], int marker);
	Boundary *add_quad_boundary(uint vtcs[], int marker);

	// data
	JudyArray<Vertex *>   vertices;
//	MapOrd<Edge>      edges;
	JudyArray<Element *>  elements;
	JudyArray<Boundary *> boundaries;
	MapOrd<Facet *>   facets;

protected:
	Tetra *create_tetra(uint vtcs[]);
	Hex *create_hex(uint vtcs[]);
	Prism *create_prism(uint vtcs[]);


};

#endif
