#include "asm.h"

static int CheckExt(size_t *len, const char* path)
{
	const char* ptr = NULL;

	if (!path || !len)
		return 0;

	while (*path) {
		if (*path == '.')
			ptr = path + 1;
		(*len)++;
		path++;
	}

	if (!ptr || !IStrCmp(ptr, "asm")) {
		fprintf(stderr, "Invalid file extension. use .asm!\n");
		return 0;
	}

	return 1;
}

CFile* NewFile(const char* path, int check_ext)
{
	CFile* file;
	FILE*  tmpf;
	size_t pathlen = 0;
	
	if (!path)
		return NULL;

	if (check_ext && !CheckExt(&pathlen, path))
		return NULL;

	tmpf = fopen(path, "rb");

	if (!tmpf) {
		fprintf(stderr, "Error opening file '%s'\n", path);
		return NULL;
	}

	file = (CFile*)Alloc(sizeof(CFile), ARENA_1);

	fseek(tmpf, 0, SEEK_END);

	file->fsize = ftell(tmpf);

	rewind(tmpf);

	file->src = (char*)Alloc(sizeof(char) * file->fsize, ARENA_3);

	fread(file->src, sizeof(char), file->fsize, tmpf);

	file->src[file->fsize] = '\0';

	fclose(tmpf);

	file->line   = 1;
	file->errors = 0;
	file->warns  = 0;
	file->path   = (char*)path;
	file->prev   = NULL;
	file->fileName_length = pathlen;

	return file;
}

void PushFile(CFile **file, const char *new_path)
{
	CFile* tmpf;

	if (!file || !new_path)
		return;

	tmpf = NewFile(new_path, 0);

	if (!tmpf)
		return;

	tmpf->prev = *file;

	*file = tmpf;
}

void PopFile(CFile** file)
{
	if (!file)
		return;
	*file = (*file)->prev;
}