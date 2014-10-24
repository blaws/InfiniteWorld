// Benjamin Laws
// CSE 40166 - Computer Graphics
// October 17, 2014
// Midterm Project

uniform mat4 u_mv_Matrix;
uniform float u_fMaxHeight;

attribute vec3 a_vPosition;
attribute vec3 a_vNormal;

varying vec3 v_vNormal;
varying vec3 v_vTexCoord;

//==========================================================

void main()
{
    gl_Position = u_mv_Matrix * vec4( a_vPosition, 1.0 );
    
    v_vNormal = normalize( a_vNormal );
    v_vTexCoord = vec3( a_vPosition.xy, a_vPosition.z / u_fMaxHeight );
}
