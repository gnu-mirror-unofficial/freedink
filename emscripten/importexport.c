#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zip.h>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

int IsDir(char* path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

int AddFileToZip(zip_t* archive, char* path, char* prefix) {
  zip_source_t *s;
  s = zip_source_file(archive, path, 0, -1);
  if (s == NULL) {
    fprintf(stderr, "error opening %s\n", path);
    return 0;
  }
  char* zippath = path + strlen(prefix);
  if (zip_file_add(archive, zippath, s, ZIP_FL_ENC_UTF_8) < 0) { 
    zip_source_free(s);
    fprintf(stderr, "error adding %s: %s\n", path, zip_strerror(archive));
    return 0;
  }
  return 1;
}

int AddPathToZip(zip_t* archive, char* path, char* prefix) {
  //printf("processing %s\n", path);
  struct stat st;
  if (IsDir(path)) {
    DIR* dir = opendir(path);
    if (dir == NULL) {
      perror("opendir");
      return 0;
    }
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
      //printf("dir: %s\n", ent->d_name);
      if (strcmp(ent->d_name, ".") == 0) continue;
      if (strcmp(ent->d_name, "..") == 0) continue;
      char* subpath = malloc(strlen(path) + 1 + strlen(ent->d_name) + 1);
      sprintf(subpath, "%s/%s", path, ent->d_name);
      AddPathToZip(archive, subpath, prefix);
      free(subpath);
    }
    closedir(dir);
  } else {
    AddFileToZip(archive, path, prefix);
  }
  return 1;
}

int emSavegamesExport(void) {
  int errorp;
  zip_t* archive = zip_open("savegames.zip", ZIP_CREATE|ZIP_TRUNCATE|ZIP_CHECKCONS, &errorp);
  if (archive == NULL) {
    zip_error_t error;
    zip_error_init_with_code(&error, errorp);
    fprintf(stderr, "error creating savegames.zip: %s\n", zip_error_strerror(&error));
    zip_error_fini(&error);
    return 0;
  }

  char* path = malloc(strlen(getenv("HOME"))+1+5+1);
  sprintf(path, "%s/%s", getenv("HOME"), ".dink");
  char* prefix = malloc(strlen(getenv("HOME"))+1+5+1+1);
  sprintf(prefix, "%s/%s/", getenv("HOME"), ".dink");
  AddPathToZip(archive, path, prefix);
  free(path);

  if (zip_get_num_entries(archive, 0) == 0) {
    printf("No savegames found!\n");
    return 0;
  } else if (zip_close(archive) < 0) {
    fprintf(stderr, "cannot write savegames.zip: %s\n", zip_strerror(archive));
    return 0;
  }

  chmod("savegames.zip", 00666);
  return 1;
}

#ifdef TEST
int main(void) {
  return !emSavegamesExport();
}
#endif
