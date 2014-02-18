#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <GL/glew.h>

#include "shader.h"

static char *read_file(const char *filename)
{
	char *buffer = NULL;
	int string_size, read_size;
	FILE *handler = fopen(filename, "r");

	if (handler) {
		/* seek the last byte of the file */
		fseek(handler, 0, SEEK_END);
		/* offset from the first to the last byte, or in other words, filesize */
		string_size = ftell(handler);
		/* go back to the start of the file */
		rewind(handler);

		/* allocate a string that can hold it all */
		buffer = (char *) malloc(sizeof(char) * (string_size + 1));
		/* read it all in one operation */
		read_size = fread(buffer, sizeof(char), string_size, handler);
		/* fread doesnt set it so put a \0 in the last position
		   and buffer is now officialy a string */
		buffer[string_size] = '\0';

		if (string_size != read_size) {
			/* something went wrong, throw away the memory and set
			   the buffer to NULL */
			free(buffer);
			buffer = NULL;
		}
	}

	return buffer;
}

GLuint load_concat_shaders(
	const char *vertex_header, int nvertex_files, const char *vertex_file_path[],
	const char *fragment_header, int nfragment_files, const char *fragment_file_path[])
{
	GLuint program_id = 0;
	int i;

	/* Create the shaders */
	GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

	int nvertex_shader = 0;
	char *vertex_shader_filename[nvertex_files + 1];
	GLchar *vertex_shader_code[nvertex_files + 1];

	int nfragment_shader = 0;
	char *fragment_shader_filename[nfragment_files + 1];
	GLchar *fragment_shader_code[nfragment_files + 1];

	/* Build the Vertex Shader code */
	if (vertex_header) {
		vertex_shader_filename[nvertex_shader] = strdup("string_vertex_header");
		vertex_shader_code[nvertex_shader] = strdup(vertex_header);
		nvertex_shader++;
	}

	for (i = 0; i < nvertex_files; i++) {
		char *file_shader_code = read_file(vertex_file_path[i]);
		if (!file_shader_code) {
			printf("Can't open vertex shader '%s'\n", vertex_file_path[i]);
			goto cleanup;
		}

		vertex_shader_filename[nvertex_shader] = strdup(vertex_file_path[i]);
		vertex_shader_code[nvertex_shader] = file_shader_code;
		nvertex_shader++;
	}

	/* Read the Fragment Shader code from the file */
	if (fragment_header) {
		fragment_shader_filename[nfragment_shader] = strdup("string_fragment_header");
		fragment_shader_code[nfragment_shader] = strdup(fragment_header);
		nfragment_shader++;
	}

	for (i = 0; i < nfragment_files; i++) {
		char *file_shader_code = read_file(fragment_file_path[i]);
		if (!file_shader_code) {
			printf("Can't open fragment shader '%s'\n", fragment_file_path[i]);
			goto cleanup;
		}

		fragment_shader_filename[nfragment_shader] = strdup(fragment_file_path[i]);
		fragment_shader_code[nfragment_shader] = file_shader_code;
		nfragment_shader++;
	}

	GLint result = GL_FALSE;
	int info_log_length;

	/* Compile Vertex Shader */
	glShaderSource(vertex_shader_id, nvertex_shader, (const GLchar **)&vertex_shader_code[0], NULL);
	glCompileShader(vertex_shader_id);

	/* Check Vertex Shader */
	glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		printf("Vertex shader compile error in files:\n");
		for (i = 0; i < nvertex_shader; i++) {
			printf("    %d: \"%s\"\n", i, vertex_shader_filename[i]);
		}
		glGetShaderiv(vertex_shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
		if (info_log_length > 0) {
			GLchar error_message[info_log_length + 1];
			glGetShaderInfoLog(vertex_shader_id, info_log_length, NULL, &error_message[0]);
			printf("Error:\n%s\n", error_message);
		} else {
			printf("error: unknown\n");
		}
		goto cleanup;
	}

	/* Compile Fragment Shader */
	glShaderSource(fragment_shader_id, nfragment_shader, (const GLchar **)&fragment_shader_code[0], NULL);
	glCompileShader(fragment_shader_id);

	/* Check Fragment Shader */
	glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		printf("Fragment shader compile error in files:\n");
		for (i = 0; i < nfragment_shader; i++) {
			printf("    %d: \"%s\"\n", i, fragment_shader_filename[i]);
		}
		glGetShaderiv(fragment_shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
		if (info_log_length > 0) {
			char error_message[info_log_length + 1];
			glGetShaderInfoLog(fragment_shader_id, info_log_length, NULL, &error_message[0]);
			printf("Error:\n%s\n", error_message);
		} else {
			printf("error: unknown\n");
		}
		goto cleanup;
	}

	/* Link the program */
	program_id = glCreateProgram();
	glAttachShader(program_id, vertex_shader_id);
	glAttachShader(program_id, fragment_shader_id);
	glLinkProgram(program_id);

	/* Check the program */
	glGetProgramiv(program_id, GL_LINK_STATUS, &result);
	if (result == GL_FALSE) {
		printf("Shader link error in files:\n");
		for (i = 0; i < nvertex_shader; i++) {
			printf("    vert %d: \"%s\"\n", i, vertex_shader_filename[i]);
		}
		for (i = 0; i < nfragment_shader; i++) {
			printf("    frag %d: \"%s\"\n", i, fragment_shader_filename[i]);
		}

		glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_log_length);
		if (info_log_length > 0) {
			GLchar error_message[info_log_length + 1];
			glGetProgramInfoLog(program_id, info_log_length, NULL, &error_message[0]);
			printf("Error:\n%s\n", error_message);

			glDeleteProgram(program_id);
			program_id = 0;
		} else {
			printf("error: unknown\n");
		}
		goto cleanup;
	}

cleanup:
	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);

	for (i = 0; i < nvertex_shader; i++) {
		free(vertex_shader_filename[i]);
		free(vertex_shader_code[i]);
	}
	for (i = 0; i < nfragment_shader; i++) {
		free(fragment_shader_filename[i]);
		free(fragment_shader_code[i]);
	}

	return program_id;
}

GLuint load_shaders(const char *vertex_file_path, const char *fragment_file_path)
{
	return load_concat_shaders(0, 1, &vertex_file_path, 0, 1, &fragment_file_path);
}

