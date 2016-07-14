#include <LinkedList.h>

#include <U8glib.h>

U8GLIB_SSD1306_128X64 u8g(13, 11, 10, 9);  // SW SPI Com: SCK = 13, MOSI = 11, CS = 10, A0 = 9

#define KEY_UP 1
#define KEY_DOWN 2
#define KEY_LEFT 3
#define KEY_RIGHT 4
#define GAP 4
#define GRID_WIDTH 24 // 128/4
#define GRID_HEIGTH 16 //64/4

uint8_t uiKeyUp = 7;
uint8_t uiKeyDown = 6;
uint8_t uiKeyRight = 4;
uint8_t uiKeyLeft = 5;
uint8_t uiKeyMenu = 8;

uint8_t keypressed = 0;//0 none key pressed

class Context;

class State
{
public:
  virtual void cycle(Context* c) = 0;//game logic update
  virtual void draw(Context* c) = 0;//render
};

class InGameState: public State
{
public:
  void cycle(Context* c);
  void draw(Context* c);
};

class GameSettingState: public State
{
public:
  void cycle(Context* c);
  void draw(Context* c);
};

class Context
{
private:
  State *pigs;
  State *pgss;
  State *pstate;
public:
  Context();
  void setInGameState();
  void setGameSettingState();    
  void cycle();
  void draw();
};

enum GameState{
  RUNNING,
  STAGE_CLEAR  
};

class gameStateData
{
  public:
  int stage = 1;
  int snakelen = 1;
  int gamestate = RUNNING;
};

class StateBar
{
  public:
  StateBar()
  {
    x = GRID_WIDTH * GAP;
    y = 0;
    width = u8g.getWidth() - x;
    heigth = u8g.getHeight() - y;
  }
  void show(gameStateData &gsd)
  {
    u8g.drawFrame(x,y,width,heigth);
    u8g.setFont(u8g_font_profont10r);
    u8g.setFontPosTop();    
    u8g.setPrintPos(x+2,y+1);
    //u8g.drawStr(x+2,y+1,"stage");
    u8g.print("stage");
    u8g.setPrintPos(x+2+1,y+1+9);  
    if(gsd.gamestate == STAGE_CLEAR)
    {          
      u8g.print("Clear");      
    }
    else
    { 
      u8g.print(gsd.stage);
    }
    u8g.setPrintPos(x+2,y+1+30);     
    u8g.print("len:");
    u8g.print(gsd.snakelen);
  }

  private:
  int x;
  int y;
  int width;
  int heigth;
  String strbuf;
};
class BOX
{
  public:
    BOX(U8GLIB_SSD1306_128X64 *screen)
    {
      scr = screen;
    }
    void mov(int dir)
    {
      switch (dir)
      {
        case KEY_UP:
          y <= 0 ? y = 63 : y -= GAP;
          break;
        case KEY_DOWN:
          y >= 64 ? y = 0 : y += GAP;
          break;
        case KEY_LEFT:
          x <= 0 ? x = 127 : x -= GAP;
          break;
        case KEY_RIGHT:
          x >= 128 ? x = 0 : x += GAP;
      }
    }
    void show(void)
    {
      scr->drawFrame(x, y, GAP, GAP);
    }
  protected:
    U8GLIB_SSD1306_128X64 *scr;
    int x = 0;
    int y = 0;
};
//BOX box(&u8g);
class BodyBox: public BOX
{
  public:
    BodyBox(U8GLIB_SSD1306_128X64 *screen, int gridx, int gridy): BOX(screen)
    {
      gx = gridx;
      gy = gridy;
    };
    void set(int gridx, int gridy)
    {
      gx = gridx;
      gy = gridy;
    }
    void get(int &gridx, int &gridy)
    {
      gridx = gx;
      gridy = gy;
    }
    void showBodyBox()
    {
      grid2screen(gx, gy, &x, &y);
      show();
    }
  private:
    int gx;//grid position x
    int gy;//grid position y
    void grid2screen(int xx, int yy, int *cx,  int *cy)
    {
      *cx = GAP * xx;
      *cy = GAP * yy;
    }
};
typedef int GameMap[GRID_WIDTH * GRID_HEIGTH];
				         /*0                   			     1                             2                             3 */
			           /*0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1 */
GameMap gamemap = {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,  /*0*/
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,  /*1*/
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /*2*/
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,  /*3*/
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /*4*/
                   0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /*5*/
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /*6*/
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /*7*/
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /*8*/
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /*9*/
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,  /*0*/
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /*1*/
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /*2*/
                   0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /*3*/
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /*4*/
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*5*/
                  };
class Snake
{
  public:
    Snake(GameMap *gm)
    {
      x = GRID_WIDTH >> 1;
      y = GRID_HEIGTH >> 1;
      pgm = gm;

      BodyBox* pbb = new BodyBox(&u8g, x, y);
      snakebody.add(pbb);
      state = 0;
      now_face_to = KEY_RIGHT;
    }
    //改变方向
    void changedir(int dir)
    {
      if (now_face_to == KEY_DOWN && dir == KEY_UP
          || now_face_to == KEY_UP && dir == KEY_DOWN
          || now_face_to == KEY_LEFT && dir == KEY_RIGHT
          || now_face_to == KEY_RIGHT && dir == KEY_LEFT
         )
      {
        return;
      }
      now_face_to = dir;
    }
    void move()
    {
      //移动身体
      switch (now_face_to)
      {
        case KEY_UP:
          y--;
          if( y < 0)
          {
            y = GRID_HEIGTH - 1;
          }
          break;
        case KEY_DOWN:
          y++;
          if(y > GRID_HEIGTH - 1)
          {
             y = 0;
          }
          break;
        case KEY_LEFT:
          x--;
          if(x < 0 )
          { 
            x = GRID_WIDTH - 1; 
          }
          break;
        case KEY_RIGHT:
          x++;
          if(x > GRID_WIDTH - 1)
          {
             x = 0;
          }
          break;
      }
      //头位置已经更新

      tempdestgx = x;
      tempdestgy = y;
      BodyBox* tempbb;
      for (int i = 0; i < snakebody.size(); i++)
      {
        tempbb = snakebody.get(i);
        tempbb->get(tempgx, tempgy); //保存挪之前的bodybox坐标
        tempbb->set(tempdestgx, tempdestgy); //挪当前bodybox
        //保存下一个bodybox的目标位置坐标到tempdestgx，tempdestgy
        tempdestgx = tempgx;
        tempdestgy = tempgy;
      }
      //如果处于吃到状态，理顺身体在末尾加bodybox，长度加1，清除吃到状态

      if (state == 1)
      {
        BodyBox* pnewbb = new BodyBox(&u8g, tempdestgx, tempdestgy);
        snakebody.add(pnewbb); //尾部坐标为tempdestgx，tempdestgx
        state = 0;
      }
      //检测头的位置发生碰撞了，设置状态成吃到状态，对map作用：清除已经吃掉的东西。
      if (isCollide(x, y) == 1)
      { //collide with something
        state = 1;
        set_gamemap(x, y, 0); //0表示清除（被吃掉的东西）
      }
    }
    //show the snake
    void show()
    {
      BodyBox* bb;
      for (int i = 0; i < snakebody.size(); i++)
      {
        bb = snakebody.get(i);
        bb->showBodyBox();
      }
    }
    int getSnakeBodyLen(void)
    {
      return snakebody.size();
    }
  private:
    int grid2gmindex(int gx, int gy)
    {
      return gy * GRID_WIDTH + gx;
    }
    int isCollide(int gx, int gy)
    {
      return (*pgm)[grid2gmindex(gx, gy)];
    }
    void set_gamemap(int gx, int gy, int value)
    {
      (*pgm)[grid2gmindex(gx, gy)] = value;
    }

    int x;//头的位置
    int y;

    int tempgx;
    int tempgy;
    int tempdestgx;//目标位置
    int tempdestgy;
    int now_face_to;
    LinkedList<BodyBox*> snakebody = LinkedList<BodyBox*>();
    GameMap *pgm;
    int state;//0 正常行走，1吃到了
};
class Controller
{
  public:
    Controller(Snake *snake)
    {
      psnake = snake;
    }
    void checkkey()
    {
      if (digitalRead(uiKeyUp) == LOW)
      {
        keypressed = uiKeyUp;
        psnake->changedir(KEY_UP);
        keypressed = 0;
        Serial.print("U");
      }
      else if (digitalRead(uiKeyDown) == LOW)
      {
        keypressed = uiKeyDown;
        psnake->changedir(KEY_DOWN);
        keypressed = 0;
        Serial.print("D");
      }
      else if (digitalRead(uiKeyLeft) == LOW)
      {
        keypressed = uiKeyLeft;
        psnake->changedir(KEY_LEFT);
        keypressed = 0;
        Serial.print("L");
      }
      else if (digitalRead(uiKeyRight) == LOW)
      {
        keypressed = uiKeyRight;
        psnake->changedir(KEY_RIGHT);
        keypressed = 0;
        Serial.print("R");
      }
      else if (digitalRead(uiKeyMenu) == LOW)
      {        
        keypressed = uiKeyMenu;
        Serial.print("M");
      }      
      else
      {
      }
    }
  private:
    Snake* psnake;
};
int isAllZero(GameMap &gm)
{
  int ret=1;//true
  for(int i=0; i< GRID_WIDTH * GRID_HEIGTH; i++)
  {
    if(gm[i] !=0)
    {
      ret = 0;//false
      break;
    }
  }
  return ret;
}

class Game
{
  public:
    Game(Snake *snake, GameMap *gamemap, StateBar *statebar)
    {
      psnake = snake;
      gm = gamemap;
      sb = statebar;
      gsd.stage = 1;
      gsd.snakelen = psnake->getSnakeBodyLen();
      gsd.gamestate = RUNNING;
    }
    void Update(void)
    {      
      timegap = (220 - 10 * gsd.stage < 100) ? 85 : 240 - 10 * gsd.stage;

      timenow = millis();

      if (timenow - timelast > timegap)
      {
        timelast = timenow;
        psnake->move();
        gsd.snakelen = psnake->getSnakeBodyLen();
      }      
      if(isAllZero(gamemap))
      {
        gsd.gamestate = STAGE_CLEAR;        
      }
    }
    void show()
    {
      //显示蛇
      psnake->show();
      int gx;
      int gy;
      //显示地图
      for (int i = 0; i < GRID_WIDTH * GRID_HEIGTH; i++)
      {
        if ((*gm)[i] == 1)
        {
          gx = i % GRID_WIDTH;
          gy = i / GRID_WIDTH;
          u8g.drawBox(gx * GAP, gy * GAP, GAP, GAP);
        }
      }
      //显示状态栏
      sb->show(gsd);
    }
  private:
    unsigned long timenow = 0;
    unsigned long timelast = 0;
    unsigned long timenext = 0;
    unsigned long timegap = 200;
    Snake *psnake;
    GameMap *gm;
    StateBar * sb;
    gameStateData gsd;
};

Snake snake(&gamemap);
StateBar statebar;
Game game(&snake, &gamemap, &statebar);
Controller controller(&snake);


void InGameState::cycle(Context* ct)
{
  if(keypressed == uiKeyMenu)
  {    
    ct->setGameSettingState();
    keypressed = 0;
    return;
  }
  game.Update();
}

void InGameState::draw(Context* ct)
{
  u8g.firstPage();
  do {
    game.show();
  } while ( u8g.nextPage() );
}
void GameSettingState::cycle(Context* ct)
{
  if(keypressed == uiKeyMenu)
  {    
    ct->setInGameState();
    keypressed = 0;
    return;
  }
}
void GameSettingState::draw(Context* ct)
{
  u8g.firstPage();
  do {
    u8g.drawRFrame(10,10,100,40,5);
  } while ( u8g.nextPage() );
}

Context::Context()
{
  pigs = new InGameState();
  pgss = new GameSettingState();
  pstate = pigs;
}

void Context::setInGameState()
{
  pstate = pigs;  
}

void Context::setGameSettingState()
{
  pstate = pgss;
}

void Context::cycle()
{
  pstate->cycle(this);
}
void Context::draw()
{
  pstate->draw(this);
}

Context context;

unsigned long tt1;
unsigned long tt2;
int intstate = 1;//1:on 0:off
unsigned long      time_isr = 0;//for key debouncing
unsigned long last_time_isr = 0;//for key debouncing
unsigned long debouncing_time = 150;//150ms 
void myisr(void)
{
  time_isr = millis();
  if(time_isr - last_time_isr > debouncing_time)
  {
    controller.checkkey();
    last_time_isr = time_isr;
  }
}

void mapgen(void)
{
  int fluit = 5;//5块水果
  int idx = 0;
  for (int i = 0; i < GRID_WIDTH * GRID_HEIGTH; i++)
  {
    gamemap[i] = 0;
  }
  randomSeed(analogRead(0));
  for (int i = 0; i < fluit; i++)
  {
    do {
      idx = random(0, GRID_WIDTH * GRID_HEIGTH);
    } while (gamemap[idx] == 1);
    gamemap[idx] = 1;
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(uiKeyUp, INPUT_PULLUP);           // set pin to input with pullup
  pinMode(uiKeyDown, INPUT_PULLUP);           // set pin to input with pullup
  pinMode(uiKeyRight, INPUT_PULLUP);           // set pin to input with pullup
  pinMode(uiKeyLeft, INPUT_PULLUP);           // set pin to input with pullup
  pinMode(uiKeyMenu, INPUT_PULLUP);           // set pin to input with pullup
  attachInterrupt(digitalPinToInterrupt(2), myisr, FALLING);
  mapgen();
}

void loop() {
  context.cycle();
  context.draw();
}

