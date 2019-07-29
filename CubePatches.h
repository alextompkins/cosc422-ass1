//Vertex data
float verts[] = 
{
	// Front
	-1.0, -1.0, 1.0,
	1.0, -1.0, 1.0,
	1.0,  1.0, 1.0,
	-1.0,  1.0, 1.0,
	// Back
	-1.0, -1.0, -1.0,
	-1.0,  1.0, -1.0,
	1.0,  1.0, -1.0,
	1.0, -1.0, -1.0
};

//Polygon definitions of 6 faces using quads
GLushort elems[] = 
{
	0, 1, 2, 3,
	4, 5, 6, 7,
	1, 7, 6, 2,
	2, 6, 5, 3,
	5, 4, 0, 3,
	0, 4, 7, 1
};