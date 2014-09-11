#ifndef _DTPIC_PRIVATE_H_
#define _DTPIC_PRIVATE_H_

/*** Instance data **********************************************************/
struct Dtpic_DATA
{
    struct Library *datatypesbase;
    STRPTR name;
    APTR dto;
    struct BitMapHeader *bmhd;
    struct BitMap *bm;
    LONG alpha;
    BOOL darkenselstate;
    LONG fade;
    BOOL lightenonmouse;
};

#endif /* _DTPIC_PRIVATE_H_ */
