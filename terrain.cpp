// Benjamin Laws
// CSE 40166 - Computer Graphics
// October 17, 2014
// Midterm Project

#include "mesh.h"

// window & framerate
int windowW = 700;
int windowH = 700;

// options
bool solidMesh = true;
bool viewLimitOn = true;
float viewLimit = gridSize;

//==========================================================

void calculateView()
{
    cameraPos.z = getZ( cameraPos.x, cameraPos.y ) + tripodHeight;
    
    vec4 lookAtPos( cameraPos.x + cos( cameraRotZ ) * cos( cameraRotXY ),
                    cameraPos.y + cos( cameraRotZ ) * sin( cameraRotXY ),
                    cameraPos.z + sin( cameraRotZ ),
                    1.0 );

    projStack.push( Perspective( 45.0, 1.0, 0.01, viewLimitOn ? viewLimit : 3.0 * std::fmax( gridSize, tripodHeight ) ) );
    mvStack.push( LookAt( cameraPos, lookAtPos, up ) );
}

void resetView()
{
    // camera
    tripodHeight = 4.0;
    cameraPos = vec4( gridSize/2.0, gridSize/2.0, 1.0, 1.0 );
    cameraPos.z = getZ( cameraPos.x, cameraPos.y ) + tripodHeight;
    cameraRotXY = M_PI / 2.0;
    cameraRotZ = 0.0;
    
    // matrix stacks
    mvStack.empty();
    projStack.empty();
    calculateView();
}

void createAndBufferTexture()
{
    // allocate texture
    glGenTextures( 1, &texture );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_3D, texture );
    
    // generate random pattern near base color
    GLubyte texValues[maxHeight][maxHeight][maxHeight][3];
    for( int l = 0; l < maxHeight; ++l )
    {
        for( int j = 0; j < maxHeight; ++j )
        {
            for( int i = 0; i < maxHeight; ++i )
            {
                for( int k = 0; k < 3; ++k )
                {
                    texValues[l][j][i][k] = 255 * ( baseGroundColor[k] + l / 255.0 ) + ( rand()%2 ? -1 : 1 ) * ( rand()%heightSteps / ( (float)heightSteps / 15.0 ) );
                    texValues[l][j][i][k] = std::max( (GLubyte)0, std::min( texValues[l][j][i][k], (GLubyte)255 ) );
                }
            }
        }
    }
    
    // create texture on GPU
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
    glTexImage3D( GL_TEXTURE_3D, 0, GL_RGB, maxHeight, maxHeight, maxHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, texValues );
    
    // link texture to shader variable
    textureLoc = glGetUniformLocation( shaderProgram, "u_sTexture" );
    glUniform1i( textureLoc, 0 );
}

int init( void )
{
    // general settings
    srand( time( NULL ) );
    glClearColor( skyColor.x, skyColor.y, skyColor.z, 1.0 );
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_TEXTURE_3D );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    
    // set up data and shaders
    for( int y = 0; y < 3; ++y )
    {
        for( int x = 0; x < 3; ++x )
        {
            newMesh( x, y, LEFT | DOWN );
        }
    }
    for( int y = 0; y < 3; ++y )  // normals must be calculated AFTER all vertices are set
    {
        for( int x = 0; x < 3; ++x )
        {
            createMeshNormals( x, y );
        }
    }
    resetView();
    shaderProgram = InitShader( "vert.glsl", "frag.glsl" );
    
    // buffer vertex and index data
    glGenBuffers( 9, (GLuint*)pointBuffer );
    glGenBuffers( 9, (GLuint*)indexBuffer );
    for( int x = 0; x < 3; ++x )
    {
        for( int y = 0; y < 3; ++y )
        {
            bufferData( x, y );
        }
    }
    
    // create ground texture
    createAndBufferTexture();
    
    // connect vertex attributes
    vPositionLoc = glGetAttribLocation( shaderProgram, "a_vPosition" );
    glEnableVertexAttribArray( vPositionLoc );
    vNormalLoc = glGetAttribLocation( shaderProgram, "a_vNormal" );
    glEnableVertexAttribArray( vNormalLoc );
    
    // get location of uniforms
    maxHeightLoc = glGetUniformLocation( shaderProgram, "u_fMaxHeight" );
    mvMatrixLoc = glGetUniformLocation( shaderProgram, "u_mv_Matrix" );
    
    glUniform1f( maxHeightLoc, maxHeight );
    
    return 1;
}

void keyboard( unsigned char key, int x, int y )
{
    switch( key )
    {
        case '=':
        case '+':
            tripodHeight /= 1.25;
            if( tripodHeight < 0.02 )
            {
                tripodHeight = 0.02;
            }
            break;
        case '-':
        case '_':
            tripodHeight *= 1.25;
            break;
        case '[':
        case '{':
            viewLimitOn = true;
            viewLimit /= 1.1;
            break;
        case ']':
        case '}':
            viewLimitOn = true;
            viewLimit *= 1.1;
            break;
        case '\\':
        case '|':
            viewLimitOn = !viewLimitOn;
            break;

        case 'w':
        case 'W':
            cameraRotZ += 0.1;
            if( cameraRotZ >= M_PI / 2.0 )
            {
                cameraRotZ = M_PI / 2.0 - 0.001;
            }
            break;
        case 's':
        case 'S':
            cameraRotZ -= 0.1;
            if( cameraRotZ <= -M_PI / 2.0 )
            {
                cameraRotZ = -M_PI / 2.0 + 0.001;
            }
            break;
        case 'a':
        case 'A':
            cameraRotXY += 0.1;
            if( cameraRotXY > 2.0 * M_PI )
            {
                cameraRotXY -= 2.0 * M_PI;
            }
            break;
        case 'd':
        case 'D':
            cameraRotXY -= 0.1;
            if( cameraRotXY < 0.0 )
            {
                cameraRotXY += 2.0 * M_PI;
            }
            break;
            
        case 13:   // 'ENTER'
            solidMesh = !solidMesh;
            glPolygonMode( GL_FRONT_AND_BACK, solidMesh ? GL_FILL : GL_LINE );
            if( solidMesh )
            {
                glClearColor( skyColor.x, skyColor.y, skyColor.z, 1.0 );
            }
            else
            {
                glClearColor( 0.0, 0.0, 0.0, 1.0 );
            }
            break;
            
        case 033:  // 'ESC'
            exit( EXIT_SUCCESS );
    }
    
    calculateView();
    glutPostRedisplay();
}

void keyboardSpecials( int key, int x, int y )
{
    switch( key )
    {
        case GLUT_KEY_UP:
            cameraPos.x += std::fmax( tripodHeight / 4.0, 0.5 ) * cos( cameraRotXY );
            cameraPos.y += std::fmax( tripodHeight / 4.0, 0.5 ) * sin( cameraRotXY );
            break;
        case GLUT_KEY_DOWN:
            cameraPos.x -= std::fmax( tripodHeight / 4.0, 0.5 ) * cos( cameraRotXY );
            cameraPos.y -= std::fmax( tripodHeight / 4.0, 0.5 ) * sin( cameraRotXY );
            break;
        case GLUT_KEY_LEFT:
            cameraPos.x += std::fmax( tripodHeight / 4.0, 0.5 ) * cos( cameraRotXY + M_PI / 2.0 );
            cameraPos.y += std::fmax( tripodHeight / 4.0, 0.5 ) * sin( cameraRotXY + M_PI / 2.0 );
            break;
        case GLUT_KEY_RIGHT:
            cameraPos.x -= std::fmax( tripodHeight / 4.0, 0.5 ) * cos( cameraRotXY + M_PI / 2.0 );
            cameraPos.y -= std::fmax( tripodHeight / 4.0, 0.5 ) * sin( cameraRotXY + M_PI / 2.0 );
            break;
    }

    if( cameraPos.x < 0.0 || cameraPos.x > gridSize - 1
        || cameraPos.y < 0.0 || cameraPos.y > gridSize - 1 )
    {
        moveToNextMesh();
    }
    
    calculateView();
    glutPostRedisplay();
}

void resize( int w, int h )
{
    windowW = w;
    windowH = h;
    
    // resize view to the largest square that
    // will fit inside the reshaped window
    if( w > h )
    {
        glViewport( 0, 0, h, h );
    }
    else
    {
        glViewport( 0, h - w, w, w );
    }
    
    glutPostRedisplay();
}

void display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    for( int x = 0; x < 3; ++x )
    {
        for( int y = 0; y < 3; ++y )
        {
            // set uniforms
            glUniformMatrix4fv( mvMatrixLoc, 1, GL_TRUE,
                                projStack.top() * mvStack.top() * Translate( ( x - 1 ) * ( gridSize - 1 ), ( y - 1 ) * ( gridSize - 1 ), 0.0 ) );

            // set attributes
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexBuffer[x][y] );
            glBindBuffer( GL_ARRAY_BUFFER, pointBuffer[x][y] );
            glVertexAttribPointer( vPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );
            glVertexAttribPointer( vNormalLoc, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( points[x][y].size() * sizeof( vec3 ) ) );
            
            // draw
            glDrawElements( GL_TRIANGLES, indices[x][y].size(), GL_UNSIGNED_SHORT, BUFFER_OFFSET( 0 ) );
        }
    }
    
    glutSwapBuffers();
}

//==========================================================

int main( int argc, char** argv )
{
    // initialize
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( windowW, windowH );
    glutCreateWindow( "CSE 40166 Midterm Project" );
    
    // set callbacks
    glutDisplayFunc( display );
    glutReshapeFunc( resize );
    glutKeyboardFunc( keyboard );
    glutSpecialFunc( keyboardSpecials );
    
    // setup data
    if( !init() )
    {
        exit( EXIT_FAILURE );
    }
    
    // run
    glutMainLoop();
    
    return EXIT_SUCCESS;
}
