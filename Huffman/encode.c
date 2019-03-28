#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>


const char *infilename;
const char *outfilename;


struct code {
	unsigned int symbol;
	int len;
};

typedef struct Node{
	unsigned char data;
	struct code symbol;
	int weight;
	int parent;
	int lchild;
	int rchild;
}Node;

Node Tree[512] = {0};
int Sum[512] = {0};

void error_die(const char *msg);
void encode(int i, struct code s);
struct code get_symbol(unsigned char byte);
struct code buf_add(struct code buf, struct code inbyte);
unsigned int getmask(int len);
void create_tree(int N);

void error_die(const char *msg){
	perror(msg);
	exit(-1);
}
void encode(int i, struct code s){
	Tree[i].symbol = s;
	if(Tree[i].lchild != -1){
		struct code ls;
		ls.symbol = (s.symbol << 1) + 0;
		ls.len = s.len + 1;
		encode(Tree[i].lchild, ls);
	}
	if(Tree[i].rchild != -1){
		struct code rs;
		rs.symbol = (s.symbol << 1) + 1;
		rs.len = s.len + 1;
		encode(Tree[i].rchild, rs);
	}
}

struct code get_symbol(unsigned char byte){
	int i=-1;
	while(Tree[++i].data != byte)
		;
	return Tree[i].symbol;
}

struct code buf_add(struct code buf, struct code inbyte){
	buf.symbol <<= inbyte.len;
	buf.len += inbyte.len;
	buf.symbol |= inbyte.symbol;
	return buf;
}

void create_tree(int N){
	for (int i = 0; i < 2*N-1; ++i){
		Tree[i].parent = Tree[i].lchild = Tree[i].rchild = -1;
	}
	int s1, s2, min1, min2;
	for(int i = N; i < 2*N-1; ++i){
		s1 = s2 = 0;
		min1 = min2 = 0x0FFFFFFF;
		for(int j = 0; j < i; ++j){
			if(Tree[j].parent == -1){
				if(Tree[j].weight < min1){
					min2 = min1;
					s2 = s1;
					min1 = Tree[j].weight;
					s1 = j;
				}
				else if(Tree[j].weight < min2){
					min2 = Tree[j].weight;
					s2 = j;
				}
			}
		}
		Tree[i].lchild = s1;
		Tree[i].rchild = s2;
		Tree[s1].parent = Tree[s2].parent = i;
		Tree[i].weight = Tree[s1].weight + Tree[s2].weight;
	}
}

unsigned int getmask(int len){
	unsigned int mask = 1;
	while(--len >0){
		mask <<= 1;
		mask |= 1;
	}
	return mask;
}

int main(int argc, char const *argv[])
{
	if(argc != 3)
		return -1;
	infilename = argv[1];
	outfilename = argv[2];
	FILE *fp_in = fopen(infilename, "r");
	if(fp_in == NULL)
		error_die("fopen");

	struct stat st;
	if(!stat(outfilename, &st)){
		char cmd[80];
		sprintf(cmd, "rm %s", outfilename);
		system(cmd);
	}

	FILE *fp_out = fopen(outfilename, "a");
	
	//--------init tree-------
	unsigned char byte;
	while(fscanf(fp_in, "%c", &byte) != -1){
		++Sum[byte];
	}
	int i=-1, N=0;
	while(++i<256){
		if(Sum[i] == 0)
			continue;
		Tree[N].data = i;
		Tree[N].weight = Sum[i];
		N++;
	}
	//------------------------
	
	create_tree(N);
	//------encode tree-------
	struct code e;
	e.symbol = 0;
	e.len = 1;
	encode(Tree[2*N-2].lchild, e);
	e.symbol = 1;
	e.len = 1;
	encode(Tree[2*N-2].rchild, e);
	//------------------------
	/*for (int i = 0; i < 2*N-1; ++i){
		printf("%d d:%d p:%d w:%d s:%d l:%d c:%d %d\n", i,  Tree[i].data, \
			Tree[i].parent, Tree[i].weight, Tree[i].symbol.symbol, \
			Tree[i].symbol.len, Tree[i].lchild, Tree[i].rchild);
	}*/
	//----write_tree_head------
	fprintf(fp_out, "%c", N);
	i = -1;
	char *p = NULL;
	while(Tree[++i].lchild == -1){
		p = (char *)&Tree[i];
		while(p != (char *)&Tree[i+1]){
			fprintf(fp_out, "%c", *p);
			//printf("%d\n", *p);
			++p;
		}
	}
	fseek(fp_in, 0, 0);
	//-------------------------
	struct code buf = {0};

	while(fscanf(fp_in, "%c", &byte) != -1){
		struct code inbyte;
		inbyte = get_symbol(byte);
		buf = buf_add(buf, inbyte);
		while(buf.len >= 8){
			int lastnum = buf.len - 8;
			byte = buf.symbol >> lastnum;
			//printf("%d\n", byte);
			fprintf(fp_out, "%c", byte);
			unsigned int mask = getmask(lastnum);
			buf.symbol &= mask;
			buf.len = lastnum;
			//printf("%d %u\n", lastnum, mask);
		}
	}
	if(buf.len){
		int lastnum = 8 - buf.len;
		byte = buf.symbol << lastnum;
		fprintf(fp_out, "%c", byte);
	}
	//printf("%d\n", buf.len);
	fclose(fp_out);
	fclose(fp_in);
	return 0;
}
