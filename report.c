#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct Customer {
	int cid;
	char mname[20];
	struct Customer *next;
};

struct Order {
	int oid;
	int cid;
	char odate[20];
	struct Order *next;
};

struct LineItem {
	int oid;
	float price;
	char shipDate[20];
	struct LineItem *next;
};

struct CalcArgv {
	char mname[20];
	char odate[20];
	char shipDate[20];
	int limit;
	
	struct MiddleTableItem *mhead;
	struct MiddleTableItem *mtail;
	struct SumTableItem *shead;
	struct SumTableItem *stail;
};

struct MiddleTableItem {
	int oid;
	char odate[20];
	float price;
	struct MiddleTableItem *next;
};

struct SumTableItem {
	int oid;
	char odate[20];
	float revenue;
	struct SumTableItem *next;
};

struct Customer * customers;
struct Order * orders;
struct LineItem * lineitems;

struct CalcArgv calcArgv[10];

struct Customer * read_customers(char fileName[20]);
struct Order * read_orders(char fileName[20]);
struct LineItem * read_lineitems(char fileName[20]);

void print_customers(int lines);
void print_orders(int lines);
void print_lineitems(int lines);
void print_middle_items(struct CalcArgv *argv);
void print_result(struct CalcArgv *argv);

void filter_chain(struct CalcArgv *argv);
void filter_orders(struct CalcArgv *argv, struct Customer *pc);
void filter_lineitems(struct CalcArgv *argv, struct Customer *pc, struct Order *po);
struct MiddleTableItem * gen_table_item(struct Customer *pc, struct Order *po, struct LineItem *pi);
void insert_middle_item(struct CalcArgv *argv, struct MiddleTableItem *item);
void sum_by_group(struct CalcArgv *argv);
void sort_result(struct CalcArgv *argv);
void insert_sum_item(struct SumTableItem *head, struct SumTableItem *item);
void print_result(struct CalcArgv *argv);

void main(int argc, char *argv[]){
	
	// parse command arguments
	char customer_file_name[20];
	char order_file_name[20];
	char lineitem_file_name[20];
	int calcNum;
	
	strcpy(customer_file_name, argv[1]);
	strcpy(order_file_name, argv[2]);
	strcpy(lineitem_file_name, argv[3]);
	calcNum=atoi(argv[4]);
	
	for(int i=0; i<calcNum; i++){
		struct CalcArgv *p=&(calcArgv[i]);
		strcpy(p->mname, argv[5+i*4]);
		strcpy(p->odate, argv[6+i*4]);
		strcpy(p->shipDate, argv[7+i*4]);
		p->limit=atoi(argv[8+i*4]);
		
		p->mhead=(struct MiddleTableItem *)malloc(sizeof(struct MiddleTableItem));
		p->mhead->next=NULL;
		p->mtail=NULL;
		p->shead=(struct SumTableItem *)malloc(sizeof(struct SumTableItem));
		p->shead->next=NULL;
		p->stail=NULL;
	}
	
	for(int i=0; i<calcNum; i++){
		struct CalcArgv *p=&(calcArgv[i]);
		printf("%s %s %s %d\n", p->mname, p->odate, p->shipDate, p->limit);
	}
	
	// read files
	customers=read_customers(customer_file_name);
	//print_customers(5);
	orders=read_orders(order_file_name);
	//print_orders(5);
	lineitems=read_lineitems(lineitem_file_name);
	//print_lineitems(5);
	
	// calc 
	for(int i=0; i<calcNum; i++){
		struct CalcArgv *p=&(calcArgv[i]);
		filter_chain(p);
		print_middle_items(p);
		sum_by_group(p);
		sort_result(p);
		print_result(p);
	}
	
}

void sum_by_group(struct CalcArgv *argv){
	struct SumTableItem *scurr;
	struct MiddleTableItem *mcurr=argv->mhead->next;
	if(mcurr==NULL)	return;

	scurr=(struct SumTableItem *)malloc(sizeof(struct SumTableItem));
	scurr->next=NULL;
	scurr->oid=mcurr->oid;
	strcpy(scurr->odate, mcurr->odate);
	scurr->revenue=mcurr->price;
	argv->shead->next=scurr;

	mcurr=mcurr->next;

	while(mcurr!=NULL){
		if(mcurr->oid!=scurr->oid||strcmp(mcurr->odate, scurr->odate)!=0){
			struct SumTableItem *p=(struct SumTableItem *)malloc(sizeof(struct SumTableItem));
			p->next=NULL;
			p->oid=mcurr->oid;
			strcpy(p->odate, mcurr->odate);
			p->revenue=mcurr->price;
			scurr->next=p;
			scurr=p;
		}else{
			scurr->revenue += mcurr->price;
		}
		
		mcurr=mcurr->next;
	}
	
}

void sort_result(struct CalcArgv *argv){
	struct SumTableItem * newHead=argv->shead;
	struct SumTableItem * curr=argv->shead->next;
	
	newHead->next=NULL;
	while(curr!=NULL){
		struct SumTableItem *next=curr->next;
		insert_sum_item(newHead, curr);
		
		curr=next;
	}
}

void insert_sum_item(struct SumTableItem *head, struct SumTableItem *item){
	struct SumTableItem *prev=head;
	struct SumTableItem *curr=head->next;
	
	while(curr!=NULL){
		if(item->revenue>=curr->revenue){
			// insert before
			prev->next=item;
			item->next=curr;
			return;
		}
		prev=curr;
		curr=curr->next;
	}
	
	// append after
	prev->next=item;
	item->next=NULL;
	
}
	
void print_middle_items(struct CalcArgv *argv){
	
	struct MiddleTableItem *curr=argv->mhead->next;
	printf("--------MiddleTableItem---------\n");
	while(curr!=NULL){
		printf("%d|%s|%f\n", curr->oid, curr->odate, curr->price);
		curr=curr->next;
	}
	printf("\n");
	
}

void print_result(struct CalcArgv *argv){
	
	struct SumTableItem *curr=argv->shead->next;
	int limit=argv->limit;
	printf("l_orderkey|o_orderdate|revenue\n");
	for(int i=0; i<limit; i++){
		if(curr==NULL)	break;
		printf("%d|%s|%f\n", curr->oid, curr->odate, curr->revenue);
		curr=curr->next;
	}
	
}

void filter_chain(struct CalcArgv *argv){
	
	struct Customer *p=customers->next;
	while(p!=NULL){
		if(strcmp(p->mname,argv->mname)==0){
			filter_orders(argv, p);
			//printf("filter_chain: mname=%s\n", p->mname);
		}
		
		p=p->next;
	}
	
}

void filter_orders(struct CalcArgv *argv, struct Customer *pc){
	
	struct Order *po=orders->next;
	while(po!=NULL){
		if(po->cid==pc->cid && strcmp(po->odate, argv->odate)<0){
			filter_lineitems(argv, pc, po);
			//printf("filter_orders: argv->odate=%s curr->odate=%s\n", argv->odate, po->odate);
		}
		
		po=po->next;
	}
	
}

void filter_lineitems(struct CalcArgv *argv, struct Customer *pc, struct Order *po){
	
	struct LineItem *pi=lineitems->next;
	while(pi!=NULL){
		if(po->oid==pi->oid && strcmp(pi->shipDate, argv->shipDate)>0){
			struct MiddleTableItem * pt=gen_table_item(pc, po, pi);
			insert_middle_item(argv, pt);
		}
		
		pi=pi->next;
	}
	
}

struct MiddleTableItem * gen_table_item(struct Customer *pc, struct Order *po, struct LineItem *pi){
	struct MiddleTableItem *pt=(struct MiddleTableItem *)malloc(sizeof(struct MiddleTableItem));
	if(pt==NULL)
		exit(1);
	
	pt->oid=po->oid;
	strcpy(pt->odate, po->odate);
	pt->price=pi->price;
	pt->next=NULL;
	
	return pt;
}

void insert_middle_item(struct CalcArgv *argv, struct MiddleTableItem *item){
	struct MiddleTableItem *prev=argv->mhead;
	struct MiddleTableItem *curr=argv->mhead->next;
	
	while(curr!=NULL){
		if(item->oid<curr->oid||(item->oid==curr->oid&&strcmp(item->odate, curr->odate)<=0)){
			// insert before
			prev->next=item;
			item->next=curr;
			return;
		}
		prev=curr;
		curr=curr->next;
	}
	
	// append after
	prev->next=item;
	
}

void print_customers(int lines){
	
	struct Customer *p=customers->next;
	
	printf("-------Customer-------\n");
	int i=0;
	while(p!=NULL){
		printf("%d|%s\n", p->cid, p->mname);
		p=p->next;
		i++;
		if(i>=lines)
			break;
	}
	
}

void print_orders(int lines){
	
	struct Order *p=orders->next;
	
	printf("-------Order-------\n");
	int i=0;
	while(p!=NULL){
		printf("%d|%ld|%s\n", p->oid, p->cid, p->odate);
		p=p->next;
		i++;
		if(i>=lines)
			break;
	}
	
}

void print_lineitems(int lines){
	
	struct LineItem *p=lineitems->next;
	
	printf("-------LineItem-------\n");
	int i=0;
	while(p!=NULL){
		printf("%d|%f|%s\n", p->oid, p->price, p->shipDate);
		p=p->next;
		i++;
		if(i>=lines)
			break;
	}
	
}

struct Customer * read_customers(char fileName[20]){

	struct Customer *head=(struct Customer *)malloc(sizeof(struct Customer));
	head->next=NULL;
	
	FILE *fp=fopen(fileName, "r");
	if(fp==NULL)
		return head;
	
	int id;
	char name[20];
	struct Customer *curr=head;
	while(!feof(fp)){
		fscanf(fp, "%d|%s", &id, name);
		if(feof(fp))	break;
		
		struct Customer *p=(struct Customer *)malloc(sizeof(struct Customer));
		p->cid=id;
		strcpy(p->mname, name);
		p->next=NULL;
		
		curr->next=p;
		curr=p;
	}
	
	fclose(fp);
	return head;
}

struct Order * read_orders(char fileName[20]){

	struct Order *head=(struct Order *)malloc(sizeof(struct Order));
	head->next=NULL;
	
	FILE *fp=fopen(fileName, "r");
	if(fp==NULL)
		return head;
	
	int oid;
	int cid;
	char odate[20];
	struct Order *curr=head;
	while(!feof(fp)){
		fscanf(fp, "%d|%d|%s", &oid, &cid, odate);
		if(feof(fp))	break;
		
		struct Order *p=(struct Order *)malloc(sizeof(struct Order));
		p->oid=oid;
		p->cid=cid;
		strcpy(p->odate, odate);
		p->next=NULL;
		
		curr->next=p;
		curr=p;
	}
	
	fclose(fp);
	return head;
}

struct LineItem * read_lineitems(char fileName[20]){

	struct LineItem *head=(struct LineItem *)malloc(sizeof(struct LineItem));
	head->next=NULL;
	
	FILE *fp=fopen(fileName, "r");
	if(fp==NULL)
		return head;
	
	int oid;
	float price;
	char shipDate[20];
	struct LineItem *curr=head;
	while(!feof(fp)){
		fscanf(fp, "%d|%f|%s", &oid, &price, shipDate);
		if(feof(fp))	break;
		
		struct LineItem *p=(struct LineItem *)malloc(sizeof(struct LineItem));
		p->oid=oid;
		p->price=price;
		strcpy(p->shipDate, shipDate);
		p->next=NULL;
		
		curr->next=p;
		curr=p;
	}
	
	fclose(fp);
	return head;
}

