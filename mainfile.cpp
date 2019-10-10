//Sanjoy Chowdhury 20172123
#include <iostream>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>


using namespace std;
int mode =0;
char filename[30];
vector<char> vec;
int linetrack=0;
int totallines=0;
char out[1000][1000];
int outman[1000];

void printscreen();
 vector<int> linechar;

void readfile(char *filename)
{
	FILE *fptr =fopen(filename, "r");
	
	if(fptr==NULL)
	{
	
      totallines=1;
      vec.push_back('a');
      vec.push_back('b');
      linechar.push_back(2);
   
}
else{
	 
	 char c =fgetc(fptr);
	 int count=0;
	 while (c != EOF)
    {  	if(c=='\n')
		{totallines++;
		count++;
		linechar.push_back(count);
		count=0;}
        
        vec.push_back(c);
        c = fgetc(fptr);
        count++;
    
}}
    fclose(fptr);
   // printscreen();
	
}

struct ourterm
{
struct termios y;
int screencol;
int screenrows;
int cx;
int cy;
};
struct ourterm z;
struct buffer
{
	char* str;
	int len;
};
#define ABUF_INIT {NULL, 0}
void rawmodeoff()
{
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &z.y);
	write(STDOUT_FILENO, "\033c", 3);
	write(STDOUT_FILENO, "\x1b[H", 3);
}
void getwindowsize()
{
	struct winsize ws;
	if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws)!=-1 && ws.ws_col !=0)
	{
		z.screenrows=ws.ws_row;
		z.screencol=ws.ws_col;
		}
	
	
}
void rawmodeon()
{
	
	tcgetattr(STDIN_FILENO, &z.y);
	struct termios x =z.y;
	x.c_lflag &= ~(ECHO);
	x.c_lflag &= ~(ICANON);
	x.c_iflag &= ~(IXON);
	//x.c_oflag &= ~(OPOST);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &x);

}
void addbuffer(struct buffer *bu,char *s,int len)
{
	char* nw = (char*)realloc(bu->str,bu->len+len);	
	if(nw == NULL)return;
	memcpy(&nw[bu->len],s,len);
	bu->str=nw;
	bu->len+=len;	
}
void freebuffer(struct buffer *bu){
	free(bu->str);
}
void outputinit(char out[1000][1000])
{
	for(int i=0;i<z.screenrows;i++)
	{
		for(int j=0; j<z.screencol;j++)
		{
			out[i][j]='\0';
		}
	}
}
void freebufferasync(struct buffer *bu){
	free(bu->str);
}
void addbufferasync(struct buffer *bu,char *s,int len)
{
	char* nw = (char*)realloc(bu->str,bu->len+len);	
	if(nw == NULL)return;
	memcpy(&nw[bu->len],s,len);
	bu->str=nw;
	bu->len+=len;	
}
void refreshscreen(const vector<char>&)
{
	struct buffer bu = ABUF_INIT ;
	addbuffer(&bu, "\033c", 3);
	addbuffer(&bu, "\x1b[H", 3);
	getwindowsize();
	int flag =1;
	unsigned int k=0;
	
	outputinit(out);
	int cex,cey;
	cex=z.screenrows;
	cey=z.screencol;
	int i,j;
	int count=0;
	while(count<linetrack)
	{
		if((vec[k++])=='\n')count++;
	}
	int lineflag=0;
	int x=-1;;
	for(i=0;i<z.screenrows-1;i++)
	{
		if(lineflag==0){ outman[i]= x++;}
		else{outman[i]=x;}
		lineflag=1;
		for( j=0; j<z.screencol;j++)
		{
			if(k==vec.size()-1){flag=0;break;}
			else if(vec[k]=='\n'){out[i][j]='\n';lineflag=0;k++;break;}
			else{out[i][j]=vec[k++];}
		}
		if(flag==2){cex=i;cey=j;break;}
	}

	char b;
	for(int i=0;i<=cex;i++)
	{
		for(int j=0; j<=cey;j++)
		{
		if(out[i][j]=='\n'){b='\n';char *a=&b;addbuffer(&bu,a,1);break;}
		else 
		{
		b=out[i][j];
		char *a=&b;
		addbuffer(&bu,a,1);
		}
	}
		
	}
	char buff[64];
	snprintf(buff,sizeof(buff),"\x1b[%d;%dH", z.cy+1,z.cx+1);
	addbuffer(&bu,buff,strlen(buff));
	
	
	//addbuffer(&bu, "\033c", 3);
	//addbuffer(&bu, "\x1b[H", 3);
	write(STDOUT_FILENO, bu.str, bu.len);
	
	freebuffer(&bu);
	
}
void movecursor(char a)
{
	switch (a)
	{
		case 'h':if(z.cx!=0)z.cx--;break;
		case 'j':if(z.cy!=z.screenrows-1){z.cy++;break;}else if(linetrack!=totallines-z.screenrows) linetrack++;break;
		case 'k':if(z.cy!=0){z.cy--;break;}else if(linetrack!=0) linetrack--;break;
		case 'l':if(z.cx!=z.screencol-1&&z.cx<=linechar[z.cy]-4)z.cx++;break;
}




}
void replace()
{
	char c;
	int pos=0;
	int alpha;
	if(z.cy==0)
	alpha=0;
	else alpha =1;
	int curralpha=outman[z.cy];
	for(int i=0;i<z.cy;i++)
	{
		if(outman[i]==curralpha)alpha++;
	}
	for(int i=0;i<outman[z.cy]+linetrack;i++)
	{
		pos+= linechar[i]-1;;
	}
	
	pos=z.cx+pos+alpha*z.screencol;
	read(STDIN_FILENO,&c,1);
	vec.erase(vec.begin()+pos);	
	vec.insert(vec.begin()+pos,c);
}
void insert()
{
	char c='\0';
	mode=1;
	int pos=0;
	
	int alpha;
	if(z.cy==0)
	alpha=0;
	else alpha =1;
	int curralpha=outman[z.cy];
	for(int i=0;i<z.cy;i++)
	{
		if(outman[i]==curralpha)alpha++;
	}
	for(int i=0;i<outman[z.cy]+linetrack;i++)
	{
		pos+=linechar[i]-1;;
	}
	pos=z.cx+pos+alpha*z.screencol;
	//while((int)c!=27)
	//{
	
	read(STDIN_FILENO,&c,1);
	if((int)c==27){mode =0;}
	else if((int)c==127){vec.erase(vec.begin()+pos);z.cx--;}
	else
	{
	vec.insert(vec.begin()+pos,c);
	linechar[z.cy+linetrack]++;
	z.cx++;	
	}
}
int gcount=0;
void filewrite()
{
	FILE *fp;
	fp=fopen(filename, "w");
	for(unsigned int ch = 0 ; ch <= vec.size(); ch++ )
   {
      fputc(vec[ch], fp);
   }
   fclose(fp);
}
void commandmode()
{
	char buff[64];
	int pid,p;
	snprintf(buff,sizeof(buff),"\x1b[%d;%dH", z.screenrows,1);
	write(STDOUT_FILENO,buff,strlen(buff));
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &z.y);
	char c[10];
	scanf("%s",c);
	if(strcmp(c,"wq")==0)
	{
		filewrite();
		exit(0);
		
	}
	else if(strcmp(c,"q")==0)
	{
		exit(0);
		
	}
	else if(strcmp(c,"w")==0)
	{
		filewrite();
		
	}
	
	else if(c[0]=='!')
	{
		if(pid=!fork())
		{	
			char* ch[3];
			ch[0]=c+1;
			ch[1]=NULL;
			write(STDOUT_FILENO,"\033c", 3);
			if(execvp(ch[0],ch)==-1)
			perror("exec");

			}
			else
			{
				char c;
				wait(&p);
				fprintf(stderr,"\nPress any hey to go back\n");
				read(STDIN_FILENO,&c,1);
				fprintf(stderr,"%c",c);
				
			}
		}
		
	
	
	
	rawmodeon();
}
void textprocessor(char a)
{
	
	
	switch(a)
	{	case 'h':
		case 'j':
		case 'k':
		case 'l':
				{
				gcount=0;
				movecursor(a);
				break;
				}
		case 'r':
		{
			gcount =0;
			replace();
			break;
		}
		case 'i':
		{
			gcount=0;
			insert();
			break;
		}
		case 'g':
		{
			if(gcount==0){gcount=1;}
			else if(gcount == 1)
			{
				linetrack=0;
				gcount=0;
				z.cx=0;
				z.cy=0;
			}
			break;
		}
		case 'G':

			{
				linetrack=totallines-z.screenrows;
				z.cx=0;
				z.cy=z.screenrows;
				gcount=0;
				break;
			}
		case ':':
			{
				commandmode();
			}
			
			
		}
	//else write(STDOUT_FILENO,&a,1);
}
void texthandler()
{
	char n ;
	if(mode ==0)
	read(STDIN_FILENO,&n,1);
	else n='i';
	textprocessor(n);
}

int main(int argc, char *argv[])
{
	rawmodeon();
	atexit(rawmodeoff);
	z.cx=0;
	z.cy=0;
	if (argc >= 2) {
    readfile(argv[1]);
    strcpy(filename,argv[1]);
  }
	while(1)
	{
		refreshscreen(vec);
		texthandler();
	}
}
