#ifndef paagllist
#define paagllist

#define AGL_DLIST_INIT(self)                                            \
    do {                                                                \
        (&(self))->prev = &(self);                                      \
        (&(self))->next = &(self);                                      \
    } while(0)

#define AGL_OFFSET(structure, member)                                    \
    ((int)((char *)((&((structure *)0)->member)) - (char *)0))

#define AGL_LIST_RELOCATE(structure, member, ptr)                       \
    ((structure *)(void *)((char *)ptr - AGL_OFFSET(structure, member)))

#define AGL_DLIST_FOR_EACH(structure, member, pos, head)                \
    for (pos = AGL_LIST_RELOCATE(structure, member, (head)->next);      \
         &pos->member != (head);                                        \
         pos = AGL_LIST_RELOCATE(structure, member, pos->member.next))

#define AGL_DLIST_FOR_EACH_SAFE(structure, member, pos, n, head)        \
    for (pos = AGL_LIST_RELOCATE(structure, member, (head)->next),      \
           n = AGL_LIST_RELOCATE(structure, member, pos->member.next);  \
         &pos->member != (head);                                        \
         pos = n,                                                       \
           n = AGL_LIST_RELOCATE(structure, member, pos->member.next))

#define AGL_DLIST_FOR_EACH_BACKWARDS(structure, member, pos, head)       \
    for (pos = AGL_LIST_RELOCATE(structure, member, (head)->prev);      \
         &pos->member != (head);                                        \
         pos = AGL_LIST_RELOCATE(structure, member, pos->member.prev))

#define AGL_DLIST_UNLINK(structure, member, elem)                       \
    do {                                                                \
        agl_dlist *after  = (elem)->member.prev;                        \
        agl_dlist *before = (elem)->member.next;                        \
        after->next = before;                                           \
        before->prev = after;                                           \
        (elem)->member.prev = (elem)->member.next = &(elem)->member;    \
    } while(0)


typedef struct agl_dlist {
	struct agl_dlist *prev;
	struct agl_dlist *next;
} agl_dlist;

#endif
