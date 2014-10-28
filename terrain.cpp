// Benjamin Laws
// CSE 40166 - Computer Graphics
// October 17, 2014
// Midterm Project

#include "mesh.h"

// window & framerate
int windowW = 700;
int windowH = 700;
const float fps = 30.0;
const float mspf = 1000.0 / fps;

// options
bool solidMesh = true;
bool viewLimitOn = true;
float viewLimit = gridSize;

// movement
const float moveInc = 0.5;
const float maxMove = gridSize / 8.0;
const float lookInc = 0.05;
direction moveDir = NONE;
direction lookDir = NONE;
direction heightDir = NONE;

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
            heightDir = UP;
            break;
        case '-':
        case '_':
            heightDir = DOWN;
            break;
        case '[':
        case '{':
            viewLimitOn = true;
            viewLimit /= 1.1;
            calculateView();
            glutPostRedisplay();
            break;
        case ']':
        case '}':
            viewLimitOn = true;
            viewLimit *= 1.1;
            calculateView();
            glutPostRedisplay();
            break;
        case '\\':
        case '|':
            viewLimitOn = !viewLimitOn;
            calculateView();
            glutPostRedisplay();
            break;

        case 'w':
        case 'W':
            lookDir = static_cast<direction>( ( lookDir & ~DOWN ) | UP );
            break;
        case 's':
        case 'S':
            lookDir = static_cast<direction>( ( lookDir & ~UP ) | DOWN );
            break;
        case 'a':
        case 'A':
            lookDir = static_cast<direction>( ( lookDir & ~RIGHT ) | LEFT );
            break;
        case 'd':
        case 'D':
            lookDir = static_cast<direction>( ( lookDir & ~LEFT ) | RIGHT );
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
            glutPostRedisplay();
            break;
            
        case 033:  // 'ESC'
            exit( EXIT_SUCCESS );
            
        default:
            break;
    }
}

void keyboardUp( unsigned char key, int x, int y )
{
    switch( key )
    {
        case '=':
        case '+':
        case '-':
        case '_':
            heightDir = NONE;
            break;
            
        case 'w':
        case 'W':
            lookDir = static_cast<direction>( lookDir & ~UP );
            break;
        case 's':
        case 'S':
            lookDir = static_cast<direction>( lookDir & ~DOWN );
            break;
        case 'a':
        case 'A':
            lookDir = static_cast<direction>( lookDir & ~LEFT );
            break;
        case 'd':
        case 'D':
            lookDir = static_cast<direction>( lookDir & ~RIGHT );
            break;
        default:
            break;
    }
}

void keyboardSpecials( int key, int x, int y )
{
    switch( key )
    {
        case GLUT_KEY_UP:
            moveDir = static_cast<direction>( ( moveDir & ~DOWN ) | UP );
            break;
        case GLUT_KEY_DOWN:
            moveDir = static_cast<direction>( ( moveDir & ~UP ) | DOWN );
            break;
        case GLUT_KEY_RIGHT:
            moveDir = static_cast<direction>( ( moveDir & ~LEFT ) | RIGHT );
            break;
        case GLUT_KEY_LEFT:
            moveDir = static_cast<direction>( ( moveDir & ~RIGHT ) | LEFT );
            break;
        default:
            break;
    }
}

void keyboardSpecialsUp( int key, int x, int y )
{
    switch( key )
    {
        case GLUT_KEY_UP:
            moveDir = static_cast<direction>( moveDir & ~UP );
            break;
        case GLUT_KEY_DOWN:
            moveDir = static_cast<direction>( moveDir & ~DOWN );
            break;
        case GLUT_KEY_RIGHT:
            moveDir = static_cast<direction>( moveDir & ~RIGHT );
            break;
        case GLUT_KEY_LEFT:
            moveDir = static_cast<direction>( moveDir & ~LEFT );
            break;
        default:
            break;
    }
}

void move( int t )
{
    // skip if no movement
    if( moveDir == NONE && lookDir == NONE && heightDir == NONE )
    {
        glutTimerFunc( mspf, move, 0 );
        return;
    }

    // move
    if( moveDir & UP )
    {
        cameraPos.x += std::fmax( std::fmin( maxMove, tripodHeight / 4.0 ), moveInc ) * cos( cameraRotXY ) * moveInc;
        cameraPos.y += std::fmax( std::fmin( maxMove, tripodHeight / 4.0 ), moveInc ) * sin( cameraRotXY ) * moveInc;
    }
    else if( moveDir & DOWN )
    {
        cameraPos.x -= std::fmax( std::fmin( maxMove, tripodHeight / 4.0 ), moveInc ) * cos( cameraRotXY ) * moveInc;
        cameraPos.y -= std::fmax( std::fmin( maxMove, tripodHeight / 4.0 ), moveInc ) * sin( cameraRotXY ) * moveInc;
    }
    if( moveDir & RIGHT )
    {
        cameraPos.x -= std::fmax( std::fmin( maxMove, tripodHeight / 4.0 ), moveInc ) * cos( cameraRotXY + M_PI / 2.0 ) * moveInc;
        cameraPos.y -= std::fmax( std::fmin( maxMove, tripodHeight / 4.0 ), moveInc ) * sin( cameraRotXY + M_PI / 2.0 ) * moveInc;
    }
    else if( moveDir & LEFT )
    {
        cameraPos.x += std::fmax( std::fmin( maxMove, tripodHeight / 4.0 ), moveInc ) * cos( cameraRotXY + M_PI / 2.0 ) * moveInc;
        cameraPos.y += std::fmax( std::fmin( maxMove, tripodHeight / 4.0 ), moveInc ) * sin( cameraRotXY + M_PI / 2.0 ) * moveInc;
    }

    // look
    if( lookDir & UP )
    {
        cameraRotZ += lookInc;
        if( cameraRotZ >= M_PI / 2.0 )
        {
            cameraRotZ = M_PI / 2.0 - lookInc;
        }
    }
    else if( lookDir & DOWN )
    {
        cameraRotZ -= lookInc;
        if( cameraRotZ <= -M_PI / 2.0 )
        {
            cameraRotZ = -M_PI / 2.0 + lookInc;
        }
    }
    if( lookDir & RIGHT )
    {
        cameraRotXY -= lookInc;
        if( cameraRotXY < 0.0 )
        {
            cameraRotXY += 2.0 * M_PI;
        }
    }
    else if( lookDir & LEFT )
    {
        cameraRotXY += lookInc;
        if( cameraRotXY > 2.0 * M_PI )
        {
            cameraRotXY -= 2.0 * M_PI;
        }
    }
    
    // height
    if( heightDir == UP )
    {
        tripodHeight *= 1.0 + lookInc;
    }
    else if( heightDir == DOWN )
    {
        tripodHeight /= 1.0 + lookInc;
        if( tripodHeight < moveInc )
        {
            tripodHeight = moveInc;
        }
    }
    
    // check mesh edges
    if( cameraPos.x < 0.0 || cameraPos.x > gridSize - 1
       || cameraPos.y < 0.0 || cameraPos.y > gridSize - 1 )
    {
        moveToNextMesh();
    }
    
    // redraw
    calculateView();
    glutPostRedisplay();

    // repeat
    glutTimerFunc( mspf, move, 0 );
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
    glutKeyboardUpFunc( keyboardUp );
    glutSpecialUpFunc( keyboardSpecialsUp );
    glutTimerFunc( mspf, move, 0 );
    
    // setup data
    if( !init() )
    {
        exit( EXIT_FAILURE );
    }
    
    // run
    glutMainLoop();
    
    return EXIT_SUCCESS;
}
