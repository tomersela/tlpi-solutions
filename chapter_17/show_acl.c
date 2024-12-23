#include <acl/libacl.h>
#include <sys/acl.h>
#include <sys/stat.h>

#include "ugid_functions.h"
#include "tlpi_hdr.h"


#define permset_to_mod(permset) (acl_get_perm(permset, ACL_READ) << 2) | \
                                (acl_get_perm(permset, ACL_WRITE) << 1) | \
                                (acl_get_perm(permset, ACL_EXECUTE))


static void
usage_error(char *prog_name)
{
    usageErr(
        "\n"
        "  %s u <username>\n"
        "  %s g <groupname>\n",
        prog_name, prog_name);
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    acl_entry_t entry;
    uid_t uid = -1;
    gid_t gid = -1;
    acl_permset_t permset;

    char* prog_name = argv[0];
    if (argc != 4) {
        usage_error(prog_name);
    }

    char* id_type = argv[1];
    char* id_value = argv[2];
    char* path = argv[3];

    if (strcmp(id_type, "u") == 0) {
        uid = (uid_t) userIdFromName(id_value);
        if (uid == -1)
            errExit("Invalid user %s", (long) id_value);
    } else if (strcmp(id_type, "g") == 0) {
        gid = (gid_t) groupIdFromName(id_value);
        if (gid == -1)
            errExit("Invalid group %s", (long) id_value);
    } else {
        usage_error(prog_name);
    }

    struct stat file_stat;
    if (stat(path, &file_stat) == -1)
        errExit("stat");
    
    acl_t acl;
    acl = acl_get_file(path, ACL_TYPE_ACCESS);
    if (acl == NULL)
        errExit("acl_get_file");

    // iterate acl entries and get their corresponding permissions
    int user_obj_perm_mod = 0;
    int group_obj_perm_mod = 0;
    int user_perm_mod = 0;
    int group_perm_mod = 0;
    int mask_perm_mod = 0x777; // by default, if there's no mask acl entry, then we consider the mask as allow all
    int other_perm_mod = 0;

    uid_t *uid_entry;
    gid_t *gid_entry;

    int entry_id;
    for (entry_id = ACL_FIRST_ENTRY; ; entry_id = ACL_NEXT_ENTRY) {
        if (acl_get_entry(acl, entry_id, &entry) != 1)
            break; // exit on error or if there are no more entries

        acl_tag_t tag;
        if (acl_get_tag_type(entry, &tag) == -1)
            errExit("acl_get_tag_type");

        switch (tag) {
            case ACL_USER_OBJ:
                if (uid == -1 || file_stat.st_uid != uid)
                    break;
                if (acl_get_permset(entry, &permset) == -1)
                    errExit("get_permset");
                user_obj_perm_mod = permset_to_mod(permset);
                break;
            case ACL_GROUP_OBJ:
                if (gid == -1 || file_stat.st_gid != gid)
                    break;
                if (acl_get_permset(entry, &permset) == -1)
                    errExit("get_permset");
                group_obj_perm_mod = permset_to_mod(permset);
                break;
            case ACL_USER:
                if ((uid_entry = acl_get_qualifier(entry)) == NULL)
                    errExit("acl_get_qualifier");
                if (uid == -1 || uid != *uid_entry) // is this the UID entry we're looking for?
                    break;
                if (acl_get_permset(entry, &permset) == -1)
                    errExit("get_permset");
                user_perm_mod = permset_to_mod(permset);
                break;
            case ACL_GROUP:
                if ((gid_entry = acl_get_qualifier(entry)) == NULL)
                    errExit("acl_get_qualifier");
                if (gid == -1 || gid != *gid_entry)  // is this the GID entry we're looking for?
                    break;
                if (acl_get_permset(entry, &permset) == -1)
                    errExit("get_permset");
                group_perm_mod = permset_to_mod(permset);
                break;
            case ACL_MASK:
                if (acl_get_permset(entry, &permset) == -1)
                    errExit("get_permset");
                mask_perm_mod = permset_to_mod(permset);
                break;
            case ACL_OTHER:
                if (acl_get_permset(entry, &permset) == -1)
                    errExit("get_permset");
                other_perm_mod = permset_to_mod(permset);
                break;
            default:
                errExit("Unknow tag type %ld", (long) tag);
        }
    }

    // apply mask
    group_obj_perm_mod &= mask_perm_mod;
    group_perm_mod &= mask_perm_mod;
    user_perm_mod &= mask_perm_mod;

    int joined_permissions = user_obj_perm_mod  | group_obj_perm_mod | user_perm_mod |
        group_perm_mod | other_perm_mod;

    int has_read_access = joined_permissions & 0x04;
    int has_write_access = joined_permissions & 0x2;
    int has_execute_access = joined_permissions & 0x1;

    printf("%c%c%c\n",
        has_read_access ? 'r' : '-',
        has_write_access ? 'w' : '-',
        has_execute_access ? 'x' : '-');

    if (acl_free(acl) == -1)
        errExit("acl_free");

    exit(EXIT_SUCCESS);
}
