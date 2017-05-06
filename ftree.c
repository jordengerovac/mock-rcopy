#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>

#include "hash.h"

int copy_ftree(const char *src, const char *dest) {
	// for each file in src I have to compare with all files in dest to
	// check if the dest directory already contains the contents in src
	DIR *srcdir;
	struct dirent *sde;
	int processes = 1;
	char spath[200];
	char dpath[200];
	strcpy(dpath, dest);
	strcpy(spath, src);
	struct stat sbuf;
	struct stat dbuf;
	if (lstat(src, &sbuf) == -1) {
		perror("lstat");
		exit(-1);
	}
	if (lstat(dest, &dbuf) == -1) {
		perror("lstat");
		exit(-1);
	}

	// if src is not a directory
	if ((!(S_ISDIR(sbuf.st_mode))) || (!(S_ISDIR(dbuf.st_mode)))) {
		printf("Usage: ./fcopy [src dir] [dest dir]\n");
		exit(-1);
	}
	// if src is a directory, mkdir
	else {
		if (!(srcdir = opendir(src))) {
			perror("opendir");
			exit(-1);
		}
		strcat(dpath, "/");
		strcat(dpath, src);
		if (lstat(dpath, &dbuf) == -1) {
			if (mkdir(dpath, (sbuf.st_mode & 0777)) == -1) {
				perror("mkdir");
				exit(-1);
			}
		}

		while ((sde = readdir(srcdir)) != NULL) {
			strcpy(spath, src);
			strcat(spath, "/");
			strcat(spath, sde->d_name);
			strcpy(dpath, dest);
			strcat(dpath, "/");
			strcat(dpath, src);
			strcat(dpath, "/");
			strcat(dpath, sde->d_name);

			if (lstat(spath, &sbuf) == -1) {
				perror("lstat");
				exit(-1);
			}
			if ((sde->d_name[0] != '.') && (!(S_ISLNK(sbuf.st_mode)))) {
				// fork on sub directory
				if (S_ISDIR(sbuf.st_mode)) {
					int r = fork();
					// child
					if (r == 0) {
						int p = copy_ftree(spath, dest);
						exit(p);
					}
					//parent
					else {
						int status;
						wait(&status);
						processes += WEXITSTATUS(status);
					}
				}
				// if file doesn't exist create file
				else if ((lstat(dpath, &dbuf)) == -1) {
					FILE *sf;
					FILE *df;
					char b[2];
					if (!(sf = fopen(spath, "r"))) {
						perror("fopen");
						exit(-1);
					}
					if (!(df = fopen(dpath, "w"))) {
						perror("fopen");
						exit(-1);
					}
				
					// getting size of file then setting file pointer back to beginning of file
					fseek(sf, 0, SEEK_END);
					long int size = ftell(sf);
					fseek(sf, 0, SEEK_SET);

					// copying files from source to dest
					for (int i = 0; i < size; i++) {
						fread(b, 1, 1, sf);
						fwrite(b, 1, 1, df);
					}
					if (chmod(dpath, (sbuf.st_mode & 0777)) == -1) {
						perror("chmod");
						exit(-1);
					}
				}

				// if file exists, check size and hash. 
				// if size or hash are different, create file.
				else {
					if (lstat(spath, &sbuf) == -1) {
						perror("lstat");
						exit(-1);
					}
					if (lstat(dpath, &dbuf) == -1) {
						perror("lstat");
						exit(-1);
					}
					if ((S_ISREG(sbuf.st_mode) && S_ISREG(dbuf.st_mode)) || (S_ISDIR(sbuf.st_mode) && S_ISDIR(dbuf.st_mode))) {
						FILE *sf;
						FILE *df;
						if (!(sf = fopen(spath, "r"))) {
							perror("fopen");
							exit(-1);
						}
						if (!(df = fopen(dpath, "w"))) {
							perror("fopen");
							exit(-1);
						}

						if ((sbuf.st_size != dbuf.st_size) || (hash(sf) != hash(df))) { 
							char b[2];

							fseek(sf, 0, SEEK_END);
							long int size = ftell(sf);
							fseek(sf, 0, SEEK_SET);

							for (int i = 0; i < size; i++) {
								fread(b, 1, 1, sf);
								fwrite(b, 1, 1, df);
							}
							if (chmod(dpath, (sbuf.st_mode & 0777)) == -1) {
								perror("chmod");
								exit(-1);
							}
						}
					}
				}
			}
		}
	}
	return processes;
}

