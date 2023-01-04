#include "stdafx.h"

#include "Primitive3D.hpp"

using namespace r3;

float cubeCoords[24] = {
    -1.0f, 1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
     1.0f, 1.0f, 1.0f,
     1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
     1.0f, 1.0f,-1.0f,
     1.0f,-1.0f,-1.0f
};
u32 cubeIndices[36] = {
    0, 2, 3,	0, 3, 1, // front
    6, 4, 5,	6, 5, 7, // back
    4, 0, 1,	4, 1, 5, // left
    2, 6, 7,	2, 7, 3, // right
    0, 4, 6,	0, 6, 2, // top
    1, 7, 5,	1, 3, 7, // bottom
};


float arrowHeadCoords[42] = {
    -1.0f, 1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
     1.0f, 1.0f, 1.0f,
     1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
     1.0f, 1.0f,-1.0f,
     1.0f,-1.0f,-1.0f,

     3.0f, 1.0f, 0.0f,
     3.0f,-1.0f, 0.0f,
     1.0f, 1.0f, 1.5f,
     1.0f,-1.0f, 1.5f,
     1.0f, 1.0f,-1.5f,
     1.0f,-1.0f,-1.5f,
};
u32 arrowHeadIndices[72] = {
    0, 2, 3,	0, 3, 1, // front
    6, 4, 5,	6, 5, 7, // back
    4, 0, 1,	4, 1, 5, // left
    0, 4, 6,	0, 6, 2, // top
    1, 7, 5,	1, 3, 7, // bottom

    2, 6, 8,    7, 3, 9, // center
    2, 8, 10,   9, 3, 11, // top front
    8, 6, 12,   7, 9, 13, // top back

    8, 9, 11,	8, 11, 10, // side front
    8, 13, 9,	8, 12, 13, // side back

    10, 11, 3,  10, 3, 2,
    13, 12, 7,  12, 6, 7
};

int icoIndex[] = {
    0, 5, 11, 0, 1, 5, 0, 7, 1, 0, 10, 7, 0, 11, 10,
    11, 2, 10, 5, 4, 11, 1, 9, 5, 7, 8, 1, 10, 6, 7,
    3, 4, 9, 3, 2, 4, 3, 6, 2, 3, 8, 6, 3, 9, 8,
    9, 1, 8, 4, 5, 9, 2, 11, 4, 6, 10, 2, 8, 7, 6
};

Cube3D::Cube3D(r2::Node* parent) : Sprite3D(parent) {
	depthWrite = true;
	depthRead = true;
}

void Cube3D::drawSubmitGeometry(rs::GfxContext* _g)  {
	Pasta::Graphic* g = _g->gfx;
	Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();
	const int stride = 3;
	float vertexData[8 * stride]; // pos

	for (int i = 0; i < 8; i++) {
		float* pos = &vertexData[i * stride];
		pos[0] = cubeCoords[i * 3 + 0];
		pos[1] = cubeCoords[i * 3 + 1];
		pos[2] = cubeCoords[i * 3 + 2];
	}

	g->drawIndexed(Pasta::PT_TRIANGLE_STRIP, vertexData, 8, cubeIndices, 36);
}

r3::Icosphere3D::Icosphere3D(r2::Node* parent, int order) : Sprite3D(parent) {
	depthWrite = true;
	depthRead = true;
    makeIcosphere(order);
}

r3::Icosphere3D::~Icosphere3D() {
    if (vertexData) free(vertexData); vertexData = nullptr;
    if (indexData) free(indexData); indexData = nullptr;
}

#define SETONE(index) vertexData[index] = -1; vertexData[index + stride] = 1; vertexData[index + stride * 2] = -1; vertexData[index + stride * 3] = 1;
#define SETZERO(index) vertexData[index] = 0; vertexData[index + stride] = 0; vertexData[index + stride * 2] = 0; vertexData[index + stride * 3] = 0;
#define SETF(index) vertexData[index] = f; vertexData[index + stride] = f; vertexData[index + stride * 2] = -f; vertexData[index + stride * 3] = -f;

void r3::Icosphere3D::makeIcosphere(int order) {
    // set up a 20-triangle icosahedron
    float f = (1.0f + powf(5.0f, 0.5f)) / 2.0f;
    int T = pow(4, order);
    
    int stride = 3;
    vertexCount = (10 * T + 2);
    vertexData = (float*)malloc(vertexCount * stride * sizeof(float));
    SETONE(0); SETF(1); SETZERO(2);
    SETZERO(4 * stride); SETONE(4 * stride + 1); SETF(4 * stride + 2);
    SETF(4 * stride * 2); SETZERO(4 * stride * 2 + 1); SETONE(4 * stride * 2 + 2);

    indexCount = 60 * pow(4, order);
    indexData = (u32*)malloc(indexCount * sizeof(u32));
    for (int i = 0; i < 60; i++)
        indexData[i] = icoIndex[i];

    if (order) {
        u32 v = 12;
        std::unordered_map<u32, u32> midCache;

        auto addMidPoint = [&](u32 a, u32 b) {
            u32 key = floor((a + b) * (a + b + 1) / 2) + std::min(a, b); // Cantor's pairing function

            auto iPos = midCache.find(key);
            if (midCache.find(key) != midCache.end()) {
                u32 i = midCache[key];
                midCache.erase(iPos);
                return i;
            }
            midCache[key] = v;

            for (int k = 0; k < 3; k++) vertexData[stride * v + k] = (vertexData[stride * a + k] + vertexData[stride * b + k]) / 2.0f;
            
            return v++;
        };

        int triangleCount = 60;
        auto tmp = (u32*)malloc(indexCount * sizeof(u32));
        for (int i = 0; i < order; i++) {
            for (int k = 0; k < triangleCount; k += 3) {
                u32 v1 = indexData[k + 0];
                u32 v2 = indexData[k + 1];
                u32 v3 = indexData[k + 2];
                u32 a = addMidPoint(v1, v2);
                u32 b = addMidPoint(v2, v3);
                u32 c = addMidPoint(v3, v1);
                u32 t = k * 4;
                tmp[t++] = v1; tmp[t++] = a; tmp[t++] = c;
                tmp[t++] = v2; tmp[t++] = b; tmp[t++] = a;
                tmp[t++] = v3; tmp[t++] = c; tmp[t++] = b;
                tmp[t++] = a;  tmp[t++] = b; tmp[t++] = c;
            }
            triangleCount *= 4;
            auto swap = tmp;
            tmp = indexData;
            indexData = swap;
        }
        free(tmp);
    }

    // normalize vertices
    for (int i = 0; i < vertexCount * stride; i += stride) {
        Vector3 vec(vertexData[i + 0], vertexData[i + 1], vertexData[i + 2]);
        vec.normalize();
        vertexData[i + 0] = vec.x;
        vertexData[i + 1] = vec.y;
        vertexData[i + 2] = vec.z;
    }
}

void r3::Icosphere3D::drawSubmitGeometry(rs::GfxContext* _g) {
	Pasta::Graphic* g = _g->gfx;
	Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();
	g->drawIndexed(Pasta::PT_TRIANGLES, vertexData, vertexCount, indexData, indexCount);
}

r3::Arrow3D::Arrow3D(r2::Node* parent) : Sprite3D(parent) {
    depthWrite = true;
    depthRead = true;
    makeArrow();
}

void r3::Arrow3D::makeArrow() {
    for (int n = 0; n < 3; n++) {
        for (int i = 0; i < 12; i++) {
            indices.push_back(cubeIndices[i * 3 + 0] + buff.size() / 3);
            indices.push_back(cubeIndices[i * 3 + 1] + buff.size() / 3);
            indices.push_back(cubeIndices[i * 3 + 2] + buff.size() / 3);
        }
        for (int i = 0; i < 8; i++) {
            float posX = cubeCoords[i * 3 + 0] * 10 + n * 40;
            buff.push_back(posX);
            buff.push_back(cubeCoords[i * 3 + 1] * 4);
            buff.push_back(cubeCoords[i * 3 + 2] * 10);
        }
    }

    for (int i = 0; i < 24; i++) {
        indices.push_back(arrowHeadIndices[i * 3 + 0] + buff.size() / 3);
        indices.push_back(arrowHeadIndices[i * 3 + 1] + buff.size() / 3);
        indices.push_back(arrowHeadIndices[i * 3 + 2] + buff.size() / 3);
    }
    for (int i = 0; i < 14; i++) {
        float posX = arrowHeadCoords[i * 3 + 0] * 10 + 3 * 40;
        buff.push_back(posX);
        buff.push_back(arrowHeadCoords[i * 3 + 1] * 4);
        buff.push_back(arrowHeadCoords[i * 3 + 2] * 10);
    }
    // range: -10 : 150

    //-4x^2+4x
}

void r3::Arrow3D::drawSubmitGeometry(rs::GfxContext* _g) {
    Pasta::Graphic* g = _g->gfx;
    Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();
    g->drawIndexed(Pasta::PT_TRIANGLES, buff.data(), buff.size() / 3, indices.data(), indices.size());
}
