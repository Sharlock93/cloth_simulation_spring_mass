typedef struct obj_file_t {
	pos4 *verts;
	sh_vec2 *tex_coord;
	sh_vec3 *normals;
	i32 **faces;
	i32 *face_tex;
	i32 *face_norm;
} obj_file_t;


void assert_token(token_t *t, char *token_name) {

	if(strncmp(t->start, token_name, t->size) != 0) {
		puts("token mismatch.");
		exit(1);
	}

}

int expect_token(char *tok_name) {

	if(strncmp(current_token->start, tok_name, current_token->size) != 0) {
		return 0;
	}

	return 1;
}

token_t* match_token(char *tok_name) {

	if(strncmp(current_token->start, tok_name, current_token->size) != 0) {
		puts("token mismatch.");
		exit(1);
	}

	token_t *c = current_token;

	current_token++;

	return c;
}

float parse_float() {
	return strtof(current_token->start, NULL);
}


obj_file_t parse_obj_file(char *file_name) {
	buf_clear(tokens);

	parse_file(file_name);

	obj_file_t obj = {0};


	while(current_token->size != -1) {

		switch(current_token->start[0]) {
			case '#': current_token++; break;
			case 'o': {
				// file name
				current_token += 2;
			} break;

			case 'v': {

				if(current_token->start[1] == 'n') {

					current_token++;
					f32 f1 = parse_float();current_token++;
					f32 f2 = parse_float();current_token++;
					f32 f3 = parse_float();current_token++;
					buf_push(obj.normals, (sh_vec3){f1, f2, f3});

				} else if(current_token->start[1] == 't') {

					current_token++;
					f32 f1 = parse_float();current_token++;
					f32 f2 = parse_float();current_token++;
					buf_push(obj.tex_coord, (sh_vec2){f1, f2});

				} else {

					current_token++;
					f32 f1 = parse_float();current_token++;
					f32 f2 = parse_float();current_token++;
					f32 f3 = parse_float();current_token++;
					buf_push(obj.verts, (pos4){f1, f2, f3, 1});
				}
			} break;

			case 'f': {
				current_token++;

				i32 *f = NULL;

				while(
					current_token->start[0] != 'f' &&
					current_token->size != -1
				) {

					i32 v1 = (i32)parse_float();current_token++;
					buf_push(f, v1);

					if(current_token->start[0] == '/') {
						current_token++;
						if(isdigit(current_token->start[0])) {
							buf_push(obj.face_tex, (i32)parse_float());
							current_token++;
						}
					}

					if(current_token->start[0] == '/') {
						current_token++;
						if(isdigit(current_token->start[0])) {
							buf_push(obj.face_norm, (i32)parse_float());
							current_token++;
						}
					}

				}

				buf_push(obj.faces, f);

			} break;

			case 's': {
				current_token += 2;
			} break;

			default: {
				current_token++;
			}

		}

	}

	return obj;
}
