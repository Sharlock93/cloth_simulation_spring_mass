typedef struct token_t {
	char *start;
	int size;
} token_t;

u8 *current_position = NULL;
u8 *gl_file = NULL;
token_t *tokens = NULL;
token_t *current_token = NULL;

void eat_whitespace() {
	while(isspace(current_position[0])) current_position++;
}

token_t next_token() {

	eat_whitespace();

	token_t tok = {.start = (char*)current_position, .size = 0 };


	switch(current_position[0]) {

		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H':
		case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
		case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
		case 'Y': case 'Z': case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
		case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v':
		case 'w': case 'x': case 'y': case 'z': {

			tok.start = (char*)current_position;
			while(isalnum(current_position[0]) || current_position[0] == '_') current_position++;

			tok.size = (int)((char*)current_position - tok.start);

		} break;

		case '#': {
			current_position++;
			while(current_position[0] != '\n') {
				current_position++;
			}

			tok.size = (int)((char*)current_position - tok.start);

		} break;

		case '{': case '}': case ':': case '.': case '/': {
			tok.size = 1;
			current_position++;
		} break;

		case '-':
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9': {

			if(current_position[0] == '-') current_position++;

			while(isdigit(current_position[0]) || current_position[0] == '.') current_position++;
			tok.size = (int)((char*)current_position - tok.start);
		} break;

		case '\0': {
			tok.size = -1;
		} break;

	}

	return tok;
}


void print_token(token_t *tok) {
	printf("%.*s\n", tok->size, tok->start);
}


void parse_file(char *file_name) {

	gl_file = (u8*)read_file(file_name, NULL);
	current_position = gl_file;

	token_t tok = next_token();

	buf_push(tokens, tok);

	while(tok.size != -1) {
		tok = next_token();
		buf_push(tokens, tok);
	}

	current_token = tokens;
}

