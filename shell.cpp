#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<limits.h>

#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>

#include<dirent.h>

#define BUFF_SIZE 512

char buff[BUFF_SIZE],buff1[BUFF_SIZE],input[BUFF_SIZE];//buff数组存储用户指令
char *envpath[10];
int flag=0;
pid_t pid1=0;

int exec(char *);
void setflag();
int is_found()

int main(){

	while(1){
	  char c,*arg[20],*temp,*temp1,*argv,exec_temp[BUFF_SIZE];
	  int i=0,j=0,is_bg=0,pid=0;
	  memset(buff,'/0',BUFF_SIZE);
	  memset(buff1,'/0',BUFF_SIZE);
	  memset(imput,'/0',BUFF_SIZE);
	  memset(exec_temp,'/0',BUFF_SIZE);
	  
	  arg[0]=(char*)malloc(BUFF_SIZE);
	  //获取用户命令
	  while((c=getchar())==' '||c=='/t'||c==EOF);
	  if(c=='/n') continue;
	  while(c!='/n'){
	      buff[i]=c;
		  i++;
	      c=getchar();
	  }
	buff[i]='/0';
	strcpy(input,buff);
	strcpy(arg[0],buff);

	temp=(char*)malloc(BUFF_SIZE);
	strcpy(temp,arg[0]);
	temp1=(char*)malloc(BUFF_SIZE);
	strcpy(temp1,temp);
	temp1=strtok(temp," ");

	while(temp1!=NULL){
	   j++;
	   arg[j]=(char*)malloc(BUFF_SIZE);
	   strcpy(arg[j],temp1);
	   temp1=strtok(NULL," ");
	}

	 strcpy(exec_temp,(char*)get_current_dir_name());
	 if(strcmp((char *)get_current_dir_name(),"/")!=0) 
     strcat(exec_temp,"/"); 


	 for(k=0;k<=i;k++){
		 if(arg[1][k]=='&'&&arg[1][k+1]=='/0'){
		 is_bg=1;
		 arg[1][k]='/0';
		 break;	 
		 } 
	 }
	 strcat(exec_temp,arg[1]);


	 if(is_found(arg[1])==0){
	 printf("the commond can not find/n");
	 for(i=0;i<j;i++)
		 free(arg[i]);
	 continue;
	 }
	 //命令执行
	 if((pid=fork())==0){
	   if(is_bg==1)
	      while(sig_flag==0)
			  signal(SIGUSR1,setflag); 
	 
	   flag=0;
	   if(j==2) execl(buff1,buff1,NULL); 
	   else if(j==3) execl(buff1,buff1,arg[2],NULL);
	   else if(j==4) execl(buff1,buff1,arg[2],arg[3],NULL);
	   else if(j==5) execl(buff1,buff1,arg[2],arg[3],arg[4],NULL);
	}else{
	   pid1=pid;
	   if(is_bg==1){
	   add_node(input,pid1);
	   kill(pid,SIGUSR1);
	   pid1=0;
	   }
	    if(is_bg==0)
	    waitpid(pid,&status,0); 
	}
	 if(is_bg==1)
		 sleep(1);
	 for(i=0;i<j;i++)
		 free(arg[i]);

    }
}
//查询路径下的文件是否存在且可执行
int exec(char *path){
	struct stat statbuff;
	int stream;
	if((stream=open(path,O_RDONLY))==-1){
		return 0;
	}
    fstat(stream,&statbuf);
    close(stream);
   if   (statbuf.st_mode&S_IXUSR) return 1;
   return 0;
}

//查找外部命令所对应文件是否存在
int is_found(char *command){
     int i=0;
	 struct dirent *direntp;
	 DIR *dir; 
	 while(i<sum){
		 if((dir=opendir(envpath[i]))==NULL){
		  printf("opendir error!/n"); 
		  exit(0);
		 }
		 while((direntp=readdir(dir))!= NULL){
			 if(strcmp(cmd,direntp->d_name)==0) {
			   strcpy(buff1,envpath[i]);
			   strcat(buff1,"/");
			   strcat(buff1,direntp->d_name);
			   strcpy(buff,buff1); 
			   closedir(dir);
			   return 1;
			 }
		 
		 }
	    i++;
	 
	 }
		return 0;	
}
void setflag(){  
	flag=1;
}