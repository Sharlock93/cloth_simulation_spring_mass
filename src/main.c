#define VP_LOC 0
#define COLOR_LOC 1
#define UV_LOC 2
#define NORMAL_LOC 9
#define MODEL_UNIFORM_LOC 3
#define CAMERA_UNIFORM_LOC 4
#define PROJECTION_UNIFORM_LOC 5
#define USE_UNIFORM_COLOR_LOC 6
#define UNIFORM_COLOR_LOC 7
#define HAS_TEXTURE_LOC 8
#define USE_LIGHT 10

#define COLOR_HEX_POS4(a) (pos4){ ((a & 0xFF0000) >> 16)/255.0f, ((a & 0x00FF00) >> 8)/255.0f, ((a & 0x0000FF))/255.0f, 1 }
#define COLOR_HEX_FLOAT4(a) (float[4]){ ((a & 0xFF0000) >> 16)/255.0f, ((a & 0x00FF00) >> 8)/255.0f, ((a & 0x0000FF))/255.0f, 1 }
#define TO_RADIANS(angle) (angle)*3.14159f/180.0f

#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#pragma comment(lib, "lib/glfw3dll.lib")
#pragma comment(lib, "lib/glew32s.lib")
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>


#include "sh_tools.c"
#include "parser.c"
#include "gl_load_funcs.h"
#include "sh_simple_vec_math.c"
#include "table_read_utils.c"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL4_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include "nuklear.h"
#include "nuklear_glfw_gl4.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

struct nk_context *ctx;
struct nk_colorf bg;
struct nk_image img;

#include "opengl_window.c"

#include "obj_parser.c"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// #define MIN(a, b) ((a) < (b) ? a : b )


char file_name_buffer[1024];


i32 saving = 0;
i32 recording = 0;

typedef struct point_mass_system_t point_mass_system_t;

void read_texture(char *name);
void write_system_to_obj(point_mass_system_t *sys, char *filename);

void set_color(pos4 *p) {
	glUniform4fv(UNIFORM_COLOR_LOC, 1, &p->x);
}

typedef struct plane_t { 
	f32 distance;
	sh_vec3 point;
	sh_vec3 normal;
} plane_t;

typedef struct sphere_t { 
	sh_vec3 center;
	f32 radius;
} sphere_t;

typedef struct ray_t { 
	sh_vec3 start;
	sh_vec3 direction;
} ray_t;


typedef struct collision_t {
	i8 collided;
	f32 t;
	sh_vec3 intersection_point;
} collision_t;


typedef struct cube_t {
	sh_vec3 start;
	sh_vec3 end;
} cube_t;

ray_t build_mouse_ray();
void draw_line(float *p, float *p1, float *c);

i32 option = -1;
i32 wind = 0;

i32 move_sphere = 0;
i32 should_sim = 0;
i32 apply_rot = 0;
f32 cur_angle = 0;


i32 tex_width;
i32 tex_height;
u32 texture_id = 0;
u32 uv_vbo = 0;

u32 plane_vao = 0;
u32 plane_vbo = 0;
u32 plane_vert_size = 0;

u32 normal_plane_vbo = 0;

u32 axis_vbo = 0;
u32 axis_vao = 0;

u32 line_vbo = 0;
u32 line_vao = 0;

u32 point_vbo = 0;
u32 normal_vbo = 0;
u32 cube_vbo = 0;
u32 cube_index_vbo = 0;

pos4 *lines_to_draw = NULL;
pos4 *pts_to_draw = NULL;
sh_vec3 *normals_grid = NULL;

pos4 *sphere_data = NULL;
f32 has_locked_t = 0;
u32 sphere_vbo = 0;

i32 cube_indices[] = {
		0, 2, 1,
		0, 2, 3,

		7, 3, 2,
		7, 2, 5,

		4, 6, 5,
		4, 5, 7,

		4, 1, 0,
		4, 1, 6,

		6, 2, 1,
		6, 2, 5,

		7, 0, 3,
		7, 4, 0
	};



cube_t cube_at_zero = {
	{0, 0, 0},
	{5, 5,  5},
};

plane_t ground_plane = {
	.distance = 0,
	.normal = {1, 1, 1},
	.point = {0, 0, 0}
};


sphere_t sphere_at_zero = {
	.center = {5, 0, 5},
	.radius = 4
};

i8 collision_point_cube(pos4 *p, cube_t *c) {

	f32 min_x = MIN(c->start.x, c->end.x);
	f32 min_y = MIN(c->start.y, c->end.y);
	f32 min_z = MIN(c->start.z, c->end.z);

	f32 max_x = max(c->start.x, c->end.x);
	f32 max_y = max(c->start.y, c->end.y);
	f32 max_z = max(c->start.z, c->end.z);


	if(
		p->x < min_x ||
		p->y < min_y ||
		p->z < min_z || 

		p->x > max_x ||
		p->y > max_y ||
		p->z > max_z

	  ) {

		return 0;
	}

	return 1;

}

collision_t collision_ray_plane(ray_t *ray, plane_t *p) {
	collision_t t = {0};

	f32 ray_dir_dot_n = sh_vec3_dot(&ray->direction, &p->normal);

	if(ray_dir_dot_n == 0) {
		return t;
	}

	f32 p_dot_n = sh_vec3_dot(&ray->start, &p->normal) + p->distance;

	t.collided = 1;
	t.t = -p_dot_n/ray_dir_dot_n;

	sh_vec3 dir = ray->direction;
	sh_vec3_mul_scaler(&dir, t.t);

	t.intersection_point = sh_vec3_new_add_vec3(&ray->start, &dir);

	return t;
}

collision_t collision_ray_sphere(ray_t *ray, sphere_t *sp) {
	collision_t t = {0};

	sh_vec3 to_center = sh_vec3_sub(sp->center, ray->start);
	f32 b = sh_vec3_dot(&to_center, &ray->direction);
	f32 c = sh_vec3_dot(&to_center, &to_center) - sp->radius*sp->radius;

	if((b*b - c) < 0) {
		return t;
	}

	t.collided = 1;
	t.t = -b + (f32)sqrt(b*b - c);


	t.intersection_point = ray->start;
	sh_vec3 point = sh_vec3_new_mul_scaler(&ray->direction, -t.t);
	sh_vec3_add_vec3(&t.intersection_point, &point);
	
	return t;
}


collision_t collision_point_sphere(sh_vec3 *p, sphere_t *sp) {
	collision_t t = {0};

	sh_vec3 dist = sh_vec3_new_sub_vec3(p, &sp->center);
	f32 len_sq = sh_vec3_dot(&dist, &dist);

	if(len_sq < (sp->radius*sp->radius)) {
		t.collided = 1;
		sh_vec3_normalize_ref(&dist);
		sh_vec3_mul_scaler(&dist, sp->radius);
		sh_vec3_add_vec3(&dist, &sp->center);
		t.intersection_point = dist;
	}


	return t;
}


void render_plane() {


	glDisable(GL_DEPTH_TEST);
	i32 depth_func = 0;
	glGetIntegerv(GL_DEPTH_FUNC, &depth_func);

	glDepthFunc(GL_GEQUAL);
	glBindVertexArray(plane_vao);
	glUniform1i(USE_UNIFORM_COLOR_LOC, 1);
	glUniform1i(HAS_TEXTURE_LOC, 0);
	pos4 c = COLOR_HEX_POS4(0x89AEB2);
	glUniform4fv(UNIFORM_COLOR_LOC, 1, &c.x);
	glDrawArrays(GL_LINES, 0, plane_vert_size);

	glDepthFunc(depth_func);
	glEnable(GL_DEPTH_TEST);
}

void render_axis() {

	glBindVertexArray(axis_vao);


	glUniform1i(USE_UNIFORM_COLOR_LOC, 0);
	glUniform1i(HAS_TEXTURE_LOC, 0);

	glBindBuffer(GL_ARRAY_BUFFER, axis_vbo);

	glEnableVertexAttribArray(VP_LOC);
	glEnableVertexAttribArray(COLOR_LOC);
	glDisableVertexAttribArray(NORMAL_LOC);

	glVertexAttribPointer(VP_LOC, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(COLOR_LOC, 4, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(pos4)*6));


	glLineWidth(1.5);
	glDrawArrays(GL_LINES, 0, 6);
	glLineWidth(1);
}


void setup_axis() {

	glGenVertexArrays(1, &axis_vao);

	glBindVertexArray(axis_vao);

	pos4 lines[12] = {
		{0.0f, 0.01f, 0.0f, 1.0f},
		{100.0f, 0.01f, 0.0f, 1.0f},

		{0.0f, 0.01f, 0.0f, 1.0f},
		{0.0f, 100.0f, 0.0f, 1.0f},

		{0.0f, 0.01f, 0.0f, 1.0f},
		{0.0f, 0.01f, 100.0f, 1.0f},

		COLOR_HEX_POS4(0x3da4ab),
		COLOR_HEX_POS4(0x3da4ab),

		COLOR_HEX_POS4(0xf6cd61),
		COLOR_HEX_POS4(0xf6cd61),

		COLOR_HEX_POS4(0xfe8a71),
		COLOR_HEX_POS4(0xfe8a71),
	};

	glGenBuffers(1, &axis_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, axis_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos4)*12, lines, GL_STATIC_DRAW);

	glEnableVertexAttribArray(VP_LOC);
	glEnableVertexAttribArray(COLOR_LOC);

	glVertexAttribPointer(VP_LOC, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(COLOR_LOC, 4, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(pos4)*6));

	glBindBuffer(GL_ARRAY_BUFFER, 0);


	glBindVertexArray(0);

}



void setup_plane() {


	float z = 0.0f;

	pos4 *plane = NULL; 
	int size = 100;
	pos4 start = { -100, 0, -100, 1};
	pos4 end = {    100, 0, -100, 1};

	pos4 start2 = { -100, 0, -100, 1 };
	pos4 end2 = {   -100, 0, 100, 1 };

	float step = (end.x - start.x)/size;

	buf_push(plane, start);
	buf_push(plane, end);

	buf_push(plane, start2);
	buf_push(plane, end2);



	for(int  i = 0; i < size; i++) {
		start.z += step;
		end.z += step;

		start2.x += step;
		end2.x +=   step;

		buf_push(plane, start);
		buf_push(plane, end);

		buf_push(plane, start2);
		buf_push(plane, end2);


	}

	glGenVertexArrays(1, &plane_vao);

	glBindVertexArray(plane_vao);

	glGenBuffers(1, &plane_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, plane_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos4)*buf_len(plane), plane, GL_STATIC_DRAW );

	glEnableVertexAttribArray(VP_LOC);
	glVertexAttribPointer(VP_LOC, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glUniform1f(USE_UNIFORM_COLOR_LOC, 0);

	float color[4] = { 0.5, 0.5, 0.5, 1};
	glUniform4fv(UNIFORM_COLOR_LOC, 1, color);

	plane_vert_size = buf_len(plane);

	glBindVertexArray(0);
	buf_free(plane);
}


pos4 add_pos4_point(pos4 *p1, pos4 *p2) {
	return (pos4){ p1->x + p2->x, p1->y + p2->y, p1->z + p2->z, 1 };
}


void setup(void) {

	gl_ctx = (sh_window_context){ 720, 1366, 0, 0, 0, .window_name = "editor" };
	gl_ctx.font_size = 64;

	HWND wnd = sh_window_setup();

	glClearColor(61.0f/255.0f, 41.0f/255.0f, 94.0f/255.0f, 1);

	ctx = nk_glfw3_init(gl_ctx.window, NK_GLFW3_INSTALL_CALLBACKS, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);


	struct nk_font_atlas *atlas;
	nk_glfw3_font_stash_begin(&atlas);
	nk_glfw3_font_stash_end();

	{
		int tex_index = 0;
		enum {tex_width = 256, tex_height = 256};
		char pixels[tex_width * tex_height * 4];
		memset(pixels, 128, sizeof(pixels));
		tex_index = nk_glfw3_create_texture(pixels, tex_width, tex_height);
		img = nk_image_id(tex_index);
	}




	glClearColor(61.0f/255.0f, 41.0f/255.0f, 94.0f/255.0f, 1);
	setup_plane();
	setup_axis();

}




void draw_rect(f32 x, f32 y, f32 width, f32 height, pos4 c) {

	u32 buf = 0;
	pos4 quad[6] = {
		{x, y, 0, 1},
		{x, y+height, 0, 1},
		{x+width, y+height, 0, 1},

		{x, y, 0, 1},
		{x+width, y+height, 0, 1},
		{x+width, y, 0, 1},

	};


	setup_for_ortho_draw();

	glGenBuffers(1, &buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos4)*6, quad, GL_STATIC_DRAW);

	glUniform1f(7, 1);
	glUniform1f(10, 0);

	glUniform4fv(8, 1, &c.x);

	glUniform1i(5, 0);

	glEnableVertexAttribArray(0);

	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(9);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDeleteBuffers(1, &buf);
}



void draw_point(float *p, float *color) {

	glBindVertexArray(gl_ctx.vao);

	if(point_vbo == 0) {
		glGenBuffers(1, &point_vbo);
	}

	glBindBuffer(GL_ARRAY_BUFFER, point_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4, p, GL_STATIC_DRAW);

	glEnableVertexAttribArray(VP_LOC);
	glDisableVertexAttribArray(COLOR_LOC);
	glDisableVertexAttribArray(UV_LOC);

	glUniform1f(USE_UNIFORM_COLOR_LOC, 1);
	glUniform1f(HAS_TEXTURE_LOC, 0);

	glUniform4fv(UNIFORM_COLOR_LOC, 1, color);

	glVertexAttribPointer(VP_LOC, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glUniformMatrix4fv(PROJECTION_UNIFORM_LOC, 1, GL_TRUE, gl_ctx.screen_scale.m);
	glUniformMatrix4fv(CAMERA_UNIFORM_LOC, 1, GL_TRUE, gl_ctx.camera.m);

	glDrawArrays(GL_POINTS, 0, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// glDeleteBuffers(1, &buf);

}

void draw_cube(cube_t *c, float color[4]) {

	glBindVertexArray(gl_ctx.vao);

	pos4 p[16] = {
		{c->start.x,            c->start.y, c->start.z, 1},
		{c->start.x + c->end.x, c->start.y, c->start.z, 1},
		{c->start.x + c->end.x, c->start.y, c->start.z + c->end.z, 1},
		{c->start.x,            c->start.y, c->start.z + c->end.z, 1},

		{c->start.x,            c->start.y + c->end.y, c->start.z, 1},
		{c->start.x + c->end.x, c->start.y + c->end.y, c->start.z + c->end.z, 1},
		{c->start.x + c->end.x, c->start.y + c->end.y, c->start.z, 1},
		{c->start.x,            c->start.y + c->end.y, c->start.z + c->end.z, 1},

		{1, 0, 0, 1},
		{0, 1, 0, 1},
		{0, 0, 1, 1},
		{0, 0, 0, 1},

		{1, 1, 0, 1},
		{1, 0, 1, 1},
		{0, 1, 1, 1},
		{1, 1, 1, 1},

	};

	
	if(cube_vbo == 0) {
		glGenBuffers(1, &cube_vbo);
	}

	if(cube_index_vbo == 0) {
		glGenBuffers(1, &cube_index_vbo);

	}

	glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos4)*16, p, GL_STATIC_DRAW);

	glEnableVertexAttribArray(VP_LOC);
	glEnableVertexAttribArray(COLOR_LOC);

	glVertexAttribPointer(VP_LOC, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(COLOR_LOC, 4, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(pos4)*8));


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_index_vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32)*sizeof(cube_indices)/sizeof(cube_indices[0]), cube_indices, GL_STATIC_DRAW);

	// glDisableVertexAttribArray(COLOR_LOC);
	glDisableVertexAttribArray(UV_LOC);

	glUniform1f(USE_UNIFORM_COLOR_LOC, 0);
	glUniform1f(HAS_TEXTURE_LOC, 0);

	glUniform4fv(UNIFORM_COLOR_LOC, 1, color);

	glUniformMatrix4fv(PROJECTION_UNIFORM_LOC, 1, GL_TRUE, gl_ctx.screen_scale.m);
	glUniformMatrix4fv(CAMERA_UNIFORM_LOC, 1, GL_TRUE, gl_ctx.camera.m);

	glDrawElements(GL_TRIANGLES, 12*3, GL_UNSIGNED_INT , 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// glDeleteBuffers(1, &buf);

}

void draw_sphere(sphere_t *c, float color[4]) {
	glBindVertexArray(gl_ctx.vao);
	buf_clear(sphere_data);

	i32 step_line = 60;
	f32 angle_step = 360.0f/(f32)step_line;

	i32 step_around = 60;
	f32 angle_step_y = 360.0f/(f32)step_around;

	f32 radius = c->radius - .1;
	for(i32 i = 0; i < step_line; i++ ) {
		
		for(i32 j = 0; j < step_around; j++) {
			f32 x = c->center.x + radius*cosf(TO_RADIANS(angle_step*i))*cosf(TO_RADIANS(angle_step*j));
			f32 y = c->center.y + radius*sinf(TO_RADIANS(angle_step_y*j))*cosf(TO_RADIANS(angle_step*i));
			f32 z = c->center.z + radius*sinf(TO_RADIANS(angle_step*i));

			buf_push(sphere_data, (pos4){x, y , z, 1});
		}

	}
	
	if(sphere_vbo == 0) {
		glGenBuffers(1, &sphere_vbo);
	}


	glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos4)*buf_len(sphere_data), sphere_data, GL_STATIC_DRAW);

	glEnableVertexAttribArray(VP_LOC);
	glDisableVertexAttribArray(COLOR_LOC);
	glDisableVertexAttribArray(UV_LOC);

	glVertexAttribPointer(VP_LOC, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glUniform1f(USE_UNIFORM_COLOR_LOC, 1);
	glUniform1f(HAS_TEXTURE_LOC, 0);

	glUniform4fv(UNIFORM_COLOR_LOC, 1, color);

	glUniformMatrix4fv(PROJECTION_UNIFORM_LOC, 1, GL_TRUE, gl_ctx.screen_scale.m);
	glUniformMatrix4fv(CAMERA_UNIFORM_LOC, 1, GL_TRUE, gl_ctx.camera.m);

	glDrawArrays(GL_LINE_LOOP, 0, buf_len(sphere_data));


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	// glDeleteBuffers(1, &buf);

}

void draw_plane(plane_t *p, float color[4]) {

	glBindVertexArray(gl_ctx.vao);
	// buf_clear(plane_data);


	sh_vec3 p1;
	sh_vec3 p2;


	if(p->normal.z != 0) {
		f32 z = (p->distance - p->normal.x*(5 - p->point.x) - p->normal.y*(5 - p->point.y))/p->normal.z;
		p1.x = 5;
		p1.y = 5;
		p1.z = z;
	} else if(p->normal.x != 0) {
		f32 x = (p->distance - p->normal.z*(5 - p->point.z) - p->normal.y*(5 - p->point.y))/p->normal.x + p->point.x;
		p1.x = x;
		p1.y = 5;
		p1.z = 5;
	} else {
		f32 y = (p->distance - p->normal.x*(5 - p->point.x) - p->normal.y*(5 - p->point.z))/p->normal.y + p->point.y;
		p1.x = 5;
		p1.y = y;
		p1.z = 5;
	}

	if(p->normal.z != 0) {
		f32 z = (p->distance - p->normal.x*(-5 - p->point.x) - p->normal.y*(5 - p->point.y))/p->normal.z;
		p2.x = -5;
		p2.y = 5;
		p2.z = z;
	} else if(p->normal.x != 0) {
		f32 x = (p->distance - p->normal.z*(5 - p->point.z) - p->normal.y*(-5 - p->point.y))/p->normal.x + p->point.x;
		p2.x = x;
		p2.y = -5;
		p2.z = 5;
	} else {
		f32 y = (p->distance - p->normal.x*(-5 - p->point.x) - p->normal.y*(5 - p->point.z))/p->normal.y + p->point.y;
		p2.x = -5;
		p2.y = y;
		p2.z = 5;
	}

	sh_vec3 t = sh_vec3_new_sub_vec3(&p1, &p2);
	sh_vec3_normalize_ref(&t);
	sh_vec3 b = sh_vec3_cross(&t, &p->normal);
	sh_vec3_normalize_ref(&b);

	sh_vec3 p_1 = p->point;
	sh_vec3 p_2 = p->point;
	sh_vec3 p_3 = p->point;
	sh_vec3 p_4 = p->point;

	sh_vec3_mul_scaler(&t, 5);
	sh_vec3_mul_scaler(&b, 5);

	sh_vec3_sub_vec3(&p_1, &t);
	sh_vec3_sub_vec3(&p_1, &b);

	sh_vec3_sub_vec3(&p_2, &t);
	sh_vec3_add_vec3(&p_2, &b);

	sh_vec3_add_vec3(&p_3, &t);
	sh_vec3_add_vec3(&p_3, &b);

	sh_vec3_add_vec3(&p_4, &t);
	sh_vec3_sub_vec3(&p_4, &b);



	pos4 points[4] = {
		{p_1.x, p_1.y, p_1.z, 1},
		{p_2.x, p_2.y, p_2.z, 1},
		{p_3.x, p_3.y, p_3.z, 1},
		{p_4.x, p_4.y, p_4.z, 1}
	};


	if(normal_plane_vbo == 0) {
		glGenBuffers(1, &normal_plane_vbo);
	}


	glBindBuffer(GL_ARRAY_BUFFER, normal_plane_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos4)*4, points, GL_STATIC_DRAW);

	glEnableVertexAttribArray(VP_LOC);
	glDisableVertexAttribArray(COLOR_LOC);
	glDisableVertexAttribArray(UV_LOC);

	glVertexAttribPointer(VP_LOC, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glUniform1f(USE_UNIFORM_COLOR_LOC, 1);
	glUniform1f(HAS_TEXTURE_LOC, 0);

	glUniform4fv(UNIFORM_COLOR_LOC, 1, color);

	glUniformMatrix4fv(PROJECTION_UNIFORM_LOC, 1, GL_TRUE, gl_ctx.screen_scale.m);
	glUniformMatrix4fv(CAMERA_UNIFORM_LOC, 1, GL_TRUE, gl_ctx.camera.m);

	glDrawArrays(GL_LINE_LOOP, 0, 4);

}




void draw_line(float *p, float *p1, float *c) {

	i32 depth_func = 0;
	glGetIntegerv(GL_DEPTH_FUNC, &depth_func);

	glDepthFunc(GL_ALWAYS);

	if(line_vbo == 0) {
		glGenBuffers(1, &line_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*8, NULL, GL_STATIC_DRAW);
	}

	glBindBuffer(GL_ARRAY_BUFFER, line_vbo);

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*4, p);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*4, sizeof(float)*4, p1);

	glDisableVertexAttribArray(COLOR_LOC);
	glDisableVertexAttribArray(UV_LOC);

	glUniform1f(USE_UNIFORM_COLOR_LOC, 1);
	glUniform1f(HAS_TEXTURE_LOC, 0);


	glUniform4fv(UNIFORM_COLOR_LOC, 1, c);

	glVertexAttribPointer(VP_LOC, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_LINES, 0, 2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDepthFunc(depth_func);

}



typedef struct point_mass_t {
	pos4 position;
	sh_vec3 velocity;
	sh_vec3 acceleration;
	
	float mass;
	sh_vec3 total_force;

	i8 lock;
	i8 use_gravity;

} point_mass_t;

typedef struct spring_t {
	f32 k; //stiffness?
	f32 damper;
	f32 rest_length;
	point_mass_t *p1;
	point_mass_t *p2;
	sh_vec3 damper_force;
	pos4 color;
} spring_t;

typedef struct point_mass_system_t {
	point_mass_t *pts;
	spring_t *springs;
	i32 *faces;
	u32 index_vbo;
	sh_vec2 *uvs;
} point_mass_system_t;

point_mass_system_t *gl_system = NULL;

i32 grid_size = 10;
f32 distance_between = 2.0f;
f32 dt = 1.0f/100.0f;

sh_vec3 gravity = {0, -9.8f, 0};


point_mass_system_t* make_point_mass_system(i32 point_nums, i32 spring_count) {
	point_mass_system_t* x = (point_mass_system_t*)calloc(1, sizeof(point_mass_system_t));
	buf_fit(x->pts, point_nums);
	buf_fit(x->springs, spring_count);
	return x;
}


point_mass_system_t* init_point_mass_from_obj(obj_file_t *obj) {

	point_mass_system_t *sys = make_point_mass_system(buf_len(obj->verts), buf_len(obj->faces)); //assumption being that each face is 3 springs i.e: a triangle

	for(i32 i = 0; i < buf_len(obj->verts); i++) {

		point_mass_t point = {
			.position = obj->verts[i],
			.velocity = {0},
			.acceleration = {0},
			.mass = 1,
			.lock = 0,
			.total_force = {0},
			.use_gravity = 1
		};

		buf_push(sys->pts, point);
	}

	i32 len = buf_len(obj->faces);
	for(i32 i = 0; i < len; i++ ){

		i32 *face = obj->faces[i];
		i32 faces_len = buf_len(face);
		for(i32 i = 0; i < faces_len; i++) {
			point_mass_t *p1 = sys->pts + face[i] - 1;

			buf_push(sys->faces, face[i]-1);

			for(i32 j = i+1; j < faces_len; j++) {
				point_mass_t *p2 = sys->pts + face[j] - 1;

				spring_t sp = {
					.k = 200,
					.rest_length = sh_pos4_pos4_length(&p1->position, &p2->position),
					.p1 = p1, 
					.p2 = p2,
					.damper = 1
				};

				buf_push(sys->springs, sp);
			}

		}

		
	}

	for(i32 i = 0; i < buf_len(obj->face_tex); i++) {
		buf_push(sys->uvs, obj->tex_coord[obj->face_tex[i] - 1]);// = obj->tex_coord;
	}


	glGenBuffers(1, &sys->index_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sys->index_vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32)*buf_len(sys->faces), sys->faces, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	len = buf_len(sys->pts);
	buf_fit(normals_grid, len);

	for(i32 i = 0; i < len; i += 3) {
		point_mass_t *p1 = sys->pts + sys->faces[i];
		point_mass_t *p2 = sys->pts + sys->faces[i+1];
		point_mass_t *p3 = sys->pts + sys->faces[i+2];
		sh_vec3 d1 = sh_vec3_new_sub_vec3((sh_vec3*)&p1->position, (sh_vec3*)&p2->position);
		sh_vec3 d2 = sh_vec3_new_sub_vec3((sh_vec3*)&p1->position, (sh_vec3*)&p3->position);
		sh_vec3 n = sh_vec3_cross(&d2, &d1);
		sh_vec3_normalize_ref(&n);
		normals_grid[sys->faces[i]] = n;
		normals_grid[sys->faces[i+1]] = n;
		normals_grid[sys->faces[i+2]] = n;
	}



	return sys;

}



// we assume roap is in the Y axis, i.e: it will start at start_point and go down in the Y axis
point_mass_system_t* init_point_mass_system_roap(i32 roap_point_count, float roap_distance, sh_vec3 start_point, sh_vec3 direction) {

	point_mass_system_t *sys = make_point_mass_system(roap_point_count, roap_point_count-1);

	f32 step = roap_distance/roap_point_count;

	for(i32 i = 0; i < roap_point_count; i++) {

		point_mass_t point = {
			.position = {start_point.x + direction.x*i*step, start_point.y  + direction.y*i*step, start_point.z + direction.z*i*step, 1},
			.velocity = {0},
			.acceleration = {0},
			.mass = 10 ,//(i == roap_point_count - 1) ? 10 : 1,
			.lock = 0,//(i == roap_point_count - 1) ? 1 : 0,
			.total_force = {0},
			.use_gravity = 1
		};

		buf_push(sys->pts, point);
	}

	for(i32 i = 0; i < roap_point_count - 1; i++ ) {
		spring_t sp = {
			.k = 1,
			.rest_length = step,
			.p1 = sys->pts + i,
			.p2 = sys->pts + i + 1,
			.damper = 0 
		};

		buf_push(sys->springs, sp);
	}

	return sys;

}

point_mass_system_t* init_point_mass_system_grid(i32 grid_x, i32 grid_y, f32 distance_between_points, sh_vec3 start_point, sh_vec3 end_point) {

	//assuming grid is on the x, Z axis
	point_mass_system_t *sys = make_point_mass_system(grid_x*grid_y, 0);

	f32 z_length = end_point.z - start_point.z;
	f32 x_length = end_point.x - start_point.x;

	f32 z_step = z_length/grid_y;
	f32 x_step = x_length/grid_x;

	for(i32 i = 0; i < grid_x; i++) {
		for(i32 j = 0; j < grid_y; j++) {
			point_mass_t point = {
				.position = {start_point.x + j*x_step, start_point.y, start_point.z +  i*z_step, 1},
				.velocity = {0},
				.acceleration = {0},
				.mass = .5,
				.total_force = {0},
				.lock = 0,
				.use_gravity = 1
			};

			// if(i == 0 ) {
			// 	point.lock = 1;
			// }

			buf_push(sys->pts, point);

			buf_push(sys->uvs, (sh_vec2){ ((f32)j*(tex_width/grid_x) )/(f32)tex_width, ((f32)i*(tex_height/grid_y))/(f32)tex_height });
		}
	}

	for(i32 i = 0; i < grid_x; i++) {
		for(i32 j = 0; j < grid_y; j++) {
			spring_t sp = {
				.k = 400,
				.damper = 1,
				.color = {1, 0, 0, 1}
			};

			
			if(i == 0 || j == 0) {
				sp.color = (pos4){0, 1, 0, 1};
			}

			if(j < grid_y - 1) {
				sp.p1 = sys->pts + i*grid_y + j;
				sp.p2 = sys->pts + i*grid_y + j + 1;

				sp.rest_length = x_step;

				if(i == 0 || i == (grid_x - 1)) {
					sp.color = (pos4){0, 1, 0, 1};
				} else {
					sp.color = (pos4){1, 0, 0, 1};
				}
				buf_push(sys->springs, sp);
			}

			
			if(i < grid_x -1 ) {
				sp.p1 = sys->pts + i*grid_y     + j;
				sp.p2 = sys->pts + (i+1)*grid_y +j;

				sp.rest_length = z_step;

				if(j == 0 || j == (grid_y - 1)) {
					sp.color = (pos4){0, 1, 0, 1};
				} else {
					sp.color = (pos4){1, 0, 0, 1};
				}

				
				buf_push(sys->springs, sp);
			}

			if(i < (grid_x - 1) && j < (grid_y - 1) ) {
				sp.p1 = sys->pts + i*grid_y     + j;
				sp.p2 = sys->pts + (i+1)*grid_y + j + 1;

				sp.rest_length = (f32)sqrt(x_step*x_step + z_step*z_step);
				sp.color = (pos4){0, 0, 1, 1};


				buf_push(sys->springs, sp);
			}

			if(j > 0 && i < (grid_x - 1) ) {
				sp.p1 = sys->pts + i*grid_y     + j;
				sp.p2 = sys->pts + (i+1)*grid_y + j - 1;


				sp.rest_length = (f32)sqrt(x_step*x_step + z_step*z_step);

				// if(j == 0 || j == (grid_y - 1)) {
				sp.color = (pos4){0, 0, 1, 1};
				// } else {
				// 	sp.color = (pos4){1, 0, 0, 1};
				// }


				buf_push(sys->springs, sp);
			}
#if 1

			if(j < grid_y - 2) {
				sp.p1 = sys->pts + i*grid_y     + j;
				sp.p2 = sys->pts + (i)*grid_y + j + 2;
				sp.rest_length = x_step*2;

				buf_push(sys->springs, sp);
			}

			if(i < grid_x - 2) {
				sp.p1 = sys->pts + i*grid_y     + j;
				sp.p2 = sys->pts + (i+2)*grid_y + j;
				sp.rest_length = z_step*2;

				buf_push(sys->springs, sp);
			}
#endif

			if(1) {
				if(i < (grid_y - 1) && j < (grid_x - 1)) {
					buf_push(sys->faces, i*grid_y + j);
					buf_push(sys->faces, i*grid_y + j+1);
					buf_push(sys->faces, (i+1)*grid_y + j+1);

					buf_push(sys->faces, i*grid_y + j);
					buf_push(sys->faces, (i+1)*grid_y + j+1);
					buf_push(sys->faces, (i+1)*grid_y + j);

				}
			}

		}
	}


	glGenBuffers(1, &sys->index_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sys->index_vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32)*buf_len(sys->faces), sys->faces, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	i32 len = buf_len(sys->pts);
	buf_fit(normals_grid, len);

	for(i32 i = 0; i < len; i += 3) {
		point_mass_t *p1 = sys->pts + sys->faces[i];
		point_mass_t *p2 = sys->pts + sys->faces[i+1];
		point_mass_t *p3 = sys->pts + sys->faces[i+2];
		sh_vec3 d1 = sh_vec3_new_sub_vec3((sh_vec3*)&p1->position, (sh_vec3*)&p2->position);
		sh_vec3 d2 = sh_vec3_new_sub_vec3((sh_vec3*)&p1->position, (sh_vec3*)&p3->position);
		sh_vec3 n = sh_vec3_cross(&d2, &d1);
		sh_vec3_normalize_ref(&n);
		normals_grid[sys->faces[i]] = n;
		normals_grid[sys->faces[i+1]] = n;
		normals_grid[sys->faces[i+2]] = n;
	}


	return sys;
}


void render_point_mass_system(point_mass_system_t *system, float point_color[4], float spring_color[4], float border_color[4]) {


	if(system == NULL) return;

	buf_clear(lines_to_draw);
	buf_clear(pts_to_draw);
	buf_clear(normals_grid);

	i32 len = buf_len(system->pts);
	for(i32 i = 0; i < len; i++) {
		point_mass_t *p = system->pts + i;
		buf_push(pts_to_draw, p->position);
		// buf_push(normals_grid, (sh_vec3){0});
	}

	buf_fit(normals_grid, len);

	len = buf_len(system->faces);

	for(i32 i = 0; i < len; i += 3) {
		point_mass_t *p1 = system->pts + system->faces[i];
		point_mass_t *p2 = system->pts + system->faces[i+1];
		point_mass_t *p3 = system->pts + system->faces[i+2];

		sh_vec3 d1 = sh_vec3_new_sub_vec3((sh_vec3*)&p1->position, (sh_vec3*)&p2->position);
		sh_vec3 d2 = sh_vec3_new_sub_vec3((sh_vec3*)&p1->position, (sh_vec3*)&p3->position);
		// sh_vec3_normalize_ref(&d1);
		// sh_vec3_normalize_ref(&d1);

		sh_vec3 n = sh_vec3_cross(&d2, &d1);
		sh_vec3_normalize_ref(&n);

		normals_grid[system->faces[i]] = n;
		normals_grid[system->faces[i+1]] = n;
		normals_grid[system->faces[i+2]] = n;

	}

	len = buf_len(system->springs);
	for(i32 i = 0; i < len; i++) {

			spring_t *p = system->springs + i;


			buf_push(lines_to_draw, p->p1->position);
			buf_push(lines_to_draw, p->p2->position);

			// draw_line(p->p1->position._d, p->p2->position._d, p->color._d);

			float damper_force[4] = {
				p->p1->position.x + p->damper_force.x,
				p->p1->position.y + p->damper_force.y,
				p->p1->position.z + p->damper_force.z,
				1
			};

			float damper_force2[4] = {
				p->p2->position.x + p->damper_force.x,
				p->p2->position.y + p->damper_force.y,
				p->p2->position.z + p->damper_force.z,
				1
			};


	}


	if(line_vbo == 0) {
		glGenBuffers(1, &line_vbo);
	}

	
	glBindVertexArray(gl_ctx.vao);

#if 1

	if(point_vbo == 0 || normal_vbo == 0 || uv_vbo == 0) {
		glGenBuffers(1, &point_vbo);
		glGenBuffers(1, &normal_vbo);
		glGenBuffers(1, &uv_vbo);
	}


	// glPointSize(4);

	glBindVertexArray(gl_ctx.vao);
	glBindBuffer(GL_ARRAY_BUFFER, point_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos4)*buf_len(pts_to_draw), pts_to_draw,  GL_STATIC_DRAW);

	glEnableVertexAttribArray(VP_LOC);
	glVertexAttribPointer(VP_LOC, 4, GL_FLOAT,    GL_FALSE, 0, 0);

	glDisableVertexAttribArray(COLOR_LOC);
	glUniform1f(USE_UNIFORM_COLOR_LOC, 1);
	glUniform4fv(UNIFORM_COLOR_LOC, 1, point_color);

	glUniform1f(USE_LIGHT, 1);
	glEnableVertexAttribArray(NORMAL_LOC);
	glBindBuffer(GL_ARRAY_BUFFER, normal_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sh_vec3)*buf_len(pts_to_draw), normals_grid, GL_STATIC_DRAW);
	glVertexAttribPointer(NORMAL_LOC, 3, GL_FLOAT, GL_FALSE, 0, 0);

	if(system->uvs) {
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glUniform1f(HAS_TEXTURE_LOC, 1);
		glEnableVertexAttribArray(UV_LOC);
		glBindBuffer(GL_ARRAY_BUFFER, uv_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(sh_vec2)*buf_len(system->uvs), system->uvs, GL_STATIC_DRAW);
		glVertexAttribPointer(UV_LOC, 2, GL_FLOAT, GL_FALSE, 0, 0);
	} else {
		glUniform1f(HAS_TEXTURE_LOC, 0);
		glDisableVertexAttribArray(UV_LOC);
		glBindBuffer(GL_ARRAY_BUFFER, uv_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(sh_vec2)*buf_len(system->uvs), system->uvs, GL_STATIC_DRAW);
		glVertexAttribPointer(UV_LOC, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, system->index_vbo);
	glDrawElements(GL_TRIANGLES, buf_len(system->faces), GL_UNSIGNED_INT, 0);
	 
	glUniform1f(USE_LIGHT, 0);
	
#endif

#if 0
	glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos4)*buf_len(lines_to_draw), lines_to_draw, GL_STATIC_DRAW);
	glDisableVertexAttribArray(COLOR_LOC);
	glDisableVertexAttribArray(UV_LOC);
	glUniform1f(USE_UNIFORM_COLOR_LOC, 1);
	glUniform1f(HAS_TEXTURE_LOC, 0);
	glUniform4fv(UNIFORM_COLOR_LOC, 1, border_color);
	glVertexAttribPointer(VP_LOC, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_LINES, 0, buf_len(lines_to_draw));

	if(point_vbo == 0) {
		glGenBuffers(1, &point_vbo);
	}


	glPointSize(4);

	glBindVertexArray(gl_ctx.vao);

	glBindBuffer(GL_ARRAY_BUFFER, point_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos4)*buf_len(pts_to_draw), pts_to_draw,  GL_STATIC_DRAW);

	glEnableVertexAttribArray(VP_LOC);
	glEnableVertexAttribArray(COLOR_LOC);

	glVertexAttribPointer(VP_LOC, 4, GL_FLOAT,    GL_FALSE, sizeof(pos4)*2, 0);
	glVertexAttribPointer(COLOR_LOC, 4, GL_FLOAT, GL_FALSE, sizeof(pos4)*2, (void*)(sizeof(pos4)));


	glUniform4fv(UNIFORM_COLOR_LOC, 1, point_color);

	glUniform1f(USE_UNIFORM_COLOR_LOC, 1);
	glUniform1f(HAS_TEXTURE_LOC, 0);

	glDrawArrays(GL_POINTS, 0, buf_len(pts_to_draw)/2);

	// glBindBuffer(GL_ARRAY_BUFFER, point_vbo);
	// glBufferData(GL_ARRAY_BUFFER, sizeof(pos4)*buf_len(pts_to_draw), pts_to_draw, GL_STATIC_DRAW);
	// glDrawArrays(GL_POINTS, 0, 100);


#endif




}

void update_point(point_mass_t *p) {

	if(p->lock) {

		p->total_force.x = 0;
		p->total_force.y = 0;
		p->total_force.z = 0;

		return;
	}

	sh_vec3 *force = &p->total_force;

	sh_vec3 gf = sh_vec3_new_mul_scaler(&gravity, p->mass);

	if(p->use_gravity) {
		sh_vec3_add_vec3(force, &gf);
	}

	if(wind) {
		sh_vec3 wind = {0, 0, -10};
		sh_vec3_add_vec3(force, &wind);
	}

	sh_vec3 cur_vel = p->velocity;
	collision_t t = {0};
	if(option == 1) {
		t = collision_point_sphere((sh_vec3*)&p->position, &sphere_at_zero);
	}

	if(t.collided) {
		p->position.x = t.intersection_point.x;
		p->position.y = t.intersection_point.y;
		p->position.z = t.intersection_point.z;

		// sh_vec3 norm = t.intersection_point;
		// sh_vec3_sub_vec3(&norm, &sphere_at_zero.center);
		// sh_vec3_normalize_ref(&norm);

		// sh_vec3 down = norm;
		// sh_vec3_mul_scaler(&down, -1);
		// f32 down_amount = sh_vec3_dot(force, &down);
		// sh_vec3_mul_scaler(&down, -down_amount);
		// sh_vec3_add_vec3(force, &down);

			
	}


	sh_vec3_div_scaler(force, p->mass); // this is accel delta now

	sh_vec3_mul_scaler(force, dt);
	sh_vec3_add_vec3(&p->velocity, force);
	
	
	sh_vec3 v_delta = sh_vec3_new_mul_scaler(&p->velocity, dt);
	sh_pos4_as_vec3_add_vec3(&p->position, &v_delta);
	if(option == 1) {
		t = collision_point_sphere((sh_vec3*)&p->position, &sphere_at_zero);
	}

	if(t.collided) {

		p->position.x = t.intersection_point.x;
		p->position.y = t.intersection_point.y;
		p->position.z = t.intersection_point.z;

		sh_vec3 norm = t.intersection_point;
		sh_vec3_sub_vec3(&norm, &sphere_at_zero.center);
		sh_vec3_normalize_ref(&norm);

		sh_vec3 n = norm;

		sh_vec3_mul_scaler(&norm, -1);
		sh_vec3 down = norm;
		f32 down_amount = sh_vec3_dot(&p->velocity, &down);
		sh_vec3_mul_scaler(&down, -down_amount);
		sh_vec3_add_vec3(&p->velocity, &down);

		// f32 mul = sh_vec3_length(&p->velocity)/sh_vec3_length(&cur_vel);
		// sh_vec3_mul_scaler(&n, mul);
		// sh_vec3_add_vec3(&p->velocity, &n);
	}

	sh_vec3_mul_scaler(&p->velocity, 0.998f);


	if(p->position.y < 0)
		p->position.y = 0;
	
	p->total_force.x = 0;
	p->total_force.y = 0;
	p->total_force.z = 0;

}

i32 drop = 0;

void update_spring(spring_t *sp) {

	f32 length = 0;
	sh_vec3 direction = {0};

	sh_pos4_as_vec3_pos4_as_vec3_length_and_direction(&sp->p1->position, &sp->p2->position, &length, &direction);

	sh_vec3 vel_diff = sh_vec3_new_sub_vec3(&sp->p2->velocity, &sp->p1->velocity);

	f32 damper_scaler_force = sh_vec3_dot(&vel_diff, &direction)*sp->damper;

	sp->damper_force = vel_diff;
	sh_vec3_mul_scaler(&sp->damper_force, -damper_scaler_force);

	length = sp->k*(sp->rest_length - length) - damper_scaler_force;

	sh_vec3_mul_scaler(&direction, length);
	sh_vec3_add_vec3(&sp->p2->total_force, &direction);
	sh_vec3_mul_scaler(&direction, -1);
	sh_vec3_add_vec3(&sp->p1->total_force, &direction);

}

void step_system(point_mass_system_t *system) {

	if(system == NULL) return;
	has_locked_t = 9999;

	for(i32 i = 0; i < buf_len(system->springs); i++) {
		update_spring(system->springs + i);
	}


	for(i32 i = 0; i < buf_len(system->pts); i++) {
		point_mass_t *p = system->pts + i;

		if(apply_rot && i == 0) {

			f32 l = sh_pos4_length(&p->position);

			sh_vec3 rot_force = {
				l*cosf(TO_RADIANS(cur_angle))*p->position.x + l*sinf(TO_RADIANS(cur_angle))*p->position.z,
				l*p->position.y,
				-l*sinf(TO_RADIANS(cur_angle))*p->position.x + l*cosf(TO_RADIANS(cur_angle))*p->position.z,
			};

			// p->total_force.x += rot_force.x*dt;
			//  p->total_force.y += rot_force.y*dt;
			//  p->total_force.z += rot_force.z*dt;

			sh_vec3_mul_scaler(&rot_force, 2*dt);
			sh_vec3_add_vec3(&p->total_force, &rot_force);

		}

		update_point(p);


	}


}

ray_t build_mouse_ray() {

	pos4 m = {(f32)mouse.cur_x, ((f32)mouse.cur_y-23), -1, 1};

	m.x = (m.x - gl_ctx.width/2)/gl_ctx.width;
	m.y = (-m.y + gl_ctx.height/2)/gl_ctx.height;


	m.x *= 2.0f;
	m.y *= 2.0f;

	// pos4 m = {(f32)mouse.cur_x, ((f32)mouse.cur_y), -1, 1};



	m = mul_mat4_pos4(&m, &gl_ctx.inv_screen_scale);
	m.z = -1;
	m.w = 0;

	m = mul_mat4_pos4(&m, &gl_ctx.inv_camera);
	m.w = 0;

	sh_pos4_normalize_ref(&m);

	ray_t ray = {
		.direction = {m.x, m.y, m.z},
		.start = gl_ctx.eye_pos
	};


	return ray;
}


void read_texture(char *file_name) {
	i32 tex_chann;

	printf("%s\n", file_name);
	u8 *data = stbi_load(file_name, &tex_width, &tex_height, &tex_chann, 3);

	printf("%d %d\n", tex_width, tex_height);
	printf("%p\n", data);

	if(texture_id == 0) {
		glGenTextures(1, &texture_id);
	}

	glBindTexture(GL_TEXTURE_2D, texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

}

void render_gui() {

	if (nk_begin(ctx, "Demo", nk_rect(0, 0, 300, 350), NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {

		nk_layout_row_dynamic(ctx, 30, 1);
		LPSTR file_long = NULL;

		if (nk_button_label(ctx, "Open OBJ File")) {
			OPENFILENAME file_open = {0};
			file_open.lStructSize = sizeof(OPENFILENAME);
			file_open.lpstrFile = file_name_buffer;
			file_open.nMaxFile = 1024;
			GetOpenFileName(&file_open);

			DWORD err = CommDlgExtendedError();
			if(err == 0 && file_name_buffer[0] != '\0') {
				obj_file_t obj = parse_obj_file(file_name_buffer);
				gl_system = init_point_mass_from_obj(&obj);
				memset(file_name_buffer, '\0', 1024 );
			}
		}

		if (nk_button_label(ctx, "Open Texture File")) {
			OPENFILENAME file_open = {0};
			file_open.lStructSize = sizeof(OPENFILENAME);
			file_open.lpstrFile = file_name_buffer;
			file_open.nMaxFile = 1024;
			GetOpenFileName(&file_open);

			DWORD err = CommDlgExtendedError();
			if(err == 0 && file_name_buffer[0] != '\0') {
				read_texture(file_name_buffer);
				memset(file_name_buffer, '\0', 1024 );
			}
		}


		if (nk_button_label(ctx, "Save OBJ File")) {
			DWORD err = CommDlgExtendedError();
			if(err == 0) {
				write_system_to_obj(gl_system, "obj_output.obj");
			}
		}


		nk_layout_row(ctx, NK_DYNAMIC, 30, 3, (float[]){ 0.25f, .60f, 0.15f });

		if(saving) {
			// nk_value_int(ctx, "Saving", gl_ctx.frame_number);
			nk_labelf(ctx, NK_TEXT_LEFT, "Saving...", NULL);
		}

		if(recording) {
			// nk_value_int(ctx, "Saving", gl_ctx.frame_number);
			nk_labelf(ctx, NK_TEXT_LEFT, "Recording...", NULL);
		}

		i32 cur_option = option;

		nk_layout_row(ctx, NK_DYNAMIC, 30, 1, (float[]){1});
		if (nk_button_symbol_label(ctx, (option == 0)?NK_SYMBOL_CIRCLE_OUTLINE:NK_SYMBOL_CIRCLE_SOLID, "Cloth to ground", NK_TEXT_LEFT))
			option = 0;
		nk_layout_row(ctx, NK_DYNAMIC, 30, 1, (float[]){1});
		if (nk_button_symbol_label(ctx, (option == 1)?NK_SYMBOL_CIRCLE_OUTLINE:NK_SYMBOL_CIRCLE_SOLID, "Cloth to Sphere", NK_TEXT_LEFT))
			option = 1;
		nk_layout_row(ctx, NK_DYNAMIC, 30, 1, (float[]){ 1 });
		if (nk_button_symbol_label(ctx, (option == 2)?NK_SYMBOL_CIRCLE_OUTLINE:NK_SYMBOL_CIRCLE_SOLID, "Cloth Held", NK_TEXT_LEFT))
			option = 2;


		nk_layout_row(ctx, NK_DYNAMIC, 30, 2, (float[]){ .5, .5 });
		nk_checkbox_label(ctx, "Move sphere", &move_sphere);
		nk_checkbox_label(ctx, "Wind sphere", &wind);

		switch(option) {

			case 0: {
				i32 grid_x = 10;
				i32 grid_y = 10;

				if(cur_option != option) {
					gl_system = init_point_mass_system_grid(grid_x, grid_y, 1, (sh_vec3){-5, 5, -5}, (sh_vec3){10, 5, 10});
				}
			} break;
			case 1: {
				i32 grid_x = 80;
				i32 grid_y = 80;

				if(cur_option != option) {
					gl_system = init_point_mass_system_grid(grid_x, grid_y, 1, (sh_vec3){-5, 5, -5}, (sh_vec3){10, 5, 10});
				}


			} break;

			case 2: {

				i32 grid_x = 80;
				i32 grid_y = 80;

				if(cur_option != option) {
					gl_system = init_point_mass_system_grid(grid_x, grid_y, 1, (sh_vec3){-5, 15, -5}, (sh_vec3){5, 15, 5});
				}

				for(i32 i = 0; i < grid_x; i++) {
					gl_system->pts[i*grid_y].lock = 1;
				}


			} break;

		}
		// nk_slider_int(ctx, 0, &gl_ctx.frame_number, current_anim.frame_number, 1);

	}

	nk_end(ctx);
}




void render() {

	glBindVertexArray(gl_ctx.vao);
	glUseProgram(gl_ctx.program);

	glUniformMatrix4fv(PROJECTION_UNIFORM_LOC, 1, GL_TRUE, gl_ctx.screen_scale.m);
	glUniformMatrix4fv(CAMERA_UNIFORM_LOC, 1, GL_TRUE, gl_ctx.camera.m);

	render_plane();
	render_axis();

	ray_t r = build_mouse_ray();
	collision_t t = collision_ray_plane(&r, &ground_plane);


	if(t.collided && move_sphere) {

		sphere_at_zero.center = t.intersection_point;

		// sh_vec3 n = sphere_at_zero.center;
		// sh_vec3_sub_vec3(&n, &t.intersection_point);

		// plane_t pl = {
		// 	.normal = n,
		// 	.point = t.intersection_point,
		// 	.distance = 0
		// };

		// draw_plane(&pl, COLOR_HEX_FLOAT4(0xF0FF0F));
	}

	if(option == 1) {
		draw_sphere(&sphere_at_zero, COLOR_HEX_FLOAT4(0xFFFF00));
	}
	// draw_cube(&cube_at_zero, COLOR_HEX_FLOAT4(0xFFFF00));


	render_point_mass_system(gl_system, COLOR_HEX_FLOAT4(0xFFFFFF), COLOR_HEX_FLOAT4(0x0FF0F0), COLOR_HEX_FLOAT4(0xFF00FF) );
}

void render_button(char *text, float x, float y) {
	ren_ctx.next_x = x;
	ren_ctx.next_y = y;
	ren_ctx.left_margin = 0;

	draw_full_text(text);
	draw_rect(x, y, 40, -80, (pos4){1, 0, 0, 1});
}



// void render_gui() {
// 	render_button("Button", 0, 0);
// }



void write_system_to_obj(point_mass_system_t *sys, char *filename) {

	FILE *f = fopen(filename, "w");

	for(i32 i = 0; i < buf_len(sys->pts); i++) {
		pos4 *p = &sys->pts[i].position;
		fprintf(f, "v % 8f % 8f % 8f\n", p->x, p->y, p->z );
	}

	for(i32 i = 0; i < buf_len(sys->pts); i++) {
		sh_vec3 *p = normals_grid + i;
		fprintf(f, "vn % 8f % 8f % 8f\n", -p->x, -p->y, -p->z );
	}

	for(i32 i = 0; i < buf_len(sys->uvs); i++) {
		sh_vec2 *p = sys->uvs + i;
		fprintf(f, "vt % 8f % 8f\n", p->x, p->y );
	}


	for(i32 i = 0; i < buf_len(sys->faces); i += 3) {
		i32 f1 = sys->faces[i];
		i32 f2 = sys->faces[i+1];
		i32 f3 = sys->faces[i+2];
		fprintf(f, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", f1+1, f1+1, f1+1,   f2+1, f2+1, f2+1, f3+1, f3+1,f3+1);
	}


}



DWORD WINAPI save_frames(LPVOID frames) {

	printf("ywes wes tart");
	u8 **f = (u8**)frames;

	saving = 1;

	static char temp_buf[128] = {0};
	for(i32 i = 0; i < buf_len(f); i++) {
		sprintf(temp_buf, "video/image%d.png", i);
		stbi_write_png(temp_buf, gl_ctx.width, gl_ctx.height, 4, f[i], gl_ctx.width*4);
	}


	saving = 0;
	return 0;
}


int main( int argc, char ** argv ) {



	setup();
	// setup_font();
	// setup_font_atlas();

	read_texture("extra/fabric.jpg");
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#if 0
	gl_system = init_point_mass_system_roap(20, 10, (sh_vec3){0, 10, 0}, (sh_vec3){1, 0, 0});

	gl_system->pts[0].lock = 1;
	gl_system->pts[buf_len(gl_system->pts)-1].lock = 1;

	i32 grid_x = 32;
	i32 grid_y = 32;
	gl_system = init_point_mass_system_grid(grid_x, grid_y, 1, (sh_vec3){-5, 5, -5}, (sh_vec3){10, 5, 10});
#endif

	

	// gl_system = init_point_mass_from_obj(&obj);


	f64 total_time = 0;
	f64 saving_frame = 0;

	glEnable(GL_MULTISAMPLE);

	glfwSwapInterval(1);

	u8 *pixels_rev = (u8*)calloc(4, gl_ctx.width*gl_ctx.height);

	i32 frame_count = 0;

	u8 **frames = NULL;

	while(!gl_ctx.should_close) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		glfwPollEvents();

		update_time();
		handle_events();


#if 1
		if(gl_ctx.keyboard[VK_UP].pressed_once) {
			step_system(gl_system);
		}

		render();

		total_time += gl_time.delta_time_sec;

		if(total_time > dt && should_sim) {
			step_system(gl_system);
			total_time = 0;
		}

		saving_frame += gl_time.delta_time_sec;

		if(recording && saving_frame > 1.0/2.0) {

			saving_frame = 0;

			u8 *pixels = (u8*)calloc(4, gl_ctx.width*gl_ctx.height);
			glReadPixels(0, 0, gl_ctx.width, gl_ctx.height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

			u32 *pps = (u32*)pixels;

			for(i32 i = 0; i < gl_ctx.height/2; i++) {
				for(i32 j = 0; j < gl_ctx.width; j++) {
					i32 index_bottom = i*gl_ctx.width + j;
					i32 index_top = (gl_ctx.height - i-1)*gl_ctx.width + j;
					u32 bottom = pps[index_bottom];
					u32 top = pps[index_top];
					pps[index_bottom] = top;
					pps[index_top] = bottom;
				}
			}

			buf_push(frames, pixels);

		}


		if(gl_ctx.keyboard['1'].pressed_once) {
			should_sim = !should_sim;
		}

		if(gl_ctx.keyboard['2'].pressed_once) {
			// apply_rot = !apply_rot;
			write_system_to_obj(gl_system, "test_output.obj");
		}

		// if(apply_rot) {
		// 	cur_angle +=1;
		// }


		// if(gl_ctx.keyboard['2'].pressed_once) {
		// 	for(i32 i = 0; i < grid_x; i++) {
		// 		for(i32 j = 0; j < grid_y; j++) {
		// 			point_mass_t *p = gl_system->pts + i*grid_y + j;
		// 			// if(i == (grid_x - 1) || j == (grid_y - 1)) {
		// 				p->lock = 0;
		// 			// }
		// 		}
		// 	}
		// }

#endif


		if(gl_ctx.keyboard['3'].pressed_once ) {

			
			if(recording == 0 && saving == 0)  {
				printf("we start recoring");
				recording = 1;
			} else if(recording == 1) {
				recording = 0;
			}


			if(recording == 1) {
				frame_count = 0;
				for(i32 i = 0; i < buf_len(frames); i++) {
					free(frames[i]);
					buf_clear(frames);
				}
			}

			if(recording == 0 && saving == 0) {
				printf("thread should start\n");
				i32 thread_id = 0;
				saving = 1;
				CreateThread(NULL, 0, save_frames, frames, 0, NULL );
			}

		}


		
		nk_glfw3_new_frame();
		render_gui();
		nk_glfw3_render(NK_ANTI_ALIASING_ON);
		glfwSwapBuffers(gl_ctx.window);


		// SwapBuffers(gl_ctx.hdc);
	}


	return 0;
}
