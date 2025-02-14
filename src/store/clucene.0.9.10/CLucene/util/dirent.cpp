#include "CLucene/StdHeader.h"

#if !defined(_CL_HAVE_DIRENT_H) && !defined(_CL_HAVE_SYS_NDIR_H) && !defined(_CL_HAVE_SYS_DIR_H) && !defined(_CL_HAVE_NDIR_H)
#include "dirent.h"

CL_NS_DEF(util)

DIR * 
opendir (const char *szPath)
{
	DIR *nd;
	char szFullPath[CL_MAX_DIR];

	errno = 0;

	if (!szPath)
	{
		errno = EFAULT;
		return NULL;
	}

	if (szPath[0] == '\0')
	{
		errno = ENOTDIR;
		return NULL;
	}

	/* Attempt to determine if the given path really is a directory. */
	struct _stat rcs;
	if ( _stat(szPath,&rcs) == -1)
	{
		/* call GetLastError for more error info */
		errno = ENOENT;
		return NULL;
	}
	if (!(rcs.st_mode & _S_IFDIR))
	{
		/* Error, entry exists but not a directory. */
		errno = ENOTDIR;
		return NULL;
	}

	/* Make an absolute pathname.  */
	_fullpath (szFullPath, szPath, CL_MAX_DIR);

	/* Allocate enough space to store DIR structure and the complete
	* directory path given. */
	//nd = (DIR *) malloc (sizeof (DIR) + _tcslen (szFullPath) + _tcslen (DIRENT_SLASH) +
	//					_tcslen (DIRENT_SEARCH_SUFFIX)+1);
	nd = new DIR;

	if (!nd)
	{
		/* Error, out of memory. */
		errno = ENOMEM;
		return NULL;
	}

	/* Create the search expression. */
	strcpy (nd->dd_name, szFullPath);

	/* Add on a slash if the path does not end with one. */
	if (nd->dd_name[0] != '\0' &&
		nd->dd_name[strlen (nd->dd_name) - 1] != '/' &&
		nd->dd_name[strlen (nd->dd_name) - 1] != '\\')
	{
		strcat (nd->dd_name, DIRENT_SLASH);
	}

	/* Add on the search pattern */
	strcat (nd->dd_name, DIRENT_SEARCH_SUFFIX);

	/* Initialize handle to -1 so that a premature closedir doesn't try
	* to call _findclose on it. */
	nd->dd_handle = -1;

	/* Initialize the status. */
	nd->dd_stat = 0;

	/* Initialize the dirent structure. ino and reclen are invalid under
	* Win32, and name simply points at the appropriate part of the
	* findfirst_t structure. */
	//nd->dd_dir.d_ino = 0;
	//nd->dd_dir.d_reclen = 0;
	nd->dd_dir.d_namlen = 0;
	nd->dd_dir.d_name = nd->dd_dta.name;

	return nd;
}


struct dirent * readdir (DIR * dirp)
{
	errno = 0;

	/* Check for valid DIR struct. */
	if (!dirp)
	{
		errno = EFAULT;
		return NULL;
	}

	if (dirp->dd_dir.d_name != dirp->dd_dta.name)
	{
		/* The structure does not seem to be set up correctly. */
		errno = EINVAL;
		return NULL;
	}

	if (dirp->dd_stat < 0)
	{
		/* We have already returned all files in the directory
		* (or the structure has an invalid dd_stat). */
		return NULL;
	}
	else if (dirp->dd_stat == 0)
	{
		/* We haven't started the search yet. */
		/* Start the search */
		dirp->dd_handle = _findfirst (dirp->dd_name, &(dirp->dd_dta));

		if (dirp->dd_handle == -1)
		{
			/* Whoops! Seems there are no files in that
			* directory. */
			dirp->dd_stat = -1;
		}
		else
		{
			dirp->dd_stat = 1;
		}
	}
	else
	{
		/* Get the next search entry. */
		if (_findnext (dirp->dd_handle, &(dirp->dd_dta)))
		{
			/* We are off the end or otherwise error. */
			_findclose (dirp->dd_handle);
			dirp->dd_handle = -1;
			dirp->dd_stat = -1;
		}
		else
		{
			/* Update the status to indicate the correct
			* number. */
			dirp->dd_stat++;
		}
	}

	if (dirp->dd_stat > 0)
	{
		/* Successfully got an entry. Everything about the file is
		* already appropriately filled in except the length of the
		* file name. */
		dirp->dd_dir.d_namlen = strlen (dirp->dd_dir.d_name);

		if ( dirp->dd_dir.d_name[0] == '.' &&
			(dirp->dd_dir.d_name[1] == 0 || 
				(dirp->dd_dir.d_name[1] == '.' && dirp->dd_dir.d_name[2] == 0)))
				return readdir(dirp);

		struct _stat buf;
		char buffer[CL_MAX_DIR];
		size_t bl = strlen(dirp->dd_name)-strlen(DIRENT_SEARCH_SUFFIX);
		strncpy(buffer,dirp->dd_name,bl);
		buffer[bl]=0;
		strcat(buffer, dirp->dd_dir.d_name);
		if ( _stat(buffer,&buf) == -1 )
			return readdir(dirp);

		return &dirp->dd_dir;
	}

	return NULL;
}


int32_t
closedir (DIR * dirp)
{
	int32_t rc;

	errno = 0;
	rc = 0;

	if (!dirp)
	{
		errno = EFAULT;
		return -1;
	}

	if (dirp->dd_handle != -1)
	{
		rc = _findclose (dirp->dd_handle);
	}

	/* Delete the dir structure. */
	_CLVDELETE(dirp);

	return rc;
}
CL_NS_END
#endif //HAVE_DIRENT_H

