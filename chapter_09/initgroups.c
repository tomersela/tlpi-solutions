#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>

#include "tlpi_hdr.h"

int myinitgroups(const char *user, gid_t group);

int
myinitgroups(const char *user, gid_t group)
{
    struct passwd *pwd;
    struct group *grp;
    gid_t gids[NGROUPS_MAX + 1];
    size_t groups_size = 0;
    char *member;

    if(geteuid() != 0) { // validate permissions
        return errno = EPERM, -1;
    }

    errno = 0;
    pwd = getpwnam(user);
    if (pwd == NULL) {
        if (errno == 0)
            return errno = ENOENT, -1;
        else
            return -1;
    }

    gids[0] = group;

    setgrent();
    while ((grp = getgrent()) != NULL) {
        for (int i = 0; (member = grp->gr_mem[i]) != NULL; i++) {
            if (!strcmp(member, user)) {
                ++groups_size;
                gids[groups_size] = grp->gr_gid;
            }
        }
    }
    endgrent();


    return setgroups(groups_size, gids);
}

static void
print_supplementary_group_ids()
{
    gid_t grouplist[NGROUPS_MAX + 1];

    int gcnt = getgroups(NGROUPS_MAX + 1, grouplist);
    for (int i = 0; i < gcnt; i++) {
        printf("%ld ", (long) grouplist[i]);
    }
}

int
main(int argc, char *argv[])
{
    printf("Before calling initgroups:\n");
    print_supplementary_group_ids();
    printf("\n");

    if (myinitgroups("tomersela", 1000) != 0) {
        errExit("initgroups");
    }

    printf("After calling initgroups:\n");
    print_supplementary_group_ids();
}
