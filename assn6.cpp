//Alex Pinard
//To Do: Implement IBO/interweaved vbos

//Includes vec, mat, and other include files as well as macro defs
#define GL3_PROTOTYPES
#define PI acos(-1.) //used for resizing

// Include the vector and matrix utilities from the textbook, as well as some
// macro definitions.
#include <stdio.h>
#include <OpenGL/gl3.h> //ifdef __APPLE__
#include "include/parser.h"
#include "include/init.h"
//#include "FTGL/ftgl.h"

typedef Angel::vec4 point4;

using namespace::std;

/*
Scene vars:
	vector<float> proj_params;
	char proj_type[256];
	vec3 eye, at, up;
	vector<string> obj_files;
Scene init method:
	input: char *scene_file
	output: initialize all vars

Obj vars:
	char obj_name[256];
	char obj_type[256];
	int wireframe_state;
	vector<vec4> vertices;
	vector<vec4> normals;
	vector<vec4> colors; //used in object manipulator
	GLuint vao;
Obj constructor method:
    input: string file_name, bool initialize
    output: initialize wireframe state, and obj_name (and vertices/normals if true)
*/

GLuint projection;
GLuint model_view;
GLuint program;
GLuint color_flag;
int wh, ww, current_id, start_x, start_y, prev_translate_change,
	prev_rotate_change, prev_scale_change;
float view_bottom, view_top, view_left, view_right, view_near, view_far, difference, fov;
char mode = 't';
vector<Obj> object;
Scene scene;

mat4 view, mv;
int shader_mode = 0;
int change_shader = 1;
int shades = 4;


void *font = GLUT_BITMAP_9_BY_15;
// OpenGL initialization
void init(int argc, char **argv){
	scene.Init(argv[1]);

	//read each obj file and append vertex and normal data
	for(int i = 0; i < scene.obj_files.size(); i++){
		Obj new_object(scene.obj_files[i]);
		object.push_back(new_object);
	}
	
	// Load shaders and use the resulting shader program
	program = InitShader( "shaders/vshader.glsl",
	                      "shaders/fshader.glsl" );
 	//get ready to use normals rather than colors
	color_flag = glGetUniformLocation(program, "colorFlag");
	glUniform1i(color_flag, 0);
	
	glUseProgram(program);

	//initialize lighting shader params...?
	initLighting(program);

	//initialize model_view and projection...?
	initModelView(scene.eye, scene.at, scene.up, projection, model_view, program, mv);
	
	//generate vertex array for each object, create vbo and fill with data
	for(int i = 0; i < object.size(); i++){
		glGenVertexArrays(1, &object[i].vao);
		glBindVertexArray(object[i].vao);
	
		// Create and initialize a buffer object large enough for vertices
		// and normals.
		GLuint buffer;
		glGenBuffers( 1, &buffer );
		glBindBuffer( GL_ARRAY_BUFFER, buffer );
		glBufferData(GL_ARRAY_BUFFER,
		             (object[i].vertices.size() + object[i].normals.size()) * sizeof(vec4),
		             NULL,
		             GL_STATIC_DRAW );

		// Fill first part of buffer with vertices; second part with normals
		glBufferSubData(GL_ARRAY_BUFFER, 0, object[i].vertices.size() * sizeof(vec4),
		                &object[i].vertices[0]);
		glBufferSubData(GL_ARRAY_BUFFER, object[i].vertices.size() * sizeof(vec4),
		                object[i].normals.size() * sizeof(vec4), &object[i].normals[0]);
		
		// Specify location and format of data in buffer
		GLuint vPosition = glGetAttribLocation( program, "vPosition" );
		glEnableVertexAttribArray( vPosition );
		glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		                       BUFFER_OFFSET(0));
		
		glUniform1i(glGetUniformLocation(program, "outlineMode"), 0);

		GLuint vNormal = glGetAttribLocation( program, "vNormal" );
		glEnableVertexAttribArray( vNormal );
		glVertexAttribPointer( vNormal, 4, GL_FLOAT, GL_FALSE, 0,
		                       BUFFER_OFFSET(object[i].vertices.size() * sizeof(vec4)));

		//generate vertex array for each manipulator, create vbo and fill with data
		for(int j = 0; j < object[i].manipulators.size(); j++){
			glGenVertexArrays(1, &object[i].manipulators[j].vao);
			glBindVertexArray(object[i].manipulators[j].vao);
			
			// Create and initialize a buffer object large enough for vertices
			// and normals.
			GLuint buffer;
			glGenBuffers( 1, &buffer );
			glBindBuffer( GL_ARRAY_BUFFER, buffer );
			glBufferData(GL_ARRAY_BUFFER,
			             (object[i].manipulators[j].vertices.size() +
			              object[i].manipulators[j].colors.size()) * sizeof(vec4),
			             NULL,
			             GL_STATIC_DRAW );
			
			// Fill first part of buffer with vertices; second part with normals
			glBufferSubData(GL_ARRAY_BUFFER, 0,
			                object[i].manipulators[j].vertices.size() * sizeof(vec4),
			                &object[i].manipulators[j].vertices[0]);

			glBufferSubData(GL_ARRAY_BUFFER,
		                object[i].manipulators[j].vertices.size() * sizeof(vec4),
		                object[i].manipulators[j].colors.size() * sizeof(vec4),
		                &object[i].manipulators[j].colors[0]);
			
			// Specify location and format of data in buffer
			GLuint vPosition = glGetAttribLocation( program, "vPosition" );
			glEnableVertexAttribArray( vPosition );
			glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			                       BUFFER_OFFSET(0));
			
			GLuint vColor = glGetAttribLocation( program, "vColor" );
			glEnableVertexAttribArray( vColor );
			glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
			                       BUFFER_OFFSET(object[i].manipulators[j].vertices.size()
			                                     * sizeof(vec4)));
		}
	}
	
	view = identity();
	//use z-buffer algorithm
    glEnable(GL_DEPTH_TEST);
    //black background
    glClearColor(0.0, 0.0, 0.0, 1.0);
}

/*void output(int x, int y, float r, float g, float b, void** font, char *string){
	glColor3f(r, g, b);
	glRasterPos2f(x, y);
	int len, i;
	len = (int)strlen(string);
	for(i = 0; i < len; i++){
		glutBitmapCharacter(font, string[i]);
	}
	}*/

void setOrthographicProjection() {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, ww, 0, wh);
	glScalef(1, -1, 1);
	glTranslatef(0, -wh, 0);
	glMatrixMode(GL_MODELVIEW);
}
void resetPerspectiveProjection() {
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}
void renderBitmapString(float x, float y, void *font,const char *string){
	const char *c;
	glRasterPos2f(x, y);
	for (c=string; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
	}
}

//----------------------------------------------------------------------------
void display(void){
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glViewport(0, 0, ww, wh);
	if(change_shader){
		if(shader_mode == 2){ //cel shading
			//set number of shades
			glUniform1i(glGetUniformLocation(program, "Shades"), shades);
		}
		glUniform1i(glGetUniformLocation(program, "mode"), shader_mode);
		change_shader = 0;
	}
	
    //disable anti-aliasing and set line width for manipulator
    glDisable(GL_LINE_SMOOTH);

    //holds model-view matrix unaffected by view or model transforms
    mat4 old_mv = mv;
    
    //check wireframe state of each object to determine what mode to draw in
    for(int i = 0; i < object.size(); i+=1){
	    //calculate model view matrix after applying transforms
	    mv = mv * view;
	    mv = mv * object[i].model;

	    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
	    //restore model-view back to default for next iteration
	    mv = old_mv;
	    
	    if(object[i].wireframe_state){		    
		    //draw object with outlined polygons
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		    glPolygonOffset(1.0, 2);

		    if(shader_mode == 2){
			    glEnable(GL_CULL_FACE);
			    glCullFace(GL_FRONT);

			    glUniform1i(glGetUniformLocation(program, "outlineMode"), 1);
			    glBindVertexArray(object[i].vao);
			    glDrawArrays( GL_TRIANGLES, 0, object[i].vertices.size());
			    glUniform1i(glGetUniformLocation(program, "outlineMode"), 0);
			    
			    glDisable(GL_CULL_FACE);
		    }
		    glBindVertexArray(object[i].vao);
		    glDrawArrays( GL_TRIANGLES, 0, object[i].vertices.size());			    
		    
	    } else {
		    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		    if(shader_mode == 2){
			    glEnable(GL_CULL_FACE);
			    glCullFace(GL_FRONT);

			    glUniform1i(glGetUniformLocation(program, "outlineMode"), 1);
			    glBindVertexArray(object[i].vao);
			    glDrawArrays( GL_TRIANGLES, 0, object[i].vertices.size());
			    glUniform1i(glGetUniformLocation(program, "outlineMode"), 0);
			    
			    glDisable(GL_CULL_FACE);
		    }
		    //draw object with filled polygons
		    glBindVertexArray(object[i].vao);
		    glDrawArrays( GL_TRIANGLES, 0, object[i].vertices.size());
		}
    }

    for(int i = 0; i < object.size(); i++){
	    if(object[i].wireframe_state){		    
		    //disable depth test and draw manipulator on top of window
		    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDisable(GL_DEPTH_TEST);
			//get ready to use colors rather than normals
			glUniform1i(color_flag, 1);
			for(int j = 0; j < object[i].manipulators.size(); j++){
				//draw each manipulator if object is in wireframe mode
				mv = mv * view;
				mv = mv * object[i].manipulators[j].model;
				glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
				mv = old_mv;
				glBindVertexArray(object[i].manipulators[j].vao);
				glDrawArrays(GL_TRIANGLES, 0, object[i].manipulators[j].vertices.size());
			}
			//get ready to use colors rather than normals
			glUniform1i(color_flag, 0);
			glEnable(GL_DEPTH_TEST);
	    }
    }

/*	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3d(1.0, 0.0, 0.0);
	setOrthographicProjection();
	glPushMatrix();
	glLoadIdentity();
	renderBitmapString(200,200,(void *)font,"Font Rendering - Programming Techniques");
	renderBitmapString(300,240,(void *)font,"Esc - Quit");
	glPopMatrix();
	resetPerspectiveProjection();    
*/    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void keyboard( unsigned char key, int x, int y ){
    switch( key ) {
	case 033:  // Escape key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;
    case 'd': case 'D':
	    for(int i = 0; i < object.size(); i++){
		    object[i].wireframe_state = 0;
	    }
	    glutPostRedisplay();
	    break;
    case 't': case 'T':
	    //change mode to translate (t), rotate (r), or scale (s)
	    mode = 't';
	    break;
	case 'r': case 'R':
		mode = 'r';
	    break;
	case 's': case 'S':
		mode = 's';
	    break;
    case 'g':
	    shader_mode = 0;
	    change_shader = 1;
	    glutPostRedisplay();
	    break;
    case 'p':
	    shader_mode = 1;
	    change_shader = 1;
	    glutPostRedisplay();
	    break;
    case 'c':
	    shader_mode = 2;
	    change_shader = 1;
	    glutPostRedisplay();
	    break;
    case '2':
	    shades = 2;
	    change_shader = 1;
	    glutPostRedisplay();
	    break;
    case '3':
	    shades = 3;
	    change_shader = 1;
	    glutPostRedisplay();
	    break;
    case '4':
	    shades = 4;
	    change_shader = 1;
	    glutPostRedisplay();
	    break;
    }
}

//---------------------------------------------------------------------------

void reshape(int w, int h){
	int scene_width = w;
	glViewport(0, 0, scene_width, h);
	float ar = 1.0 * scene_width / h;
	mat4 proj;
	wh = h;
	ww = w;
	
	//Orthographic view case
	if(strcmp(scene.proj_type, "Orthographic") == 0){
	    if(scene.proj_params.size() == 6){
		    if(ar < 1) { //taller
			    //proj_params: 0 left 1 right 2 bottom 3 top
			    //Ortho(left, right, bottom, top, nearval, farval);
			    //change bottom and top variables to avoid vertical distortion
			    view_bottom = scene.proj_params[0];
			    view_top = scene.proj_params[1];
			    view_left = scene.proj_params[2] * (GLfloat)h / (GLfloat)scene_width;
			    view_right = scene.proj_params[3] * (GLfloat)h / (GLfloat)scene_width;
			    view_near = scene.proj_params[4];
			    view_far = scene.proj_params[5];
			    difference = view_top - view_bottom;
			    
			    proj = Ortho(scene.proj_params[0], scene.proj_params[1],
			                 scene.proj_params[2] * (GLfloat)h / (GLfloat)scene_width,
			                 scene.proj_params[3] * (GLfloat)h / (GLfloat)scene_width,
			                 scene.proj_params[4], scene.proj_params[5]);		
		    } else {
			    //change left and right variables to avoid horizontal distortion
			    view_bottom = scene.proj_params[0] * (GLfloat)scene_width / (GLfloat)h;
			    view_top = scene.proj_params[1] * (GLfloat)scene_width / (GLfloat)h;
			    view_left = scene.proj_params[2];
			    view_right = scene.proj_params[3];
			    view_near = scene.proj_params[4];
			    view_far = scene.proj_params[5];
			    difference = view_top - view_bottom;

			    proj = Ortho(scene.proj_params[0] * (GLfloat)scene_width / (GLfloat)h,
			                 scene.proj_params[1] * (GLfloat)scene_width / (GLfloat)h,
			                 scene.proj_params[2],
			                 scene.proj_params[3], scene.proj_params[4], scene.proj_params[5]);
		    }
		    glUniformMatrix4fv(projection, 1, GL_FALSE, &proj[0][0]);
	    } else {
		    //if not enough parameters are specified, print and exit
		    printf("Parameters invalid for %s projection: ", scene.proj_type);
		    for(int i = 0; i < scene.proj_params.size(); i++){
			    printf("%.2f ", scene.proj_params[i]);
		    }
		    printf("\n");
		    exit(-1);
	    }
	} else if(strcmp(scene.proj_type, "Perspective") == 0){
		if(scene.proj_params.size() == 4){
			//Perspective view case
			//proj_params: 0 fovy 1 aspect 2 near 3 far
			//Perspective(fovy, aspect, near, far);
			if(ar < 1){ //taller
				//only change aspect ratio to new aspect ratio, maintain
				//fov so everything is drawn
				float aspect = scene.proj_params[1] * ar;
                //convert to Frustum? I don't think so...
				fov = (180 / PI) * 2 * \
					atan(tan(scene.proj_params[0] * (PI / 180) / 2) / aspect);

				//store view volume params for use in coordinating mouse drag transformations
				difference = 2 * (scene.proj_params[3]-scene.proj_params[2]/2) * tan(fov * PI / 360);
				view_bottom = -scene.proj_params[2] * tan(fov * PI / 360);
				view_top = -view_bottom;
				view_left = -aspect * view_top; 
			    view_right = -view_left;
			    view_near = scene.proj_params[2];
			    view_far = scene.proj_params[3];				

			    proj = Frustum(view_left, view_right, view_bottom, view_top, view_near, view_far);
			    
//				proj = Perspective(fov, aspect,
//				                   scene.proj_params[2], scene.proj_params[3]);
				glUniformMatrix4fv(projection, 1, GL_TRUE, &proj[0][0]);
			} else { //wider
				float aspect = scene.proj_params[1] * ar;
				fov = scene.proj_params[0];

				difference = 2 * (scene.proj_params[3]-scene.proj_params[2]/2) * tan(fov * PI / 360);
				view_bottom = -1 * tan(fov / 360 * PI) * aspect;
				view_top = -view_bottom;
			    view_left = -aspect * view_top; 
			    view_right = -view_left;
			    view_near = scene.proj_params[2];
			    view_far = scene.proj_params[3];				
				
				proj = Perspective(fov, aspect,
				                   scene.proj_params[2], scene.proj_params[3]);
				glUniformMatrix4fv(projection, 1, GL_TRUE, &proj[0][0]);
			}
		} else {
			//if not enough parameters are specified, print and exit
			printf("Parameters invalid for %s projection: ", scene.proj_type);
		    for(int i = 0; i < scene.proj_params.size(); i++){
			    printf("%.2f ", scene.proj_params[i]);
		    }
		    printf("\n");
		    exit(-1);
		}
	}
/*	glViewport(0, 0, scene_width, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-ar, ar, -1.0, 1.0, 2.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
*/
}

//---------------------------------------------------------------------------

int get_id(int x, int y){
	GLuint idcolor;
	int r, g, b;
	mat4 old_mv;
	//clear color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, ww, wh);
	//calculate a unique color for each separate obj/vao
	for(int i = 1; i <= object.size(); i++){
		r = (i & 0x000000FF) >> 0;
		g = (i & 0x0000FF00) >> 8;
		b = (i & 0x00FF0000) >> 16;
		//draw each obj using its unique color
		//DONT SWAP THE BUFFERS

		old_mv = mv;
		//calculate model view matrix after applying transforms
	    mv = mv * view;
	    mv = mv * object[i - 1].model;
	    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
	    //restore model-view back to default for next iteration
	    mv = old_mv;

	    //draw object in unique color
		glBindVertexArray(object[i - 1].vao);
		idcolor = glGetUniformLocation(program, "idColor");
		glUniform4f(idcolor, r/255.0, g/255.0, b/255.0, 1.0);
		glDrawArrays(GL_TRIANGLES, 0, object[i - 1].vertices.size());	   
	}

	//manipulator time
	glDisable(GL_DEPTH_TEST);
	for(int i = 1; i <= object.size(); i++){
		int j = (object.size() + 1) * i;
		if(object[i - 1].wireframe_state == 1){
			for(int k = 0; k < object[i - 1].manipulators.size(); k++){
				r = (j & 0x000000FF) >> 0;
				g = (j & 0x0000FF00) >> 8 ;
				b = (j & 0x00FF0000) >> 16;
				//draw each obj using its unique color
				//DONT SWAP THE BUFFERS						
				idcolor = glGetUniformLocation(program, "idColor");
				glUniform4f(idcolor, r/255.0, g/255.0, b/255.0, 1.0);
				
				old_mv = mv;
				//calculate model view matrix after applying transforms
				mv = mv * view;
				mv = mv * object[i - 1].manipulators[k].model;
				glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
				//restore model-view back to default for next iteration
				mv = old_mv;
				
				glBindVertexArray(object[i - 1].manipulators[k].vao);
				glDrawArrays(GL_TRIANGLES, 0, object[i - 1].manipulators[k].vertices.size());
				
				j++;
			}
		}
	}
	glEnable(GL_DEPTH_TEST);

	//read the pixel color and calculate the obj id based on color
	GLubyte pixel[4];
	glReadBuffer(GL_BACK);
	glReadPixels(x, wh - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
	int id = pixel[0] + (pixel[1] * 256) + (pixel[2] * 65536);
	//set idcolor back to 0 to switch to regular color mode
	glUniform4f(idcolor, 0.0, 0.0, 0.0, 1.0);
//	printf("%d\n", id - 1);
	return id - 1;
}

void toggle(int &val){
	//flips bit in vector at position pos
	if(val == 0){
		val = 1;
	} else if(val == 1) {
		val = 0;
	}
}

void get_corresponding_object_and_axis(int id, int &obj_id, int &axis_id){
	//obtain axis and id from original unique id
	obj_id = (id - object.size()) / 3;
	axis_id = (id-object.size()) % 3;
}

void xfrm_all_models(mat4 &transform, Obj &selected_obj){
	//transform both model and model's manipulators
	selected_obj.model *= transform;
	for(int i = 0; i < selected_obj.manipulators.size(); i++){
		selected_obj.manipulators[i].model *= transform;
	}
}

float viewport_to_scene_units(int viewport_amount, int obj_id, int axis_id){
	//transforms a change in mouse position on the screen to an equivalent amount in the world
	//first, get a fractional change to the view volume
	float fraction_change = (float)viewport_amount * 1.0 / (float)wh;
	//calculate the height of the view volume at the object
	float height_at_object = difference + (-2 * object[obj_id].manipulators[axis_id].model[2][3] *
	                            tan(fov * PI / 360));
	//find how far away the eye is from the object
	float factor;
	if(strcmp(scene.proj_type, "Perspective") == 0){
		factor = length(scene.eye);		
	} else {
		vec3 fvec = vec3(scene.eye[0] + object[obj_id].manipulators[axis_id].model[0][3],
		                 scene.eye[1] + object[obj_id].manipulators[axis_id].model[1][3],
		                 scene.eye[2] + object[obj_id].manipulators[axis_id].model[2][3]);
		factor = length(fvec);
	}
	//multiply together to get equivalent screen change
	float scene_amount = fraction_change * height_at_object * factor;
	return scene_amount;
}

void xfrm_obj_model(char current_mode, int obj_id, int axis_id, int change){
	mat4 transform, inverse_transform, inverse_prev_transform;
	vec3 transform_vec = vec3(0.0, 0.0, 0.0);
	vec3 inverse_transform_vec = vec3(0.0, 0.0, 0.0);
	vec3 scale_vec = vec3(1.0, 1.0, 1.0);
	vec3 inverse_scale_vec = vec3(1.0, 1.0, 1.0);
	float transform_val, inverse_transform_val, inverse_prev_transform_val;
	//translate based on change since previous measurement
	float translate_units = viewport_to_scene_units(change - prev_translate_change, obj_id, axis_id);
	float scale_units = viewport_to_scene_units(change - prev_scale_change, obj_id, axis_id);
	//undo all current transforms
	object[obj_id].model *= object[obj_id].inverse_rotate_xfrm;
	object[obj_id].model *= object[obj_id].inverse_scale_xfrm;

	xfrm_all_models(object[obj_id].inverse_translate_xfrm, object[obj_id]);

	switch(current_mode){
	case 't':
		//update translate
		transform_vec[axis_id] = translate_units;
		inverse_transform_vec[axis_id] = -transform_vec[axis_id];

		transform = Translate(transform_vec);
		inverse_transform = Translate(inverse_transform_vec);			   

		object[obj_id].inverse_translate_xfrm *= inverse_transform;
		object[obj_id].translate_xfrm *= transform;
		break;
		
	case 'r':
		//update rotate
		transform_val = (float)(change - prev_rotate_change) * (1.0 / (float)wh) * 360 * 2;
		inverse_transform_val = -transform_val;
		
		switch(axis_id){
		case 0:
			transform = RotateX(transform_val);
			inverse_transform = RotateX(inverse_transform_val);
			break;

		case 1:
			transform = RotateY(transform_val);
			inverse_transform = RotateY(inverse_transform_val);
			break;

		case 2:
			transform = RotateZ(transform_val);
			inverse_transform = RotateZ(inverse_transform_val);
			break;
		}

		object[obj_id].inverse_rotate_xfrm = object[obj_id].inverse_rotate_xfrm * inverse_transform;
		object[obj_id].rotate_xfrm = transform * object[obj_id].rotate_xfrm;
		break;

	case 's':
		//update scale
		scale_vec[axis_id] = scale_units + 1;
		inverse_scale_vec[axis_id] = 1 / (scale_units + 1);

		inverse_transform = Scale(inverse_scale_vec);
		transform = Scale(scale_vec);

		object[obj_id].inverse_scale_xfrm *= inverse_transform;
		object[obj_id].scale_xfrm *= transform;
		break;
	}

	//reapply all transforms
	xfrm_all_models(object[obj_id].translate_xfrm, object[obj_id]);
	object[obj_id].model *= object[obj_id].scale_xfrm;
	object[obj_id].model *= object[obj_id].rotate_xfrm;

	//keep track of change in position
	prev_translate_change = change;
	prev_rotate_change = change;
	prev_scale_change = change;
}

void drag(int x, int y){
//	printf("hello\n");
	//if manipulator is selected, get corresponding object and transform based on mode
	int change_x = start_x - x;
	int change_y = start_y - y;

	int obj_id, axis_id;
	get_corresponding_object_and_axis(current_id, obj_id, axis_id);
	//transform object around given axis, when mouse is moved along change_x/y
	if(axis_id == 1){
		xfrm_obj_model(mode, obj_id, axis_id, change_y);	
	} else if (axis_id == 0){
		xfrm_obj_model(mode, obj_id, axis_id, -change_x);	
	} else if (axis_id == 2){
		xfrm_obj_model(mode, obj_id, axis_id, change_x);	
	}
	glutPostRedisplay();
}

void mouse(GLint button, GLint state, GLint x, GLint y){
	//check if left mouse button is clicked
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
		current_id = get_id(x, y);		
		if(current_id >= 0 && current_id < object.size()){
			//if object is selected, toggle selection state
			toggle(object[current_id].wireframe_state);
		} else if (current_id >= (int)object.size() && current_id < 65973){
			//if manipulator is selected, set drag function to cause transformation
			//set all changes to 0 to clear any previous transformation storage if mouse
			//is newly clicked
			start_x = x;
			start_y = y;
			prev_translate_change = 0;
			prev_rotate_change = 0;
			prev_scale_change = 0;
			glutMotionFunc(drag);
		}
		glutPostRedisplay();
	} else {
		glutMotionFunc(NULL);
	}
}

//----------------------------------------------------------------------------

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); //__APPLE__
    /* if not __APPLE__:
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitContextVersion (3, 2);
    glutInitContextFlags (GLUT_FORWARD_COMPATIBLE);
     */
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(500, 300);
    glutCreateWindow("3D Obj Selector");
    printf("%s\n%s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION));

//    glewExperimental = GL_TRUE; //__APPLE__?
//    glewInit(); //__APPLE__?

    //NOTE:  callbacks must go after window is created!!!
    init(argc, argv);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMainLoop();
    
    return(0);
}
