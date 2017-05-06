#include <stdio.h>
#include <stdlib.h>

char *hash(FILE *f) {
	fseek(f, 0, SEEK_END);
	long int size = ftell(f);
	fseek (f, 0, SEEK_SET);
	char *hash_val;
	hash_val = malloc(8*sizeof(char));
	int i = 0;
	char b;
	
	for (int j = 0; j < 8; j++) {
		hash_val[j] = '\0';
	}

	for (int j = 0; j < size; j++) {
		hash_val[i] = ((hash_val[i]) ^ (fread(&b, 1, 1, f)));
		if (i == 7) {
			i = 0;
		}
	}
	return hash_val;
}
