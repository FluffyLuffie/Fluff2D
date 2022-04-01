#pragma once
#include <vector>
#include "Vertex.h"

//based off of https://github.com/TassuP/GodotStuff/blob/master/DelaunayTriangulator/Delaunay.gd

class Triangulator
{
public:
	static void triangulate(std::vector<Vertex> &vertices, std::vector<unsigned int>& indices);
private:
	class Triangle
	{
	public:
		unsigned int p1, p2, p3;
		Triangle(int _p1, int _p2, int _p3)
		{
			p1 = _p1;
			p2 = _p2;
			p3 = _p3;
		}
	};

	class Edge
	{
	public:
		int p1, p2;

		Edge(unsigned int _p1, unsigned int _p2)
		{
			p1 = _p1;
			p2 = _p2;
		}

		bool Equals(Edge other)
		{
			return ((p1 == other.p2) && (p2 == other.p1)) || ((p1 == other.p1) && (p2 == other.p2));
		}
	};

	static bool inCircle(glm::vec2 p, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);
};

