#include "include/init.h"

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

void initLighting(GLuint program){
	// Initialize shader lighting parameters
	// RAM: No need to change these...we'll learn about the details when we
	// cover Illumination and Shading
	point4 light_position( 1.5, 1.5, 2.0, 1.0 );
	color4 light_ambient( 0.2, 0.2, 0.2, 1.0 );
	color4 light_diffuse( 1.0, 1.0, 1.0, 1.0 );
	color4 light_specular( 1.0, 1.0, 1.0, 1.0 );
	
	color4 material_ambient( 0.0, 0.5, 1.0, 1.0 );
	color4 material_diffuse( 0.0, 0.5, 1.0, 1.0 );
	color4 material_specular( 1.0, 1.0, 1.0, 1.0 );
	float  material_shininess = 100.0;
	
	color4 ambient_product = light_ambient * material_ambient;
	color4 diffuse_product = light_diffuse * material_diffuse;
	color4 specular_product = light_specular * material_specular;
	
	glUniform4fv( glGetUniformLocation(program, "AmbientProduct"),
	              1, ambient_product );
	glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"),
	              1, diffuse_product );
	glUniform4fv( glGetUniformLocation(program, "SpecularProduct"),
	              1, specular_product );
	glUniform4fv( glGetUniformLocation(program, "LightPosition"),
	              1, light_position );
	glUniform1f( glGetUniformLocation(program, "Shininess"),
	             material_shininess );
}

void initModelView(vec3 &eye, vec3 &at, vec3 &up,
                   GLuint &projection, GLuint &model_view, GLuint program, mat4 &mv){
	projection = glGetUniformLocation( program, "Projection" );
	model_view = glGetUniformLocation( program, "ModelView" );
	
	point4  eye_coords( eye.x, eye.y, eye.z, 1.0);
	point4  at_coords( at.x, at.y, at.z, 1.0 );
	vec4    up_coords( up.x, up.y, up.z, 0.0 );
	
	mv = LookAt( eye_coords, at_coords, up_coords);
	
	glUniformMatrix4fv( model_view, 1, GL_TRUE, mv );
}
