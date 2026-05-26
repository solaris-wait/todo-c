#include <stdio.h>
#include <string.h>
#include<time.h>
#include<stdlib.h>

#ifdef _WIN32
    #include<windows.h>
    #define CLEAR_SCREEN() system("cls")
    #define SLEEP(ms) Sleep(ms)
#else
    #include<unistd.h>
    #define CLEAR_SCREEN() system("clear")
    #define SLEEP(ms) usleep((ms)*1000)
#endif

void init_data();
void load_data();
void print_top(const char *p);
//清除错误输入
void clear_input_buffer();
//开始任务
void start_focus();
//新建任务
void add_task();
//删除任务
void delete_task();
// 检索任务并根据状态输出
// 1->全部任务
// 0->未完成任务
void list_tasks(int state);
//展示已完成任务
void complete_task();
//今日工作统计
void show_today_report();
//保存数据
void save_data();
//扩容函数
void expend_space();
//获取最大id
int get_task_id();
//任务名称长度
#define NAME_LEN 20

#define MAX_DESC 256
//描述最多 256 个字符
//typedef {原类型} 新别名
//typedef 是 C 语言里用来给已有类型起个别名的关键字
//它的作用就是让复杂的类型名变得更短、更好理解
//我定义了一个结构体 Task 来表示任务，可以直接使用 Task 来声明变量

typedef struct {
    //任务编号
    int id;
    //描述
    char description[MAX_DESC];
    //事件优先级
    int level;
    //完成状态 0未完成 1已完成
    int completed;
    //完成任务的时间
    time_t completed_time;
    //累计时长
    int total_time;
    //任务名称
    char name[NAME_LEN];
}Task;

typedef struct {
    //申请所有任务的首地址
    Task *items;
    //目前已经存入的任务数量
    int count;
    //目前可存入的最大任务量
    int capacity;
}Taskarr;
//-------全局变量---------
Taskarr tasks;

int task_count = 0;
int main(){
    //初始化数据容器
    init_data();
    //加载本地数据
    load_data();
    //判断用户要做什么操做
    int condition;
    do{
        CLEAR_SCREEN();
        print_top("操作清单");
        printf("开始任务请输入0\n");
        printf("新建任务请输入1\n");
        printf("删除任务请输入2\n");
        printf("查看任务清单（全部包含已完成）任务请输入3\n");
        printf("查看完成任务请输入4\n");
        printf("查看未完成任务请输入5\n");
        printf("今日工作统计请输入6\n");
        printf("退出请输-1\n");
        printf("等待输入...\n");
        if(scanf("%d",&condition)!=1){
            condition=-2;
            clear_input_buffer();
            //清除错误输入
        }
        switch(condition){
            case 0:start_focus();break;
            case 1:add_task(); break;
            case 2:delete_task(); break;
            case 3:list_tasks(1); break;
            //全部任务清单
            case 4:complete_task();break;
            case 5:list_tasks(0); break;
            case 6:show_today_report(); break;
            case -1:save_data(); printf("数据已保存，再见！\n"); break;
            default :printf("输入数据有误请重新输入\n");
        }
        if(condition==-1){
            printf("回车后继续...\n");
            getchar();
            //卡住页面
        }
    }while(condition!=-1);
}

void print_top(const char *p){
    printf("<========%s========>\n",p);
}
void clear_input_buffer(){
    char c;
    while((c=getchar())!='\n'&&c!=EOF);
}


void add_task(){
    if(tasks.count==tasks.capacity) {
        expend_space();
    }
    tasks.items[tasks.count].id=get_task_id()+1;
    printf("请输入任务名称\n");
    fgets(tasks.items[tasks.count].name,NAME_LEN,stdin);
    tasks.items[tasks.count].name[strcspn(tasks.items[tasks.count].name,"\n")]='\0';
    
    printf("请输入任务描述\n");
    fgets(tasks.items[tasks.count].description,MAX_DESC,stdin);
    tasks.items[tasks.count].description[strcspn(tasks.items[tasks.count].description,"\n")]='\0';
    
    printf("请输入任务优先级\n");
    printf("1低\n");
    printf("2中\n");
    printf("3高\n");
    do{
        scanf("%d",&tasks.items[tasks.count].level);
        clear_input_buffer();
    }while(tasks.items[tasks.count].level<1||tasks.items[tasks.count].level>3);
    tasks.items[tasks.count].completed_time=0;
    tasks.items[tasks.count].completed=0;
    tasks.items[tasks.count].total_time=0;
    tasks.count++;
}


void expend_space(){
    //记录旧的capacity
    int old_cap=tasks.capacity;
    //判断扩容数量
    int num=(tasks.capacity==0)?10:2*tasks.capacity;
    Task* temp=realloc(tasks.items,num*sizeof(Task));
    if(temp==NULL){
            printf("扩容失败请重试\n");
            return;
    }
    if(num>old_cap){
        memset(temp+old_cap,0,(num-old_cap)*sizeof(Task));
    }
    tasks.items=temp;
    tasks.capacity=num;
}
void init_data(){
    tasks.items=NULL;
    tasks.count=0;
    tasks.capacity=0;
}
int get_task_id(){
    int temp=0;
    for(int i=0;i<tasks.count;i++){
        if(tasks.items[i].id>temp){
            temp=tasks.items[i].id;
        }
    }
    return temp;
}