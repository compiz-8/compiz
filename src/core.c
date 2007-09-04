/*
 * Copyright © 2007 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#include <string.h>

#include <compiz-core.h>

CompCore core;

static char *corePrivateIndices = 0;
static int  corePrivateLen = 0;

static int
reallocCorePrivate (int  size,
		    void *closure)
{
    void *privates;

    privates = realloc (core.object.privates, size * sizeof (CompPrivate));
    if (!privates)
	return FALSE;

    core.object.privates = (CompPrivate *) privates;

    return TRUE;
}

int
allocCoreObjectPrivateIndex (CompObject *parent)
{
    return allocatePrivateIndex (&corePrivateLen,
				 &corePrivateIndices,
				 reallocCorePrivate,
				 0);
}

void
freeCoreObjectPrivateIndex (CompObject *parent,
			    int	       index)
{
    freePrivateIndex (corePrivateLen, corePrivateIndices, index);
}

CompBool
forEachCoreObject (CompObject         *parent,
		   ObjectCallBackProc proc,
		   void		      *closure)
{
    return TRUE;
}

char *
nameCoreObject (CompObject *object)
{
    return NULL;
}

int
allocateCorePrivateIndex (void)
{
    return compObjectAllocatePrivateIndex (NULL, COMP_OBJECT_TYPE_CORE);
}

void
freeCorePrivateIndex (int index)
{
    compObjectFreePrivateIndex (NULL, COMP_OBJECT_TYPE_CORE, index);
}

static CompBool
initCorePluginForObject (CompPlugin *p,
			 CompObject *o)
{
    return TRUE;
}

static void
finiCorePluginForObject (CompPlugin *p,
			 CompObject *o)
{
}

static void
fileWatchAdded (CompCore      *core,
		CompFileWatch *fileWatch)
{
}

static void
fileWatchRemoved (CompCore      *core,
		  CompFileWatch *fileWatch)
{
}

static CompBool
setOptionForPlugin (CompObject      *object,
		    const char	    *plugin,
		    const char	    *name,
		    CompOptionValue *value)
{
    CompPlugin *p;

    p = findActivePlugin (plugin);
    if (p && p->vTable->setObjectOption)
	return (*p->vTable->setObjectOption) (p, object, name, value);

    return FALSE;
}

CompBool
initCore (void)
{
    compObjectInit (&core.object, 0, COMP_OBJECT_TYPE_CORE);

    core.initPluginForObject = initCorePluginForObject;
    core.finiPluginForObject = finiCorePluginForObject;

    core.fileWatchAdded   = fileWatchAdded;
    core.fileWatchRemoved = fileWatchRemoved;

    core.fileWatch	     = NULL;
    core.lastFileWatchHandle = 1;

    core.setOptionForPlugin = setOptionForPlugin;

    return TRUE;
}

void
finiCore (void)
{
}

CompFileWatchHandle
addFileWatch (const char	    *path,
	      int		    mask,
	      FileWatchCallBackProc callBack,
	      void		    *closure)
{
    CompFileWatch *fileWatch;

    fileWatch = malloc (sizeof (CompFileWatch));
    if (!fileWatch)
	return 0;

    fileWatch->path	= strdup (path);
    fileWatch->mask	= mask;
    fileWatch->callBack = callBack;
    fileWatch->closure  = closure;
    fileWatch->handle   = core.lastFileWatchHandle++;

    if (core.lastFileWatchHandle == MAXSHORT)
	core.lastFileWatchHandle = 1;

    fileWatch->next = core.fileWatch;
    core.fileWatch = fileWatch;

    (*core.fileWatchAdded) (&core, fileWatch);

    return fileWatch->handle;
}

void
removeFileWatch (CompFileWatchHandle handle)
{
    CompFileWatch *p = 0, *w;

    for (w = core.fileWatch; w; w = w->next)
    {
	if (w->handle == handle)
	    break;

	p = w;
    }

    if (w)
    {
	if (p)
	    p->next = w->next;
	else
	    core.fileWatch = w->next;

	(*core.fileWatchRemoved) (&core, w);

	if (w->path)
	    free (w->path);

	free (w);
    }
}