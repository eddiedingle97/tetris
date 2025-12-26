#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <raylib.h>

#define WINDOWWIDTH 500
#define WINDOWHEIGHT 500
#define TARGETFPS 60

#define GRIDROWSIZE 10
#define GRIDROWCOUNT 15
#define GRIDOFFSETX 10
#define GRIDOFFSETY 10
#define GRIDCOLOR GRAY
#define BLOCKPIXELCOUNT 25

#define MAXFALLTIME 30
#define MINFALLTIME 7

#define ROWCLEARTIMER 15
#define ROWCLEARFLASHES 8

#define ROWCLEARSCORESCALE 100
#define TETGROUNDSCORE 10
#define TETPLACETIMESKIPSCORE 1
#define LINESPERDIFFICULTY 10

#define TEXTOFFSETX 300
#define TEXTOFFSETY 200
#define TEXTINC 25
#define TEXTSIZE 18
#define TEXTCOLOR WHITE

#define NEXTBOXCOLOR GRAY
#define NEXTBOXX 300
#define NEXTBOXY 60
#define NEXTBOXSIDELENGTH BLOCKPIXELCOUNT * 5

#define GAMEOVERTEXTX 10
#define GAMEOVERTEXTY 40
#define GAMEOVERTEXTSIZE 30
#define GAMEOVERTEXTCOLOR RED

int grid[GRIDROWCOUNT][GRIDROWSIZE] = {0};

enum TETTYPE{NONE, LONG, TEE, LEFTL, RIGHTL, LEFTS, RIGHTS, SQUARE};
Color tetcolor[] = {BLANK, SKYBLUE, DARKPURPLE, LIME, DARKBLUE, RED, YELLOW, DARKGREEN};
enum STATES{START, FALL, GROUNDED, ROWCLEAR, END};

struct tet
{
	int type;
	int x;
	int y;
	int offsetx[4];
	int offsety[4];
} falling;

int tetoffsetx[][4] = {
	{0, 0, 0, 0},//NONE
	{0, 0, 0, 0},//LONG
	{0, 0, -1, 1},//TEE
	{0, 0, 0, -1},//LEFTL
	{0, 0, 0, 1},//RIGHTL
	{-1, 0, 0, 1},//LEFTS
	{-1, 0, 0, 1},//RIGHTS
	{0, 0, 1, 1}//SQUARE
};

int tetoffsety[][4] = {
	{0, 0, 0, 0},//NONE
	{-1, 0, 1, 2},//LONG
	{0, -1, 0, 0},//TEE
	{1, 0, -1, -1},//LEFTL
	{1, 0, -1, -1},//RIGHTL
	{1, 1, 0, 0},//LEFTS
	{0, 0, 1, 1},//RIGHTS
	{0, 1, 1, 0}//SQUARE
};

int tetnextboxoffsets[][2] = {
	{0, 0},//NONE
	{-BLOCKPIXELCOUNT / 2, 0},//LONG
	{-BLOCKPIXELCOUNT / 2, -BLOCKPIXELCOUNT * 3 / 4},//TEE
	{0, -BLOCKPIXELCOUNT / 2},//LEFTL
	{-BLOCKPIXELCOUNT, -BLOCKPIXELCOUNT / 2},//RIGHTL
	{-BLOCKPIXELCOUNT / 2, 0},//LEFTS
	{-BLOCKPIXELCOUNT / 2, 0},//RIGHTS
	{-BLOCKPIXELCOUNT, 0}//SQUARE
};

int resetfallcounter(int difficulty);
void rotatefalling(int inputrot);
int checkcollision();
void initnewfallingtet(int type);
int randomtettype();

int main()
{
	//printf("hello world\n");
	//
	
	InitWindow(WINDOWWIDTH, WINDOWHEIGHT, "tetris");

	SetTargetFPS(60);

	int state = START;
	int nexttettype;
	int drawfalling;
	int fallcounter;
	int inputleftright;
	int inputrot;
	int rowstoclear[4] = {-1, -1, -1, -1};
	int rowcleartimer = ROWCLEARTIMER;
	int rowclearflashes = ROWCLEARFLASHES;
	int rowscratch[4][GRIDROWSIZE] = {0};
	int totalscore = 0;
	int totallines = 0;
	int printgameovertext = 0;

	srand(time(NULL));
	
	if(DEBUG)
	{
		int j;
		for(j = 0; j < 9; j++)
		{
			grid[0][j] = TEE;
		}
	}

	while(!WindowShouldClose())
	{
		//do game tick
	
		switch(state)
		{
			case START:
				nexttettype = randomtettype();
						  
				initnewfallingtet(randomtettype());

				drawfalling = 1;
				fallcounter = resetfallcounter(totallines / LINESPERDIFFICULTY);
				state = FALL;
				break;
			case FALL:
				fallcounter--;
				if(fallcounter == 0)
				{
					fallcounter = resetfallcounter(totallines / LINESPERDIFFICULTY);
					falling.y -= 1;
					if(checkcollision())
					{
						falling.y += 1;
						state = GROUNDED;
						break;
					}
				}

				inputleftright = 0;
				inputrot = 0;
				int inputdown = 0;
				
				inputleftright = -IsKeyPressed(KEY_LEFT) + IsKeyPressed(KEY_RIGHT);

				inputrot = IsKeyPressed(KEY_UP);
				
				inputdown = IsKeyDown(KEY_DOWN);
					
				if(inputdown)
				{
					fallcounter = 1;
					totalscore += TETPLACETIMESKIPSCORE;
				}
				
				falling.x += inputleftright;

				if(checkcollision())
					falling.x -= inputleftright;

				if(inputrot != 0)
					rotatefalling(inputrot);

				if(checkcollision())
					rotatefalling(-inputrot);

				break;
			case GROUNDED:
				int i;

				fallcounter--;
				if(fallcounter == 0)
				{
					fallcounter = resetfallcounter(totallines / LINESPERDIFFICULTY);
					
					int i;
					for(i = 0; i < 4; i++)
					{
						int c = falling.x + falling.offsetx[i];
						int r = falling.y + falling.offsety[i];

						if(grid[r][c] != 0)
							state = END;

						grid[r][c] = falling.type;
					}
					if(state == END)
						break;
					
					int r, c, count = 0, consecutive = 0, rowcomplete = 1, score = 0;
					for(r = 0; r < GRIDROWCOUNT; r++)
					{
						rowcomplete = 1;
						for(c = 0; c < GRIDROWSIZE; c++)
						{
							rowcomplete = rowcomplete && grid[r][c];
						}
						if(rowcomplete)
						{
							rowstoclear[count] = r;
							count++;
							consecutive++;

							if(r == GRIDROWCOUNT - 1)
								score += consecutive * consecutive;
						}
						else
						{
							score += consecutive * consecutive;
							consecutive = 0;
						}
					}

					totallines += count;
					totalscore += TETGROUNDSCORE + score * ROWCLEARSCORESCALE;

					if(count == 0)
					{
						state = FALL;
						initnewfallingtet(nexttettype);	
						nexttettype = randomtettype();
						break;
					}
					else
						state = ROWCLEAR;
				}
								
				break;
			case ROWCLEAR:
				drawfalling = 0;
				
				rowcleartimer--;
				if(rowcleartimer == 0)
				{
					rowclearflashes--;

					int r, c;
					for(r = 0; r < 4; r++)
					{
						if(rowstoclear[r] >= 0)
						{
							for(c = 0; c < GRIDROWSIZE; c++)
							{
								if(rowclearflashes == 0)
								{
									rowscratch[r][c] = 0;
									grid[rowstoclear[r]][c] = 0;
								}
								else
								{
									int temp = rowscratch[r][c];
									rowscratch[r][c] = grid[rowstoclear[r]][c];
									grid[rowstoclear[r]][c] = temp;
								}
							}
						}
					}

					rowcleartimer = ROWCLEARTIMER;
				}
				if(rowclearflashes == 0)
				{
					int i, r, c;
					for(i = 0; i < 4; i++)
					{
						if(rowstoclear[i] >= 0)
						{
							rowstoclear[i] -= i;
							for(r = rowstoclear[i]; r < GRIDROWCOUNT - 1; r++)
							{
								for(c = 0; c < GRIDROWSIZE; c++)
								{
									grid[r][c] = grid[r + 1][c];
								}
							}
						}
					}

					rowclearflashes = ROWCLEARFLASHES;
					initnewfallingtet(nexttettype);
					nexttettype = randomtettype();
					drawfalling = 1;
					state = FALL;
					
					for(i = 0; i < 4; i++)
						rowstoclear[i] = -1;
				}

				break;
			case END:
				printgameovertext = 1;
				drawfalling = 0;
				break;
		}

		//do draw
		BeginDrawing();

			ClearBackground(BLACK);

			if(DEBUG)
				DrawFPS(50,50);

			char buf[256];
			int inc = 0;	
			DrawText("level:", TEXTOFFSETX, TEXTOFFSETY + inc++ * TEXTINC, TEXTSIZE, TEXTCOLOR);
			snprintf(buf, 255, "%d", totallines / LINESPERDIFFICULTY);
			DrawText(buf, TEXTOFFSETX, TEXTOFFSETY + inc++ * TEXTINC, TEXTSIZE, TEXTCOLOR);


			DrawText("lines:", TEXTOFFSETX, TEXTOFFSETY + inc++ * TEXTINC, TEXTSIZE, TEXTCOLOR);
			snprintf(buf, 255, "%d", totallines);
			DrawText(buf, TEXTOFFSETX, TEXTOFFSETY + inc++ * TEXTINC, TEXTSIZE, TEXTCOLOR);
			
			DrawText("score:", TEXTOFFSETX, TEXTOFFSETY + inc++ * TEXTINC, TEXTSIZE, TEXTCOLOR);
			snprintf(buf, 255, "%d", totalscore);
			DrawText(buf, TEXTOFFSETX, TEXTOFFSETY + inc++ * TEXTINC, TEXTSIZE, TEXTCOLOR);

			int i;
			int gridoffsety = WINDOWHEIGHT - GRIDOFFSETY;

			if(drawfalling)
			{
				Color fallcolor = tetcolor[falling.type];

				int centerx = GRIDOFFSETX + falling.x * BLOCKPIXELCOUNT;
				int centery = gridoffsety - GRIDROWCOUNT * BLOCKPIXELCOUNT - (falling.y - GRIDROWCOUNT) * BLOCKPIXELCOUNT;
				for(i = 0; i < 4; i++)
					DrawRectangle(centerx + falling.offsetx[i] * BLOCKPIXELCOUNT, centery - BLOCKPIXELCOUNT - falling.offsety[i] * BLOCKPIXELCOUNT, BLOCKPIXELCOUNT, BLOCKPIXELCOUNT, fallcolor);
			}

			DrawRectangleLines(NEXTBOXX, NEXTBOXY, NEXTBOXSIDELENGTH, NEXTBOXSIDELENGTH, NEXTBOXCOLOR);

			for(i = 0; i < 4; i++)
			{
				DrawRectangle(NEXTBOXX + NEXTBOXSIDELENGTH / 2 + tetoffsetx[nexttettype][i] * BLOCKPIXELCOUNT + tetnextboxoffsets[nexttettype][0], NEXTBOXY + NEXTBOXSIDELENGTH / 2 - tetoffsety[nexttettype][i] * BLOCKPIXELCOUNT + tetnextboxoffsets[nexttettype][1], BLOCKPIXELCOUNT, BLOCKPIXELCOUNT, tetcolor[nexttettype]);
				DrawRectangleLines(NEXTBOXX + NEXTBOXSIDELENGTH / 2 + tetoffsetx[nexttettype][i] * BLOCKPIXELCOUNT + tetnextboxoffsets[nexttettype][0], NEXTBOXY + NEXTBOXSIDELENGTH / 2 - tetoffsety[nexttettype][i] * BLOCKPIXELCOUNT + tetnextboxoffsets[nexttettype][1], BLOCKPIXELCOUNT, BLOCKPIXELCOUNT, GRIDCOLOR);
			}

			int r, c;
			for(r = 0; r < GRIDROWCOUNT; r++)
			{
				for(c = 0; c < GRIDROWSIZE; c++)
				{
					DrawRectangle(GRIDOFFSETX + c * BLOCKPIXELCOUNT, gridoffsety - BLOCKPIXELCOUNT - r * BLOCKPIXELCOUNT, BLOCKPIXELCOUNT, BLOCKPIXELCOUNT, tetcolor[grid[r][c]]);
				}
			}

			for(i = 0; i < GRIDROWSIZE + 1; i++)
			{
				int iterx = GRIDOFFSETX + i * BLOCKPIXELCOUNT;
				DrawLine(iterx, gridoffsety - GRIDROWCOUNT * BLOCKPIXELCOUNT, iterx, gridoffsety, GRIDCOLOR);
			}
			for(i = 0; i < GRIDROWCOUNT + 1; i++)
			{
				int itery = gridoffsety - i * BLOCKPIXELCOUNT;
				DrawLine(GRIDOFFSETX, itery, GRIDOFFSETX + GRIDROWSIZE * BLOCKPIXELCOUNT, itery, GRIDCOLOR);
			}
		
			if(printgameovertext)
			{
				snprintf(buf, 255, "GAME OVER");
				DrawText(buf, GAMEOVERTEXTX, GAMEOVERTEXTY, GAMEOVERTEXTSIZE, GAMEOVERTEXTCOLOR);
			}

		EndDrawing();
	}

	return 0;
}

void rotatefalling(int inputrot)
{
	int i;
	for(i = 0; i < 4; i++)
	{
		int tempx = falling.offsetx[i];
		falling.offsetx[i] = inputrot * falling.offsety[i];
		falling.offsety[i] = -inputrot * tempx;
	}
}

int checkcollision()
{
	int i;
	for(i = 0; i < 4; i++)
	{
		int blockx = falling.x + falling.offsetx[i];
		int blocky = falling.y + falling.offsety[i];

		if(blocky < 0)
			return 1;
		else if(blockx < 0 || blockx >= GRIDROWSIZE)
			return 1;
		else if(blocky >= GRIDROWCOUNT);
		else if(grid[blocky][blockx])
			return 1;
	}
	return 0;
}

int resetfallcounter(int difficulty)
{
	if(difficulty < 0)
		return MAXFALLTIME;
	return MAXFALLTIME - difficulty > MINFALLTIME ? MAXFALLTIME - difficulty : MINFALLTIME;
}

void initnewfallingtet(int type)
{
	falling.type = type;
	falling.x = GRIDROWSIZE / 2;
	falling.y = GRIDROWCOUNT - 2;
	memcpy(falling.offsetx, tetoffsetx[falling.type], sizeof(int) * 4);
	memcpy(falling.offsety, tetoffsety[falling.type], sizeof(int) * 4);
}

int randomtettype()
{
	return (rand() % 7) + 1;
}
