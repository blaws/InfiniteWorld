// Benjamin Laws
// CSE 40166 - Computer Graphics
// October 17, 2014
// Midterm Project

#include <vector>
#include <stack>
#include "Angel.h"

#define TWODIM(x,y) ( (x) + (y) * gridSize )

// constants
const GLshort gridSize = 250;
const GLshort maxHeight = 50;
const GLshort heightSteps = 1000;
const vec4 up( 0.0, 0.0, 1.0, 0.0 );
const vec3 baseGroundColor( 0.086, 0.357, 0.192 );
const vec3 skyColor( 0.529, 0.808, 0.922 );

// vertices
extern std::vector<vec3> points[3][3];
extern std::vector<GLushort> indices[3][3];
extern std::vector<vec3> normals[3][3];
extern std::stack<mat4> mvStack;
extern std::stack<mat4> projStack;

// shader handles
extern GLuint shaderProgram;
extern GLuint pointBuffer[3][3];
extern GLuint indexBuffer[3][3];
extern GLuint mvMatrixLoc;
extern GLuint vPositionLoc;
extern GLuint vNormalLoc;
extern GLuint maxHeightLoc;
extern GLuint texture;
extern GLuint textureLoc;

// camera and display
extern vec4 cameraPos;
extern float cameraRotXY;
extern float cameraRotZ;
extern float tripodHeight;

enum direction
{
    NONE = 0x0000,
    LEFT = 0x0001,
    RIGHT = 0x0010,
    UP = 0x0100,
    DOWN = 0x1000
};

//==========================================================

void bufferData( int meshX, int meshY );

void cloudBoxTerrain( std::vector<vec3>& mesh, int left, int bottom, int right, int top );

void connectEdges( int meshX, int meshY, int saveEdges );

float getZ( float x, float y, int meshX = 1, int meshY = 1 );

void createMeshNormals( int meshX, int meshY );

void createMeshIndices( std::vector<vec3>& curPoints, std::vector<GLushort>& curIndices );

void newMesh( int meshX, int meshY, int saveEdges );

void moveToNextMesh();
