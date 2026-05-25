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
void clear_input_buffer();
//清除错误输入
void start_focus();
//开始任务
void add_task();
//新建任务
void delete_task();
//删除任务
void list_tasks(int state);
//检索任务并根据状态输出
//1->全部任务
//0->未完成任务
void complete_task();
//展示已完成任务
void show_today_report();
//今日工作统计
void save_data();
//保存数据


#define MAX_TASKS 100
//最多存 100 个任务
#define MAX_DESC 256
//描述最多 256 个字符
//typedef {原类型} 新别名
//typedef 是 C 语言里用来给已有类型起个别名的关键字
//它的作用就是让复杂的类型名变得更短、更好理解
//我定义了一个结构体 Task 来表示任务，可以直接使用 Task 来声明变量
typedef struct {
    char description[MAX_DESC];
    // 0 = 未完成，1 = 已完成
    int completed;
} Task;

Task tasks[MAX_TASKS];
int task_count = 0;
int main(){
    init_data();
    //初始化数据容器
    load_data();
    //加载本地数据
    int condition;
    //判断用户要做什么操做
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
        if(scanf("%d",condition)!=1){
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
        if(condition!=0){
            printf("回车后继续...\n");
            getchar();
            //卡住页面
        }
    }while(condition!=0);
}



void add_task() {
    //检查是否已满
    if (task_count >= MAX_TASKS) {
        printf("待办列表已满，无法添加！\n");
        return;
    }

    //获取用户输入
    //这个 . 是结构体成员访问运算符，用来访问结构体变量的成员
    printf("请输入新的待办描述：");
    fgets(tasks[task_count].description, MAX_DESC, stdin);

    //去掉输入末尾的换行符
    //删不删好像无所谓，删了好看些
    size_t len = strlen(tasks[task_count].description);
    if (len > 0 && tasks[task_count].description[len - 1] == '\n') {
        tasks[task_count].description[len - 1] = '\0';
    }

    //设置为未完成
    tasks[task_count].completed = 0;

    //总数加一
    task_count++;

    printf("已添加待办事项。\n");
}
void print_top(const char *p){
    printf("<========%s========>\n",p);
}
void clear_input_buffer(){
    char c;
    while((c=getchar())!='\n'&&c!=EOF);
}