typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;


float yaw = 0;
float pitch = 0;

typedef struct sh_reloadable_shader {
	FILETIME last_write;
	i32 type;
	i32 handle;
	i8 needs_reload;
	char *file_name;

} sh_reloadable_shader;

typedef struct key_button {
	u8 pressed;
	u8 pressed_once;
} key_button;


typedef struct mouse_info {
	i32 cur_x;
	i32 cur_y;
	i32 prev_x;
	i32 prev_y;

	i32 delta_x;
	i32 delta_y;

	key_button left;
	key_button right;


} mouse_info;

typedef struct time_info {
	uint64 tick;
	uint64 delta_tick;
	
	uint64 time_nano;
	uint64 time_micro;
	uint64 time_milli;
	float  time_sec;
	
	uint64 delta_time_nano;
	uint64 delta_time_micro;
	uint64 delta_time_milli;
	float delta_time_sec;

	uint64 tick_per_sec;
	uint64 start_tick;
} time_info;


time_info gl_time;
mouse_info mouse;

typedef struct character_info {
	pos4 uv;
	i32 x_offset;
	i32 y_offset;
	f32 advance_width;
} character_info;

typedef struct sh_window_context {
	i16 height;
	i16 width;
	i16 x;
	i16 y;
	i8 should_close;
	u8 move_camera;
	char *window_name;
	HDC hdc;
	mat4 screen_scale;
	mat4 inv_screen_scale;
	mat4 text_ortho;
	mat4 camera;
	mat4 inv_camera;

	u32 vbo;
	u32 vao;
	i32 font_size;
	f32 quad_height;
	f32 quad_width;
	char input[256];
	i32 input_size;
	u8 *font_atlas;
	u8 *ft_atlas;
	u32 font_atlas_texture;
	u32 ft_atlas_texture;
	i32 atlas_height;
	i32 atlas_width;
	i32 ft_atlas_height;
	i32 ft_atlas_width;


	character_info char_cache[94];
	character_info ft_char_cache[94];


	char *text;
	i32 line_numbers;
	float left_add;
	float top_add;


	sh_vec3 eye_pos;
	sh_vec3 look_at;
	sh_vec3 up;
	key_button keyboard[256];
	i32 frame_number;

	GLFWwindow *window;
	i32 program;

} sh_window_context;


sh_window_context gl_ctx;
font_directory *font;



i32 sh_create_shader(char *file_name, i32 type) {
	char *shader_source = read_file(file_name, NULL);
	i32 shader = glCreateShader(type);
	glShaderSource(shader, 1, &shader_source, NULL);
	glCompileShader(shader);

	GLint shader_compile_status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_compile_status);

	free(shader_source);
	if(shader_compile_status != GL_TRUE) {
		int log_size;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
		char *log = malloc(log_size);
		glGetShaderInfoLog(shader, log_size, &log_size, log);
		printf("shader (%s) has errors: %s\n", file_name, log);
		free(log);
		return 0;
	}

	return shader;
}

sh_reloadable_shader sh_create_reloadable_shader(char *file_name, i32 type) {
	sh_reloadable_shader shader = {0};

	shader.handle = sh_create_shader(file_name, type);

	if(shader.handle == 0) {
		printf("error creating reloadable shader.\n");
	} else {
		i32 file_name_size = (i32)strlen(file_name)+1;
		shader.file_name = malloc(file_name_size);
		CopyMemory(shader.file_name, file_name, file_name_size);
		shader.type = type;
		shader.last_write = sh_get_file_last_write(shader.file_name);
	}


	return shader;
}

i32 sh_create_program(i32 *shaders, i32 shader_count) {
	i32 program = glCreateProgram();
	for(int i = 0; i < shader_count; ++i) {
		glAttachShader(program, shaders[i]);
		// printf("shader %d\n", shaders[i]);
	}
	glLinkProgram(program);

	i32 program_link_status;
	glGetProgramiv(program, GL_LINK_STATUS, &program_link_status);

	if(program_link_status != GL_TRUE) {
		i32 log_length = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

		char *log = malloc(log_length);
		glGetProgramInfoLog(program, log_length, &log_length, log);
		printf("program linking error(%d): %s\n", __LINE__, log);
		free(log);
		return 0;
	}

	return program;
}

void init_opengl() {

	glGenVertexArrays(1, &gl_ctx.vao);
	glBindVertexArray(gl_ctx.vao);

	sh_reloadable_shader vs = sh_create_reloadable_shader("shader/vertex_shader.glsl", GL_VERTEX_SHADER);
	sh_reloadable_shader fg = sh_create_reloadable_shader("shader/fragment_shader.glsl", GL_FRAGMENT_SHADER);

	i32 shaders[2] = {vs.handle, fg.handle};
	i32 texture_prog = sh_create_program(shaders, 2);

	gl_ctx.program = texture_prog;

	glUseProgram(texture_prog);

	float width_half = gl_ctx.width/2.0f;
	float height_half = gl_ctx.height/2.0f;

	gl_ctx.screen_scale = perspective(60.0f, (float)gl_ctx.width/gl_ctx.height, 0.1f, 1000.0f);
	gl_ctx.inv_screen_scale = inverse(&gl_ctx.screen_scale);
	gl_ctx.text_ortho = ortho(-width_half, width_half, -height_half, height_half, -1, 1);

	glUniformMatrix4fv(PROJECTION_UNIFORM_LOC, 1, GL_TRUE, gl_ctx.screen_scale.m);

	gl_ctx.eye_pos = (sh_vec3){10, 8, 10};
	// gl_ctx.eye_pos = (sh_vec3){2, 2, 2};
	gl_ctx.look_at = (sh_vec3){-1, -1, -1};
	gl_ctx.up = (sh_vec3){0, 1, 0};

	gl_ctx.camera = lookat(gl_ctx.eye_pos, gl_ctx.look_at, gl_ctx.up);
	glUniformMatrix4fv(CAMERA_UNIFORM_LOC, 1, GL_TRUE, gl_ctx.camera.m);

	glViewport(0, 0, gl_ctx.width, gl_ctx.height);

	glGenBuffers(1, &gl_ctx.vbo);

	glEnableVertexAttribArray(VP_LOC);

	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	fflush(stdout);
}

LRESULT sh_window_proc(HWND h_wnd, UINT msg, WPARAM w_param, LPARAM l_param) {

	LRESULT result = 0; 

	switch(msg) {
		case WM_DESTROY: {
			PostQuitMessage(0);
			gl_ctx.should_close = 1;
		} break;

		case WM_CHAR: {

			gl_ctx.input[gl_ctx.input_size] = (char)w_param;
			gl_ctx.input_size++;

		} break;

		case WM_LBUTTONDOWN: {
			mouse.cur_y = (i32)(l_param >> 16 ) ;
			mouse.cur_x = (i32)( l_param & 0xFFFF );


			mouse.cur_x = mouse.cur_x - gl_ctx.width/2;
			mouse.cur_y = -mouse.cur_y + gl_ctx.height/2;

			mouse.left.pressed = 1;
		} break;

		case WM_LBUTTONUP: {
			mouse.cur_y = (i32)(l_param >> 16 ) ;
			mouse.cur_x = (i32)( l_param & 0xFFFF );

			mouse.cur_x = mouse.cur_x - gl_ctx.width/2;
			mouse.cur_y = -mouse.cur_y + gl_ctx.height/2;

			mouse.left.pressed_once = 1;
			mouse.left.pressed = 0;
		} break;

		case WM_MOUSEMOVE: {
			mouse.cur_y = (i32)(l_param >> 16 ) ;
			mouse.cur_x = (i32)( l_param & 0xFFFF );

			mouse.cur_x = mouse.cur_x - gl_ctx.width/2;
			mouse.cur_y = -mouse.cur_y + gl_ctx.height/2;

		} break;

		default: result =  DefWindowProc(h_wnd, msg, w_param, l_param);
	}

	return result;
}

void load_modern_wgl() {

	HWND fake_window = CreateWindowEx(
			0, "STATIC", "TEMP",
			WS_OVERLAPPED,
			CW_USEDEFAULT,
			CW_USEDEFAULT, 
			CW_USEDEFAULT, 
			CW_USEDEFAULT,
			NULL,
			NULL,
			NULL,
			NULL
			);


	HDC fake_dc = GetDC(fake_window);

	PIXELFORMATDESCRIPTOR sh_px = {0};
	sh_px.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	sh_px.nVersion = 1;
	sh_px.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	sh_px.cColorBits = 24;

	int index = ChoosePixelFormat(fake_dc, &sh_px);


	DescribePixelFormat(fake_dc, index, sizeof(sh_px), &sh_px);
	SetPixelFormat(fake_dc, index, &sh_px);

	HGLRC gl_temp = wglCreateContext(fake_dc);
	wglMakeCurrent(fake_dc, gl_temp);

	load_modern_context_creation();

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(gl_temp);
	ReleaseDC(fake_window, fake_dc);
	DestroyWindow(fake_window);


}


void setup_opengl(HWND h_wnd) {

	GLint pixel_format_attr[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB, 24,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_STENCIL_BITS_ARB, 8,
		WGL_SAMPLE_BUFFERS_ARB, 1,
		WGL_SAMPLES_ARB, 4,
		0
	};


	HDC dc = GetDC(h_wnd);

	i32 pixel_format = 0;
	u32 num_format = 0;

	wglChoosePixelFormatARB(dc, pixel_format_attr, NULL, 1, &pixel_format, &num_format);

	PIXELFORMATDESCRIPTOR m = {0};
	m.nSize = sizeof(PIXELFORMATDESCRIPTOR);

	DescribePixelFormat(dc, pixel_format, sizeof(PIXELFORMATDESCRIPTOR), &m);

	SetPixelFormat(dc, pixel_format, &m);

	GLint context_attr[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 5,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};

	HGLRC gl_c = wglCreateContextAttribsARB(dc, NULL, context_attr);

	wglMakeCurrent(dc, gl_c);

	load_ext();
	gl_ctx.hdc = dc;


	init_opengl();
}

void init_time() {
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	gl_time.tick_per_sec = li.QuadPart;

	QueryPerformanceCounter(&li);

	gl_time.start_tick = li.QuadPart;
	gl_time.tick = li.QuadPart;
}



void handle_char(GLFWwindow *win, i32 key, i32 scancode, i32 action, i32 mod) {

	if(action == GLFW_PRESS) {
		char c = (char)key;
		switch(key) {
			case GLFW_KEY_ESCAPE: {
				gl_ctx.should_close = 1; 
			} break;

		}
	}

	fflush(stdout);
}



HWND sh_window_setup(void) {

	GLFWwindow *window;
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

	window = glfwCreateWindow(gl_ctx.width, gl_ctx.height, "Hello World", NULL, NULL);
	glfwMakeContextCurrent(window);

	gl_ctx.window = window;

	glewExperimental = 1;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to setup GLEW\n");
		exit(1);
	}

	// load_ext();
	init_opengl();
	init_time();
	glfwSetKeyCallback(window, handle_char);

	return NULL;




	load_modern_wgl();

	WNDCLASS wndclass = {0};
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.lpfnWndProc = (WNDPROC)sh_window_proc;
	wndclass.hInstance = NULL;
	wndclass.hbrBackground = (HBRUSH) (COLOR_BACKGROUND);
	wndclass.lpszClassName = "sh_gui";
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClass(&wndclass);

	i32 style = WS_OVERLAPPEDWINDOW;
	RECT win_size = {
		.left = 20,
		.right = gl_ctx.width,
		.top = 20,
		.bottom = gl_ctx.height };

	AdjustWindowRect(&win_size, style, FALSE);

	HWND wn = CreateWindow(
			"sh_gui",
			gl_ctx.window_name,
			WS_VISIBLE | style,
			win_size.left,
			win_size.top,
			win_size.right - win_size.left,
			win_size.bottom - win_size.top,
			NULL,
			NULL,
			NULL,
			NULL
			);


	setup_opengl(wn);


	init_time();
	// init_raw_input();
	return wn;

}



void update_time() {
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);

	uint64 cur_tick = li.QuadPart - gl_time.start_tick;

	gl_time.delta_tick = cur_tick - gl_time.tick;
	gl_time.tick = cur_tick;


	gl_time.time_nano = (1000*1000*1000*gl_time.tick)/(gl_time.tick_per_sec);
	gl_time.time_micro = (1000*1000*gl_time.tick)/(gl_time.tick_per_sec);
	gl_time.time_milli = (1000*gl_time.tick)/(gl_time.tick_per_sec);
	gl_time.time_sec = (float)gl_time.tick/(float)gl_time.tick_per_sec;
	
	gl_time.delta_time_nano = (1000*1000*1000*gl_time.delta_tick)/(gl_time.tick_per_sec);
	gl_time.delta_time_micro = (1000*1000*gl_time.delta_tick)/(gl_time.tick_per_sec);
	gl_time.delta_time_milli = (1000*gl_time.delta_tick)/(gl_time.tick_per_sec);
	gl_time.delta_time_sec = (float)gl_time.delta_tick/(float)gl_time.tick_per_sec;
}





void update_keys() {
	static u8 keys[256]; 
	GetKeyboardState(keys);

	for(i32 i = 0; i < 256; i++) {
		u8 new_state = keys[i] >> 7; 
		gl_ctx.keyboard[i].pressed_once = !gl_ctx.keyboard[i].pressed && new_state;
		gl_ctx.keyboard[i].pressed = new_state;
	}
}

void update_mouse() {


	double xpos, ypos;
	glfwGetCursorPos(gl_ctx.window, &xpos, &ypos);

	int state = glfwGetMouseButton(gl_ctx.window, GLFW_MOUSE_BUTTON_LEFT);

	mouse.left.pressed_once = !mouse.left.pressed && state;
	mouse.left.pressed = state;

	mouse.cur_x = xpos;
	mouse.cur_y = ypos;

	mouse.delta_x = xpos - mouse.prev_x;
	mouse.delta_y = ypos - mouse.prev_y;

	mouse.prev_x = mouse.cur_x;
	mouse.prev_y = mouse.cur_y;
}

void handle_events() {


	// MSG msg;
	// while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
	// 	TranslateMessage(&msg);
	// 	DispatchMessage(&msg);
	// }


	update_mouse();

	update_keys();

	if(gl_ctx.keyboard[VK_ESCAPE].pressed) {
		gl_ctx.should_close = 1;
	}

#define start 1
	static float left = -10;
	static float right = 10; 
	static float top =   10;
	static float bottom = -10;
	static float near_plane = -10;
	static float far_plane =  10;
#undef start

	float speed = 50;

	if(gl_ctx.keyboard[VK_SPACE].pressed_once) {
		gl_ctx.move_camera = !gl_ctx.move_camera;
		printf("Camera toggled %d\n", gl_ctx.move_camera);
		fflush(stdout);
	}

	if(gl_ctx.keyboard['W'].pressed) {
		sh_vec3 f = gl_ctx.look_at;
		sh_vec3_mul_scaler(&f, 20*gl_time.delta_time_sec);
		sh_vec3_add_vec3(&gl_ctx.eye_pos, &f);
	}

	if(gl_ctx.keyboard['S'].pressed) {
		sh_vec3 f = gl_ctx.look_at;
		sh_vec3_mul_scaler(&f, 20*gl_time.delta_time_sec);
		sh_vec3_sub_vec3(&gl_ctx.eye_pos, &f);
	}

	if(gl_ctx.keyboard['A'].pressed) {
		sh_vec3 f = sh_vec3_cross(&gl_ctx.look_at, &gl_ctx.up );
		sh_vec3_mul_scaler(&f, 20*gl_time.delta_time_sec);
		sh_vec3_sub_vec3(&gl_ctx.eye_pos, &f);
	}

	if(gl_ctx.keyboard['D'].pressed) {

		sh_vec3 f = sh_vec3_cross(&gl_ctx.up, &gl_ctx.look_at);
		sh_vec3_mul_scaler(&f, 20*gl_time.delta_time_sec);
		sh_vec3_sub_vec3(&gl_ctx.eye_pos, &f);
	}


	if(mouse.left.pressed && (mouse.delta_x != 0 || mouse.delta_y != 0)) {
		yaw +=   0.1f*mouse.delta_x;
		pitch -= 0.1f*mouse.delta_y;

		float sin_pitch = sinf(TO_RADIANS(pitch));
		float cos_pitch = cosf(TO_RADIANS(pitch));
		float cos_yaw = cosf(TO_RADIANS(yaw));
		float sin_yaw = sinf(TO_RADIANS(yaw));

		gl_ctx.look_at.x = cos_yaw*cos_pitch;
		gl_ctx.look_at.y = sin_pitch;
		gl_ctx.look_at.z = sin_yaw*cos_pitch;

		sh_vec3_normalize_ref(&gl_ctx.look_at);

	}






#if 0
	if(gl_ctx.keyboard[VK_LEFT].pressed) {

		if(gl_ctx.keyboard[VK_SHIFT].pressed) {

			if(gl_ctx.move_camera) {

				if(gl_ctx.keyboard[VK_CONTROL].pressed) {
					gl_ctx.eye_pos.x -= speed*gl_time.delta_time_sec;
				} else {
					gl_ctx.eye_pos.x += speed*gl_time.delta_time_sec;
				}


			} else {

				if(gl_ctx.keyboard[VK_CONTROL].pressed) {
					left -= gl_time.delta_time_sec;
				} else {
					left += gl_time.delta_time_sec;
				}


			}

		}

	}

	if(gl_ctx.keyboard[VK_RIGHT].pressed) {
		if(gl_ctx.keyboard[VK_SHIFT].pressed) {

			if(gl_ctx.move_camera) {

				if(gl_ctx.keyboard[VK_CONTROL].pressed) {
					gl_ctx.eye_pos.y -= speed*gl_time.delta_time_sec;
				} else {
					gl_ctx.eye_pos.y += speed*gl_time.delta_time_sec;
				}

			} else {

				if(gl_ctx.keyboard[VK_CONTROL].pressed) {
					right -= gl_time.delta_time_sec;
				} else {
					right += gl_time.delta_time_sec;
				}
			}
		}
	}


	if(gl_ctx.keyboard[VK_UP].pressed) {

		if(gl_ctx.keyboard[VK_SHIFT].pressed) {

			if(gl_ctx.move_camera) {

				if(gl_ctx.keyboard[VK_CONTROL].pressed) {
					gl_ctx.eye_pos.z -= speed*gl_time.delta_time_sec;
				} else {
					gl_ctx.eye_pos.z += speed*gl_time.delta_time_sec;
				}

			} else {

				if(gl_ctx.keyboard[VK_CONTROL].pressed) {
					top -= gl_time.delta_time_sec;
				} else {
					top += gl_time.delta_time_sec;
				}
			}

		} else {


			if(gl_ctx.keyboard[VK_CONTROL].pressed) {
				if(gl_ctx.keyboard[VK_MENU].pressed) {
					near_plane  -= speed*gl_time.delta_time_sec; 
				} else {
					near_plane  += speed*gl_time.delta_time_sec; 
				}
			}

		}
		
	}

	if(gl_ctx.keyboard[VK_DOWN].pressed) {
		if(gl_ctx.keyboard[VK_SHIFT].pressed) {
			if(gl_ctx.keyboard[VK_CONTROL].pressed) {
				bottom -= gl_time.delta_time_sec;
			} else {
				bottom += gl_time.delta_time_sec;
			}
		} else {
			if(gl_ctx.keyboard[VK_CONTROL].pressed) {
				if(gl_ctx.keyboard[VK_MENU].pressed) {
					far_plane  -= speed*gl_time.delta_time_sec; 
				} else {
					far_plane  += speed*gl_time.delta_time_sec; 
				}
			}

		}
	}


#endif

	if(gl_ctx.keyboard['P'].pressed) {
		printf("%f %f %f %f %f %f %f\n", left, right, bottom, top, near_plane, far_plane, gl_ctx.top_add);
		fflush(stdout);
	}

	gl_ctx.camera = lookat(gl_ctx.eye_pos, sh_vec3_new_add_vec3(&gl_ctx.eye_pos, &gl_ctx.look_at), gl_ctx.up);
	gl_ctx.inv_camera = inverse(&gl_ctx.camera);
	// gl_ctx.camera = lookat(gl_ctx.eye_pos, gl_ctx.look_at, gl_ctx.up);

	glUniformMatrix4fv(CAMERA_UNIFORM_LOC, 1, GL_TRUE, gl_ctx.camera.m);

	if(gl_ctx.keyboard[VK_UP].pressed) {
		gl_ctx.frame_number++;
	}

	if(gl_ctx.keyboard[VK_DOWN].pressed) {
		gl_ctx.frame_number--;
	}


}

typedef struct line {
	float x1;
	float y1;
	float x2;
	float y2;
} line;


int comp_sh_point(const void *a, const void *b) {
	line *edge_a = (line*)a;
	line *edge_b = (line*)b;

	float a_y = MIN(edge_a->y1, edge_a->y2);
	float b_y = MIN(edge_b->y1, edge_b->y2);

	if(a_y > b_y) {
		return 1;
	} else if(a_y < b_y) {
		return -1;
	}

	return 0;
}

typedef struct intersection {
	struct intersection *next;
	line *edge;
	float intersect;
	int slope_direction;
	float slope;
} intersection; 


void bubble_sort_intersection(intersection **head) {
	intersection *actual_head = *head;
	intersection **to = head;
	int sorted = 1;

	while(true) {
		to = head;
		sorted = 1;
		while(*to != NULL && (*to)->next != NULL) {
			if((*to)->intersect > (*to)->next->intersect) {
				intersection *temp = (*to)->next;
				(*to)->next = temp->next;
				temp->next = *to;
				*to = temp;
				sorted = 0;
			}
			to = &((*to)->next);
		}
		if(sorted) break;
	}
}

int counter_add_to_link = 0;
void add_to_linked_list(intersection **head, intersection *new_intersection) {
	intersection *new_node = (intersection*) calloc(1, sizeof(intersection));
	*new_node = *new_intersection;
	if(*head == NULL) {
		*head = new_node;
	} else {
		intersection *m = *head;
		while(m->next) { m = m->next; }
		m->next = new_node;
	}
}


line* generate_edges(pos2 *pts, int contour_counts, int *contour_end_index, int *edges_num) {

	if(pts == NULL) return NULL;
	line *edges = NULL;
	int size_aprox = contour_end_index[contour_counts-1];
	edges = (line*) calloc(sizeof(line)*size_aprox, 1);
	int j = 0;
	int counter = 0;
	for(int i = 0; i < contour_counts; i++) {
		for(; j < contour_end_index[i]-1; j++) {
			if(pts[j].y == pts[j+1].y) continue;
			edges[counter].x1 = pts[j].x;
			edges[counter].x2 = pts[j+1].x;
			edges[counter].y1 = pts[j].y;
			edges[counter].y2 = pts[j+1].y;
			counter++;
		}
		j++;
	}
	*edges_num = counter;

	return edges;
}

void tesselate_bezier_quadratic(pos2 **pts, pos2 p0, pos2 p1, pos2 p2, int steps) {

	float step_val = 1.0f/steps;
	for(int i = 1; i <= steps; ++i) {
		float t = step_val*i;

		float x = p0.x*(1.0f-t)*(1.0f-t) + p1.x*2.0f*(1.0f-t)*t + p2.x*t*t;
		float y = p0.y*(1.0f-t)*(1.0f-t) + p1.y*2.0f*(1.0f-t)*t + p2.y*t*t;
		pos2 new_point = {x, y};
		buf_push(*pts, new_point);
	}
}

pos2* generate_points(sh_glyph_outline *glyph, float scale, int tesselate_amount, int *pts_len, int *num_contour, int **contour_end_index) {

	pos2 *pts = NULL;//malloc(sizeof(pos2)*256);
	int contour_start = 1;
	int first_point_off = 0;

	int x_offset = glyph->p1.x;
	int y_offset = glyph->p1.y;

	*num_contour = glyph->contour_count;

	int i =0;
	pos2 pflt;
	f32 m = 0;
	for(int j = 0; j < *num_contour; ++j) {
		int i = j != 0 ? glyph->contour_last_index[j-1]+1 : 0;
		int x = i;
		contour_start = 1;

		for(; i < glyph->contour_last_index[j]; ++i) {
			sh_font_point *p = glyph->points + i;;
			pflt.x = (p->x - x_offset)*scale;
			m = (p->x - x_offset)*scale;
			pflt.y = (p->y - y_offset)*scale;
			if(p->flag.on_curve) {
				buf_push(pts, pflt);
			} else {
				pos2 mid_p = {0};
				if(!p[1].flag.on_curve) {
					mid_p.x = p->x + (f32)( (p[1].x - p->x)/2 );
					mid_p.y = p->y + (f32)( (p[1].y - p->y)/2 );
				} else {
					mid_p.x = p[1].x;
					mid_p.y = p[1].y;
				}

				mid_p.x -= x_offset;
				mid_p.x *= scale;

				mid_p.y -= y_offset;
				mid_p.y *= scale;

				if(contour_start) {
					first_point_off = 1;
					//we are starting on an off curve
					if(p[1].flag.on_curve) continue; //next point on curve then just move to the next
					buf_push(pts, mid_p); continue; //point after is off curve too so just make a mid out of the two
				}

				int len = buf_len(pts)-1;
				tesselate_bezier_quadratic(&pts, pts[len], pflt, mid_p, tesselate_amount);
			}

			contour_start = 0;
		}

		sh_font_point *p = glyph->points + i;
		pflt = (pos2){(p->x-x_offset)*scale, (p->y - y_offset)*scale};
		if(first_point_off) {
			if(p->flag.on_curve)  {
				buf_push(pts, pflt);
			} else {
				pos2 mid_p = (pos2) {
					(p->x + ((glyph->points[x].x - p->x)/2) - x_offset)*scale,
					(p->y + ((glyph->points[x].y - p->y)/2) - y_offset)*scale
				};
				tesselate_bezier_quadratic(&pts, pts[buf_len(pts)-1], pflt , mid_p, tesselate_amount);
			}
		} else {
			buf_push(pts, pflt);
		}

		int last_point_contour = j ? (*contour_end_index)[j-1] : 0;
		buf_push(pts, pts[last_point_contour]); //add the first point again so we have an actual loop
		buf_push(*contour_end_index, buf_len(pts)); //mark the end of the contour
		first_point_off = 0;

	}

	return pts;
}

//@mem(temp_texture.bitmap.mem, UINT8, 1, temp_texture.bitmap.x, temp_texture.bitmap.y, temp_texture.bitmap.x)
//@mem(mem, UINT8, 1, w, h, w)

void rasterize_glyph(line *edges, int num_edges, unsigned char *mem, int m_size, int w, int h) {

	if(!edges) return;
	qsort(edges, num_edges, sizeof(line), comp_sh_point);
	// memset(mem, 0, m_size);
	intersection *edge_intersection = NULL;

	int div_scanline = 15;
	float alpha = 255.0f/div_scanline;
	float step_per_line = 1.0f/div_scanline;
	int from_edge = 0;

	for(int i = 0; i < h; ++i) {
		int scanline = i;

		for(int n = 0; n < div_scanline; ++n) {

			float scanline_offset = scanline + n*step_per_line;
			intersection **update_head = &edge_intersection;

			while(update_head && *update_head) {
				line *edge = (*update_head)->edge;
				float bigger_y = MAX(edge->y1, edge->y2);

				if(bigger_y < scanline_offset) {
					intersection *free_node = *update_head;
					(*update_head) = (*update_head)->next;
					free(free_node);
					continue;
				}

				(*update_head)->intersect += (*update_head)->slope;
				update_head = &(*update_head)->next;
			}

			for(int j = from_edge; j < num_edges; ++j) {
				line *edge = edges + j;

				float bigger_y = MAX(edge->y1, edge->y2);
				float smaller_y = MIN(edge->y1, edge->y2);

				if(smaller_y >= scanline_offset) {
					from_edge = j;
					break;
				}

				if(bigger_y < scanline_offset) continue;

				float dy = edge->y2 - edge->y1;
				float dx = edge->x2 - edge->x1;
				float slope = dx/dy;
				float x_intersect = -1;
				if(dx == 0) {
					x_intersect = edge->x1;
				} else {
					x_intersect = ((scanline_offset - edge->y1)*slope + edge->x1);
					if(x_intersect < 0) {
						x_intersect = dx > 0 ? edge->x1 : edge->x2;
					} else if(x_intersect > w) {
						x_intersect = dx > 0 ? edge->x2 : edge->x1;
					}
				}

				intersection intrsct = {
					.slope = slope*step_per_line,
					.edge = edge,
					.slope_direction = dy > 0 ? 1 : -1,
					.intersect = x_intersect,
				};

				add_to_linked_list(&edge_intersection, &intrsct);
				from_edge++;
			}

			bubble_sort_intersection(&edge_intersection);
			unsigned char *current_scanline = mem + scanline*w;

			if(edge_intersection) {
				int first_pixel_index = (int)edge_intersection->intersect;
				float f_pixel = edge_intersection->intersect;
				float l_pixel = 0;

				int last_pixel_index = -1;
				int count_edge_direction = edge_intersection->slope_direction;
				intersection *edge_check = edge_intersection->next;

				while(edge_check) {
					if(count_edge_direction == 0) {
						first_pixel_index = (int)edge_check->intersect;
						f_pixel = edge_check->intersect;
						count_edge_direction += edge_check->slope_direction;
						edge_check = edge_check->next;
						continue;
					}

					count_edge_direction += edge_check->slope_direction;

					if(count_edge_direction == 0) {
						last_pixel_index = (int)edge_check->intersect;
						l_pixel = edge_check->intersect;

						if(first_pixel_index == last_pixel_index) {
							float cover = l_pixel - f_pixel;
							if(cover == 0) continue;
							current_scanline[first_pixel_index] += (u8)(alpha*cover );
						} else {
							float cover = 1 + (first_pixel_index) - f_pixel;
							current_scanline[first_pixel_index] += (u8)( alpha*cover );
							cover =  l_pixel - last_pixel_index ;
							current_scanline[last_pixel_index] += (u8)( alpha*cover );

							for(int j = first_pixel_index + 1; j < last_pixel_index; ++j) {
								current_scanline[j] += (u8)alpha;
							}
						}
					}

					edge_check = edge_check->next;
				}

			}

		}
	}

}


void render_letter(unsigned char *mem, int m_size, int m_w, int m_h, font_directory *font, char letter, int font_size_pixel) {

	sh_glyph_outline g = get_glyph_outline(font, letter);

	// int h = g.p2.y - g.p1.y;
	int as_ds = font->hhea.ascent - font->hhea.descent;
	float ft_scale = (float) font_size_pixel/(as_ds);

	int num_contours = 0;
	int *contour_ends_index = NULL;
	int num_edges = 0;
	int pts_len = 0;

	pos2 *pts = generate_points(&g, ft_scale, 5, &pts_len, &num_contours, &contour_ends_index); //the amount of tessilation affects the speed a ton
	line *edges = generate_edges(pts, num_contours, contour_ends_index, &num_edges);
	rasterize_glyph(edges, num_edges, mem, m_size, m_w, m_h);

	buf_free(contour_ends_index);
	free(edges);
	buf_free(pts);
	sh_free_glyph_outline(&g);

}



void setup_font_atlas() {

	int as_ds = font->hhea.ascent - font->hhea.descent;
	float ft_scale = (float) gl_ctx.font_size/(as_ds);

	i32 max_char_width  =  (i32)( ( font->head.xMax - font->head.xMin ) * ft_scale );
	i32 max_char_height =  (i32)( (as_ds) * ft_scale );


	i32 char_count = 94;
	char start_char = ' ';

	i32 pixel_gap = 2;
	i32 char_per_row = ((i32)sqrt(char_count)) + 1;
	i32 atlas_width  = ( max_char_width + pixel_gap )*char_per_row;

	i32 rows = char_count/char_per_row;

	i32 atlas_height = 	( max_char_height + pixel_gap )*rows;

	gl_ctx.font_atlas = (u8*)calloc((atlas_height)*(atlas_width), sizeof(u8));

	gl_ctx.atlas_height = atlas_height;
	gl_ctx.atlas_width = atlas_width;

	u8 *mem_loc = gl_ctx.font_atlas;

	i32 x = 0;
	i32 y = 0;

	i32 max_row_height = 0;


	for(i32 i = start_char; i < start_char + char_count; i++ ) {

		// render_letter(int m_size, int m_w, int m_h, font_directory *font, char letter, int font_size_pixel) {
		render_letter(
				mem_loc + x + y*atlas_width,
				atlas_width*atlas_height,
				atlas_width,
				gl_ctx.font_size,
				font,
				(char)i,
				gl_ctx.font_size
				);

		sh_glyph_outline g = get_glyph_outline(font, i);
		sh_glyph_metric metric = sh_get_glyph_metric(font, &g, ft_scale);
		i32 w = (i32)( ( g.p2.x - g.p1.x )*ft_scale );
		i32 h = (i32)( ( g.p2.y - g.p1.y )*ft_scale );

		
		gl_ctx.char_cache[i - start_char].uv.x = (f32)x;
		gl_ctx.char_cache[i - start_char].uv.y = (f32)y;
		gl_ctx.char_cache[i - start_char].uv.z = (f32)( w+pixel_gap );
		gl_ctx.char_cache[i - start_char].uv.w = (f32)( h+pixel_gap );
		gl_ctx.char_cache[i - start_char].advance_width = metric.x_advance;

		gl_ctx.char_cache[i - start_char].x_offset = (i32)( g.p1.x*ft_scale );
		gl_ctx.char_cache[i - start_char].y_offset = (i32)( g.p1.y*ft_scale );

		if(h > max_row_height) max_row_height = h;

		x += (i32)(w) + pixel_gap;
		if((x +(i32)(w)+pixel_gap) > atlas_width ) {
			y += max_row_height + pixel_gap;
			max_row_height = 0;
			x = 0;
		}

		
	}

	glGenTextures(1, &gl_ctx.font_atlas_texture);
	glBindTexture(GL_TEXTURE_2D, gl_ctx.font_atlas_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RED, atlas_width, atlas_height, 0,
			GL_RED, GL_UNSIGNED_BYTE, gl_ctx.font_atlas);




	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

}

void setup_font() {
	size_t mem_size = 0;
	char* ttf_file = read_file("envy.ttf", &mem_size);
	char* file_loc = ttf_file;
	font = sh_init_font((u8*)ttf_file);
}


void draw_letter(char c) {


	pos4 position = {0, 0, 0, 1};
	pos4 char_uv = gl_ctx.char_cache[(c - ' ')].uv;
	pos2 size = {char_uv.z, char_uv.w };
	pos2 size_uv = {char_uv.z, char_uv.w};

	f32 w = (f32)gl_ctx.atlas_width;
	f32 h = (f32)gl_ctx.atlas_height;

	u32 letter_vbo = 0;;

	pos4 letter_data[] = {
		{position.x, position.y, 0, 1},
		{position.x, position.y + size.y, 0, 1},
		{position.x + size.x, position.y + size.y, 0, 1},

		{position.x, position.y, 0, 1},
		{position.x + size.x, position.y + size.y, 0, 1},
		{position.x + size.x, position.y, 0, 1},


		{char_uv.x/w, char_uv.y/h, 0, 1},
		{( char_uv.x )/w, ( char_uv.y + size_uv.y )/h, 0,1 },
		{( char_uv.x + size_uv.x )/w, ( char_uv.y + size_uv.y )/h, 0, 1},

		{char_uv.x/w, char_uv.y/h, 0, 1},
		{( char_uv.x + size_uv.x )/w, ( char_uv.y + size_uv.y )/h, 0, 1},
		{( char_uv.x + size_uv.x )/w, ( char_uv.y )/h, 0, 1},
	};


	glGenBuffers(1, &letter_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, letter_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos4)*(6 + 6), letter_data, GL_STATIC_DRAW);

	glEnableVertexAttribArray(UV_LOC);
	glVertexAttribPointer(VP_LOC, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(UV_LOC, 4, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(pos4)*6));
	glUniform1i(HAS_TEXTURE_LOC, 1);
	glUniform1i(USE_UNIFORM_COLOR_LOC, 1);
	pos4 black = {1, 1, 1, 1};
	glUniform4fv(UNIFORM_COLOR_LOC, 1, &black.x);

	mat4 m = {0};

	for(i32 i = 0; i < 4; i++) {
		m.m[i*4 + i] = 1;
	}

	glUniformMatrix4fv(PROJECTION_UNIFORM_LOC, 1, GL_FALSE, gl_ctx.text_ortho.m);
	glUniformMatrix4fv(MODEL_UNIFORM_LOC, 1, GL_FALSE, m.m);
	glUniformMatrix4fv(CAMERA_UNIFORM_LOC, 1, GL_FALSE, m.m);

	glBindTexture(GL_TEXTURE_2D, gl_ctx.font_atlas_texture);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &letter_vbo);


}

typedef struct split_lines {
	char **lines;
	i32 *line_length;
	i32 line_nums;
} split_lines;

split_lines split_text(char *text, i32 str_length) {


	split_lines lines = {0};


	if(str_length <= 0) return lines;

	buf_push(lines.lines, text);
	buf_push(lines.line_length, 0);
	lines.line_nums++;


	for(i32 i = 0; i < str_length; i++, text++) {

		if(text[0] == '\n' || text[0] == '\r') {
			char *prev_line = lines.lines[lines.line_nums-1];
			lines.line_length[lines.line_nums-1] = (i32)( text - prev_line );;

			buf_push(lines.lines, text + 1);
			buf_push(lines.line_length, 0);

			lines.line_nums++;
		}
	}

	char *prev_line = lines.lines[lines.line_nums-1];
	lines.line_length[lines.line_nums-1] = (i32)( text - prev_line );

	return lines;
}

typedef struct render_context {
	f32 next_x;
	f32 next_y;
	f32 row_height; // pixels
	f32 left_margin;
	f32 top_margin;
} render_context;

typedef struct cursor {
	i32 row_number;
	i32 column_number;
} cursor;

typedef struct text_rect {
	f32 x;
	f32 y;
	f32 width;
	f32 height;
	f32 u_s;
	f32 v_s;
	f32 u_e;
	f32 v_e;
} text_rect;

render_context ren_ctx = {0, 0, 64, 5, 3};
cursor curs = {0, 0};
text_rect *line_quads = NULL;

i32 quad_vbo = 0;
i32 index_vbo = 0;
i32 *indices = NULL;
i32 index_count = 0;
pos4 *quad_vertices = NULL;
i32 vert_count = 0;

void setup_line_quads(split_lines lines) {
	i32 index_start = 0;

	for(i32 i = 0; i < buf_len(line_quads); i++) {

		text_rect *r = line_quads + i;

		buf_fit(quad_vertices, 8);

		buf_push(quad_vertices, (pos4){r->x, r->y - r->height, 0, 1});
		buf_push(quad_vertices, (pos4){r->x, r->y, 0, 1});
		buf_push(quad_vertices, (pos4){r->x + r->width, r->y, 0, 1});
		buf_push(quad_vertices, (pos4){r->x + r->width, r->y - r->height, 0, 1});

		buf_push(quad_vertices, (pos4){r->u_s, r->v_s, 0, 1});
		buf_push(quad_vertices, (pos4){r->u_s, r->v_e, 0, 1});
		buf_push(quad_vertices, (pos4){r->u_e, r->v_e, 0, 1});
		buf_push(quad_vertices, (pos4){r->u_e, r->v_s, 0, 1});


		buf_push(indices, index_start + 0);
		buf_push(indices, index_start + 1);
		buf_push(indices, index_start + 2);

		buf_push(indices, index_start + 0);
		buf_push(indices, index_start + 2);
		buf_push(indices, index_start + 3);

		vert_count += 8;
		index_start += 8;
		index_count += 6;

	}
}

void setup_quad_vbo(void) {
	// this is dumb

	if(quad_vbo == 0) {
		glGenBuffers(1, &quad_vbo);
	}

	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos4)*vert_count, quad_vertices, GL_DYNAMIC_DRAW);

	if(index_vbo == 0) {
		glGenBuffers(1, &index_vbo);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(i32)*(vert_count/8)*6, indices, GL_DYNAMIC_DRAW);
}


void render_text(char *text) {

	if(text == NULL) return;

	i32 y = (i32)strlen(text);
	split_lines l = split_text(text, y);

	int as_ds = font->hhea.ascent - font->hhea.descent;
	float ft_scale = (float) gl_ctx.font_size/(as_ds);

	f32 w = (f32)gl_ctx.atlas_width;
	f32 h = (f32)gl_ctx.atlas_height;

	f32 top = ren_ctx.next_y - ren_ctx.top_margin;

	f32 baseline_y = top - font->hhea.ascent*ft_scale;

	for(i32 i = 0; i < l.line_nums; i++) {


		for(i32 j = 0; j < l.line_length[i]; j++) {

			character_info *c = gl_ctx.char_cache + (l.lines[i][j] - ' ');
			pos4 char_uv = c->uv;
			pos2 size = {char_uv.z, char_uv.w};
			pos2 size_uv = {char_uv.z, char_uv.w};

			buf_push(line_quads,
					(text_rect){
					(f32)ren_ctx.next_x + ren_ctx.left_margin,
					(f32)baseline_y + size.y + c->y_offset,
					size.x,
					size.y,
					char_uv.x/w,
					char_uv.y/h,
					( char_uv.x + size_uv.x )/w,
					( char_uv.y + size_uv.y )/h
					});

			ren_ctx.next_x += c->advance_width;
		}


		ren_ctx.next_x = 0;
		baseline_y -= gl_ctx.font_size;
	}

	setup_line_quads(l);
}

void setup_for_ortho_draw() {
	mat4 m = {0};

	for(i32 i = 0; i < 4; i++) {
		m.m[i*4 + i] = 1;
	}

	glUniformMatrix4fv(PROJECTION_UNIFORM_LOC, 1, GL_FALSE, gl_ctx.text_ortho.m);
	glUniformMatrix4fv(CAMERA_UNIFORM_LOC, 1, GL_FALSE, m.m);
	glUniformMatrix4fv(MODEL_UNIFORM_LOC, 1, GL_FALSE, m.m);

}

void draw_full_text(char *text) {
	glBindVertexArray(gl_ctx.vao);

	render_text(text);
	setup_quad_vbo();

	setup_for_ortho_draw();
	
	glUniform1i(HAS_TEXTURE_LOC, 1);
	glUniform1i(USE_UNIFORM_COLOR_LOC, 1);

	glEnableVertexAttribArray(UV_LOC);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(VP_LOC, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(UV_LOC, 4, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(pos4)*4));

	glBindTexture(GL_TEXTURE_2D, gl_ctx.font_atlas_texture);

	pos4 white = {1, 1, 1, 1};
	glUniform4fv(UNIFORM_COLOR_LOC, 1, &white.x);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo);
	glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT , 0);

	buf_clear(indices);
	buf_clear(quad_vertices);
	buf_clear(line_quads);
	vert_count = 0;
	index_count = 0;

}

