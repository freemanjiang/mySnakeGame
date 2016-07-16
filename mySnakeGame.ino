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

uint8_t uiKeyUp = 5;
uint8_t uiKeyDown = 6;
uint8_t uiKeyRight = 4;
uint8_t uiKeyLeft = 7;
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
class GameOptionState: public State
{
  public:
    void cycle(Context* c);
    void draw(Context* c);
};
class StageClearState: public State
{
  public:
    void cycle(Context* c);
    void draw(Context* c);
  private:
    char buf[20];
};
class MainMenuState: public State
{
  public:
    void cycle(Context* c);
    void draw(Context* c);
};
class GameOverState: public State
{
  public:
    void cycle(Context* c);
    void draw(Context* c);
};
class Game;
class Snake;
class Context
{
  private:
    State* pInGameState;
    State* pGameSettingState;
    State* pGameOptionState;
    State* pStageClearState;
    State* pMainMenuState;
    State* pGameOverState;
    State* pCurrentState;
  public:
    Context();
    void setInGameState();
    void setGameSettingState();
    void setGameOptionState();
    void setStageClearState();
    void setMainMenuState();
    void setGameOverState();
    void cycle();
    void draw();
    Game* pgame;
    Snake* psnake;
    int stage;//关卡号
};

class inGameData
{
  public:
    int stage = 1;
    int snakelen = 1;
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
    void show(inGameData &igd)
    {
      u8g.drawFrame(x, y, width, heigth);
      u8g.setFont(u8g_font_profont10r);
      u8g.setFontPosTop();

      u8g.setPrintPos(x + 2, y + 1);
      u8g.print("stage");
      u8g.setPrintPos(x + 2, y + 1 + 10);
      u8g.print(igd.stage);

      u8g.setPrintPos(x + 2, y + 1 + 10 + 15);
      u8g.print("len:");
      u8g.print(igd.snakelen);
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
    U8GLIB_SSD1306_128X64 *scr = NULL;
    int x = 0;
    int y = 0;
};

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

/*
   GameMap[n]的每一bit代表该地上有对应的物品，bit定义：
   bit  0 可吃的水果
        1 蛇的身体（撞上自生身体后会game over）
        2 石块（撞上后会game over）
        ...后续再添加
*/
#define FRUIT 0x01
#define SNAKE_BODY 0x02
#define STONE_BLOCK 0x04

typedef int GameMap[GRID_WIDTH * GRID_HEIGTH];
/*.................0.............................1.............................2......... */
/*.................0..1..2..3..4..5..6..7..8..9..0..1..2..3..4..5..6..7..8..9..0..1..2..3 */
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
      snakefull = 0;
      now_face_to = KEY_RIGHT;
    }
    ~Snake()
    {
      removeSnake();
    }
    //改变方向
    void changedir(int dir)
    {
      if (now_face_to == KEY_DOWN && dir != KEY_UP
          || now_face_to == KEY_UP && dir != KEY_DOWN
          || now_face_to == KEY_LEFT && dir != KEY_RIGHT
          || now_face_to == KEY_RIGHT && dir != KEY_LEFT
         )
      {
        now_face_to = dir;
      }
    }
    void move(Context* ct)
    {
      //移动身体
      switch (now_face_to)
      {
        case KEY_UP:
          y--;
          if ( y < 0)
          {
            y = GRID_HEIGTH - 1;
          }
          break;
        case KEY_DOWN:
          y++;
          if (y > GRID_HEIGTH - 1)
          {
            y = 0;
          }
          break;
        case KEY_LEFT:
          x--;
          if (x < 0 )
          {
            x = GRID_WIDTH - 1;
          }
          break;
        case KEY_RIGHT:
          x++;
          if (x > GRID_WIDTH - 1)
          {
            x = 0;
          }
          break;
      }
      //头位置已经更新

      if (isCollide(x, y) == 1)
      { //发生碰撞
        ct->setGameOverState();
      }
      tempdestgx = x;
      tempdestgy = y;
#if 0//算法一
      BodyBox* tempbb;
      for (int i = 0; i < snakebody.size(); i++)
      {
        tempbb = snakebody.get(i);
        tempbb->get(tempgx, tempgy); //保存挪之前的bodybox坐标
        clear_snake_body_in_gamemap_place(tempgx, tempgy);//在地图上清除蛇的body标记
        tempbb->set(tempdestgx, tempdestgy); //挪当前bodybox
        set_snake_body_in_gamemap_place(tempdestgx, tempdestgy);//在地图上标记有蛇的body
        //保存下一个bodybox的目标位置坐标到tempdestgx，tempdestgy
        tempdestgx = tempgx;
        tempdestgy = tempgy;
      }
      //如果处于吃到状态，理顺身体在末尾加bodybox，长度加1，清除吃到状态
      if (snakefull == 1)
      {
        BodyBox* pnewbb = new BodyBox(&u8g, tempdestgx, tempdestgy);
        snakebody.add(pnewbb); //尾部坐标为tempdestgx，tempdestgx
        set_snake_body_in_gamemap_place(tempdestgx, tempdestgy);//在地图上标记有蛇的body
        snakefull = 0;
      }
#else//算法二
      //BodyBox* pnewbb = new BodyBox(&u8g, tempdestgx, tempdestgy);
      snakebody.add(0, new BodyBox(&u8g, tempdestgx, tempdestgy));
      set_snake_body_in_gamemap_place(tempdestgx, tempdestgy);//在地图上标记有蛇的body
      if (snakefull == 0)
      { //若不是吃到的状态，就对尾部box进行删除
        BodyBox* pTailBox = snakebody.pop();
        pTailBox->get(tempgx, tempgy);
        clear_snake_body_in_gamemap_place(tempgx, tempgy);//在地图上清除蛇的body标记
        delete(pTailBox);
      }
      else
      { //若是吃到的状态，就跳过这次对尾部box的删除
        snakefull = 0;
      }
#endif

      //检测头的位置若吃到了水果，设置状态成吃到状态，对map作用：清除已经吃掉的东西。
      int place = get_gamemap_place(x, y);
      if (place & FRUIT == FRUIT)
      {
        //collide with fruit
        snakefull = 1;
        place &= ~FRUIT;
        set_gamemap_place(x, y, place); //在地图place上清除被吃掉的东西
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
    void removeSnake()
    {
      BodyBox* bb;
      for (int i = 0; i < snakebody.size(); i++)
      {
        bb = snakebody.get(i);
        delete(bb);
      }
      snakebody.clear();
    }
    int grid2gmindex(int gx, int gy)
    {
      return gy * GRID_WIDTH + gx;
    }
    int get_gamemap_place(int gx, int gy)
    {
      return (*pgm)[grid2gmindex(gx, gy)];
    }
    int isCollide(int gx, int gy)
    {
      if ((get_gamemap_place(gx, gy) & (SNAKE_BODY | STONE_BLOCK)) == 0)
      { //未撞上蛇身体或石块
        return 0;
      }
      else
      {
        return 1;
      }
    }
    void set_gamemap_place(int gx, int gy, int value)
    {
      (*pgm)[grid2gmindex(gx, gy)] = value;
    }
    void set_snake_body_in_gamemap_place(int gx, int gy)
    {
      int place = get_gamemap_place(gx, gy);
      place |= SNAKE_BODY;
      set_gamemap_place(gx, gy, place);
    }
    void clear_snake_body_in_gamemap_place(int gx, int gy)
    {
      int place = get_gamemap_place(gx, gy);
      place &= ~SNAKE_BODY;
      set_gamemap_place(gx, gy, place);
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
    int snakefull;//0 饿，没吃到的状态；1饱，吃到了的状态
};


int isNoFruitInMap(GameMap &gm)
{
  int ret = 1; //true
  for (int i = 0; i < GRID_WIDTH * GRID_HEIGTH; i++)
  {
    if ((gm[i]&FRUIT) != 0)
    {
      ret = 0;//false，存在水果
      break;
    }
  }
  return ret;
}

class Game
{
  public:
    Game(Snake *snake, GameMap *gamemap, StateBar *statebar, int stagenum)
    {
      psnake = snake;
      gm = gamemap;
      sb = statebar;
      igd.stage = stagenum;
      igd.snakelen = psnake->getSnakeBodyLen();
      (stagenum < 10) ? mapGen(stagenum + 4) : mapGen(10);
    }
    void Update(Context* ct)
    {
      (igd.stage < 10) ? (timegap = 240 - 20 * igd.stage) : (timegap = 40);

      timenow = millis();

      if (timenow - timelast > timegap)
      {
        timelast = timenow;
        psnake->move(ct);
        igd.snakelen = psnake->getSnakeBodyLen();
      }
      if (isNoFruitInMap(gamemap))
      {
        ct->setStageClearState();
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
      sb->show(igd);
    }
  private:
    unsigned long timenow = 0;
    unsigned long timelast = 0;
    unsigned long timenext = 0;
    unsigned long timegap = 200;
    Snake *psnake;
    GameMap *gm;
    StateBar * sb;
    inGameData igd;
};

StateBar statebar;

void InGameState::cycle(Context* ct)
{
  if (keypressed == uiKeyUp)
  {
    ct->psnake->changedir(KEY_UP);
  }
  else if ( keypressed == uiKeyDown)
  {
    ct->psnake->changedir(KEY_DOWN);
  }
  else if (keypressed == uiKeyLeft)
  {
    ct->psnake->changedir(KEY_LEFT);
  }
  else if (keypressed == uiKeyRight)
  {
    ct->psnake->changedir(KEY_RIGHT);
  }
  else if (keypressed == uiKeyMenu)
  {
    ct->setGameSettingState();
  }
  else
  {
  }
  keypressed = 0;
  ct->pgame->Update(ct);
}

void InGameState::draw(Context* ct)
{
  u8g.firstPage();
  do {
    ct->pgame->show();
  } while ( u8g.nextPage() );
}

void GameSettingState::cycle(Context* ct)
{
  if (keypressed == uiKeyMenu)
  {
    ct->setInGameState();
  }
  keypressed = 0;
}
void GameSettingState::draw(Context* ct)
{
  u8g.firstPage();
  do {
    u8g.drawRFrame(4, 2, 120, 60, 6);
    u8g.setFont(u8g_font_helvR14);
    u8g.drawStr(0, 14, "Setting");
  } while ( u8g.nextPage() );
}
void GameOptionState::cycle(Context* ct)
{
  if (keypressed == uiKeyMenu)
  {
    ct->setMainMenuState();
  }
  else if (keypressed == uiKeyUp)
  {
    ct->stage++;
  }
  else if (keypressed == uiKeyDown)
  {
    if (ct->stage > 1)
    {
      ct->stage--;
    }
  }
  else
  {

  }
  keypressed = 0;
}
void GameOptionState::draw(Context* ct)
{
  u8g.firstPage();
  do {
    u8g.drawRFrame(4, 14, 120, 48, 6);
    u8g.setFont(u8g_font_helvR14);
    u8g.drawStr(0, 14, "Option");
    u8g.setFont(u8g_font_profont10r);
    u8g.setPrintPos(10, 32);
    u8g.print("choose level: ");
    u8g.print(ct->stage);
  } while ( u8g.nextPage() );
}
void StageClearState::cycle(Context* ct)
{
  if (ct->pgame != NULL)
  {
    delete(ct->pgame);
    ct->pgame = NULL;
  }
  if (keypressed == uiKeyMenu)
  {
    ct->stage++;
    ct->pgame = new Game(ct->psnake, &gamemap, &statebar, ct->stage);
    ct->setInGameState();
  }
  keypressed = 0;
}
void StageClearState::draw(Context* ct)
{
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_helvR14);
    sprintf(buf, "Stage %d Clear", ct->stage);
    u8g.drawStr(0, 40, buf);
  } while ( u8g.nextPage() );
}

void MainMenuState::cycle(Context* ct)
{
  if (keypressed == uiKeyMenu)
  {
    ct->psnake = new Snake(&gamemap);
    ct->pgame = new Game(ct->psnake, &gamemap, &statebar, ct->stage);
    ct->setInGameState();

  }
  else if (keypressed == uiKeyRight)
  {
    ct->setGameOptionState();
  }
  else
  {
  }
  keypressed = 0;
}
void MainMenuState::draw(Context* ct)
{
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_helvR14);
    u8g.drawStr(5, 20, "Greedy Snake");
    u8g.drawHLine(5, 23, 120);
    u8g.setFont(u8g_font_5x8);
    u8g.drawStr(35, 50, "[Right] option ");
    u8g.drawStr(35, 62, "[M] start ");
  } while ( u8g.nextPage() );
}

void GameOverState::cycle(Context* ct)
{
  if (ct->pgame != NULL)
  {
    delete(ct->pgame);
    ct->pgame = NULL;
  }
  if (ct->psnake != NULL)
  {
    delete(ct->psnake);
    ct->psnake = NULL;
  }
  if (keypressed == uiKeyMenu)
  {
    ct->stage = 1;
    ct->setMainMenuState();
  }
  keypressed = 0;
}
void GameOverState::draw(Context* ct)
{
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_helvR14);
    u8g.drawStr(5, 40, "Game Over");
    u8g.setFont(u8g_font_04b_03r);
    u8g.drawStr(0, 62, "press m key to return");
  } while ( u8g.nextPage() );
}
Context::Context()
{
  pMainMenuState = new MainMenuState();
  pInGameState = new InGameState();
  pGameSettingState = new GameSettingState();
  pGameOptionState = new GameOptionState();
  pStageClearState = new StageClearState();
  pGameOverState = new GameOverState();
  pCurrentState = pMainMenuState;
  pgame = NULL;
  psnake = NULL;
  stage = 1;
}

void Context::setInGameState()
{
  pCurrentState = pInGameState;
}

void Context::setGameSettingState()
{
  pCurrentState = pGameSettingState;
}
void Context::setGameOptionState()
{
  pCurrentState = pGameOptionState;
}
void Context::setStageClearState()
{
  pCurrentState = pStageClearState;
}

void Context::setMainMenuState()
{
  pCurrentState = pMainMenuState;
}
void Context::setGameOverState()
{
  pCurrentState = pGameOverState;
}

void Context::cycle()
{
  pCurrentState->cycle(this);
}
void Context::draw()
{
  pCurrentState->draw(this);
}

void checkkey()
{
  if (digitalRead(uiKeyUp) == LOW)
  {
    keypressed = uiKeyUp;
  }
  else if (digitalRead(uiKeyDown) == LOW)
  {
    keypressed = uiKeyDown;
  }
  else if (digitalRead(uiKeyLeft) == LOW)
  {
    keypressed = uiKeyLeft;
  }
  else if (digitalRead(uiKeyRight) == LOW)
  {
    keypressed = uiKeyRight;
  }
  else if (digitalRead(uiKeyMenu) == LOW)
  {
    keypressed = uiKeyMenu;
  }
  else
  {
  }
}

Context context;

unsigned long      time_isr = 0;//for key debouncing
unsigned long last_time_isr = 0;//for key debouncing
unsigned long debouncing_time = 150;//150ms
void myisr(void)
{
  time_isr = millis();
  if (time_isr - last_time_isr > debouncing_time)
  {
    checkkey();
    last_time_isr = time_isr;
  }
}

void mapGen(int fruitNum)
{
  int fruit = fruitNum;
  int idx = 0;
  for (int i = 0; i < GRID_WIDTH * GRID_HEIGTH; i++)
  {
    gamemap[i] = 0;
  }
  randomSeed(analogRead(0));
  for (int i = 0; i < fruit; i++)
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
}

void loop() {
  context.cycle();
  context.draw();
}

