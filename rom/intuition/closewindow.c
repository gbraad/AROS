/*
    (C) 1995-96 AROS - The Amiga Research OS
    $Id$
    $Log$
    Revision 1.14  1999/03/19 20:21:35  nlorentz
    Fixed race condition bug between CloseWindow()/inputhandler by doing most of window closing on inputhandlers context. Also Closewindow() was called direcly from inputhandler, that would cause FreeSignal() in DeleteMsgPort() to be called on the wrong task context

    Revision 1.13  1998/12/31 21:43:18  nlorentz
    Bugfix: CloseWindow should no longer free win->RPort as that is done in intui_CloseWindow()

    Revision 1.12  1998/10/20 16:45:53  hkiel
    Amiga Research OS

    Revision 1.11  1998/01/16 23:07:16  hkiel
    Always #undef DEBUG to assure proper behaviour with cpak

    Revision 1.10  1998/01/05 21:06:43  hkiel
    Added masquerade to #include <aros/debug.h> for cpak.

    Revision 1.9  1997/01/27 00:36:36  ldp
    Polish

    Revision 1.8  1996/12/10 14:00:01  aros
    Moved #include into first column to allow makedepend to see it.

    Revision 1.7  1996/11/08 11:28:00  aros
    All OS function use now Amiga types

    Moved intuition-driver protos to intuition_intern.h

    Revision 1.6  1996/10/31 13:50:55  aros
    Don't forget to free the RastPort

    Revision 1.5  1996/10/24 15:51:18  aros
    Use the official AROS macros over the __AROS versions.

    Revision 1.4  1996/10/15 15:45:31  digulla
    Two new functions: LockIBase() and UnlockIBase()
    Modified code to make sure that it is impossible to access illegal data (ie.
	fields of a window which is currently beeing closed).

    Revision 1.3  1996/09/21 14:16:26  digulla
    Debug code
    Only change the ActiveWindow is it is beeing closed
    Search for a new ActiveWindow

    Revision 1.2  1996/08/29 13:33:30  digulla
    Moved common code from driver to Intuition
    More docs

    Revision 1.1  1996/08/13 15:37:26  digulla
    First function for intuition.library


    Desc:
    Lang: english
*/
#include "intuition_intern.h"
#include <proto/exec.h>
#include <proto/graphics.h>

#ifndef DEBUG_CloseWindow
#   define DEBUG_CloseWindow 0
#endif
#undef DEBUG
#if DEBUG_CloseWindow
#   define DEBUG 1
#endif
#	include <aros/debug.h>

/*****************************************************************************

    NAME */
#include <proto/intuition.h>

	AROS_LH1(void, CloseWindow,

/*  SYNOPSIS */
	AROS_LHA(struct Window *, window, A0),

/*  LOCATION */
	struct IntuitionBase *, IntuitionBase, 12, Intuition)

/*  FUNCTION
	Closes a window. Depending on the display, this might not happen
	at the time when this function returns, but you must not use
	the window pointer after this function has been called.

    INPUTS
	window - The window to close

    RESULT
	None.

    NOTES
	The window might not have been disappeared when this function returns.

    EXAMPLE

    BUGS

    SEE ALSO
	OpenWindow(), OpenWindowTags()

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    intuition_lib.fd and clib/intuition_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct IntuitionBase *,IntuitionBase)
    
#define IW(x) ((struct IntWindow *)x)    
    struct IntuiMessage * im;

    D(bug("CloseWindow (%p)\n", window));
    
    /* We take a very simple approach to avoid race conditions with the
       intuition input handler running one input.device 's task:
       We just send it a msg about closing the window
    */

    im = IW(window)->closeMessage;
    im->Class    = IDCMP_WBENCHMESSAGE;
    im->Code     = IMCODE_CLOSEWINDOW;
    im->IAddress = window;
    
    PutMsg(window->WindowPort, (struct Message *)im);
    


    /* Obviously this should be done on the application's context.
       (DeleteMsgPort() calls FreeSignal()
    */

    if (window->UserPort)
    {
	/* Delete all pending messages */
	Forbid ();
	

	while ((im = (struct IntuiMessage *) GetMsg (window->UserPort)))
	    ReplyMsg ((struct Message *)im);
	    
	Permit ();

	/* Delete message port */
	DeleteMsgPort (window->UserPort);
    }
    

    ReturnVoid ("CloseWindow");
    AROS_LIBFUNC_EXIT
} /* CloseWindow */



/* This is called from the intuition input handler */
VOID int_closewindow(struct Window *window)
{
#define IW(x) ((struct IntWindow *)x)    

    /* Free everything except the applications messageport */
    ULONG lock;

    D(bug("CloseWindow (%p)\n", window));

    lock = LockIBase (0);
    
    if (window == IntuitionBase->ActiveWindow)
	IntuitionBase->ActiveWindow = NULL;

    /* Remove window from the chain and find next active window */
    if (window->Descendant)
    {
	window->Descendant->Parent = window->Parent;
	ActivateWindow (window->Descendant);
    }
    if (window->Parent)
    {
	window->Parent->NextWindow =
	    window->Parent->Descendant =
	    window->Descendant;

	if (!IntuitionBase->ActiveWindow)
	    ActivateWindow (window->Parent);
    }

    /* Make sure the Screen is still valid */
    if (window == window->WScreen->FirstWindow)
	window->WScreen->FirstWindow = window->NextWindow;

    UnlockIBase (lock);

    /* Free resources */
    
    CloseFont (window->RPort->Font);
    
    /* Let the driver clean up. Driver wil dealloc window's rastport */
    intui_CloseWindow (window, IntuitionBase);

    if (IW(window)->closeMessage)
	free_intuimessage(IW(window)->closeMessage, IntuitionBase);

    
    /* Free memory for the window */
    FreeMem (window, intui_GetWindowSize ());
    
    return;
    
} /* CloseWindow */

