#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

struct AI {
	AI() {
		this->points = 0;
	}
	double n;//pawn multiplication constant
	double p;//pawn power constant
	double m;//king multiplication constant
	double q;//king power constant
	double winConstant;
	int points;
};


struct action {
	int startX;
	int startY;
	int endX;
	int endY;
	action(int startX, int startY, int endX, int endY)
	{
		this->startX = startX;
		this->startY = startY;
		this->endX = endX;
		this->endY = endY;
	}
	
};

struct actionListItem {
	actionListItem* nextItem;
	action* data;
	actionListItem() {
		this->nextItem = nullptr;
		this->data = nullptr;
	}
	actionListItem(action* data) {
		this->nextItem = nullptr;
		this->data = data;
	}
	~actionListItem() {
		delete this->data;
		this->data = nullptr;
		delete this->nextItem;
		this->nextItem = nullptr;
	}
};

double plotMoves(int depth, int array[8][8], int i, int moveValueCounter);
double takingPossibilities(int isOdd, int y, int x, int yOffset, int xOffset, int array[8][8], int depth, int i, int moveValueCounter);
void makeMove(int currentArray[8][8], int i, int moveValueCounter);
void printArray(int array[8][8]);
void copyArray(int newArray[8][8], int oldArray[8][8]);

int maxDepth = 2;
int width[10] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
int movesLim = 1000;
int isOdd[10];

void evolve(double* bestN, double* bestP, double* bestM, double* bestQ, double* bestWinConstant);
int move(int i, int currentArray[8][8], int moveValueCounter);
void playGames(int i, int currentArray[8][8], int moveValueCounter);

void listAddAction(actionListItem* header, int startX, int startY, int endX, int endY);
void printListActions(actionListItem* header);
void listReset(actionListItem listArray[100]);

actionListItem* actionPointerHolder[10];
actionListItem possibleActions[10][100];
double possibleMovesValues[10][100];
int topAI[10];
actionListItem* list[10];
int max[10] = { 0,0,0,0,0,0,0,0,0,0 };
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
int gameArray[10][8][8];
int gameArrayInUse[10] = { 0,0,0,0,0,0,0,0,0,0 };
int startKingDif[10] = { 0,0,0,0,0,0,0,0,0,0 };
int startPawnDif[10] = { 0,0,0,0,0,0,0,0,0,0 };
int pawnDif[10] = { 0,0,0,0,0,0,0,0,0,0 };
int kingDif[10] = { 0,0,0,0,0,0,0,0,0,0 };

AI AIList[100000];
std::ofstream AIScores;

int main() {
	
	AIScores.open("testing.txt", std::ios::out);
	int magReductions = 2;
	double magnitude = 1;
	double bestN = 5;
	double bestP = 5;
	double bestM = 5;
	double bestQ = 5;
	double bestWinConstant = 5;
	int currentItem;
	for (int i = 0; i < magReductions; i++) {
		for (int a = 0; a < 10; a++) {
			for (int b = 0; b < 10; b++) {
				for (int c = 0; c < 10; c++) {
					for (int d = 0; d < 10; d++) {
						for (int e = 0; e < 10; e++) {
							currentItem = (10000 * a) + (1000 * b) + (100 * c) + (10 * d) + e;
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
		std::cout << "Start of evolution on magnitude:" << magnitude << std::endl;
		evolve(&bestN, &bestP, &bestM, &bestQ, &bestWinConstant);
		std::cout << "End of evolution on magnitude:" << magnitude << std::endl;
		magnitude = magnitude / 10;
	}
	return 0;
}

void evolve(double* bestN, double* bestP, double* bestM, double* bestQ, double* bestWinConstant) {

	const int threadAmount = 10;
	std::thread threadArray [threadAmount];

	for (int i = 0; i < 100000; i++) {
		for (int t = 0; t < threadAmount; t++) {
			if (gameArrayInUse[t] == 0) {
				copyArray(gameArray[t], startArray);
				gameArrayInUse[t] = 1;
				threadArray[t] = std::thread(playGames, i, gameArray[t], t);
				//thread(i, gameArray[t]);
				gameArrayInUse[t] = 0;
				std::cout << "Completed:" << std::to_string((i / 1e5) * 100) << "%" << std::endl;
				break;
			}
			if (t == threadAmount - 1) {
				t = -1;
			}
		}
		if (i == 100) {
			AIScores.close();
			AIScores.open("testing.txt", std::ios::app);
		}
	}

	for (size_t i = 0; i < threadAmount; i++)
	{
		threadArray[i].join();
	}
}

void playGames(int i, int currentArray[8][8], int moveValueCounter) {
	int end = 0;
	int moves = 0;
	int men = 0;
	for (int t = 0; t < 100000; t++) {
		end = 0;
		topAI[moveValueCounter] = 0;
		moves = 0;
		while (end == 0) {
			moves++;
			startPawnDif[moveValueCounter] = 0;
			startKingDif[moveValueCounter] = 0;
			for (int y = 0; y < 8; y++) {
				for (int x = 0; x < 8; x++) {
					if (currentArray[y][x] < 3) {
						if (currentArray[y][x] == (1 + topAI[moveValueCounter])) {
							startPawnDif[moveValueCounter]++;
						}
						else {
							startPawnDif[moveValueCounter]--;
						}
					}
					else {
						if (currentArray[y][x] == (3 + topAI[moveValueCounter])) {
							startKingDif[moveValueCounter]++;
						}
						else {
							startKingDif[moveValueCounter]--;
						}
					}
				}
			}
			if (moves == movesLim) {
				end = -1;
			}
			else if (men == 0) {
				topAI[moveValueCounter]++;
				end = move(i, currentArray, moveValueCounter);
			}
			else {
				topAI[moveValueCounter]--;
				end = move(t, currentArray, moveValueCounter);
			}
		}
		if (end != -1) {
			if (topAI[moveValueCounter] == 1) {
				AIList[t].points--;
				AIList[i].points++;
			}
			else {
				AIList[i].points--;
				AIList[t].points++;
			}
		}
	}
	AIScores << "{n:" << AIList[i].n << ",p:" << AIList[i].p << ",m:" << AIList[i].m << ",q:" << AIList[i].q << ",winConst:" << AIList[i].winConstant << ",pts:" << AIList[i].points << "}";
}

int move(int i, int currentArray[8][8], int moveValueCounter) {
	
	//-----------------------------------------------checking if end
	int t;
	int lim;
	int canJump;
	int yOffset;
	int xOffset;
	int possibleMoves = 0;
	
	for (int y = 0; y<8; y++) {
		for (int x = 0; x<8; x++) {
			if ((currentArray[y][x] != 0) && ((currentArray[y][x] % 2) == topAI[moveValueCounter])) {
				//---------------checking possible moves---------------
				t = -2 + (2 * topAI[moveValueCounter]);
				lim = 0 + (2 * topAI[moveValueCounter]);
				if ((currentArray[y][x] - 2) > 0) {//I think is more efficient than 'innerArray[y][x] == 3 || innerArray[y][x] == 4'
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
						if (currentArray[y + yOffset][x + xOffset] == 0) {
							possibleMoves++;
						}
						else if ((canJump == 1) && ((currentArray[y + yOffset][x + xOffset] % 2) == (1 - topAI[moveValueCounter])) && (currentArray[y + (2 * yOffset)][x + (2 * xOffset)] == 0)) {
							possibleMoves++;
						}
					}
				}
			}
		}
	}
	if (possibleMoves == 0) {
		return 1;
	} 
	else {
		width[moveValueCounter] = -1;
		listReset(possibleActions[moveValueCounter]);
		for (int t = 0; t < 100; t++) {
			possibleMovesValues[moveValueCounter][t] = 0;
			//printf("\npossibleActions[%d].data->startX:%d", i, possibleActions[i].data->startX);
		}
		plotMoves(0, currentArray, i, moveValueCounter);
		std::cout << "far out checker" << std::endl;
		makeMove(currentArray, i, moveValueCounter);
		return 0;
	}
}

void makeMove(int currentArray[8][8], int i, int moveValueCounter) {
	max[moveValueCounter] = 0;
	for (int t = 0; t < width[moveValueCounter] + 1; t++) {
		if (possibleMovesValues[moveValueCounter][t] > possibleMovesValues[moveValueCounter][max[moveValueCounter]]) {
			max[moveValueCounter] = t;
		}
	}
	/*printf("\n----------------------------");
	printf("\n|       men:%d              |", men);
	printf("\n|       max:%f       |", possibleMovesValues[max]);
	printf("\n|     ----------------     |");*/
	list[moveValueCounter] = &possibleActions[moveValueCounter][max[moveValueCounter]];
	while (list != nullptr)
	{
		//printf("\n|     |(%d,%d) -> (%d,%d)|     |", list->data->startX, list->data->startY, list->data->endX, list->data->endY);
		if (list[moveValueCounter]->data->endY == (7 * topAI[moveValueCounter])) {
			currentArray[list[moveValueCounter]->data->endY][list[moveValueCounter]->data->endX] = 4 - topAI[moveValueCounter];
		}
		else {
			currentArray[list[moveValueCounter]->data->endY][list[moveValueCounter]->data->endX] = currentArray[list[moveValueCounter]->data->startY][list[moveValueCounter]->data->startX];
		}
		currentArray[list[moveValueCounter]->data->startY][list[moveValueCounter]->data->startX] = 0;
		if (abs(list[moveValueCounter]->data->endX - list[moveValueCounter]->data->startX) == 1 + topAI[moveValueCounter]) {
			currentArray[(list[moveValueCounter]->data->startY + list[moveValueCounter]->data->endY) / 2][(list[moveValueCounter]->data->startX + list[moveValueCounter]->data->endX) / 2] = 0;
		}
		list[moveValueCounter] = list[moveValueCounter]->nextItem;
	}
	//printf("\n|     ----------------     |");
	//printf("\n----------------------------");
	//printArray(gameArray);
}

double plotMoves(int depth, int currentArray[8][8], int i, int moveValueCounter) {
	//----------------evaluates current board favorability----------------
	if (depth == maxDepth) {
		pawnDif[moveValueCounter] = 0;
		kingDif[moveValueCounter] = 0;
		for (int y = 0; y < 8; y++) {
			for (int x = 0; x < 8; x++) {
				if (currentArray[y][x] < 3) {
					if (currentArray[y][x] == (2 - topAI[moveValueCounter])) {
						pawnDif[moveValueCounter]++;
					}
					else {
						pawnDif[moveValueCounter]--;
					}
				}
				else {
					if (currentArray[y][x] == (4 - topAI[moveValueCounter])) {
						kingDif[moveValueCounter]++;
					}
					else {
						kingDif[moveValueCounter]--;
					}
				}
			}
		}
		kingDif[moveValueCounter] -= startKingDif[moveValueCounter];
		pawnDif[moveValueCounter] -= startPawnDif[moveValueCounter];
		if (depth == 1) {//has to be depth 1 since depth 0 would only be 1 result which would be the value of taking no action
			possibleMovesValues[moveValueCounter][width[moveValueCounter]] = ((AIList[i].n*pow(pawnDif[moveValueCounter], AIList[i].p)) + (AIList[i].m*pow(kingDif[moveValueCounter], AIList[i].q)));
			//printf("\nhit adding item to array(estimate:%f)\nwidth:%d\n", ((AI->n*pow(pawnDif, AI->p)) + (AI->m*pow(kingDif, AI->q))), width);
			//system("pause");
		}
		//double test = ((AI->n*pow(pawnDif, AI->p)) + (AI->m*pow(kingDif, AI->q)));
		//printf("AI->n:%f, pawnDif:%f, AI->p%f, AI->m:%f, kingDif:%f, AI->q:%f\n((AI->n*pow(pawnDif, AI->p)) + (AI->m*pow(kingDif, AI->q))):%f", AI->n, pawnDif, AI->p, AI->m, kingDif, AI->q, ((AI->n*pow(pawnDif, AI->p)) + (AI->m*pow(kingDif, AI->q))));
		//printf("\nestimate:%f\nwidth:%d\n", test, ((AI->n*pow(pawnDif, AI->p)) + (AI->m*pow(kingDif, AI->q))), width);
		//system("pause");
		return ((AIList[i].n*pow(pawnDif[moveValueCounter], AIList[i].p)) + (AIList[i].m*pow(kingDif[moveValueCounter], AIList[i].q)));
	}
	//--------------------------------------------------------------------
	
	//----------checks if any men turn into kings----------
	int innerArray[8][8];
	copyArray(innerArray, currentArray);
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			if (innerArray[y][x] != 0) {
				isOdd[moveValueCounter] = 0;
				if ((innerArray[y][x] % 2) == 1) {
					isOdd[moveValueCounter] = 1;
				}
				if (y == (isOdd[moveValueCounter] * 7)) {
					innerArray[y][x] = (4 - isOdd[moveValueCounter]);
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
		isOdd[moveValueCounter] = topAI[moveValueCounter];
	}
	else if (topAI[moveValueCounter] == 1) {//probably some way of simplifying this
		isOdd[moveValueCounter] = 0;
	}
	else if (topAI[moveValueCounter] == 0) {
		isOdd[moveValueCounter] = 1;
	}
	//----------------------------------------------------
	double internalWidth = 0;
	//--------------------------------counting number of possible moves and checking if game ended--------------------------------
	for (int y = 0; y<8; y++) {
		for (int x = 0; x<8; x++) {
			if ((innerArray[y][x] != 0) && ((innerArray[y][x] % 2) == isOdd[moveValueCounter])) {
				//---------------checking possible moves---------------
				t = -2 + (2 * isOdd[moveValueCounter]);
				lim = 0 + (2 * isOdd[moveValueCounter]);
				if ((innerArray[y][x] - 2) > 0) {//I think is more efficient than 'innerArray[y][x] == 3 || innerArray[y][x] == 4'
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
						else if ((canJump == 1) && ((innerArray[y + yOffset][x + xOffset] % 2) == (1 - isOdd[moveValueCounter])) && (innerArray[y + (2 * yOffset)][x + (2 * xOffset)] == 0)) {
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
	if (internalWidth == 0) {
		if (depth == 1) {//has to be depth 1 since depth 0 would only be 1 result which would be the value of taking no action
			possibleMovesValues[moveValueCounter][width[moveValueCounter]] = AIList[i].winConstant;
			//printf("\nhit adding item to array(interalWidth)\nwidth:%d\nvalueAdded:%f\nvalueIn:%f\n", width, AI->winConstant, possibleMovesValues[width]); printf("\nhit adding item to array\nwidth:%d\nvalue:%f\n", width, possibleMovesValues[width]);
			//system("pause");
		}
		if (isAI == 1) {
			//printf("\n-(AI->winConstant):%f\n", -(AI->winConstant));
			//system("pause");
			return -(AIList[i].winConstant);//-winConstant = AI lose
		}
		else if (isAI == 0) {
			//printf("\nAI->winConstant:%f\n", AI->winConstant);
			//system("pause");
			return AIList[i].winConstant;//winConstant = AI win
		}
	}
	int tempArray[8][8];
	//----------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------carrying out moves or jumps------------------------------------------------------
	if (willJump == 1) {
		for (int y = 0; y<8; y++) {
			for (int x = 0; x<8; x++) {
				if (innerArray[y][x] != 0 && (innerArray[y][x] % 2) == isOdd[moveValueCounter]) {
					//---------------checking possible moves---------------
					t = -2 + (2 * isOdd[moveValueCounter]);
					lim = 0 + (2 * isOdd[moveValueCounter]);
					if ((innerArray[y][x] - 2) > 0) {//I think is more efficient than 'innerArray[y][x] == 3 || innerArray[y][x] == 4'
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
						if ((yOffset != 0) && ((innerArray[y + yOffset][x + xOffset] % 2) == (1 - isOdd[moveValueCounter])) && (innerArray[y + yOffset][x + xOffset] != 0) && (innerArray[y + (2 * yOffset)][x + (2 * xOffset)] == 0)) {
							copyArray(tempArray, innerArray);
							tempArray[y + (2 * yOffset)][x + (2 * xOffset)] = tempArray[y][x];
							tempArray[y + yOffset][x + xOffset] = 0;
							tempArray[y][x] = 0;
							if (depth == 0) {
								width[moveValueCounter]++;
								possibleActions[moveValueCounter][width[moveValueCounter]] = *(new actionListItem(new action(x, y, x + (2 * xOffset), y + (2 * yOffset))));
							}
							totalValue += takingPossibilities(isOdd[moveValueCounter], y, x, yOffset, xOffset, tempArray, depth, i, moveValueCounter);
						}
					}
				}
			}
		}
	}
	else {
		for (int y = 0; y<8; y++) {
			for (int x = 0; x<8; x++) {
				if (innerArray[y][x] != 0 && (innerArray[y][x] % 2) == isOdd[moveValueCounter]) {
					//---------------checking possible moves---------------
					t = -2 + (2 * isOdd[moveValueCounter]);
					lim = 0 + (2 * isOdd[moveValueCounter]);
					if ((innerArray[y][x] - 2) > 0) {//I think is more efficient than 'innerArray[y][x] == 3 || innerArray[y][x] == 4'
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
							copyArray(tempArray, innerArray);
							tempArray[y + yOffset][x + xOffset] = innerArray[y][x];
							tempArray[y][x] = 0;
							if (depth == 0) {
								width[moveValueCounter]++;
								possibleActions[moveValueCounter][width[moveValueCounter]] = *(new actionListItem(new action(x, y, x + xOffset, y + yOffset)));
							}
							totalValue += plotMoves((depth + 1), tempArray, i, moveValueCounter);
						}
					}
				}
			}
		}
	}
	//---------------------------------------------------------------------------------------------------------------------------------------
	if (depth == 1) {//has to be depth 1 since depth 0 would only be 1 result which would be the value of taking no action
		possibleMovesValues[i][width[moveValueCounter]] = (totalValue / internalWidth);
		//printf("\nhit adding item to array(totalValue:%f)\nwidth:%d\nvalueAdded:%f\nvalueIn:%f\n", totalValue, width, (totalValue / internalWidth), possibleMovesValues[width]);
		//system("pause");
	}
	//printf("\n(totalValue / internalWidth):%f\n", (totalValue / internalWidth));
	//system("pause");
	return (totalValue / internalWidth);//think this is better than returning '(totalValue / internalWidth)'
}

double takingPossibilities(int isOdd, int y, int x, int yOffset, int xOffset, int takingArray[8][8], int depth, int i, int moveValueCounter) {
	//listAdd(&possibleMoves[width], new move(x, y, x + (2 * xOffset), y + (2 * yOffset)), width);
	double totalValue = 0;
	actionPointerHolder[moveValueCounter] = nullptr;
	int tempJumpArray[8][8];
	copyArray(tempJumpArray, takingArray);
	totalValue += plotMoves((depth + 1), tempJumpArray, i, moveValueCounter);
	y = y + (2 * yOffset);
	x = x + (2 * xOffset);
	//------------------------------
	int t = -2 + (2 * isOdd);
	int lim = 0 + (2 * isOdd);
	if ((takingArray[y][x] - 2) > 0) {//I think is more efficient than 'currentArray[y][x] == 3 || currentArray[y][x] == 4'
		t = -2;
		lim = 2;
	}
	//------------------------------
	for (t; t < lim; t++) {
		yOffset = 0;//this one fucking line took me 2 hours to think to write to fix a bug
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
				std::cout << "outer check" << std::endl;
				/*printArray(takingArray);
				system("pause");*/
				//objectPointerHolder = &possibleMoves[width];
				actionPointerHolder[moveValueCounter] = &possibleActions[moveValueCounter][width[moveValueCounter]];
				width[moveValueCounter]++;
				while (actionPointerHolder[moveValueCounter] != nullptr) {
					if (actionPointerHolder[moveValueCounter]->nextItem != nullptr || (actionPointerHolder[moveValueCounter]->data->endX == x && actionPointerHolder[moveValueCounter]->data->endY == y)) {
						listAddAction(&possibleActions[moveValueCounter][width[moveValueCounter]], actionPointerHolder[moveValueCounter]->data->startX, actionPointerHolder[moveValueCounter]->data->startY, actionPointerHolder[moveValueCounter]->data->endX, actionPointerHolder[moveValueCounter]->data->endY);
						actionPointerHolder[moveValueCounter] = actionPointerHolder[moveValueCounter]->nextItem;
					}
					else {
						break;
					}
				}
				listAddAction(&possibleActions[moveValueCounter][width[moveValueCounter]], x, y, x + (2 * xOffset), y + (2 * yOffset));
			}
			int tempTakingArray[8][8];
			copyArray(tempTakingArray, takingArray);
			tempTakingArray[y + (2 * yOffset)][x + (2 * xOffset)] = takingArray[y][x];
			tempTakingArray[y + yOffset][x + xOffset] = 0;
			tempTakingArray[y][x] = 0;
			totalValue += takingPossibilities(isOdd, y, x, yOffset, xOffset, tempTakingArray, depth, i, moveValueCounter);
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

void copyArray(int newArray[8][8], int oldArray[8][8]) {
	for (int i = 0; i < 8; i++) {
		for (int t = 0; t < 8; t++) {
			newArray[i][t] = oldArray[i][t];
		}
	}
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
		std::cout << "checker" << std::endl;
		header->data = new action(startX, startY, endX, endY);
	}
	else {
		actionListItem* pointer = header;
		while (pointer->nextItem != nullptr)
		{
			pointer = pointer->nextItem;
		}
		pointer->nextItem = new actionListItem(new action(startX, startY, endX, endY));
	}
}

void printListActions(actionListItem* header) {
	printf("--------------");
	actionListItem* pointer = header;
	while (pointer != nullptr)
	{
		printf("\n(%d,%d) -> (%d,%d)", pointer->data->startX, pointer->data->startY, pointer->data->endX, pointer->data->endY);
		pointer = pointer->nextItem;
	}
	printf("\n--------------");
}