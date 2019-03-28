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

Node Tree[256] = {0};
int Sum[256] = {0};

void error_die(const char *msg);
void create_tree(int N);
void encode(int i, struct code s);
int decode(int i, unsigned char bit);
struct code buf_add(struct code buf, FILE *fp_in);
unsigned int getmask(int len);


void error_die(const char *msg){
	perror(msg);
	exit(-1);
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

int decode(int i, unsigned char bit){
	if(bit == 0)
		return Tree[i].lchild;
	else
		return Tree[i].rchild;
}

struct code buf_add(struct code buf, FILE *fp_in){
	struct code inbyte = {0};
	if(fscanf(fp_in, "%c", (char *)&inbyte.symbol) == -1)
		return buf;
	inbyte.len = 8;
	buf.symbol <<= inbyte.len;
	buf.len += inbyte.len;
	buf.symbol |= inbyte.symbol;
	return buf;
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
//------   recreat tree-----------------------
	int N = 0;
	fscanf(fp_in, "%c", (char *)&N);

	char *p = NULL;
	int i=0;
	while(i<N){
		p = (char *)&Tree[i];
		while(p != (char *)&Tree[i+1])
			fscanf(fp_in, "%c", p++);
		++i;
	}
	create_tree(N);

	struct code e;
	e.symbol = 0;
	e.len = 1;
	encode(Tree[2*N-2].lchild, e);
	e.symbol = 1;
	e.len = 1;
	encode(Tree[2*N-2].rchild, e);

	/*for (int i = 0; i < 2*N-1; ++i){
		printf("%d d:%d p:%d w:%d s:%d l:%d c:%d %d\n", i,  Tree[i].data, \
			Tree[i].parent, Tree[i].weight, Tree[i].symbol.symbol, \
			Tree[i].symbol.len, Tree[i].lchild, Tree[i].rchild);
	}*/
//----Count all node----------------------------
	unsigned int Sum = 0, count = 0;
	for (int i = 0; i < N; ++i)
		Sum += Tree[i].weight;

//----------------------------------------------
	struct code buf = {0};
	unsigned int mask, bit;
	int pos;
	buf = buf_add(buf, fp_in);
	while(buf.len > 0){
		i = 2*N-2;//from root
		pos = buf.len;//buf_len
		mask = 1 << pos;//bit mask
		bit = buf.symbol & mask;
		while(Tree[i].lchild != -1){//if node is leaf
			if(--pos < 0){//if buf is empty buf_add
				buf = buf_add(buf, fp_in);
				pos += 8;
				mask <<= 8;
			}
			mask >>= 1;
			bit = buf.symbol & mask;//ensure bit
			i = decode(i, bit);//get child tree node
		}
		if(++count <= Sum)
			fprintf(fp_out, "%c", Tree[i].data);
		//printf("%c %d\n", Tree[i].data, Tree[i].symbol.len);
		buf.len -= Tree[i].symbol.len;//update buf_len
		buf.symbol &= getmask(buf.len);//update buf_data
		//if(buf.len >-1)
		//printf("%d %d %d\n", buf.len, Tree[i].symbol.len, mask);
		if(buf.len == 0)//file maybe not end
			buf = buf_add(buf, fp_in);
	}

	fclose(fp_out);
	fclose(fp_in);
	return 0;
}
