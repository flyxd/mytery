#define  _GNU_SOURCE
#include<errno.h> 
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
#define NO_PIPE -1  
#define FD_READ 0  
#define FD_WRITE 1  
#define HISNUM 10 

typedef struct NODE{  
    pid_t pid;  
    char cmd[100];  
    char state[10];  
    struct NODE *link;  
}LIST;//jobs链表

typedef struct ENV_HISTROY{  
    int start;  
    int end;  
    char his_cmd[HISNUM][100];  
} ENV_HISTORY;

char buff[BUFF_SIZE],buff1[BUFF_SIZE],input[BUFF_SIZE];//buff数组存储用户指令
char *envpath[10];
int flag=0,sum=4;
int sig_z=0;
pid_t pid1=0;
LIST *head,*end; 
ENV_HISTORY envhis;

int exec(char *path);
void setflag();
int is_found();
void getenviron();
void init_environ();

void jobs_cmd();
void add_node();
void del_node();
//void ctr_z();

void add_history();
void history_cmd();

void cd_cmd();
//int get_line(int fd,char *buf);

int main(){
             init_environ();    //chushihua
	while(1){
	  char c,*arg[20],*temp,*temp1,*argv,exec_temp[BUFF_SIZE];
	  int i=0,j=1,is_bg=0,pid=0,k=0,status=0;
        int is_pr=0;

    if(printf("linux@%s>",get_current_dir_name())==EOF)  
        {  
            fflush(stdout);  
            continue;  
        }  
        fflush(stdout);  

	  memset(buff,'\0',BUFF_SIZE);
	  memset(buff1,'\0',BUFF_SIZE);
	  memset(input,'\0',BUFF_SIZE);
	  memset(exec_temp,'\0',BUFF_SIZE);
	  
	  arg[0]=(char*)malloc(BUFF_SIZE);
	  //获取用户命令
	  while((c=getchar())==' '||c=='\t'||c==EOF);
	  if(c=='\n') continue;
	  while(c!='\n'){
	      buff[i]=c;
		  i++;
	      c=getchar();
	  }
	buff[i]='\0';
	strcpy(input,buff);
	strcpy(arg[0],input);

	temp=(char*)malloc(BUFF_SIZE);
	strcpy(temp,arg[0]);
	temp1=(char*)malloc(BUFF_SIZE);
	//strcpy(temp1,temp);
	temp1=strtok(temp," ");

	while(temp1!=NULL){
	   arg[j]=(char*)malloc(BUFF_SIZE);
	   strcpy(arg[j],temp1);
	   temp1=strtok(NULL," ");
	   j++;
	}

	 strcpy(exec_temp,(char*)get_current_dir_name());
	 if(strcmp((char *)get_current_dir_name(),"/")!=0) 
         strcat(exec_temp,"/"); 
//////////////////////////////////////////////////////////////////////////////////////
        for(k=0;k<=i;k++)  
        {  
            if(arg[0][k]=='<'||arg[0][k]=='>'||arg[0][k]=='|')  
            {  
                if (arg[0][k]=='|')  
                {  
                    pipel(arg[0],i);  
                    //printf("检测%c/n",input[i]);  
                    add_history(arg[0]);  //add_history加入链表
                }  
                else  
                {  
                    redirect(arg[0],i);  
                    //printf("检测%c/n",input[i]);  
                    add_history(arg[0]);   //add_history加入链表
                }  
                is_pr=1;     //将标志管道和重定向的设置为1  
                break;  
            }   
        }  
        if(is_pr==1)      
        {  
            continue;  
        }



	 for(k=0;k<=i;k++){
		 if(arg[1][k]=='&'&&arg[1][k+1]=='\0'){
		 is_bg=1;
		 arg[1][k]='\0';
		 break;	 
		 } 
	 }
//////////////////////////////////////////////////////////////////////////////////////
	 strcat(exec_temp,arg[1]);

//////////////////////////////////////////////////////////////////////////////////////
       if(strcmp("jobs",arg[1])==0){
               add_history(arg[0]);
               jobs_cmd();  
               continue;
         }else if(strcmp("history",arg[0])==0){
                history_cmd();  
                add_history(arg[0]);  
                continue;
         }else if(strcmp("history",arg[1])==0&&strcmp("-c",arg[2])==0){
                     envhis.start=0;
                     envhis.end=0;
                     add_history(arg[0]);
                     continue;
         }else if(strcmp("exit",arg[0])==0||strcmp("logout",arg[0])==0){
                add_history(arg[0]);
                printf("good bye\n");
                break;
           }else if(strcmp("cd",arg[1])==0){
                 cd_cmd(arg[2]);
                 add_history(arg[0]);
                 continue;
           }else if(exec(exec_temp)==1){
               add_history(arg[0]);  
                if((pid=fork())==0){     
                    execl(exec_temp,exec_temp,NULL);  
                }  
                else if(pid>0){  
                    wait(NULL);  
                }  
                continue;

             }

	 if(is_found(arg[1])==0){
	 printf("the commond can not find\n");
	 for(i=0;i<j;i++)
		 free(arg[i]);
	 continue;
	 }
	 //命令执行
       add_history(arg[0]);
	 if((pid=fork())==0){
	   if(is_bg==1)
	      while(flag==0)
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
    fstat(stream,&statbuff);
    close(stream);
   if   (statbuff.st_mode&S_IXUSR) return 1;
   return 0;
}

//查找外部命令所对应文件是否存在
int is_found(char *command){
     int i=0;
	 struct dirent *direntp;
	 DIR *dir; 
	 while(i<sum){
		 if((dir=opendir(envpath[i]))==NULL){
		  printf("opendir error!\n"); 
		  exit(0);
		 }
		 while((direntp=readdir(dir))!= NULL){
			 if(strcmp(command,direntp->d_name)==0) {
			   strcpy(buff1,envpath[i]);
			  // strcat(buff1,"\");
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
//////////////////////////////////////////////////////////////////////////////////////
void setflag(){  
	flag=1;
}

int get_line(int fd,char *buf){
    int i=0;  
    char ch;  
    while (read(fd,&ch,1))  
    {  
        buf[i]=ch;
        i++;  
        if (ch=='\n')  
        {  
            buf[i-1]='\0';  
            return i;  
        }  
    }  
    return i; 


}

/*
void getenviron(int n,char *s){
      int i=0,j=0;
      char ch,buff[80],*p;
      while ((ch=s[i])!='='){
        buff[i]=ch;
        i++;
        }      
        buff[i]='\0';
        i++;
        if (strcmp(buff,"PATH")==0){
            while (s[i]!='\0'){
                if (s[i]==':')  
            {                              //用“：”分隔查找路径,并设置到envpath[]中 
                buff[j++]='/';  
                buff[j]='\0';   //串尾标识符  
                p=(char *)malloc(strlen(buff)+1);  
                strcpy(p,buff);  
                envpath[sum++]=p;  //将路径保存在envpath[sum]中 
                envpath[sum]=NULL; //字符串尾置空
                j=0;               //为下一个envpath[i]作准备
                i++;  
            }else{
           buff[j]=s[i];
           i++;
           j++;
               }  
            }
         }
      else fprintf(stderr,"No match");
}

void init_environ(){
    int fd,n,i;
    char buf[80];
    if((fd=open("/root/test_profile",O_RDONLY))==0){
     // if((fd=open("/etc/profile",O_RDONLY))==0){
        perror("Can not open test_profile file!!!");
        fflush(stderr);
     }
     while (n=get_line(fd,buf))  
        getenviron(n,buf);


}*/

void init_environ(){
	envpath[0]="/bin";
	envpath[1]="/sbin";
	envpath[2]="/usr/bin";
	envpath[3]="/usr/sbin";
      head=end=NULL;
      envhis.start=0;
      envhis.end=0;
}
////////////////////////////////////////////////////////////////////////////////////pipe
int pipel(char *input,int len)  
{  
    char *argv[10][30],*filename[0];  
    int i,j,k,is_bg=0;  
    int yan_cmd=0,fd[10][1],pipe_in=-1,pipe_out=-1,flag=0;  //fd[10][1]
    pid_t pid;  
  

    for (i=0,j=0,k=0;i<=len;i++)   
    {  
        if(input[i]==' '||input[i]=='\t'||input[i]=='\0'||input[i]=='|'  
           ||input[i]=='>'||input[i]=='\n')   
        {  
            if (input[i]=='|'||input[i]=='>')   
            {  
                if (input[i]=='>') flag=1;  
                if (j>0)   
                {  
                    buff[j++]='\0';  
                    argv[yan_cmd][k]=(char *)malloc(sizeof(char)*j);  
                    strcpy(argv[yan_cmd][k],buff);  
                    k++;  
                }  
                argv[yan_cmd][k]=(char *)0;  
                yan_cmd++;  
                k=0;  
                j=0; 
            }  
            if (j==0)  
                continue;  
            else{  
                buff[j++]='\0';  
                if (flag==0)   
                {                                            
                    argv[yan_cmd][k]=(char *)malloc(sizeof(char)*j);  //flag=0,命令中不含重定向命令
                    strcpy(argv[yan_cmd][k],buff);  
                    k++;  
                }  
                else   
                { 
                    filename[0]=(char *)malloc(sizeof(char)*j);  //flag=1,命令中包含重定向命令,重定向后为文件名
                    strcpy(filename[0],buff);  
                }  
            }  
            j=0; //为下一次作准备  
        }else{  
            if (input[i]=='&'&&input[i++]=='\0'){//带后台符号的命令
                is_bg=1;  
                continue;  
            }  
            buff[j++]=input[i];  
        }  
    }  
    argv[yan_cmd][k++]=(char *)0;//最后一参数置空 
  
    for (i=0;i<=10;i++)   
    { //初始化文件描述符 
        fd[i][FD_READ]=NO_PIPE;  
        fd[i][FD_WRITE]=NO_PIPE;  
    }  
  
    for (i=0;i<yan_cmd;i++)   
    { //联结命令和管道 
        if (pipe(fd[i])==-1)   
        {  
            printf("Can not open pipe!\n");  
            return 0;  
        }  
    }  
  
    //寻找命令文件,执行命令 
    for (i=0;i<=yan_cmd;i++)   
    {  
        if (is_found(argv[i][0])==0)   
        {  
            printf("Can not found the command!\n");  
            break;  
        }  
  
        //将pipe_in指向到管道的读端,不考虑第一条命令  
        if (i!=0)  
            pipe_in=fd[i-1][FD_READ];  
        else pipe_in=NO_PIPE;//第一条命令  
  
        //将pipe_out指向到管道的写端,不考虑最后一条命令 
        if (i!=yan_cmd)  
            pipe_out=fd[i][FD_WRITE];  
        else   
            if (flag==1)   
            {//包含重定向命令 
                //将pipe_out指向到文件  
                if ((pipe_out=open(filename[0],  
                    O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR))==-1)  
                {  
                   printf ("Can not open %s \n",filename[0]);  
                   break;  
                }  
            }  
            else pipe_out=NO_PIPE; //最后一条命令
  
        if ((pid=fork())<0)  
        {  
            printf("Fork failed!\n");  
            return 0;  
        }  
  
        if (pid==0)   
        { //父进程 
            if (pipe_in==NO_PIPE)  
                close(pipe_in);  
            if (pipe_out==NO_PIPE)  
                close(pipe_out);  
            if (pipe_out!=NO_PIPE)   
            {  
                //将标准输出重定向到管道的写端,将该子进程的输出变为写入管道了 
                dup2(pipe_out,1);  
                close(pipe_out);  
            }  
  
            if (pipe_in!=NO_PIPE)   
            {  
                //将标准输入重定向到管道的读端,让管道的数据可以被读到  
                dup2(pipe_in,0);  
                close(pipe_in);  
            }  
            execv(buff,argv[i]);//调用execv() 
        }  
        else   
        {  
            if (is_bg==0)  
                waitpid(pid,NULL,0);  
                close(pipe_in);  
                close(pipe_out);  
        }  
    }  
    return 0;  
  
} ///////////////////////////////////////////////////////////////////////////////////////////////////////// redirect
int redirect(char *in,int len)  
{  
    char *argv[20],*filename[20];  
    pid_t pid;  
    int i,j,k,fd_in,fd_out,is_in=-1,is_out=-1,num=0;//num存放重定向符号的出现次数 
    int is_back=0,status=0;  
  
    /*命令解析*/  
    //argv[]用于存放命令和参数,filename用于存放重定向文件名,is_in, 
      //is_out分别是重定向输入标记和输出标记
    for (i=0,j=0,k=0;i<=len;i++)   
    {  
        if (in[i]==' '||in[i]=='\t'||in[i]=='\0'||in[i]=='<'||in[i]=='>')   
        {  
            if (in[i]=='>'||in[i]=='<')   
            {  
                if (num<3)  
                {  
                    num++;  
                    if (in[i]=='<')  
                        is_in=num-1; //存在重定向输入is_in置-1 
                    else  
                      is_out=num-1; //存在重定向输出is_out置-1  
  
                    if (j>0 && num==1)  
                    {//处理命令和重定向符号相连的问题  
                        buff[j++]='\0';  
                        argv[k]=(char *)malloc(sizeof(char)*j);  
                        strcpy(argv[k],buff);  
                        k++;  
                        j=0;//为读取下一命令或参数作准备  
                    }  
                }  
                else   
                {  
                    printf("Error command!\n");  
                    return 0;  
                }  
            }  
            if (j==0)  
                continue;  
            else  
            {  
                buff[j++]='\0';  
                if (num==0)   
                { //没有遇到重定向符号,字符串是参数或命令  
                    argv[k]=(char *)malloc(sizeof(char)*j);  
                    strcpy(argv[k],buff);  
                    k++;  
                    j=0;  
                }  
                else   
                { //遇到重定向符号，字符串是文件名  
                    filename[status]=(char *)malloc(sizeof(char)*j);  
                    strcpy(filename[status++],buff);  
                    j=0;  
                }  
            }  
        }  
        else   
        { //父进程  
            if (in[i]=='&' && in[i+1]=='\0')   
            { //是否为后台命令  
                is_back=1;  
                continue;  
            }  
            buff[j++]=in[i];  
        }  
    }  
  
    //寻找命令文件 
    argv[k]=(char *)malloc(sizeof(char));  
    argv[k]=(char *)0;//最后一参数置空,也可用'\0'  
  
    if (is_found(argv[0])==0)   
    { //查找命令文件  
        printf("The command can not find!!\n");  
        for (i=0;i<=k;i++)/*释放空间*/  
            free(argv[i]);  
        return 0;  
    }  
  
    //命令的执行  
    if ((pid=fork())==0)   
    {  
        if (is_out!=-1)   
        { //存在输出重定向  
            if ((fd_out=open(filename[is_out],O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR))==-1)   
            {//将文件描述符fd_out指向文件  
                  printf("Can not open %s \n",filename[is_out]);  
                  return 0;  
            }  
        }  
          
        if (is_in!=-1)   
        { //存在输入重定向  
            if ((fd_in=open(filename[is_in],O_RDONLY,S_IRUSR|S_IWUSR))==-1)  
            //将文件描述符fd_in指向文件  
            {  
                printf("Can not open %s \n",filename[is_in]);  
                return 0;  
            }  
        }  
  
        if (is_out!=-1)  
        {  
           //使用dup2函数将标准输出重定向到fd_out上， 
             // 将要输出到标准输出的内容，输出到fd_out所指向的文件--重定向
            if (dup2(fd_out,STDOUT_FILENO)==-1)   
            {  
                printf("Redirect standard out error!\n");  
                exit(1);  
            }  
        }  
  
        if (is_in!=-1)   
        {  
            //使用dup2函数将标准输入重定向到fd_in上， 
              //将原先从标准输入输入的内容，改为从fd_in所指向的文件输入--重定向  
            if (dup2(fd_in,STDIN_FILENO)==-1)   
            {  
                printf("Redirect standard in error!\n");  
                exit(1);  
            }  
        }  
  
        execv(buff,argv);  
    }  
    else   
    { //父进程  
        if (is_back==0)//判断是否需要等待子进程的完成  
            waitpid(pid,&status,0);  
    }  
  
    //释放空间  
    for (i=0;i<=k;i++)  
        free(argv[i]);  
    if (is_out!=-1)   
    {  
        free(filename[is_out]);  
        close(fd_out);  
    }  
    if (is_in!=-1)   
    {  
        free(filename[is_in]);  
        close(fd_in);  
    }  
    return 0;  
} 


///////////////////////////////////////////////////////////////////job链表相关函数
void jobs_cmd()  
{  
    LIST *p;  
    int i=1;  
    p=head;  
    /*遍历链表，显示相关内容*/  
    if (head!=NULL) {   /*链表不为空*/  
        do {  
            printf("%d  %d  %s\t%s\n",i,p->pid,p->state,p->cmd);  
            i++;  
            p=p->link;  
        }while (p!=NULL);  
    } else printf("No jobs!\n");/*链表为空*/  
}  
  
void add_node(char *input_cmd,int node_pid)  
{  
     /*申请新节点*/  
        LIST *p;  
        p=(LIST *)malloc(sizeof(LIST));  
        /*设置节点信息*/  
        p->pid=node_pid;  
        /*设置工作状态*/  
        strcpy(p->state,"Running");  
        /*保存命令信息*/  
        strcpy(p->cmd,input_cmd);  
        p->link=NULL;  
    /*若链表为空*/  
        if(head==NULL){  
                head=p;  
                end=p;  
        }  
        /*链表不为空*/  
        else{  
                /*将链表尾节点指针指向新节点*/  
                end->link=p;  
                /*将end指向链表尾*/  
                end=p;  
        }  
}  
  
  
void del_node(int sig,siginfo_t *sip)/*删除节点*/  
{  
    LIST *q,*p;  
    int id;  
    /*当按下ctrl-z时，也会收到SIGCHLD信号，这种情况不删除节点*/  
    if (sig_z==1) {
      sig_z=0;
      return; }  
      
    id=sip->si_pid;/*获得发送SIGCHLD信号的进程的进程号*/  
    p=q=head;  
    if (head==NULL) return; /*链表为空*/  
    /*遍历链表找到相应节点*/  
    while (p->pid!=id&&p->link!=NULL)/*找到要删除的节点p*/  
        p=p->link;  
    if (p->pid!=id) return;  
    if (p==head) {/*p为头节点*/  
        head=head->link;  
    }else {  
        while(q->link!=p)/*找到p的前驱节点q*/  
            q=q->link;  
        if (p==end) {/*p为尾节点*/  
            end=q;  
            q->link=NULL;  
        }else q->link=p->link;   
  
    }   
    free(p);  
     return;  
} 
/////////////////////////////////////////////////////////////////////////////////history
void add_history(char *inputcmd)  
{  
    envhis.end=(envhis.end+1)%HISNUM;//end前移一位  
    if (envhis.end==envhis.start){//end和start指向同一数组  
        envhis.start=(envhis.start+1)%HISNUM;//start前移一位  
    }  
   strcpy(envhis.his_cmd[envhis.end],inputcmd);//将命令复制到end指向的数组中
}

void history_cmd()  
{  
    int i,start,end,k=1;  
    start=envhis.start;  
    end=envhis.end;  
    if(start==0)  
        for(i=start;i<end;i++)  
        {  
            printf("     %d          %s\n",k,envhis.his_cmd[k]);  
            k++;  
        }     
    else if(end<start)  
    {  
        for(i=start;i<HISNUM;i++)  
        {  
             printf("     %d          %s\n",k,envhis.his_cmd[k]);  
             k++;  
        }  
        for(i=0;i<end;i++)         
        {     
             printf("     %d          %s\n",k,envhis.his_cmd[k]);  
             k++;  
        }         
    }  
    else if(start<end)  
    {  
        for(i=start;i<end;i++)  
        {  
             printf("     %d          %s\n",k,envhis.his_cmd[k]);  
             k++;  
        }  
    }  
}
///////////////////////////////////////////////////////////////////////////////////////cd
void cd_cmd(char *route)  
{   
    if(route!=NULL)  
    {   
        if(chdir(route)<0)   
            switch(errno)  
            {   
                case ENOENT:   
                    fprintf(stderr,"DIRECTORY NOT FOUND\n");   
                    break;   
                case ENOTDIR:   
                    fprintf(stderr,"NOT A DIRECTORY NAME\n");   
                    break;   
                case EACCES:   
                    fprintf(stderr,"YOU DO NOT HAVE RIGHT TO ACCESS\n");   
                    break;   
                default:   
                    fprintf(stderr,"SOME ERROR HAPPENED IN CHDIR\n");   
            }   
    }  
}

