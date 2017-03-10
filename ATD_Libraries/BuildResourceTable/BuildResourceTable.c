// A simple program that builds a table of the uir resources
#include <stdio.h>
#include <string.h>
 
int main (int argc, char *argv[])
{
  char line [1000];
  FILE *fin = NULL;
  FILE *fout = NULL;

  // make sure we have enough arguments
  if (argc < 2)
  {
	return 1;
  }
  
  // open the input file
  fin = fopen(argv[1],"r");

  if (!fin)
  {
	return 1;
  }

   // open the output file
  fout = fopen(argv[2],"w");
  
  if (!fout)
  {
	return 1;
  }
  
  // create a source file with a table of square roots
  fprintf(fout,"#ifndef _GENERATED_RESOURCES_H\n#define _GENERATED_RESOURCES_H_\nchar* resource_files[] = {\n");
  
  while ( fgets ( line, sizeof line, fin ) != NULL ) /* read a line */      
  {          
    // Remove \n
	line[strlen(line) - 1] = '\0';
	fprintf(fout,"\"%s\",\n",line);
  }
 
  // close the table with a zero
  fprintf(fout,"NULL };\n#endif\n");
  fclose(fout);
  fclose(fin);

  return 0;
}
