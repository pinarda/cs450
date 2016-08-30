#version 150

in  vec4 vPosition;
in  vec4 vNormal;
//gouraud
in  vec4 vColor;
out vec4 color;
//phong, cel
out vec3 N;
out vec3 pos;
out vec4 Color;

uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform vec4 LightPosition;
uniform float Shininess;
uniform vec4 idColor; //ADDED
uniform int colorFlag;
uniform int mode;
uniform int outlineMode;

void cel(){
     N = (ModelView * vNormal).xyz;
     Color = vColor;
     pos = (ModelView * vPosition).xyz;

     if(outlineMode == 1){
          gl_Position = Projection * ModelView * (vPosition + 0.015 * vNormal);
     } else {
          gl_Position = Projection * ModelView * vPosition;
     }
}

void phong(){
     pos = (ModelView * vPosition).xyz;
     N = (ModelView * vNormal).xyz;
     Color = vColor;
     gl_Position = Projection * ModelView * vPosition;
}

void gouraud(){
    // Transform vertex position into eye coordinates
    vec3 pos = (ModelView * vPosition).xyz;

    vec3 L = normalize( (LightPosition).xyz - pos );
    vec3 E = normalize( -pos );
    vec3 H = normalize( L + E );  //halfway vector

    // Transform vertex normal into eye coordinates
    vec3 N = normalize( ModelView*vNormal ).xyz;

    //To correctly transform normals
    // vec3      N = (normalize (transpose (inverse (ModelView))*vNormal).xyz

    // Compute terms in the illumination equation
    vec4 ambient = AmbientProduct;

    float dr = max( dot(L, N), 0.0 );
    vec4  diffuse = dr *DiffuseProduct;

    float sr = pow( max(dot(N, H), 0.0), Shininess );
    vec4  specular = sr * SpecularProduct;

    if( dot(L, N) < 0.0 ) {
	    specular = vec4(0.0, 0.0, 0.0, 1.0);
    }

    gl_Position = Projection * ModelView * vPosition;

    //ADDED if STATEMENT
    if(idColor.r + idColor.g + idColor.b > 0.0){
	    color = idColor;
    } else if(colorFlag == 1){
        color = vColor;
    } else {
        color = ambient + diffuse + specular;
    }
    color.a = 1.0;
}

void main()
{
    if(mode == 0){
        gouraud();
    } else if(mode == 1){
        phong();
    } else if(mode == 2){
        cel();
    }
}
