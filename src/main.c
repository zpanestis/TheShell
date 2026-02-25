#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <linux/limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

char **parse_input(char str[]);
void execute_command(char **args);
int handle_builtin(char **args);
void makeTree(char *path);

int main(void){
    
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags =0;

    sigaction(SIGINT,&sa,NULL);
    char buffer[1024];
    while(1){
        printf(">");
        if(fgets(buffer, 1024,stdin)== NULL){
            printf("\nSomething went wrong, buffer is null.\n");
            break;
        }
        
        buffer[strcspn(buffer, "\n")] = '\0';

        if(buffer[0]=='\0'){
            continue;
        }

        if(strcmp(buffer,"exit")==0){
            break;
        }
                                                           
        char **args = parse_input(buffer);

        if(handle_builtin(args)){ // handle_builtin returns 1 if not builtin
            execute_command(args);
        }
        

        free(args);
    }
    printf("The Shell Closed!\n");
    
    return 0;
}

char **parse_input(char str[]){
    char**A=NULL;
    char* token = strtok(str, " ");
    int c=0; //counts size of A
    while (token != NULL){
        A = realloc(A,sizeof(char *)*(c+1));
        A[c] = token;
        token=strtok(NULL," ");
        c++;
    }
    A = realloc(A,sizeof(char *)*(c+1));
    A[c] = NULL;

    return A;
}

void execute_command(char **args){

    pid_t pid = fork();
    if(pid == -1 ){
        printf("Fork failed successfully...");
        return;
    }
    

    if(pid== 0){
        int i=0;
        while (args[i]!= NULL){
            if (strcmp(args[i],">") == 0 ){
                int fd =open(args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);//0644 means that owner can write and everyone else reads
                dup2(fd,STDOUT_FILENO);//file descriptor 1 point to the open file
                close(fd);
                args[i] = NULL;
                break;
            }
            else if( strcmp(args[i],"<") == 0){
                int fd = open(args[i+1], O_RDONLY);
                dup2(fd,STDIN_FILENO);
                close(fd);
                args[i] = NULL;
                break;
            }
            i++;
        }

        execvp(args[0],args);
        printf("Command: \"%s\" not found.\n",args[0]);
        _exit(1);
    }else{
        waitpid(pid,NULL,0);
    }
}

int handle_builtin(char **args){
    char *dir;
    if(strcmp(args[0],"cd")==0){
        if (args[1] == NULL) {
            dir = getenv("HOME");
        } else {
            dir = args[1];
        }
    if(chdir(dir) == 0){
           printf("You are now in: %s\n", dir); 
        }else{
            printf("Error changing to: %s\n",dir);
        }
    }    
    else if(strcmp(args[0],"pwd")==0){
        char cwd[PATH_MAX];
        if(getcwd(cwd, sizeof(cwd))!=NULL){
            printf("You are in: \n\t%s\n",cwd);
        }else{
            printf("Error on getting the current directory\n");
        }
    }else if(strcmp(args[0],"tree")==0){
        printf("Tree is being created!\n");
        char path[PATH_MAX] = ".";
        makeTree(path);

    }
    else{
        return 1;
    }
    
    return 0;
}

char indent[100]="";

void makeTree(char *current_path){
    strcat(indent,"-");
    DIR *dir = opendir(current_path);
    if(dir == NULL){
        printf("Could not see directory...");
        return;
    }
    struct dirent *entry;
    
    while ((entry =readdir(dir))!=NULL ){
        if(strcmp(entry->d_name,".")!=0 && strcmp(entry->d_name,"..")!=0){
            printf("%s%s\n",indent,entry->d_name);
            if(entry->d_type == DT_DIR){ //is dyrectory (another file)
                char path[PATH_MAX];
                snprintf(path, sizeof(path), "%s/%s", current_path, entry->d_name);
                makeTree(path);
                
            }
             
        }
        
    }
    indent[strlen(indent) - 1] = '\0';

    closedir(dir);
}

