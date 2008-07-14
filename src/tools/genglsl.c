/**
 * $Id$
 *
 * Trivial tool to take two shader source files and dump them out in
 * a C file with appropriate escaping.
 *
 * Copyright (c) 2007 Nathan Keynes.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>

/**
 * Copy input to output, quoting " characters as we go.
 */
void writeShader( FILE *out, FILE *in )
{
    int ch;

    while( (ch = fgetc(in)) != EOF ) {
        if( ch == '\"' ) {
            fputc( '\\', out );
        } else if( ch == '\n') {
            fputs( "\\n\\", out );
        }
        fputc( ch, out );
    }
}

int main( int argc, char *argv[] )
{
    if( argc != 4 ) {
        fprintf( stderr, "Usage: genglsl <vertex-shader-file> <fragment-shader-file> <output-file>\n");
        exit(1);
    }

    FILE *vsin = fopen( argv[1], "ro" );
    if( vsin == NULL ) {
        perror( "Unable to open vertex shader source" );
        exit(2);
    }

    FILE *fsin = fopen( argv[2], "ro" );
    if( fsin == NULL ) {
        perror( "Unable to open fragment shader source" );
        exit(2);
    }

    FILE *out = fopen( argv[3], "wo" );
    if( out == NULL ) {
        perror( "Unable to open output file" );
        exit(2);
    }

    fprintf( out, "/**\n * This file is automatically generated - do not edit\n */\n\n" );
    fprintf( out, "const char *glsl_vertex_shader_src = \"" );

    writeShader( out, vsin );

    fprintf( out, "\";\n\n" );
    fprintf( out, "const char *glsl_fragment_shader_src = \"" );
    writeShader( out, fsin );
    fprintf( out, "\";\n\n" );
    fclose( fsin );
    fclose( vsin );
    fclose( out );
    return 0;
}
