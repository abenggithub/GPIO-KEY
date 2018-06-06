#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/select.h>
#include <sys/time.h>

#include "connect.h"

#define KEY_STATE_0         0       //  按键状态
#define KEY_STATE_1         1
#define KEY_STATE_2         2
#define KEY_STATE_3         3

#define SINGLE_KEY_TIME     3       //  SINGLE_KEY_TIME*10MS = 30MS     判定单击的时间长度，软件消抖
#define KEY_INTERVAL        30      //  KEY_INTERVAL*10MS    = 300MS 判定双击的时间间隔
#define LONG_KEY_TIME       300     //  LONG_KEY_TIME*10MS   = 3S   判定长按的时间长度

typedef enum {
    N_KEY = 0,              //  no click
    S_KEY = 1,              //  single click
    D_KEY = 2,              //  double click
    T_KEY = 3,              //  Triple click
    L_KEY = 10              //  long press
}KEY_EVENT_E;

// ----------------------------------- key_driver --------------------------
unsigned char key_driver(void) 
{     
    static unsigned char key_state = 0;
    static unsigned int  key_time = 0;
    unsigned char key_press, key_return; 

    key_return = N_KEY;                         //  清除 返回按键值

    key_press = KEY_INPUT;                      //  读取当前键值

    switch (key_state)     
    {       
        case KEY_STATE_0:                       //  按键状态0：判断有无按键按下
            if (/*!*/key_press)                 //  有按键按下
            {
                key_time = 0;                   //  清零时间间隔计数
                key_state = KEY_STATE_1;        //  然后进入 按键状态1
            }        
            break;

        case KEY_STATE_1:                       //  按键状态1：软件消抖（确定按键是否有效，而不是误触）。按键有效的定义：按键持续按下超过设定的消抖时间。
            if (key_press)                     
            {
                key_time++;                     //  一次10ms
                if(key_time>=SINGLE_KEY_TIME)   //  消抖时间为：SINGLE_KEY_TIME*10ms = 30ms;
                {
                    key_state = KEY_STATE_2;    //  如果按键时间超过 消抖时间，即判定为按下的按键有效。按键有效包括两种：单击或者长按，进入 按键状态2， 继续判定到底是那种有效按键
                }
            }         
            else key_state = KEY_STATE_0;       //  如果按键时间没有超过，判定为误触，按键无效，返回 按键状态0，继续等待按键
            break; 

        case KEY_STATE_2:                       //  按键状态2：判定按键有效的种类：是单击，还是长按
            if(!key_press)                      //  如果按键在 设定的长按时间 内释放，则判定为单击
            { 
                 key_return = S_KEY;            //  返回 有效按键值：单击
                 key_state = KEY_STATE_0;       //  返回 按键状态0，继续等待按键
            } 
            else
            {
                key_time++;                     

                if(key_time >= LONG_KEY_TIME)   //  如果按键时间超过 设定的长按时间（LONG_KEY_TIME*10ms=300*10ms=3000ms）, 则判定为 长按
                {
                    key_return = L_KEY;         //  返回 有效键值值：长按
                    key_state = KEY_STATE_3;    //  去状态3，等待按键释放
                }
            }
            break;

      case KEY_STATE_3:                         //  等待按键释放
          if (!key_press) 
          {
              key_state = KEY_STATE_0;          //  按键释放后，进入 按键状态0 ，进行下一次按键的判定
          }        
          break; 

        default:                                //  特殊情况：key_state是其他值得情况，清零key_state。这种情况一般出现在 没有初始化key_state，第一次执行这个函数的时候
            key_state = KEY_STATE_0;
            break;
    }

    return  key_return;                         //  返回 按键值
} 

// ----------------------------------- key_read --------------------------------
unsigned char key_read(void)                            
{ 
    static unsigned char key_state1=0, key_time1=0;
    unsigned char key_return,key_temp;

    key_return = N_KEY;                         //  清零 返回按键值

    key_temp = key_driver();                    //  读取键值

    switch(key_state1) 
    {         
        case KEY_STATE_0:                       //  按键状态0：等待有效按键（通过 key_driver 返回的有效按键值）
            if (key_temp == S_KEY)          //  如果是[单击]，不马上返回单击按键值，先进入 按键状态1，判断是否有[双击]的可能
            { 
                 key_time1 = 0;                 //  清零计时
                 key_state1 = KEY_STATE_1; 
            }             
            else                                //  如果不是[单击]，直接返回按键值。这里的按键值可能是：[长按]，[无效按键]
            {
                 key_return = key_temp;         //  返回 按键值
            }
            break;

        case KEY_STATE_1:                       //  按键状态1：判定是否有[双击]
            if (key_temp == S_KEY)              //  有[单击]后，如果在 设定的时间间隔（KEY_INTERVAL*10ms=30*10ms=300ms） 内，再次有[单击]，则为[双击]，但是不马上返回 有效按键值为[双击]，先进入 按键状态2，判断是否有[三击]
            {
                key_time1 = 0;                  //  清零 时间间隔
                key_state1 = KEY_STATE_2;       //  改变 按键状态值
            } 
            else                                //  有[单击]后，如果在 设定的时间间隔（KEY_INTERVAL*10ms=30*10ms=300ms）内，没有[单击]出现，则判定为 [单击]
            {
                key_time1++;                    //  计数 时间间隔
                if(key_time1 >= KEY_INTERVAL)   //  超过 时间间隔
                 { 
                    key_return = S_KEY;         //  返回 有效按键：[单击]
                    key_state1 = KEY_STATE_0;   //  返回 按键状态0，等待新的有效按键
                 }              
             }              
             break; 

        case KEY_STATE_2:                       // 按键状态2：判定是否有[三击]
            if (key_temp == S_KEY)              // 有[双击]后，如果在 设定的时间间隔（KEY_INTERVAL*10ms=30*10ms=300ms） 内，再次有[单击]，则为[三击]，由于这里只扩展到[三击]，所以马上返回 有效按键值为[三击]
            {
                 key_return = T_KEY;            // 返回 有效按键：[三击]
                 key_state1 = KEY_STATE_0;      // 返回 按键状态0，等待新的有效按键
            } 
            else                                // 有[双击]后，如果在 设定的时间间隔（KEY_INTERVAL*10ms=30*10ms=300ms）内，没有[单击]，则判定为 [双击]
            {
                key_time1++;                    // 计数 时间间隔
                if(key_time1 >= KEY_INTERVAL)   // 超过 时间间隔
                 { 
                      key_return = D_KEY;       // 返回 有效按键：[双击]
                      key_state1 = KEY_STATE_0; // 返回 按键状态0，等待新的有效按键
                 }              
             }              
             break; 

        default:                                //  特殊情况：key_state是其他值得情况，清零key_state。这种情况一般出现在 没有初始化key_state，第一次执行这个函数的时候
            key_state1 = KEY_STATE_0;
            break;
    }

    return key_return;                          // 返回 按键值
}

void milliseconds_sleep(unsigned long mSec){
    struct timeval tv;
    tv.tv_sec=mSec / 1000;
    tv.tv_usec=(mSec % 1000) * 1000;
    
    int err;
    do{
       err = select(0, NULL, NULL, NULL, &tv);
    }while(err < 0 && errno == EINTR);
}

void* key_analyze_thread(void *arg)
{
    bool run_flag = true;
    while(run_flag) {
        milliseconds_sleep(10);
        switch(key_read()) {
            case S_KEY:
                fprintf(stdout, "catch the click event.\n");
                break;
            case D_KEY:
                fprintf(stdout, "catch the double click event.\n");
                break;
            case T_KEY:
                fprintf(stdout, "catch the triple click event.\n");
                break;
            case L_KEY:
                fprintf(stdout, "catch the long press event.\n");
                break;
            default:
                break;    
        }
    }

    fprintf(stdout, "%s exit.\n", __func__);
    return NULL;
}