#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

// 常量定义
#define NAME_LEN 20
#define MAX_DESC 256
#define DATA_FILE "task.txt"      // 数据文件名称

// 优先级枚举
typedef enum {
    PRIORITY_LOW = 1,
    PRIORITY_MID = 2,
    PRIORITY_HIGH = 3
} Priority;

// 跨平台支持
#ifdef _WIN32
    #include <windows.h>
    #define CLEAR_SCREEN() system("cls")
    #define SLEEP(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define CLEAR_SCREEN() system("clear")
    #define SLEEP(ms) usleep((ms) * 1000)
#endif

// 数据结构
typedef struct {
    int id;
    char description[MAX_DESC];
    int level;               // 1低 2中 3高
    int completed;           // 0未完成 1已完成
    time_t completed_time;
    int total_time;          // 累计专注时长（分钟）
    char name[NAME_LEN];
} Task;


typedef struct {
    Task *items;      // 动态数组
    int count;        // 实际任务数
    int capacity;     // 当前容量
} TaskArr;

// 全局变量
TaskArr tasks;

// 函数声明
void init_data(void);
void load_data(void);
void save_data(void);
void free_tasks(void);
void print_top(const char *p);
void clear_input_buffer(void);
int expend_space(void);
int get_max_id(void);
void add_task(void);
void delete_task(void);
void list_tasks(int state);
//倒计时函数
int do_focus(int minutes);
//开始任务
void start_focus();
//今日工作统计
void show_today_report();
//展示已完成任务
void complete_task();


// 主函数
int main() {
    // SetConsoleOutputCP(65001);   // 设置控制台为 UTF-8 编码
    init_data();
    load_data();          // 加载历史数据

    int condition;
    do {
        CLEAR_SCREEN();
        print_top("操作清单");
        printf("新建任务请输入1\n");
        printf("删除任务请输入2\n");
        printf("查看全部任务（含已完成）请输入3\n");
        printf("开始专注请输入4\n");
        printf("查看未完成任务请输入5\n");
        printf("退出请输-1或者Ctrl+C\n");
        printf("等待输入...\n");

        if (scanf("%d", &condition) != 1) {
            condition = -2;
            clear_input_buffer();
        }
        else clear_input_buffer();

        switch (condition) {
            case 1: add_task(); break;
            case 2: delete_task(); break;
            case 3: list_tasks(1); break;
            case 4: start_focus(); break;
            case 5: list_tasks(0); break;
            case -1:
                save_data();
                printf("数据已保存，再见！\n");
                break;
            default:
                printf("输入有误，请重新输入\n");
        }

        if (condition != -1) {
            printf("\n按回车键继续...");
            // getchar();  // 消耗残留换行
            getchar();  // 等待用户按键
        }
    } while (condition != -1);

    free_tasks();   // 释放内存
    return 0;
}

// 打印标题
void print_top(const char *p) {
    printf("<========%s========>\n", p);
}

// 清除输入缓冲区，防止scanf后残留的换行符干扰后续输入
void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// 动态数组管理
void init_data(void) {
    tasks.items = NULL;
    tasks.count = 0;
    tasks.capacity = 0;
}

// 扩容：成功返回1，失败返回0
int expend_space(void) {
    int old_cap = tasks.capacity;
    int new_cap = (old_cap == 0) ? 10 : old_cap * 2;
    Task *temp = (Task*)realloc(tasks.items, new_cap * sizeof(Task));
    if (temp == NULL) {
        printf("内存扩容失败，无法添加新任务！\n");
        return 0;
    }
    // 新分配的内存清零
    if (new_cap > old_cap) {
        memset(temp + old_cap, 0, (new_cap - old_cap) * sizeof(Task));
    }
    tasks.items = temp;
    tasks.capacity = new_cap;
    return 1;
}

// 读取当前最大ID，确保新任务ID唯一且递增
int get_max_id(void) {
    int max = 0;
    for (int i = 0; i < tasks.count; i++) {
        if (tasks.items[i].id > max)
            max = tasks.items[i].id;
    }
    return max;
}

// 释放动态数组内存
void free_tasks(void) {
    if (tasks.items) {
        free(tasks.items);
        tasks.items = NULL;
    }
    tasks.count = 0;
    tasks.capacity = 0;
}

// 数据持久化
void save_data(void) {
    FILE *fp = fopen(DATA_FILE, "wb");
    if (!fp) {
        printf("警告：无法保存数据文件\n");
        return;
    }
    // 先写入任务数量
    fwrite(&tasks.count, sizeof(int), 1, fp);
    // 再写入整个任务数组
    fwrite(tasks.items, sizeof(Task), tasks.count, fp);
    fclose(fp);
}

void load_data(void) {
    FILE *fp = fopen(DATA_FILE, "rb");
    if (!fp) return;   // 首次运行，无数据文件

    int count;
    if (fread(&count, sizeof(int), 1, fp) != 1) {
        fclose(fp);
        return;
    }
    if (count <= 0) {
        fclose(fp);
        return;
    }

    // 分配足够容量
    tasks.capacity = count + 5;   // 留一点余量，免得频繁扩容
    tasks.items = (Task*)malloc(tasks.capacity * sizeof(Task));
    if (!tasks.items) {
        fclose(fp);
        return;
    }
    tasks.count = count;
    fread(tasks.items, sizeof(Task), tasks.count, fp);
    fclose(fp);
}

// 任务操作
void add_task(void) {
    // clear_input_buffer();   // 清除菜单残留的换行符，神奇Bug+1

    if (tasks.count >= tasks.capacity) {
        if (!expend_space()) {
            return;   // 扩容失败，放弃添加
        }
    }

    Task *new_task = &tasks.items[tasks.count];
    new_task->id = get_max_id() + 1;

    printf("请输入任务名称：");
    fgets(new_task->name, NAME_LEN, stdin);
    new_task->name[strcspn(new_task->name, "\n")] = '\0';

    printf("请输入任务描述：");
    fgets(new_task->description, MAX_DESC, stdin);
    new_task->description[strcspn(new_task->description, "\n")] = '\0';

    printf("请输入任务优先级 (1低 2中 3高)：");
    int input;
    while (1) {
        if (scanf("%d", &input) != 1) {
            clear_input_buffer();
            printf("输入无效，请输入数字 1、2 或 3：");
            continue;
        }
        if (input >= PRIORITY_LOW && input <= PRIORITY_HIGH) {
            new_task->level = input;
            break;
        }
        printf("优先级必须在 1~3 之间，请重新输入：");
    }
    clear_input_buffer();

    new_task->completed = 0;
    new_task->completed_time = 0;
    new_task->total_time = 0;
    tasks.count++;
    printf("任务添加成功！\n");
}

// 删除任务，根据ID删除，想做个模糊匹配，但是我不会
void delete_task(void) {
    if (tasks.count == 0) {
        printf("当前没有任何任务。\n");
        return;
    }

    int id;
    printf("请输入要删除的任务ID：");
    scanf("%d", &id);
    clear_input_buffer();

    int index = -1;
    for (int i = 0; i < tasks.count; i++) {
        if (tasks.items[i].id == id) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        printf("未找到ID为 %d 的任务。\n", id);
        return;
    }

    // 向前移动后续元素
    for (int i = index; i < tasks.count - 1; i++) {
        tasks.items[i] = tasks.items[i + 1];
    }
    tasks.count--;
    printf("任务已删除。\n");

    // 如果空闲空间过大，可缩容
     if (tasks.capacity > 10 && tasks.count < tasks.capacity / 4) {
        int new_cap = tasks.capacity / 2;
        Task *temp = (Task*)realloc(tasks.items, new_cap * sizeof(Task));
        if (temp) {
            tasks.items = temp;
            tasks.capacity = new_cap;
        }
    }
}

// 列举任务，state=0只显示未完成，state=1显示全部
void list_tasks(int state) {
    if (tasks.count == 0) {
        printf("当前没有任务。\n");
        return;
    }

    printf("\n====================================\n");
    int found = 0;
    for (int i = 0; i < tasks.count; i++) {
        Task *t = &tasks.items[i];
        int show = (state == 1) || (state == 0 && t->completed == 0);
        if (!show) continue;

        found = 1;
        const char *prio_str = (t->level == PRIORITY_LOW) ? "低" :
                                (t->level == PRIORITY_MID) ? "中" : "高";
        printf("ID: %d | %s | 优先级: %s | 状态: %s\n",
               t->id, t->name, prio_str, t->completed ? "已完成" : "未完成");
        printf("   描述: %s\n", t->description);
        if (t->completed && t->completed_time != 0) {
            char time_buf[64];
            struct tm *tm_info = localtime(&t->completed_time);
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);
            printf("   完成时间: %s\n", time_buf);
        }
        if (t->total_time > 0) {
            printf("   累计专注: %d 分钟\n", t->total_time);    // 这个功能还没做，也不知道咋写，先占位
        }
    }
    if (!found) {
        printf(state == 0 ? "没有未完成的任务。\n" : "没有符合条件的任务。\n");
    }
    printf("====================================\n");
}
void start_focus(){
    printf("请输入你要开始的任务id\n");
    int temp_id=get_max_id();
    int input_id=-1;
    while(1){
        if(scanf("%d",&input_id)!=1){
            printf("输入有误请重更新输入\n");
            clear_input_buffer();
            continue;
        }
        if(input_id>0&&input_id<=temp_id){
            clear_input_buffer();
            break;
        }
        else{
            printf("任务不存在，请重新输入\n");
        }
    }
    Task *p=NULL;
    for(int i=0;i<temp_id;i++){
        if(tasks.items[i].id==input_id){
            p=&tasks.items[i];
        }
    }
    printf("当前任务为：%s\n",p->name);
    printf("当前任务描述：%s\n",p->description);
    if(p->completed){
        printf("该任务已完成\n");
        return;
    }
    int min=do_focus(25)/60;
    p->total_time+=min;
    printf("本次专注任务完成了吗？完成输入1未完成输入0\n");
    int temp=0;
    while(1){
        if(scanf("%d",&temp)!=1){
            printf("输入有误请重新输入\n");
            clear_input_buffer();
            continue;
        }
        if(temp==0||temp==1){
            clear_input_buffer();
            break;
        }
        else{
            printf("状态不存在\n");
        }
    }
    p->completed=temp;
    if(p->completed==1){
        p->completed_time=time(NULL);
        printf("恭喜你完成任务\n");
    }
    save_data();
}
int do_focus(int minutes){
    int total_second=minutes*60;
    int var_time=total_second;
    while(var_time>0){
        int min=var_time/60;
        int sec=var_time%60;
        //进度长度
        int schedule=20;
        //完成进度
        int full=(total_second-var_time)*schedule/total_second;
        printf("\r[任务进行中]: %02d:%02d [",min,sec);
        for(int i=0;i<schedule;i++){
            if(i==full-1){
                printf("》");
            }else{
                printf(i<full?"=":" ");
            }
        }
        printf("]");
        fflush(stdout);
        SLEEP(1000);
        var_time--;
    }
    return total_second-var_time;
}