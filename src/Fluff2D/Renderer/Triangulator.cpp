#include "Triangulator.h"

void Triangulator::triangulate(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices)
{
    indices.clear();

    float xMin = vertices[0].position.x;
    float yMin = vertices[0].position.y;
    float xMax = xMin;
    float yMax = yMin;

    for (int i = 0; i < vertices.size(); i++)
    {
        glm::vec2 v = vertices[i].position;

        xMin = std::min(xMin, v.x);
        yMin = std::min(yMin, v.y);
        xMax = std::max(xMax, v.x);
        yMax = std::max(yMax, v.y);
    }

    float dX = xMax - xMin;
    float dY = yMax - yMin;
    float dMax = std::max(dX, dY);
    float xMid = (xMax + xMin) / 2.0f;
    float yMid = (yMax + yMin) / 2.0f;

    std::vector<glm::vec2> expanded;
    expanded.resize(vertices.size() + 3);
    for (int i = 0; i < vertices.size(); i++)
    {
        expanded[i] = vertices[i].position;
    }
    expanded[expanded.size() - 3] = glm::vec2(xMid - 2 * dMax, yMid - dMax);
    expanded[expanded.size() - 2] = glm::vec2(xMid, yMid + 2 * dMax);
    expanded[expanded.size() - 1] = glm::vec2(xMid + 2 * dMax, yMid - dMax);

    std::vector<Triangle> triangleList;
    triangleList.emplace_back((unsigned int)vertices.size(), (unsigned int)vertices.size() + 1, (unsigned int)vertices.size() + 2);
    for (int i1 = 0; i1 < vertices.size(); i1++)
    {
        std::vector<Edge> edgeList;
        int i2 = 0;
        while (i2 < triangleList.size())
        {
            if (inCircle(expanded[i1], expanded[triangleList[i2].p1], expanded[triangleList[i2].p2], expanded[triangleList[i2].p3]))
            {
                edgeList.emplace_back(triangleList[i2].p1, triangleList[i2].p2);
                edgeList.emplace_back(triangleList[i2].p2, triangleList[i2].p3);
                edgeList.emplace_back(triangleList[i2].p3, triangleList[i2].p1);
                triangleList.erase(triangleList.begin() + i2);
                i2--;
            }
            i2++;
        }

        i2 = (int)edgeList.size() - 2;
        while (i2 >= 0)
        {
            int i3 = (int)edgeList.size() - 1;
            while (i3 >= i2 + 1)
            {
                if (edgeList[i2].Equals(edgeList[i3]))
                {
                    edgeList.erase(edgeList.begin() + i3);
                    edgeList.erase(edgeList.begin() + i2);
                    i3--;
                }
                i3--;
            }
            i2--;
        }

        i2 = 0;
        while (i2 < edgeList.size())
        {
            triangleList.emplace_back(edgeList[i2].p1, edgeList[i2].p2, i1);
            i2++;
        }
        edgeList.clear();
    }

    for (int i = (int)triangleList.size() - 1; i >= 0; i--)
    {
        if (triangleList[i].p1 >= vertices.size() || triangleList[i].p2 >= vertices.size() || triangleList[i].p3 >= vertices.size())
            triangleList.erase(triangleList.begin() + i);
    }

    //tris = triangleList.ToArray();
    for (int i = 0; i < triangleList.size(); i++)
    {
        indices.push_back(triangleList[i].p1);
        indices.push_back(triangleList[i].p2);
        indices.push_back(triangleList[i].p3);
    }
}

bool Triangulator::inCircle(glm::vec2 p, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
{
    float floatEpsilon = 0.000001f;

    if (std::abs(p1.y - p2.y) < floatEpsilon && std::abs(p2.y - p3.y) < floatEpsilon)
        return false;

    float m1, m2, mx1, mx2, my1, my2, xc, yc;

    if (std::abs(p2.y - p1.y) < floatEpsilon)
    {
        m2 = -(p3.x - p2.x) / (p3.y - p2.y);
        mx2 = (p2.x + p3.x) / 2.0f;
        my2 = (p2.y + p3.y) / 2.0f;
        xc = (p2.x + p1.x) / 2.0f;
        yc = m2 * (xc - mx2) + my2;
    }

    else if (std::abs(p3.y - p2.y) < floatEpsilon)
    {
        m1 = -(p2.x - p1.x) / (p2.y - p1.y);
        mx1 = (p1.x + p2.x) / 2.0f;
        my1 = (p1.y + p2.y) / 2.0f;
        xc = (p3.x + p2.x) / 2.0f;
        yc = m1 * (xc - mx1) + my1;
    }
    else
    {
        m1 = -(p2.x - p1.x) / (p2.y - p1.y);
        m2 = -(p3.x - p2.x) / (p3.y - p2.y);
        mx1 = (p1.x + p2.x) / 2.0f;
        mx2 = (p2.x + p3.x) / 2.0f;
        my1 = (p1.y + p2.y) / 2.0f;
        my2 = (p2.y + p3.y) / 2.0f;
        xc = (m1 * mx1 - m2 * mx2 + my2 - my1) / (m1 - m2);
        yc = m1 * (xc - mx1) + my1;
    }

    float dx = p2.x - xc;
    float dy = p2.y - yc;
    float rSqr = dx * dx + dy * dy;
    dx = p.x - xc;
    dy = p.y - yc;
    float drSqr = dx * dx + dy * dy;

    return drSqr <= rSqr;
}
