﻿/*
* Wolfenstein: Enemy Territory GPL Source Code
* Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
*
* ET: Legacy
* Copyright (C) 2012-2017 ET:Legacy team <mail@etlegacy.com>
*
* This file is part of ET: Legacy - http://www.etlegacy.com
*
* ET: Legacy is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ET: Legacy is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ET: Legacy. If not, see <http://www.gnu.org/licenses/>.
*
* In addition, Wolfenstein: Enemy Territory GPL Source Code is also
* subject to certain additional terms. You should have received a copy
* of these additional terms immediately following the terms and conditions
* of the GNU General Public License which accompanied the source code.
* If not, please request a copy in writing from id Software at the address below.
*
* id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
*/
/**
* @file shaders2h.cpp
*/

#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstdarg>
#include <vector>
#include "tinydir.h"

using namespace std;

typedef struct
{
	string text;
	string varName;
	string name;
	bool valid;
} shader_out;

const char * headerValue = 
"/*\n"
"===========================================================================\n"
"Copyright (C) 2006-2008 Robert Beckebans <trebor_7@users.sourceforge.net>\n"
"\n"
"This file is part of XreaL source code.\n"
"\n"
"XreaL source code is free software; you can redistribute it\n"
"and/or modify it under the terms of the GNU General Public License as\n"
"published by the Free Software Foundation; either version 2 of the License,\n"
"or (at your option) any later version.\n"
"\n"
"XreaL source code is distributed in the hope that it will be\n"
"useful, but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with XreaL source code; if not, write to the Free Software\n"
"Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA\n"
"===========================================================================\n"
"*/\n"
"/* This file was generated by shdr2h, do not modify as it will get overwritten */";

static void make_c_string(string &in)
{
	string	out = "";

	for (size_t i = 0; i < in.size(); ++i)
	{
	LOOP_HEAD:

		char c = in[i];
		char c2 = (i + 1 < in.size() ? in[i + 1] : 0);
		if ('"' == c)
		{
			out += "\\\"";
		}
		else if ('\\' == c)
		{
			out += "\\\\";
		}
		else if ('/' == c && '/' == c2)
		{
			i += 2;
			while (i < in.size())
			{
				if (in[i] == '\n')
				{
					goto LOOP_HEAD;
				}
				i++;
			}
		}
		else if ('/' == c && '*' == c2)
		{
			while (i + 1 < in.size())
			{
				if (in[i] == '*' && in[i + 1] == '/')
				{
					break;
				}
				i++;
			}
			i += 2;
		}
		else if (c == '\n')
		{
			// Skip extra new lines
			while (i + 1 < in.size() && in[i + 1] == '\n')
			{
				i++;
			}

			for(size_t x = i + 1;x < in.size(); x++)
			{
				if (in[x] == '\n')
				{
					i = x;
					goto LOOP_HEAD;
				}

				if(in[x] == '/' && x + 1 < in.size() && (in[x + 1] == '/' || in[x + 1] == '*'))
				{
					i = x;
					goto LOOP_HEAD;
				}

				// yeah theres something available
				if(in[x] > 32)
				{
					break;
				}

				if(x + 1 == in.size())
				{
					i = x + 1;
				}
			}

			if (out.size() > 0)
			{
				if (i + 1 < in.size())
				{
					out += "\\n\"\n\t\"";
				}
				else
				{
					out += "\\n";
				}
			}
		}
		else if (c == ' ')
		{
			// Skip extra white spaces
			while (i + 1 < in.size() && in[i + 1] == ' ')
			{
				i++;
			}
			// Don't add white spaces before newline
			if (i + 1 < in.size() && in[i + 1] != '\n')
			{
				out += ' ';
			}
		}
		else if (c == '\t')
		{
			if (out.size() > 0)
			{
				if (i + 1 < in.size() && in[i + 1] != '\n')
				{
					out += "\\t";
				}
			}
		}
		else
		{
			out += c;
		}

	}

	// Make sure the output ends with a new line char
	if(out.size() > 0 && out[out.size() - 1] != '\n')
	{
		out += "\\n";
	}

	in = "\"" + out + "\"";
}

static bool read_file_to_c_string(const string &path, string &output)
{
	FILE* in_file = NULL;
	if (in_file = fopen(path.c_str(), "rt"))
	{
		char buff[1024];
		while (fgets(buff, sizeof(buff), in_file))
		{
			string s(buff);
			output += s;
		}

		make_c_string(output);

		fclose(in_file);
		return true;
	}

	return false;
}

static shader_out read_file(tinydir_file *file, const string &path)
{
	shader_out output = {"", "", "", false};

	if (read_file_to_c_string(file->path, output.text))
	{
		string name = file->name;
		name = name.substr(0, name.length() - 5);

		output.name = (path.size() == 0 ? "" : path + "/") + name;
		output.varName = "fallbackShader_";
		for (size_t i = 0; i < output.name.size(); ++i)
		{
			char c = output.name[i];
			if (c == '/')
			{
				output.varName += '_';
			}
			else
			{
				output.varName += c;
			}
		}

		output.valid = true;
	}

	return output;
}

static void process_folder(vector<shader_out> &shaders, const string &folder, const string &path)
{
	tinydir_dir dir;
	tinydir_open(&dir, folder.c_str());
	while (dir.has_next)
	{
		tinydir_file file;
		tinydir_readfile(&dir, &file);

		if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
		{
			tinydir_next(&dir);
			continue;
		}

		if (file.is_dir)
		{
			process_folder(shaders, file.path, (path.size() == 0 ? file.name : path + "/" + string(file.name)));
		}
		else
		{
			if (strcmp(file.extension, "glsl") == 0)
			{
				shader_out output = read_file(&file, (path.size() == 0 ? "" : path));
				if (output.valid)
				{
					shaders.push_back(output);
				}
			}
		}

		tinydir_next(&dir);
	}

	tinydir_close(&dir);
}

static int write_output(string &out_file, string &array_name, vector<shader_out> &shaders, const string &shader_def)
{
	FILE *outputFile;
	if (outputFile = fopen(out_file.c_str(), "wt"))
	{
		fprintf(outputFile, "%s\n", headerValue);

		fprintf(outputFile, "#ifndef  SHADER_HEADER_%s\n", array_name.c_str());
		fprintf(outputFile, "#define  SHADER_HEADER_%s\n\n", array_name.c_str());

		for (std::vector<shader_out>::iterator it = shaders.begin(); it != shaders.end(); ++it)
		{
			fprintf(outputFile, "const char *%s =\n\t%s;\n\n", it->varName.c_str(), it->text.c_str());
		}

		if (shader_def.size() > 0)
		{
			fprintf(outputFile, "//GLSL Shader default definitions found in renderer2/gldef folder\n");
			fprintf(outputFile, "const char *defaultShaderDefinitions =\n\t%s;\n\n", shader_def.c_str());
		}

		fprintf(outputFile, "static const char* GetFallbackShader(const char *name)\n{\n");
		bool first = true;
		for (std::vector<shader_out>::iterator it = shaders.begin(); it != shaders.end(); ++it)
		{
			fprintf(outputFile, "\t");
			if (!first)
			{
				fprintf(outputFile, "else ");
			}
			else
			{
				first = false;
			}
			fprintf(outputFile, "if(!Q_stricmp(name,\"%s\"))\n\t{\n\t\treturn %s;\n\t}\n", it->name.c_str(), it->varName.c_str());
		}
		fprintf(outputFile, "\treturn NULL;\n}\n");
		fprintf(outputFile, "#endif  // #ifdef SHADER_HEADER_%s\n", array_name.c_str());

		fclose(outputFile);
		return 0;
	}

	return 1;
}

int main(int argc, char** args)
{
	if(argc != 5)
	{
		printf("syntax error, usage :  shdr2h hdrname glsl-inputfolder shaderdef-folder outfile");
		exit(0xff);
	}

	string array_name = args[1];
	string in_file = args[2];
	string defin_file = args[3];
	string out_file = args[4];

	vector<shader_out> shaders;
	string shader_def = "";

	process_folder(shaders, in_file, "");
	read_file_to_c_string(defin_file, shader_def);
	return write_output(out_file, array_name, shaders, shader_def);
}
