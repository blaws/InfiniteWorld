// Benjamin Laws
// CSE 40166 - Computer Graphics
// October 17, 2014
// Midterm Project

uniform sampler3D u_sTexture;

varying vec3 v_vNormal;
varying vec3 v_vTexCoord;

const vec3 lightDir = vec3( 1.0, 1.0, 1.0 );
const vec3 ambient = vec3( 0.2, 0.2, 0.2 );
const vec3 diffuse = vec3( 0.7, 0.7, 0.7 );

//==========================================================

void main()
{
    float NdotL = max( 0.0, dot( normalize( lightDir ), normalize( v_vNormal ) ) );
    
    vec3 baseColor = texture3D( u_sTexture, v_vTexCoord ).xyz;
    
    gl_FragColor = vec4( ambient + NdotL * diffuse * baseColor, 1.0 );
}
