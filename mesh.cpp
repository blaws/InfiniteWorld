// Benjamin Laws
// CSE 40166 - Computer Graphics
// October 17, 2014
// Midterm Project

#include "mesh.h"

// vertices
std::vector<vec3> points[3][3];
std::vector<GLushort> indices[3][3];
std::vector<vec3> normals[3][3];
std::stack<mat4> mvStack;
std::stack<mat4> projStack;

// shader handles
GLuint shaderProgram;
GLuint pointBuffer[3][3];
GLuint indexBuffer[3][3];
GLuint mvMatrixLoc;
GLuint vPositionLoc;
GLuint vNormalLoc;
GLuint maxHeightLoc;
GLuint texture;
GLuint textureLoc;

// camera and display
vec4 cameraPos;
float cameraRotXY;
float cameraRotZ;
float tripodHeight;

//==========================================================

void bufferData( int meshX, int meshY )
{
    // indices
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexBuffer[meshX][meshY] );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices[meshX][meshY].size() * sizeof( GLshort ), &indices[meshX][meshY][0], GL_DYNAMIC_DRAW );
    
    // vertex positions
    glBindBuffer( GL_ARRAY_BUFFER, pointBuffer[meshX][meshY] );
    glBufferData( GL_ARRAY_BUFFER, ( points[meshX][meshY].size() + normals[meshX][meshY].size() ) * sizeof( vec3 ), NULL, GL_DYNAMIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, points[meshX][meshY].size() * sizeof( vec3 ), &points[meshX][meshY][0] );
    
    // vertex normals
    glBufferSubData( GL_ARRAY_BUFFER, points[meshX][meshY].size() * sizeof( vec3 ), normals[meshX][meshY].size() * sizeof( vec3 ), &normals[meshX][meshY][0] );
}

void cloudBoxTerrain( std::vector<vec3>& mesh, int left, int bottom, int right, int top )
{
    // check base of recursion
    if( right - left < 2 && top - bottom < 2 )
    {
        return;
    }
    
    // record height of corners
    float z1 = mesh[ TWODIM( left, bottom ) ].z;
    float z2 = mesh[ TWODIM( right, bottom ) ].z;
    float z3 = mesh[ TWODIM( right, top ) ].z;
    float z4 = mesh[ TWODIM( left, top ) ].z;
    
    // find midpoint lines
    int xMid = ( left + right ) / 2;
    int yMid = ( bottom + top ) / 2;
    int distFactor = std::ceil( ( right - left ) / 4.0 ) ;
    
    // fill in midpoints, if they are currently empty
    if( mesh[ TWODIM( xMid, bottom ) ].z == -1.0 )
    {
        mesh[ TWODIM( xMid, bottom ) ] = vec3( xMid, bottom, ( z1 + z2 ) / 2.0 + ( rand()%2 ? 1.0 : -1.0 ) * ( rand() % distFactor ) );
    }
    if( mesh[ TWODIM( right, yMid ) ].z == -1.0 )
    {
        mesh[ TWODIM( right, yMid ) ] = vec3( right, yMid, ( z2 + z3 ) / 2.0 + ( rand()%2 ? 1.0 : -1.0 ) * ( rand() % distFactor ) );
    }
    if( mesh[ TWODIM( xMid, top ) ].z == -1.0 )
    {
        mesh[ TWODIM( xMid, top ) ] = vec3( xMid, top, ( z3 + z4 ) / 2.0 + ( rand()%2 ? 1.0 : -1.0 ) * ( rand() % distFactor ) );
    }
    if( mesh[ TWODIM( left, yMid ) ].z == -1.0 )
    {
        mesh[ TWODIM( left, yMid ) ] = vec3( left, yMid, ( z4 + z1 ) / 2.0 + ( rand()%2 ? 1.0 : -1.0 ) * ( rand() % distFactor ) );
    }
    if( mesh[ TWODIM( xMid, yMid ) ].z == -1.0 )
    {
        float height = ( z1 + z2 + z3 + z4 ) / 4.0 + ( rand()%2 ? 1.0 : -1.0 ) * ( rand() % (int)( distFactor * sqrt( 2.0 ) ) );
        mesh[ TWODIM( xMid, yMid ) ] = vec3( xMid, yMid, height );
    }
    
    // recurse
    cloudBoxTerrain( mesh, left, bottom, xMid, yMid );
    cloudBoxTerrain( mesh, xMid, bottom, right, yMid );
    cloudBoxTerrain( mesh, xMid, yMid, right, top );
    cloudBoxTerrain( mesh, left, yMid, xMid, top );
}

void connectEdges( int meshX, int meshY, int saveEdges )
{
    if( meshX > 0 && ( saveEdges & LEFT ) )
    {
        for( int y = 0; y < gridSize; ++y )
        {
            points[meshX][meshY][ TWODIM(0,y) ] = vec3( 0.0, y, points[meshX-1][meshY][ TWODIM(gridSize-1,y) ].z );
        }
    }
    if( meshX < 3 && ( saveEdges & RIGHT ) )
    {
        for( int y = 0; y < gridSize; ++y )
        {
            points[meshX][meshY][ TWODIM(gridSize-1,y) ] = vec3( gridSize-1.0, y, points[meshX+1][meshY][ TWODIM(0,y) ].z );
        }
    }
    if( meshY > 0 && ( saveEdges & DOWN ) )
    {
        for( int x = 0; x < gridSize; ++x )
        {
            points[meshX][meshY][ TWODIM(x,0) ] = vec3( x, 0.0, points[meshX][meshY-1][ TWODIM(x,gridSize-1) ].z );
        }
    }
    if( meshY < 3 && ( saveEdges & UP ) )
    {
        for( int x = 0; x < gridSize; ++x )
        {
            points[meshX][meshY][ TWODIM(x,gridSize-1) ] = vec3( x, gridSize-1.0, points[meshX][meshY+1][ TWODIM(x,0) ].z );
        }
    }
}

float getZ( float x, float y, int meshX /* = 1 */, int meshY /* = 1 */ )
{
    // determine which mesh (x,y) falls on
    if( x < 0.0 && meshX > 0 )
    {
        x += gridSize - 1;
        --meshX;
    }
    else if( x > gridSize - 1 && meshX < 2 )
    {
        x = x - ( gridSize - 1 );
        ++meshX;
    }
    if( y < 0.0 && meshY > 0 )
    {
        y += gridSize - 1;
        --meshY;
    }
    else if( y > gridSize - 1 && meshY < 2 )
    {
        y = y - ( gridSize - 1 );
        ++meshY;
    }
    
    // clamp x and y to values with valid heights
    if( x < 0.0 || x > gridSize - 1 || y < 0.0 || y > gridSize - 1 )
    {
        x = std::fmax( 0.0, std::fmin( gridSize - 1, x ) );
        y = std::fmax( 0.0, std::fmin( gridSize - 1, y ) );
    }
    
    if( meshX < 0 || meshX > 2 || meshY < 0 || meshY > 2 )
    {
        //std::cout << "bounds error: getZ()" << std::endl;
        return -1.0;
    }
    
    // find nearest integer positions
    int x1 = std::floor( x );
    int x2 = std::ceil( x );
    int y1 = std::floor( y );
    int y2 = std::ceil( y );
    
    // check whether (x,y) is an exact vertex, for less calculation
    if( x1 == x2 && y1 == y2 )
    {
        return points[meshX][meshY][ TWODIM( x1, y1 ) ].z;
    }

    // get distances from vertices
    float dx = x - std::floor( x );
    float dy = y - std::floor( y );
    
    // calculate horizontal interpolations
    float z1 = points[meshX][meshY][ TWODIM( x1, y1 ) ].z * ( 1.0 - dx )
               + points[meshX][meshY][ TWODIM( x2, y1 ) ].z * dx;
    float z2 = points[meshX][meshY][ TWODIM( x1, y2 ) ].z * ( 1.0 - dx )
               + points[meshX][meshY][ TWODIM( x2, y2 ) ].z * dx;
    
    // return vertical interpolation of horizontal interpolations
    return z1 * ( 1.0 - dy ) + z2 * dy;
}

void createMeshNormals( int meshX, int meshY )
{
    std::vector<vec3>& curNormals = normals[meshX][meshY];
    curNormals.clear();
    
    for( int y = 0; y < gridSize; ++y )
    {
        for( int x = 0; x < gridSize; ++x )
        {
            vec3 xDiff( 2.0, 0.0, getZ( x + 1, y, meshX, meshY ) - getZ( x - 1, y, meshX, meshY ) );
            vec3 yDiff( 0.0, 2.0, getZ( x, y + 1, meshX, meshY ) - getZ( x, y - 1, meshX, meshY ) );
            
            curNormals.push_back( normalize( cross( xDiff, yDiff ) ) );
        }
    }
}

void createMeshIndices( std::vector<vec3>& curPoints, std::vector<GLushort>& curIndices )
{
    for( GLushort y = 1; y < gridSize; ++y )
    {
        for( GLushort x = 1; x < gridSize; ++x )
        {
            int v1 = TWODIM( x, y );
            int v2 = TWODIM( x - 1, y );
            int v3 = TWODIM( x, y - 1 );
            int v4 = TWODIM( x - 1, y - 1 );
            
            // align split direction of triangles with
            // the shorter of the two diagonals
            if( fabs( curPoints[v1].z - curPoints[v4].z )
               < fabs( curPoints[v2].z - curPoints[v3].z ) )
            {
                curIndices.push_back( v1 );
                curIndices.push_back( v2 );
                curIndices.push_back( v4 );
                
                curIndices.push_back( v1 );
                curIndices.push_back( v4 );
                curIndices.push_back( v3 );
            }
            else
            {
                curIndices.push_back( v1 );
                curIndices.push_back( v2 );
                curIndices.push_back( v3 );
                
                curIndices.push_back( v2 );
                curIndices.push_back( v4 );
                curIndices.push_back( v3 );
            }
        }
    }
}

void newMesh( int meshX, int meshY, int saveEdges )
{
    std::vector<vec3>& curPoints = points[meshX][meshY];
    std::vector<GLushort>& curIndices = indices[meshX][meshY];
    std::vector<vec3>& curNormals = normals[meshX][meshY];
    
    // reset vectors, but keep size of points
    curPoints.clear();
    curPoints.resize( gridSize * gridSize, vec3( -1.0, -1.0, -1.0 ) );
    curIndices.clear();
    curNormals.clear();
    
    // save edges of relevant adjacent meshes
    if( saveEdges )
    {
        connectEdges( meshX, meshY, saveEdges );
    }
    
    // create vertices
    if( curPoints[ TWODIM( 0, 0 ) ].z == -1.0 )
    {
        curPoints[ TWODIM( 0, 0 ) ] = vec3( 0.0, 0.0, rand()%heightSteps / ( (float)heightSteps / maxHeight ) );
    }
    if( curPoints[ TWODIM( gridSize-1, 0 ) ].z == -1.0 )
    {
        curPoints[ TWODIM( gridSize-1, 0 ) ] = vec3( gridSize-1.0, 0.0, rand()%heightSteps / ( (float)heightSteps / maxHeight ) );
    }
    if( curPoints[ TWODIM( gridSize-1, gridSize-1 ) ].z == -1.0 )
    {
        curPoints[ TWODIM( gridSize-1, gridSize-1 ) ] = vec3( gridSize-1.0, gridSize-1.0, rand()%heightSteps / ( (float)heightSteps / maxHeight ) );
    }
    if( curPoints[ TWODIM( 0, gridSize-1 ) ].z == -1.0 )
    {
        curPoints[ TWODIM( 0, gridSize-1 ) ] = vec3( 0.0, gridSize-1.0, rand()%heightSteps / ( (float)heightSteps / maxHeight ) );
    }
    cloudBoxTerrain( curPoints, 0, 0, gridSize - 1, gridSize - 1 );
    
    // fill in indices
    createMeshIndices( curPoints, curIndices );
}

void moveToNextMesh()
{
    // determine in which direction to move
    if( cameraPos.x < 0.0 )
    {
        cameraPos.x += gridSize - 1;
        
        for( int meshY = 0; meshY < 3; ++meshY )
        {
            GLuint tmpPB = pointBuffer[2][meshY];
            GLuint tmpIB = indexBuffer[2][meshY];
            
            for( int meshX = 2; meshX >= 1; --meshX )
            {
                points[meshX][meshY] = points[meshX-1][meshY];
                indices[meshX][meshY] = indices[meshX-1][meshY];
                normals[meshX][meshY] = normals[meshX-1][meshY];
                
                pointBuffer[meshX][meshY] = pointBuffer[meshX-1][meshY];
                indexBuffer[meshX][meshY] = indexBuffer[meshX-1][meshY];
            }
            
            pointBuffer[0][meshY] = tmpPB;
            indexBuffer[0][meshY] = tmpIB;
            
            newMesh( 0, meshY, RIGHT | DOWN );
        }
        for( int meshY = 0; meshY < 3; ++meshY )
        {
            createMeshNormals( 0, meshY );
            bufferData( 0, meshY );
        }
    }
    else if( cameraPos.x > gridSize - 1 )
    {
        cameraPos.x -= gridSize - 1;
        
        for( int meshY = 0; meshY < 3; ++meshY )
        {
            GLuint tmpPB = pointBuffer[0][meshY];
            GLuint tmpIB = indexBuffer[0][meshY];
            
            for( int meshX = 0; meshX <= 1; ++meshX )
            {
                points[meshX][meshY] = points[meshX+1][meshY];
                indices[meshX][meshY] = indices[meshX+1][meshY];
                normals[meshX][meshY] = normals[meshX+1][meshY];
                
                pointBuffer[meshX][meshY] = pointBuffer[meshX+1][meshY];
                indexBuffer[meshX][meshY] = indexBuffer[meshX+1][meshY];
            }
            
            pointBuffer[2][meshY] = tmpPB;
            indexBuffer[2][meshY] = tmpIB;
            
            newMesh( 2, meshY, LEFT | DOWN );
        }
        for( int meshY = 0; meshY < 3; ++meshY )
        {
            createMeshNormals( 2, meshY );
            bufferData( 2, meshY );
        }
    }
    if( cameraPos.y < 0.0 )
    {
        cameraPos.y += gridSize - 1;
        
        for( int meshX = 0; meshX < 3; ++meshX )
        {
            GLuint tmpPB = pointBuffer[meshX][2];
            GLuint tmpIB = indexBuffer[meshX][2];
            
            for( int meshY = 2; meshY >= 1; --meshY )
            {
                points[meshX][meshY] = points[meshX][meshY-1];
                indices[meshX][meshY] = indices[meshX][meshY-1];
                normals[meshX][meshY] = normals[meshX][meshY-1];
                
                pointBuffer[meshX][meshY] = pointBuffer[meshX][meshY-1];
                indexBuffer[meshX][meshY] = indexBuffer[meshX][meshY-1];
            }
            
            pointBuffer[meshX][0] = tmpPB;
            indexBuffer[meshX][0] = tmpIB;
            
            newMesh( meshX, 0, LEFT | UP );
        }
        for( int meshX = 0; meshX < 3; ++meshX )
        {
            createMeshNormals( meshX, 0 );
            bufferData( meshX, 0 );
        }
    }
    else if( cameraPos.y > gridSize - 1 )
    {
        cameraPos.y -= gridSize - 1;
        
        for( int meshX = 0; meshX < 3; ++meshX )
        {
            GLuint tmpPB = pointBuffer[meshX][0];
            GLuint tmpIB = indexBuffer[meshX][0];
            
            for( int meshY = 0; meshY <= 1; ++meshY )
            {
                points[meshX][meshY] = points[meshX][meshY+1];
                indices[meshX][meshY] = indices[meshX][meshY+1];
                normals[meshX][meshY] = normals[meshX][meshY+1];
                
                pointBuffer[meshX][meshY] = pointBuffer[meshX][meshY+1];
                indexBuffer[meshX][meshY] = indexBuffer[meshX][meshY+1];
            }
            
            pointBuffer[meshX][2] = tmpPB;
            indexBuffer[meshX][2] = tmpIB;
            
            newMesh( meshX, 2, LEFT | DOWN );
        }
        for( int meshX = 0; meshX < 3; ++meshX )
        {
            createMeshNormals( meshX, 2 );
            bufferData( meshX, 2 );
        }
    }
}
