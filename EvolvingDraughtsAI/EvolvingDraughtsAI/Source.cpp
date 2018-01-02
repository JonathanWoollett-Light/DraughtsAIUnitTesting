#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <time.h>


struct AI {
	double	n,//pawn multiplication constant
			p,//pawn power constant
			m,//king multiplication constant
			q,//king power constant
			winConstant;//the weight of wins
	int points;
	AI() {
		this->points = 0;//points = nubmer of wins or loses versus other AIs, -1 = 1 lose, +1 = 1 win
	}
};

struct action {
	int startX,
		startY,
		endX,
		endY;
	action(int startX, int startY, int endX, int endY)
	{
		this->startX = startX;
		this->startY = startY;
		this->endX = endX;
		this->endY = endY;
	}
};

struct actionListItem {
	actionListItem * nextItem;
	action * data;
	actionListItem() {
		this->nextItem = nullptr;
		this->data = nullptr;
	}
	actionListItem(action * data) {
		this->nextItem = nullptr;
		this->data = data;
	}
	~actionListItem() {//cyncial about this method of freeing memory but unsure

		delete this->data;
		this->data = nullptr;
		delete this->nextItem;
		this->nextItem = nullptr;
	}
};

double	takingPossibilities(int isOdd, int y, int x, int yOffset, int xOffset, int array[8][8], int depth, int AINumber, int threadNumber),
		plotMoves(int array[8][8], int AINumber, int threadNumber, int depth = 0);


void	listAddAction(actionListItem* header, int startX, int startY, int endX, int endY),
		playGames(int threadNumber, int inGameArray[8][8]),
		printListActions(actionListItem* header),
		listReset(actionListItem listArray[100]),
		printArray(int array[8][8]),
		intializeShit(),
		evolve();

const int	threadAmount = 10,//(100000 % threadAmount) must equal 0
			maxDepth = 2,
			movesLim = 1000,
			numbOfAIConsts = 5;


int move(int AINumber, int threadNumber);

actionListItem * actionPointerHolder[threadAmount];
actionListItem possibleActions[threadAmount][100];
double possibleMovesValues[threadAmount][100];

int width[threadAmount],
	isOdd[threadAmount],
	max[threadAmount],
	possibleMoves[threadAmount],
	topAI[threadAmount],
	startPawnDif[threadAmount],
	startKingDif[threadAmount],
	moves[threadAmount],
	pawnDif[threadAmount],
	kingDif[threadAmount];

//------------------------------------------------
int startArray[8][8] = {//1 is player 1, 3 is player 1 king, 2 is player 2, 4 is player 2 king
	{ 1,0,1,0,1,0,1,0 },
	{ 0,1,0,1,0,1,0,1 },
	{ 1,0,1,0,1,0,1,0 },
	{ 0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0 },
	{ 0,2,0,2,0,2,0,2 },
	{ 2,0,2,0,2,0,2,0 },
	{ 0,2,0,2,0,2,0,2 }
};
int gameArray[threadAmount][8][8];

AI AIList[100000];

double	bestN = 5.0, 
		bestP = 5.0, 
		bestM = 5.0, 
		bestQ = 5.0, 
		bestWinConstant = 5.0;

int main() {

	time_t timerOne = time(0);
	time_t timerTwo = time(0);

	const int magReductions = 2;
	int currentItem,
		bestPointer;
	for (double magnitude = 1; magnitude > pow(10, -magReductions); magnitude /= 10) {
		intializeShit();

		for (int a = 0; a < 10; a++) {
			for (int b = 0; b < 10; b++) {
				for (int c = 0; c < 10; c++) {
					for (int d = 0; d < 10; d++) {
						for (int e = 0; e < 10; e++) {
							currentItem = (10000 * a) + (1000 * b) + (100 * c) + (10 * d) + e;
							AIList[currentItem].points = 0;
							AIList[currentItem].n = ((a - 5) * magnitude) + bestN;
							AIList[currentItem].p = ((b - 5) * magnitude) + bestP;
							AIList[currentItem].m = ((c - 5) * magnitude) + bestM;
							AIList[currentItem].q = ((d - 5) * magnitude) + bestQ;
							AIList[currentItem].winConstant = ((e - 5) * magnitude) + bestWinConstant;
						}
					}
				}
			}
		}
		std::cout << "\n\n\n\nStart of evolution on magnitude: " << magnitude << std::endl;
		
		evolve();

		bestPointer = 0;
		for (int i = 1; i < 100000; i++)
		{
			if (AIList[i].points > AIList[bestPointer].points)
			{
				bestPointer = i;
			}
		}

		bestN = AIList[bestPointer].n;
		bestP = AIList[bestPointer].p;
		bestM = AIList[bestPointer].m;
		bestQ = AIList[bestPointer].q;
		bestWinConstant = AIList[bestPointer].winConstant;

		
		for (int i = 90000; i < 100000; i++)
		{
			std::cout << i << " : " << AIList[i].points << std::endl;
		}
		timerOne = time(0) - timerOne;
		std::cout << "\n\n\n\nEnd of evolution on magnitude:" << magnitude << "\nThis evolution took " << ((double)timerOne) / (1000*60*60) << " hours" << std::endl;
		std::cout << "Best AI of this evolution at index " << bestPointer << " with " << AIList[bestPointer].points << " points\n" << std::endl;
	}
	timerTwo = time(0) - timerTwo;
	std::cout << "\n\n\n\nProgram took" << ((double)timerTwo) / (1000 * 60 * 60) << " hours to complete" << std::endl;
	system("pause");
	return 0;
}

void intializeShit() {//veery unsure about this.
	for (int i = 0; i < threadAmount; i++) 
	{
		width[i] = -1;
		max[i] = 0;
		possibleMoves[i] = 0;
		moves[i] = 0;
		pawnDif[i] = 0;
		kingDif[i] = 0;
		std::memcpy(gameArray[i], startArray, 8 * 8 * sizeof int());//sets gameArray[i] = startArray
	}
}

void evolve() {
	std::thread threadArray[threadAmount];
	
	for (int i = 0; i < threadAmount; i++) {
		std::cout << "Thread numb:" << i << " | AI group:" << 100000 * i / threadAmount << " - " << (100000 * (i + 1) / threadAmount) - 1 << std::endl;
		threadArray[i] = std::thread(playGames, i, gameArray[i]);
	}
	for (int i = 0; i < threadAmount; i++) {
		threadArray[i].join();
	}
}

void playGames(int threadNumber, int inGameArray[8][8]) {
	int end = 0;
	for (int i = 100000 * threadNumber / threadAmount; i < 100000 * (threadNumber + 1) / threadAmount; i++) {//(100000 % threadAmount) must equal 0
		for (int t = 0; t < 100000; t++) {//(THE LIMIT ON T SHOULD BE 100000 BUT HAS BEEN REDUCED DUE TO LACKING COMPUTING POWER) notably 't' is not intially set to 'i' since it is neccessary each AI plays every other AI twice, starting 1st and 2nd
			if (t == i) {//checking so that the AI doesnt play itself, which would be a waste of time
				continue;
			}
			topAI[threadNumber] = 1;
			moves[threadNumber] = 0;
			do {//this 'do while' replaced a 'while' so I could avoid having to reassigned 'end' to '0' every iteration of 't'
				moves[threadNumber]++;
				startPawnDif[threadNumber] = 0;
				startKingDif[threadNumber] = 0;
				//------calculing starting board presence difference------
				for (int y = 0; y < 8; y++) {
					for (int x = 0; x < 8; x++) {
						if (inGameArray[y][x] < 3) {
							if (inGameArray[y][x] == (1 + topAI[threadNumber])) {
								startPawnDif[threadNumber]++;
							}
							else {
								startPawnDif[threadNumber]--;
							}
						}
						else {
							if (inGameArray[y][x] == (3 + topAI[threadNumber])) {
								startKingDif[threadNumber]++;
							}
							else {
								startKingDif[threadNumber]--;
							}
						}
					}
				}
				//--------------------------------------------------------
				
				
				if (moves[threadNumber] == movesLim) {//if game has reached excessive number of moves
					end = -1;//end = -1 is setting game to a draw
				}
				else if (topAI[threadNumber] == 1) {
					end = move(i, threadNumber);
				}
				else {
					end = move(t, threadNumber);
				}
				topAI[threadNumber] = abs(topAI[threadNumber] - 1);//flipping the current AI from 0 -> 1 or 1 -> 0

			 } while (end == 0);//often I find 'do while' loops a little worse for code readbility than 'while' loops but I think it's worth it

			if (end != -1) {//checks to make sure game didnt merely run out of turns
				if (topAI[threadNumber] == 1) {//on AI[i]'s move topAI = 1
					//std::cout << "AIList[" << t << "] won" << std::endl;
					AIList[i].points--;
					AIList[t].points++;
				} 
				else {//on AI[t]'s move topAI = 0
					//std::cout << "AIList[" << i << "] won" << std::endl;
					AIList[i].points++;
					AIList[t].points--;
				}
			}
			
		}
		if (AIList[i].points > 100000) {//if AIList[i].points > 100000 then it is impossible for another AI to gain more points (notably this is 100000 instead of 50000 becuase each AI must play each other AI twice)
			break;
		}
		if (i % 1000 == 0) {
			std::cout << i << std::endl;
		}
		
	}
	std::cout << "\nfinished thread" << std::endl;
}

int move(int AINumber, int threadNumber) {
	
	//-----------------------------------------------checking if end
	int t,
		lim,
		canJump = 0,//neccessary to assign due to it being used in an 'if' before it is then assigned then used (possible reording could make this unneccessary for a very slight optimisation)
		yOffset = 0,//see 'canJump' comment
		xOffset;

	possibleMoves[threadNumber] = 0;//doesnt need to be set before function, but will need to be set at the start of the 1st generation children
	
	for (int y = 0; y<8; y++) {//checking number of possible moves
		for (int x = 0; x<8; x++) {
			if (gameArray[threadNumber][y][x] != 0 && (gameArray[threadNumber][y][x] % 2) == topAI[threadNumber]) {
				if (gameArray[threadNumber][y][x] < 3) {//I think is more efficient than 'innerArray[y][x] == 3 || innerArray[y][x] == 4'
					t = -2 + (2 * topAI[threadNumber]);
					lim = t + 2;
				}
				else {
					t = -2;
					lim = 2;
				}
				//-----------------------------------------------------
				for (t; t<lim; t++) {
					
					switch (t) {
					case 1://down left
						if (y < 7 && x > 0) {//check if move is within board boundaries (same applies to following 'if's right after 'case')
							xOffset = -1;
							yOffset = 1;
							if (y < 6 && x > 1) {//check if a jump would be within board boundaries (same applies to following 'if's in same position in this switch)
								canJump = 1;//this variable and the previous if may be ultimately unneccessary but to be safe they are here (reason: don't think running comparison on array using indexs outside its bounds results in consistant results)
							}
						}
						break;
					case 0://down right
						if (y < 7 && x < 7) {//check comment on if in same position in case 1
							xOffset = 1;
							yOffset = 1;
							if (y < 6 && x < 6) {//check comment on if in same position in case 1
								canJump = 1;
							}
						}
						break;
					case -1://up right
						if (y > 0 && x < 7) {//check comment on if in same position in case 1
							xOffset = 1;
							yOffset = -1;
							if (y > 1 && x < 6) {//check comment on if in same position in case 1
								canJump = 1;
							}
						}
						break;
					case -2://up left
						if (y > 0 && x > 0) {//check comment on if in same position in case 1
							xOffset = -1;
							yOffset = -1;
							if (y > 1 && x > 1) {//check comment on if in same position in case 1
								canJump = 1;
							}
						}//despite missing a break in the final case of a switch being bad practice, this switch will never be expanded and thus I think the lack of a pointless break provides no downsides, and provides a very minor optimisation
					}
					if (yOffset != 0) {//once again checking this may be unneccessary but I don't beleive so (check comment on 'case 1:' in switch for reason)
						if (gameArray[threadNumber][y + yOffset][x + xOffset] == 0) {//could combine this 'if' and following 'else if' but doesn't make any performance difference (I think) and negatively affects readbility
							possibleMoves[threadNumber]++;
						}
						else if (canJump == 1 && ((gameArray[threadNumber][y + yOffset][x + xOffset] % 2) == (1 - topAI[threadNumber])) && (gameArray[threadNumber][y + (2 * yOffset)][x + (2 * xOffset)] == 0)) {
							possibleMoves[threadNumber]++;
						}
					}
					canJump = 0;//resetting for use in 'if' statements
					yOffset = 0;//check comment above
				}
			}
		}
	}
	if (possibleMoves[threadNumber] == 0) {//if their are no possible moves return 1 signifying a loss
		return 1;//means game ended (end = true)
	} 
	else {
		width[threadNumber] = -1;
		
		for (int t = 0; t < 100; t++) {
			possibleMovesValues[threadNumber][t] = 0;
		}
		plotMoves(gameArray[threadNumber], AINumber, threadNumber);

		max[threadNumber] = 0;
		for (int t = 0; t < width[threadNumber] + 1; t++) {//finding move with best value
			if (possibleMovesValues[threadNumber][t] > possibleMovesValues[threadNumber][max[threadNumber]]) {
				max[threadNumber] = t;
			}
		}
		//------------making the actual move------------
		for (actionListItem * i = &possibleActions[threadNumber][max[threadNumber]]; i != nullptr; i = i->nextItem)
		{
			if (i->data->endY == (7 * topAI[threadNumber])) {
				gameArray[threadNumber][i->data->endY][i->data->endX] = 4 - topAI[threadNumber];
			}
			else {
				gameArray[threadNumber][i->data->endY][i->data->endX] = gameArray[threadNumber][i->data->startY][i->data->startX];
			}
			gameArray[threadNumber][i->data->startY][i->data->startX] = 0;
			if (abs(i->data->endX - i->data->startX) == 2) {
				gameArray[threadNumber][(i->data->startY + i->data->endY) / 2][(i->data->startX + i->data->endX) / 2] = 0;
			}
		}
		//----------------------------------------------
		listReset(possibleActions[threadNumber]);
		return 0;//means game in progress (end = false)
	}
}

double plotMoves(int currentArray[8][8], int AINumber, int threadNumber, int depth) {
	//----------------evaluates current board favorability----------------
	if (depth == maxDepth) {
		pawnDif[threadNumber] = 0;
		kingDif[threadNumber] = 0;
		for (int y = 0; y < 8; y++) {
			for (int x = 0; x < 8; x++) {
				if (currentArray[y][x] < 3) {
					if (currentArray[y][x] == (2 - topAI[threadNumber])) {
						pawnDif[threadNumber]++;
					}
					else {
						pawnDif[threadNumber]--;
					}
				}
				else {
					if (currentArray[y][x] == (4 - topAI[threadNumber])) {
						kingDif[threadNumber]++;
					}
					else {
						kingDif[threadNumber]--;
					}
				}
			}
		}
		kingDif[threadNumber] -= startKingDif[threadNumber];
		pawnDif[threadNumber] -= startPawnDif[threadNumber];
		if (depth == 1) {//has to be depth 1 since depth 0 would only be 1 result which would be the value of taking no action
			possibleMovesValues[threadNumber][width[threadNumber]] = ((AIList[AINumber].n*pow(pawnDif[threadNumber], AIList[AINumber].p)) + (AIList[AINumber].m*pow(kingDif[threadNumber], AIList[AINumber].q)));
		}
		return ((AIList[AINumber].n*pow(pawnDif[threadNumber], AIList[AINumber].p)) + (AIList[AINumber].m*pow(kingDif[threadNumber], AIList[AINumber].q)));
	}
	//--------------------------------------------------------------------
	//----------checks if any men turn into kings----------
	int innerArray[8][8];
	std::memcpy(innerArray, currentArray, 8 * 8 * sizeof int());
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			if (innerArray[y][x] != 0) {
				isOdd[threadNumber] = 0;
				if ((innerArray[y][x] % 2) == 1) {
					isOdd[threadNumber] = 1;
				}
				if (y == (isOdd[threadNumber] * 7)) {
					innerArray[y][x] = (4 - isOdd[threadNumber]);
				}
			}
		}
	}
	//-----------------------------------------------------
	int xOffset;
	int yOffset;
	int t;
	int lim;
	int canJump;
	
	double totalValue = 0;
	int willJump = 0;
	//-------------check if AI or player turn-------------
	if ((depth % 2) == 0) {
		isOdd[threadNumber] = topAI[threadNumber];
	}
	else {//probably some way of simplifying this
		isOdd[threadNumber] = abs(topAI[threadNumber] - 1);
	}
	//----------------------------------------------------
	double internalWidth = 0;
	//--------------------------------counting number of possible moves and checking if game ended--------------------------------
	for (int y = 0; y<8; y++) {
		for (int x = 0; x<8; x++) {
			if ((innerArray[y][x] != 0) && ((innerArray[y][x] % 2) == isOdd[threadNumber])) {
				//---------------checking possible moves---------------
				if (innerArray[y][x] < 3) {
					t = -2 + (2 * isOdd[threadNumber]);
					lim = t + 2;
				}
				else {
					t = -2;
					lim = 2;
				}
				//-----------------------------------------------------
				for (t; t<lim; t++) {
					canJump = 0;
					yOffset = 0;
					switch (t) {
					case 1://down left
						if (y < 7 && x > 0) {
							xOffset = -1;
							yOffset = 1;
							if (y < 6 && x > 1) {
								canJump = 1;
							}
						}
						break;
					case 0://down right
						if (y < 7 && x < 7) {
							xOffset = 1;
							yOffset = 1;
							if (y < 6 && x < 6) {
								canJump = 1;
							}
						}
						break;
					case -1://up right
						if (y > 0 && x < 7) {
							xOffset = 1;
							yOffset = -1;
							if (y > 1 && x < 6) {
								canJump = 1;
							}
						}
						break;
					case -2://up left
						if (y > 0 && x > 0) {
							xOffset = -1;
							yOffset = -1;
							if (y > 1 && x > 1) {
								canJump = 1;
							}
						}
					}
					if (yOffset != 0) {
						if (innerArray[y + yOffset][x + xOffset] == 0) {
							internalWidth++;
						}
						else if ((canJump == 1) && ((innerArray[y + yOffset][x + xOffset] % 2) == (1 - isOdd[threadNumber])) && (innerArray[y + (2 * yOffset)][x + (2 * xOffset)] == 0)) {
							internalWidth++;
							willJump = 1;
						}
					}
				}
			}
		}
	}
	int isAI = 0;
	if ((depth % 2) == 0) {
		isAI = 1;
	}
	if (internalWidth == 0) {//checks if there are any possible moves
		if (depth == 1) {//has to be depth 1 since depth 0 would only be 1 result which would be the value of taking no action
			possibleMovesValues[threadNumber][width[threadNumber]] = AIList[AINumber].winConstant;//if the AI can make no moves it is pointless to return anytrhingy
		}
		if (isAI == 1) {
			return -(AIList[AINumber].winConstant);//-winConstant = AI lose
		}
		else if (isAI == 0) {
			return AIList[AINumber].winConstant;//winConstant = AI win
		}
	}
	int tempArray[8][8];
	std::memcpy(tempArray, innerArray, 8 * 8 * sizeof int());
	//----------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------carrying out move or jumps------------------------------------------------------
	if (willJump == 1) {
		for (int y = 0; y<8; y++) {
			for (int x = 0; x<8; x++) {
				if (innerArray[y][x] != 0 && (innerArray[y][x] % 2) == isOdd[threadNumber]) {
					//---------------checking possible moves---------------
					if (innerArray[y][x] < 3) {
						t = -2 + (2 * isOdd[threadNumber]);
						lim = t + 2;
					}
					else {
						t = -2;
						lim = 2;
					}
					//-----------------------------------------------------
					for (t; t<lim; t++) {
						yOffset = 0;
						switch (t) {
						case 1://down left
							if (y < 6 && x > 1) {
								xOffset = -1;
								yOffset = 1;
							}
							break;
						case 0://down right
							if (y < 6 && x < 6) {
								xOffset = 1;
								yOffset = 1;
							}
							break;
						case -1://up right
							if (y > 1 && x < 6) {
								xOffset = 1;
								yOffset = -1;
							}
							break;
						case -2://up left
							if (y > 1 && x > 1) {
								xOffset = -1;
								yOffset = -1;
							}
						}
						if ((yOffset != 0) && ((innerArray[y + yOffset][x + xOffset] % 2) == (1 - isOdd[threadNumber])) && (innerArray[y + yOffset][x + xOffset] != 0) && (innerArray[y + (2 * yOffset)][x + (2 * xOffset)] == 0)) {

							tempArray[y + (2 * yOffset)][x + (2 * xOffset)] = tempArray[y][x];
							tempArray[y + yOffset][x + xOffset] = 0;
							tempArray[y][x] = 0;
							if (depth == 0) {
								width[threadNumber]++;
								possibleActions[threadNumber][width[threadNumber]] = *(new actionListItem(new action(x, y, x + (2 * xOffset), y + (2 * yOffset))));
							}
							totalValue += takingPossibilities(isOdd[threadNumber], y, x, yOffset, xOffset, tempArray, depth, AINumber, threadNumber);
						}
					}
				}
			}
		}
	}
	else {
		for (int y = 0; y<8; y++) {
			for (int x = 0; x<8; x++) {
				if (innerArray[y][x] != 0 && (innerArray[y][x] % 2) == isOdd[threadNumber]) {
					//---------------checking possible moves---------------
					if (innerArray[y][x] < 3) {
						t = -2 + (2 * isOdd[threadNumber]);
						lim = t + 2;
					}
					else {
						t = -2;
						lim = 2;
					}
					//-----------------------------------------------------
					for (t; t<lim; t++) {
						yOffset = 0;
						switch (t) {
						case 1://down left
							if (y < 7 && x > 0) {
								xOffset = -1;
								yOffset = 1;
							}
							break;
						case 0://down right
							if (y < 7 && x < 7) {
								xOffset = 1;
								yOffset = 1;
							}
							break;
						case -1://up right
							if (y > 0 && x < 7) {
								xOffset = 1;
								yOffset = -1;
							}
							break;
						case -2://up left
							if (y > 0 && x > 0) {
								xOffset = -1;
								yOffset = -1;
							}
						}
						if ((yOffset != 0) && (innerArray[y + yOffset][x + xOffset] == 0)) {
							
							tempArray[y + yOffset][x + xOffset] = innerArray[y][x];
							tempArray[y][x] = 0;
							if (depth == 0) {
								width[threadNumber]++;
								possibleActions[threadNumber][width[threadNumber]] = *(new actionListItem(new action(x, y, x + xOffset, y + yOffset)));
							}
							totalValue += plotMoves(tempArray, AINumber, threadNumber, (depth + 1));
						}
					}
				}
			}
		}
	}
	//---------------------------------------------------------------------------------------------------------------------------------------
	if (depth == 1) {//has to be depth 1 since depth 0 would only be 1 result which would be the value of taking no action
		possibleMovesValues[threadNumber][width[threadNumber]] = (totalValue / internalWidth);
	}
	return (totalValue / internalWidth);
}

double takingPossibilities(int isOdd, int y, int x, int yOffset, int xOffset, int takingArray[8][8], int depth, int AINumber, int threadNumber) {
	double totalValue = 0;
	actionPointerHolder[threadNumber] = nullptr;
	int tempJumpArray[8][8];
	std::memcpy(tempJumpArray, takingArray, 8 * 8 * sizeof int());
	totalValue += plotMoves(tempJumpArray, AINumber, threadNumber, (depth + 1));
	y += 2 * yOffset;
	x += 2 * xOffset;
	//------------------------------
	int t,
		lim;

	if (takingArray[y][x] < 3) {//I think is more efficient than 'currentArray[y][x] == 3 || currentArray[y][x] == 4'
		t = -2 + (2 * isOdd);
		lim = t + 2;
	}
	else {
		t = -2;
		lim = 2;
	}
	//------------------------------
	for (t; t < lim; t++) {
		yOffset = 0;
		switch (t) {
		case 1://down left
			if (y < 6 && x > 1) {
				xOffset = -1;
				yOffset = 1;
			}
			break;
		case 0://down right
			if (y < 6 && x < 6) {
				xOffset = 1;
				yOffset = 1;
			}
			break;
		case -1://up right
			if (y > 1 && x < 6) {
				xOffset = 1;
				yOffset = -1;
			}
			break;
		case -2://up left
			if (y > 1 && x > 1) {
				xOffset = -1;
				yOffset = -1;
			}
		}
		if ((yOffset != 0) && ((takingArray[y + yOffset][x + xOffset] % 2) == (1 - isOdd)) && (takingArray[y + yOffset][x + xOffset] != 0) && (takingArray[y + (2 * yOffset)][x + (2 * xOffset)] == 0)) {
			if (depth == 0) {
				actionPointerHolder[threadNumber] = &possibleActions[threadNumber][width[threadNumber]];
				width[threadNumber]++;
				while (actionPointerHolder[threadNumber] != nullptr) {
					if (actionPointerHolder[threadNumber]->nextItem != nullptr || (actionPointerHolder[threadNumber]->data->endX == x && actionPointerHolder[threadNumber]->data->endY == y)) {
						listAddAction(&possibleActions[threadNumber][width[threadNumber]], actionPointerHolder[threadNumber]->data->startX, actionPointerHolder[threadNumber]->data->startY, actionPointerHolder[threadNumber]->data->endX, actionPointerHolder[threadNumber]->data->endY);
						actionPointerHolder[threadNumber] = actionPointerHolder[threadNumber]->nextItem;
					}
					else {
						break;
					}
				}
				listAddAction(&possibleActions[threadNumber][width[threadNumber]], x, y, x + (2 * xOffset), y + (2 * yOffset));
			}
			int tempTakingArray[8][8];
			std::memcpy(tempTakingArray, takingArray, 8 * 8 * sizeof int());
			tempTakingArray[y + (2 * yOffset)][x + (2 * xOffset)] = takingArray[y][x];
			tempTakingArray[y + yOffset][x + xOffset] = 0;
			tempTakingArray[y][x] = 0;
			totalValue += takingPossibilities(isOdd, y, x, yOffset, xOffset, tempTakingArray, depth, AINumber, threadNumber);
		}
	}
	return totalValue;
}

void printArray(int printArray[8][8]) {
	printf("\n  0 1 2 3 4 5 6 7\n  ----------------");
	for (int y = 0; y<8; y++) {
		printf("\n%d|", y);
		for (int x = 0; x<8; x++) {
			printf("%d,", printArray[y][x]);
		}
		printf("|%d", y);
	}
	printf("\n  ----------------\n  0 1 2 3 4 5 6 7\n");
}

void listReset(actionListItem listArray[100]) {
	for (int i = 0; i < 100; i++) {

		delete listArray[i].data;
		listArray[i].data = nullptr;
		delete listArray[i].nextItem;
		listArray[i].nextItem = nullptr;
	}
}

void listAddAction(actionListItem* header, int startX, int startY, int endX, int endY) {
	if (header->data == nullptr) {
		header->data = new action(startX, startY, endX, endY);
	}
	else {
		actionListItem * pointer = header;
		while (pointer->nextItem != nullptr)
		{
			pointer = pointer->nextItem;
		}
		pointer->nextItem = new actionListItem(new action(startX, startY, endX, endY));
	}
}

void printListActions(actionListItem* header) {
	printf("--------------");
	for (actionListItem * i = header; i != nullptr; i = i->nextItem)
	{
		printf("\n(%d,%d) -> (%d,%d)", i->data->startX, i->data->startY, i->data->endX, i->data->endY);
	}
	printf("\n--------------");
}