#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>

typedef enum{
	ROOT,
	EMPTY,
	HEADING,
	PARAGRAPH,
	TEXT_NORMAL,
	TEXT_BOLD,
	TEXT_ITALIC,
	LINE_BREAK,
} Type;

typedef enum{
	ASTRISK,
	HASHTAG,
	NEW_LINE,
	UNDERSCORE,
	TEXT,
} TokenTypes;

typedef struct {
	TokenTypes type;
	char content[100];
} Token ;

typedef struct Node{
	Type type;
	struct Node *first_child;
	struct Node *next;
	char content[100];
} Node;

Node *new_node(){
	Node *node = malloc(sizeof(Node));
	node->type = EMPTY;
	node->first_child = NULL;
	node->next = NULL;
	return node;
}

void print_ast_tree(Node *root, int indent) {
	if (root == NULL){
		return;
	}

	if(indent > 0){
		printf("|");
	}
	for(int i=0; i<indent; i++){
		printf("--");
	}

	switch (root->type) {
		case ROOT:
			printf("ROOT\n");
			return print_ast_tree(root->first_child, indent + 1);
		case EMPTY:
			printf("EMPTY\n");
			return print_ast_tree(root->next, indent);
		case HEADING:
			printf("HEADING LEVEL %s\n", root->content);	
			print_ast_tree(root->first_child, indent + 1);
			return print_ast_tree(root->next, indent);
		case PARAGRAPH:
			printf("PARAGRAPH\n");
			print_ast_tree(root->first_child, indent + 1);
			return print_ast_tree(root->next, indent);
		case TEXT_NORMAL:
			printf("TEXT:%s\n",root->content);
			return print_ast_tree(root->next, indent);
		case TEXT_BOLD:
			printf("TEXT-BOLD:%s\n",root->content);
			return print_ast_tree(root->next, indent);
		case TEXT_ITALIC:
			printf("TEXT-ITALIC:%s\n",root->content);
			return print_ast_tree(root->next, indent);
		case LINE_BREAK:
			printf("LINE_BREAK\n");
			return print_ast_tree(root->next, indent);
	}
}


int get_token_amount(const char *str){
	int num_of_tokens = 0;
	bool stringExists = false;
	for (int i=0; i<strlen(str); i++){
		if (str[i] == '*' || str[i] == '\n' || str[i] == '#' || str[i] == '_'){
			num_of_tokens++;
			if (stringExists) {
				num_of_tokens++;
				stringExists = false;
			}
		} else {
			stringExists = true;
		}
	}
	return num_of_tokens;
}

Token *tokenize(char *str, size_t *num_tokens){
	*num_tokens = get_token_amount(str);

	Token *tokens = malloc(sizeof(Token) * *num_tokens);
	int tokens_idx = 0;

	char buffer[100];
	int buffer_idx = 0;

	for(int i=0; i<strlen(str); i++){
		switch (str[i]) {
			case '*':
				if(buffer_idx != 0){
					tokens[tokens_idx].type = TEXT;
					strcpy(tokens[tokens_idx].content, buffer);
					buffer[0] = '\0';
					buffer_idx = 0;
					tokens_idx++;
				}
				tokens[tokens_idx].type = ASTRISK;		
				tokens_idx++;
				break;
			case '\n':
				if(buffer_idx != 0){
					tokens[tokens_idx].type = TEXT;
					strcpy(tokens[tokens_idx].content, buffer);
					buffer[0] = '\0';
					buffer_idx = 0;
					tokens_idx++;
				}
				tokens[tokens_idx].type = NEW_LINE;		
				tokens_idx++;
				break;
			case '#':
				if(buffer_idx != 0){
					tokens[tokens_idx].type = TEXT;
					strcpy(tokens[tokens_idx].content, buffer);
					buffer[0] = '\0';
					buffer_idx = 0;
					tokens_idx++;
				}
				tokens[tokens_idx].type = HASHTAG;		
				tokens_idx++;
				break;
			case '_':
				if(buffer_idx != 0){
					tokens[tokens_idx].type = TEXT;
					strcpy(tokens[tokens_idx].content, buffer);
					buffer[0] = '\0';
					buffer_idx = 0;
					tokens_idx++;
				}
				tokens[tokens_idx].type = UNDERSCORE;		
				tokens_idx++;
				break;
			default:
				buffer[buffer_idx] = str[i];
				buffer[buffer_idx + 1] = '\0';
				buffer_idx++;
		}
	}

	return tokens;
}

Node *parse(Token *tokens, const size_t num_tokens){
	Node *main = new_node();
	int counter = 0;
	int offset = 0;
	for(int i=0; i<num_tokens; i++){
		offset = i;
		switch (tokens[i].type) {
			case HASHTAG:
				main->type = HEADING;	
				counter++;
				sprintf(main->content, "%d", counter);
				break;
			default:
				if(i == 0){
					main->type = PARAGRAPH;
				}
				goto finish_check;
				break;
		}
	}
	finish_check: ;

	Token *prev_text_token = NULL;

	Token *prev_token = NULL;

	main->first_child = new_node();

	Node *curr = main->first_child;

	for (int i=offset; i <num_tokens; i++) {	
		switch (tokens[i].type) {
			case ASTRISK:
				if(prev_token != NULL && prev_token->type == ASTRISK){
					Node *node = new_node();
					node->type = TEXT_BOLD;
					strcpy(node->content, prev_text_token->content);
					curr->next = node;
					curr = node;
					prev_token = NULL;
				}else{
					prev_token = tokens + i;
				}
				break;
			case UNDERSCORE:
				if(prev_token != NULL && prev_token->type == UNDERSCORE){
					Node *node = new_node();
					node->type = TEXT_ITALIC;
					strcpy(node->content, prev_text_token->content);
					curr->next = node;
					curr = node;
					prev_token = NULL;
				}else{
					prev_token = tokens + i;
				}
				break;
			case NEW_LINE:
				{
					Node *node = new_node();
					node->type = LINE_BREAK;
					curr->next = node;
					curr = node;
				}
				break;
			case TEXT:
				if(prev_token == NULL){
					Node *node = new_node();
					node->type = TEXT_NORMAL;
					strcpy(node->content, tokens[i].content);
					curr->next = node;
					curr = node;
				}else{
					prev_text_token = tokens + i;
				}
				break;
			default:
				break;
		}		
	}

	return main;
}

char *compile(Node *root){
	char *res = malloc(sizeof(char) * 1024);
	res[0] = '\0';
	char buffer[100];
	Node *parent = root->first_child;
	Node *curr;
	while (parent != NULL){
		if (parent->type == EMPTY){
			parent = parent->next;
			continue;
		}

		if (parent->type == HEADING){
			sprintf(buffer, "<h%s>\n\t", parent->content);
			strcat(res, buffer);
		} else if (parent->type == PARAGRAPH){
			strcat(res, "<p>\n\t");
		}
		
		curr = parent->first_child;
		while(curr != NULL){
			if (curr->type == TEXT_NORMAL){
				strcat(res, curr->content);
			} else if (curr->type == TEXT_BOLD){
				sprintf(buffer, "<strong>%s</strong>", curr->content);
				strcat(res, buffer);
			} else if (curr->type == TEXT_ITALIC){
				sprintf(buffer, "<em>%s</em>", curr->content);
				strcat(res, buffer);
			} else if (curr->type == LINE_BREAK){
				strcat(res, "<br>");
			}

			curr = curr->next;
		}

		if (parent->type == HEADING){
			sprintf(buffer, "</h%s>\n", parent->content);
			strcat(res, buffer);
		} else if (parent->type == PARAGRAPH){
			strcat(res, "</p>\n");
		}
		parent = parent->next;
	}

	return res;
}

int main(int argc, char **argv){
	if(argc < 2){
		printf("NEED TO INPUT FILENAME");
		exit(1);
	}

	FILE *input = fopen(argv[1], "r");

	Node *root = new_node();
	root->type = ROOT;
	Node *first_child = new_node();
	first_child->type = EMPTY;
	root->first_child = first_child;
	
	Node *curr = first_child;

	char buffer[100];
	size_t num_tokens;
	Token *tokens;

	while (fgets(buffer, sizeof(buffer), input)) {
		tokens = tokenize(buffer, &num_tokens);
		curr->next = parse(tokens, num_tokens);
		curr = curr->next;
	}

	print_ast_tree(root, 0);

	char *res = compile(root);

	fclose(input);

	FILE *output = fopen("output.html", "w");

	fprintf(output, "%s", res);

	fclose(output);

	system("open output.html");

	return 0;
}

// Node *inner_lexer(char *str){
// 	const char BOLD_REGEX[] = "\\*\\*[^*]*\\*\\*";	
//
// 	Node *main = new_node();
// 	Node *curr = main;
//
// 	regex_t r;
//
// 	regex_prepare(&r, BOLD_REGEX);
//
// 	regmatch_t match[1];
//
// 	int offset = 0;
// 	while(regexec(&r, str + offset, 1, match , 0) == 0){
// 		char matched_string[100];
//
// 		int start = match[0].rm_so + offset;
// 		int end = match[0].rm_eo + offset;
//
// 		Node* text_node = new_node();
// 		text_node->type = TEXT;
// 		strncpy(text_node->content, str + offset, offset + start);
// 		curr->next = text_node;
// 		curr = text_node;
// 		
// 		strncpy(matched_string, str + start + 2, end - start - 4);
// 		matched_string[end - start] = '\0';
//
//
// 		char *found = strstr(curr->content, matched_string);
// 		if(found != NULL){
// 			int index = found - curr->content - 2;
// 			curr->content[index] = '\0';
// 		}
//
// 		Node* node = new_node();
// 		node->type = EMPHASIS_BOLD;
// 		strncpy(node->content, matched_string, strlen(matched_string));
// 		curr->next = node;
// 		curr = node;
//
// 		offset += match[0].rm_eo;
// 	}
//
// 	if(strlen(str) != 0){
// 		Node *node = new_node();
// 		node->type = TEXT;
// 		strncpy(node->content, str + offset, strlen(str) - offset);
// 		curr->next = node;
// 		curr = node;
// 	}
//
//
// 	return main;
// }
//
// void parser(Node *root, size_t indent){
// 	if (root == NULL){
// 		return;
// 	}
//
// 	if(indent > 0){
// 		printf("|");
// 	}
// 	for(int i=0; i<indent; i++){
// 		printf("--");
// 	}
//
// 	switch (root->type) {
// 		case ROOT:
// 			printf("ROOT\n");
// 			return parser(root->first_child, indent + 1);
// 		case EMPTY:
// 			printf("EMPTY\n");
// 			return parser(root->next, indent);
// 		case HEADING:
// 			printf("HEADING LEVEL %s\n", root->content);	
// 			parser(root->first_child, indent + 1);
// 			return parser(root->next, indent);
// 		case PARAGRAPH:
// 			printf("PARAGRAPH\n");
// 			parser(root->first_child, indent + 1);
// 			return parser(root->next, indent);
// 		case TEXT:
// 			printf("TEXT: %s\n",root->content);
// 			return parser(root->next, indent);
// 		case EMPHASIS_BOLD:
// 			printf("TEXT-BOLD: %s\n",root->content);
// 			return parser(root->next, indent);
// 	}
// }
//
// Node* lexer(char *str){
// 	const char HEADING_REGEX[] = "^#{1,6}[[:space:]].";
//
// 	Node *node = new_node();
//
// 	regex_t r;
// 	regmatch_t match[1];
// 	regcomp(&r, HEADING_REGEX, REG_EXTENDED);
// 	if(regexec(&r, str, 1, match, 0) == 0){
// 		int start = match[0].rm_so;
// 		int end = match[0].rm_eo;
//
// 		node->type = HEADING;
// 		char level[8];
// 		sprintf(level, "%d", end - 2);
//
// 		strncpy(node->content, level, strlen(level));
//
// 		memmove(str, str + end - 1, strlen(str));
// 	}else{
// 		node->type = PARAGRAPH;
// 	}
//
// 	node->first_child = inner_lexer(str);
//
// 	return node;
// }
//
// int main(int argc, char **argv){
// 	// char input[] = "Hello **WORLD** aaaaa **AAAAA** aaaaaa";
// 	FILE *input = fopen("test.md", "rd");
//
// 	Node *root = new_node();
// 	root->type = ROOT;
// 	Node *first_child = new_node();
// 	first_child->type = EMPTY;
// 	root->first_child = first_child;
// 	
// 	Node *curr = first_child;
//
// 	char buffer[100];
// 	while (fgets(buffer, sizeof(buffer), input)) {
// 		buffer[strlen(buffer)-1] = '\0';
//
// 		curr->next = lexer(buffer);
// 		curr = curr->next;
// 	}
//
// 	parser(root, 0);
// void regex_prepare(regex_t *r, const char *expression){
// 	int code = regcomp(r, expression, REG_EXTENDED);
// 	char buffer[100];
// 	if ( code != 0){ 
// 		regerror(code, r, buffer, 100);
// 		printf("regcomp() failed with '%s'\n", buffer);
// 		exit(EXIT_FAILURE);
// 	}
// }
// // }
