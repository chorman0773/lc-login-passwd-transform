#include <stdio.h>

#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

#include <error.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>


int main(int argc,const char** argv) {
    const char DOTDOT_DOTDOT_GROUPS[] = "../../groups/";
    if(argc<4)
        error(1,0,"Usage: %s <passwd-file> <group-file> <directory for {users,groups}>",argv[0]);
    FILE* passwd = fopen(argv[1],"r");
    FILE* group = fopen(argv[2],"r");
    int into = open(argv[3],O_RDONLY|O_DIRECTORY);
    if(into<0)
        error(1,errno,"Failed to open directory %s",argv[3]);
    if(mkdirat(into,"users",0777)<0&&errno!=EEXIST)
        error(1,errno,"Failed to create  directory %s/users",argv[3]);
    int users = openat(into,"users",O_RDONLY|O_DIRECTORY);
    if(users<0)
        error(1,errno,"Failed to open directory %s/users",argv[3]);
    if(mkdirat(into,"groups",0777)<0&&errno!=EEXIST)
        error(1,errno,"Failed to create  directory %s/groups",argv[3]);
    int groups = openat(into,"groups",O_RDONLY|O_DIRECTORY);
    if(groups<0)
        error(1,errno,"Failed to open directory %s/users",argv[3]);
    char buff[4096];
    while(fgets(buff,4096,passwd)!=NULL){
        if(strlen(buff)!=0&&buff[strlen(buff)-1]=='\n')
            buff[strlen(buff)-1]='\0';
        char* name = strtok(buff,":");
        strtok(NULL,":"); // Discard the password
        char* uid = strtok(NULL,":");
        char* gid = strtok(NULL,":");
        char* uinfo = strtok(NULL,":");
        char* home = strtok(NULL,":");
        char* shell = strtok(NULL,"");
        if(shell==NULL||strlen(shell)==0)
            shell = "/bin/sh";
        if(mkdirat(users,uid,0777)<0&&errno!=EEXIST)
            error(1,errno,"Failed to create directory %s/users/%s",argv[3],uid);
        int dir = openat(users,uid,O_RDONLY|O_DIRECTORY);
        if(dir<0)
            error(1,errno,"Failed to open directory %s/users/%s",argv[3],uid);
        if(symlinkat(uid,users,name)<0)
            error(1,errno,"Failed to create symbolic link %s/users/%s -> %s",argv[3],name,uid);
        int namef = openat(dir,"name",O_WRONLY|O_CREAT,0666);
        if(namef<0)
            error(1,errno,"Failed to create file %s/users/%s/name",argv[3],uid);
        if(write(namef,name,strlen(name))<0)
            error(1,errno,"Failed to create file %s/users/%s/name",argv[3],uid);
        close(namef);
        char* gpath = malloc(strlen(gid)+sizeof(DOTDOT_DOTDOT_GROUPS));
        strcpy(gpath,DOTDOT_DOTDOT_GROUPS);
        strcat(gpath,gid);

        if(symlinkat(gpath,dir,"group")<0)
            error(1,errno,"failed to create symlink %s/users/%s/group",argv[3],uid);
        if(symlinkat(home,dir,"home")<0)
            error(1,errno,"failed to create symlink %s/users/%s/home",argv[3],uid);
        if(symlinkat(shell,dir,"shell")<0)
            error(1,errno,"failed to create symlink %s/users/%s/group",argv[3],uid);
        int userinfof = openat(dir,"user-info",O_WRONLY|O_CREAT,0666);
        if(userinfof<0)
            error(1,errno,"Failed to create file %s/users/%s/user-info",argv[3],uid);
        if(write(userinfof,uinfo,strlen(uinfo))<0)
            error(1,errno,"Failed to create file %s/users/%s/user-info",argv[3],uid);
        close(userinfof);
        close(dir);
    }
    close(users);
    fclose(passwd);

    while(fgets(buff,4096,group)!=NULL){
        if(strlen(buff)!=0&&buff[strlen(buff)-1]=='\n')
            buff[strlen(buff)-1]='\0';
        char* name = strtok(buff,":");
        strtok(NULL,":");
        char* gid = strtok(NULL,":");
        char* members = strtok(NULL,"");

        if(mkdirat(groups,gid,0777)<0)
            error(1,errno,"Failed to create directory %s/groups/%s",argv[3],gid);
        int dir = openat(groups,gid,O_RDONLY|O_DIRECTORY);
        if(dir<0)
            error(1,errno,"Failed to open %s/groups/%s",argv[3],gid);
        if(symlinkat(gid,groups,name)<0)
            error(1,errno,"Failed to create symbolic link %s/groups/%s -> %s",argv[3],name,gid);


        {
            int namef = openat(dir, "name", O_WRONLY | O_CREAT, 0666);
            if (namef < 0)
                error(1, errno, "Failed to create %s/groups/%s/name", argv[3], gid);
            if (write(namef, name, strlen(name)) < 0)
                error(1, errno, "Failed to create %s/groups/%s/name", argv[3], gid);
            close(namef);
        }
        if(members)
        {
            int membersf = openat(dir, "members", O_WRONLY | O_CREAT, 0666);
            if (membersf < 0)
                error(1, errno, "Failed to create %s/groups/%s/members", argv[3], gid);
            if (write(membersf, members, strlen(members)) < 0)
                error(1, errno, "Failed to create %s/groups/%s/members", argv[3], gid);
            close(membersf);
        }

    }


    return 0;
}
