#include "pch.h"


using namespace std;

// Figures
wstring tetromino[7];
static int tetrominoWidth = 4;

// Field
static int nFieldWidth = 12;
static int nFieldHeight = 18;
unsigned char *pField = nullptr;

// Score Board
static int nScoreWidth = 19;
static int nScoreHeight = 3;
unsigned char *pScoreBoard = nullptr;

// Next Piece
static int nPieceWidth = 19;
static int nPieceHeight = 7;
unsigned char *pPiece = nullptr;


// win prompt size
int nScreenWidth = 80;
int nScreenHeight = 30;

namespace initHelper
{
	void createShapes()
	{
		tetromino[0].append(L"..X.");
		tetromino[0].append(L"..X.");
		tetromino[0].append(L"..X.");
		tetromino[0].append(L"..X.");

		tetromino[1].append(L"..X.");
		tetromino[1].append(L".XX.");
		tetromino[1].append(L".X..");
		tetromino[1].append(L"....");

		tetromino[2].append(L".X..");
		tetromino[2].append(L".XX.");
		tetromino[2].append(L"..X.");
		tetromino[2].append(L"....");

		tetromino[3].append(L"....");
		tetromino[3].append(L".XX.");
		tetromino[3].append(L".XX.");
		tetromino[3].append(L"....");

		tetromino[4].append(L"..X.");
		tetromino[4].append(L".XX.");
		tetromino[4].append(L"..X.");
		tetromino[4].append(L"....");

		tetromino[5].append(L".XX.");
		tetromino[5].append(L"..X.");
		tetromino[5].append(L"..X.");
		tetromino[5].append(L"....");

		tetromino[6].append(L".XX.");
		tetromino[6].append(L".X..");
		tetromino[6].append(L".X..");
		tetromino[6].append(L"....");
	}

	void initBoard() {
		pField = new unsigned char[nFieldWidth*nFieldHeight];
		for (int x = 0; x < nFieldWidth; x++) {
			for (int y = 0; y < nFieldHeight; y++) {
				pField[y*nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
			}
		}
	}

	void initScoreBoard() {
		pScoreBoard = new unsigned char[nScoreWidth*nScoreHeight];
		for (int x = 0; x < nScoreWidth; x++) {
			for (int y = 0; y < nScoreHeight; y++) {
				int nScoreBoardVal = 0;
				if (y == 0 || y == nScoreHeight - 1) nScoreBoardVal = 2;
				else if (x == 0 || x == nScoreWidth - 1) nScoreBoardVal = 1;
				pScoreBoard[y*nScoreWidth + x] = nScoreBoardVal;
			}
		}
	}

	void initNextPiece() {
		pPiece = new unsigned char[nPieceWidth*nPieceHeight];
		for (int x = 0; x < nPieceWidth; x++) {
			for (int y = 0; y < nPieceHeight; y++) {
				int nPieceVal = 0;
				if (y == 0 || y == nPieceHeight - 1) nPieceVal = 2;
				else if (x == 0 || x == nPieceWidth - 1) nPieceVal = 1;
				pPiece[y*nPieceWidth + x] = nPieceVal;
			}
		}
	}

	void deleteFields() {
		delete[] pField;
		delete[] pScoreBoard;
		delete[] pPiece;
	}
}

namespace engineHelper 
{
	int rotate(int x, int y, int rot) 
	{
		switch (rot % 4)
		{
		case 0: return  0 +	y * tetrominoWidth + x; // 0
		case 1: return 12 + y - tetrominoWidth * x; // 90
		case 2: return 15 - y * tetrominoWidth - x; // 180
		case 3: return  3 -	y + tetrominoWidth * x; // 270
		}
		return 0;
	}

	/**
		Checks if tetromino fits into the current state
		@tetromino: current 4x4 piece 
		@rot: current rotation of tetromino
		@posX: x position of top-left corner of tetromino
		@posY: y position of top-left corner of tetromino
	*/ 
	bool doesPieceFit(int nTetromino, int rot, int posX, int posY) {
		for (int x = 0; x < 4; x++) 
			for (int y = 0; y < 4; y++)
			{
				int pi = rotate(x, y, rot);
				int fi = (posX + x) + (posY + y)*nFieldWidth;

				if ((posX + x) >= 0 && (posX + x) < nFieldWidth) 
				{
					if ((posY+ y) >= 0 && (posY + y) < nFieldHeight) 
					{
						if (tetromino[nTetromino][pi] == L'X' && pField[fi] != 0) 
							return false;
					}
				}
			}
		return true;
	}

	void addPieceToBackground(int nTetromino, int rot, int posX, int posY) {
		for (int x = 0; x < 4; x++)
			for (int y = 0; y < 4; y++)
			{
				int pi = rotate(x, y, rot);
				int fi = (posX + x) + (posY + y)*nFieldWidth;
				if (tetromino[nTetromino][pi] == L'X' && pField[fi] == 0)
					pField[fi] = nTetromino+1;
			}
	}
}

namespace windowsHelper {
	HANDLE hConsole;
	wchar_t *screen = nullptr;
	DWORD dwBytesWritten = 0;

	void initConsoleWindow() {
		screen = new wchar_t[nScreenWidth*nScreenHeight];
		for (int i = 0; i < nScreenWidth*nScreenHeight; i++) screen[i] = L' ';
		hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		SetConsoleActiveScreenBuffer(hConsole);
	}

	void deleteScreen() {
		delete[] screen;
	}

	list<pair<string, int>> lHighScores;

	void getHighScores() 
	{
		lHighScores.clear();

		TCHAR path[MAX_PATH];
		GetModuleFileName(NULL, path, MAX_PATH);
		wstring folder(path);
		folder.erase(folder.find_last_of(L"\\")+1);

		fstream f(folder+L"highscore.txt", fstream::in);
		if (f.good())
		{
			string name;
			int score = 0;
			while (f >> name >> score) 
				lHighScores.push_back(pair<string, int>(name, score));
		}

		f.close();
	}

	const int nHighScoreCount = 10;

	bool checkHighScore(int score) 
	{
		if (lHighScores.size() < nHighScoreCount) return true;
		for (auto hs = lHighScores.begin(); hs != lHighScores.end(); hs++)
			if (score > hs->second) {
				return true;
			}

		return false;
	}

	void writeHighscore()
	{
		TCHAR cPath[MAX_PATH];
		GetModuleFileName(NULL, cPath, MAX_PATH);
		wstring sFolder(cPath);
		sFolder.erase(sFolder.find_last_of(L"\\") + 1);

		ofstream f(sFolder + L"highscore.txt", ofstream::trunc);
		if (f.good())
			for (auto &hs : lHighScores)
				f << hs.first << " " << hs.second << endl;
		f.close();
	}

	void updateHighscore(pair<string, int>&& pCandidate)
	{
		if (lHighScores.empty())
			lHighScores.push_back(pCandidate);
		else
		{
			for (auto hs = lHighScores.begin(); hs != lHighScores.end(); hs++)
				if (pCandidate.second > hs->second) {
					lHighScores.insert(hs, pCandidate);
					while (lHighScores.size() > nHighScoreCount) lHighScores.pop_back();
					break;
				}
		}
	}
}

namespace miscHelper {
	bool handleGameOver(int nScore) 
	{
		system("cls");
		std::cout << "---------------        " << "Game Over!! " << endl;
		this_thread::sleep_for(250ms);
		std::cout << "---------------        " << "Your Score was >>" << nScore << "<<" << endl;
		this_thread::sleep_for(250ms);

		windowsHelper::getHighScores();
		if (windowsHelper::checkHighScore(nScore))
		{

			std::cout << "---------------        " << "Congrats, you scored a High Score! :)" << endl;
			this_thread::sleep_for(250ms);
			std::cout << "---------------        " << "Type your name and press enter:" << endl;
			string name;
			cin >> name;

			windowsHelper::updateHighscore(pair<string, int>(name, nScore));
			windowsHelper::writeHighscore();
		}
		
		std::cout << "---------------        _-._.-._ " << "Highscore _-._.-._" << endl;
		std::cout << "---------------        "<< endl;
		int index = 0;
		for (auto hs : windowsHelper::lHighScores)
		{
			this_thread::sleep_for(50ms);
			std::cout << "---------------        " << ++index << ": " << hs.second << " by " << hs.first << endl;
		}

		this_thread::sleep_for(1000ms);


		std::cout << "----------------------------------------------" << endl;
		std::cout << "---------------        " << "Play Again ? (y/n) " << endl;

		string ans;
		cin >> ans;

		while (ans != "yes" && ans != "y" && ans != "no" && ans != "n")
		{
			std::cout << "----------------------------------------------" << endl;
			std::cout << "---------------        " << "Just type 'yes' or 'no' and press enter ;-)" << endl;
			cin >> ans;
		}

		if (ans != "yes" && ans != "y")
		{

			std::cout << "----------------------------------------------" << endl;
			std::cout << "---------------        " << "Oh no, you're leaving me :( :( " << endl;

			this_thread::sleep_for(2000ms);

			std::cout << "----------------------------------------------" << endl;
			std::cout << "---------------        " << "Good bye, see you next time :-)" << endl;

			this_thread::sleep_for(3000ms);
		} 

		return ans == "no" || ans == "n";
	}
}

int main()
{

	bool bExit = false;
	while (!bExit) 
	{
		initHelper::createShapes();
		initHelper::initBoard();
		initHelper::initScoreBoard();
		initHelper::initNextPiece();
	
		windowsHelper::initConsoleWindow();

		// Game Variables ===============================
	
		// Globals
		bool bGameOver = false;

		// Piece
		int nCurrentPiece = rand() % 7;
		int nNextPiece = rand() % 7;
		int nCurrentRot = 0;
		int nStartX = nFieldWidth / 2 - 2;
		int nCurrentX = nStartX;
		int nCurrentY = 0;
		int nPieceCount = 0;

		// Input
		bool bKey[4];
		bool bRotateHold = false;

		// Timing 
		int nSpeed = 18;
		int nMinSpeed = 7;
		int nTicker = 1;
		bool bForceDown = false;

		// Lines
		vector<int> vLines;
		int nScore = 0;

		while (!bGameOver) 
		{
			// Game Timing ===============================
			this_thread::sleep_for(60ms);

			if (nTicker == nSpeed) 
			{
				bForceDown = true;
				nTicker = 0;
			} else
				nTicker++;

			// Input =====================================
			for (int k = 0; k < 4; k++)								// R	L	D	ctrl-l
				bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28\xA2"[k]))) != 0;
		
			// Game Logic ================================

			nCurrentX += (bKey[0] && engineHelper::doesPieceFit(nCurrentPiece, nCurrentRot, nCurrentX + 1, nCurrentY)) ? 1 : 0;
			nCurrentX -= (bKey[1] && engineHelper::doesPieceFit(nCurrentPiece, nCurrentRot, nCurrentX - 1, nCurrentY)) ? 1 : 0;
			nCurrentY += (bKey[2] && engineHelper::doesPieceFit(nCurrentPiece, nCurrentRot, nCurrentX, nCurrentY + 1)) ? 1 : 0;
			if (bKey[3]) {
				nCurrentRot += (!bRotateHold && engineHelper::doesPieceFit(nCurrentPiece, nCurrentRot + 1, nCurrentX, nCurrentY)) ? 1 : 0;
				bRotateHold = true;
			}
			else
				bRotateHold = false;
		
			if (bForceDown) 
			{
				if (engineHelper::doesPieceFit(nCurrentPiece, nCurrentRot, nCurrentX, nCurrentY + 1))
					nCurrentY++;
				else 
				{
					// Lock it in place
					engineHelper::addPieceToBackground(nCurrentPiece, nCurrentRot, nCurrentX, nCurrentY);

					// handle full lines
					for (int y = 0; y < 4; y++) 
						if (nCurrentY + y < nFieldHeight - 1) 
						{
							bool bLine = true;
							for (int x = 1; x < nFieldWidth-1; x++) 
								if (pField[x + (nCurrentY + y)*nFieldWidth] == 0) 
								{
									bLine = false;
									break;
								}
						
							// copy all lines one down
							if (bLine) 
							{	
								// just current line different sign
								for (int x = 1; x < nFieldWidth - 1; x++)
									pField[x + (nCurrentY + y)*nFieldWidth] = 8;

								// save line
								vLines.push_back(nCurrentY + y);
							}
						}

					// create a new piece
					nCurrentX = nStartX;
					nCurrentY = 0;
					nCurrentRot = 0;
					nCurrentPiece = nNextPiece;
					nNextPiece = rand() % 7;

					// increase piece counter and adjust speed
					nPieceCount++;
					if (nPieceCount % 10 == 0 && nSpeed >= nMinSpeed)
						nSpeed--;
				
					// credit scores according to line count
					nScore += 25;
					if (!vLines.empty()) nScore += (1 << vLines.size()) * 100;

					// if new piece doesn't fit: game over
					bGameOver = !engineHelper::doesPieceFit(nCurrentPiece, nCurrentRot, nCurrentX, nCurrentY);
				}
			
				bForceDown = false;
			}

			// Render Output =============================

			// Draw Next Piece
			for (int x = 0; x < nPieceWidth; x++) // border
			{ 
				for (int y = 0; y < nPieceHeight; y++) {
					if (y == 0 || y == nPieceHeight - 1 || x == 0 || x == nPieceWidth - 1)
						windowsHelper::screen[(y + 2)*nScreenWidth + (x + nFieldWidth + 4)] = L" |-"[pPiece[y*nPieceWidth + x]];
				}
			}

			swprintf_s(&windowsHelper::screen[(nPieceHeight/2 + 2)*nScreenWidth + (1 + nFieldWidth + 5)], 6, L"Next:"); // text

			for (int x = 0; x < 4; x++) // piece
				for (int y = 0; y < 4; y++)
				{
					char target = ' ';
					if (tetromino[nNextPiece][engineHelper::rotate(x, y, 0)] == L'X')
						target = /*nNextPiece*/1 + 65;

					windowsHelper::screen[(y + 4)*nScreenWidth + (x + nFieldWidth + 14)] = target;
				}

			// Draw Scoreboard
			for (int x = 0; x < nScoreWidth; x++) // border
			{ 
				for (int y = 0; y < nScoreHeight; y++) 
				{
					if (y == 0 || y == nScoreHeight - 1 || x == 0 || x == nScoreWidth - 1)
						windowsHelper::screen[(y + 10)*nScreenWidth + (x + nFieldWidth + 4)] = L" |-"[pScoreBoard[y*nScoreWidth + x]];
				}
			}

			swprintf_s(&windowsHelper::screen[(nScoreHeight / 2 + 10)*nScreenWidth + nFieldWidth + 4 + 2], 16, L"SCORE: %8d", nScore); // text

			// Draw Field ================================
			for (int x = 0; x < nFieldWidth; x++) {
				for (int y = 0; y < nFieldHeight; y++) {
					windowsHelper::screen[(y + 2)*nScreenWidth + (x + 2)] = L" BBBBBBB=#"[pField[y*nFieldWidth + x]];
				}
			}

			// Draw Current Piece ========================
			for (int x = 0; x < 4; x++)
				for (int y = 0; y < 4; y++)
					if (tetromino[nCurrentPiece][engineHelper::rotate(x, y, nCurrentRot)] == L'X')
						windowsHelper::screen[(nCurrentY + y + 2)*nScreenWidth + nCurrentX + x + 2] = 1/*nCurrentPiece*/ + 65;

			// Draw Line Clear Effect ====================
			if (!vLines.empty()) {
				WriteConsoleOutputCharacter(windowsHelper::hConsole, windowsHelper::screen, nScreenWidth * nScreenHeight, { 0,0 }, &windowsHelper::dwBytesWritten);
				this_thread::sleep_for(250ms);

				for (auto &line : vLines)  
					for (int y = line; y > 0; y--) // from current line upwards
						for (int x = 1; x < nFieldWidth-1; x++) // leave the borders
							pField[x + y * nFieldWidth] = pField[x + (y - 1)*nFieldWidth];

				vLines.clear();
			}

			// output ====================================
			WriteConsoleOutputCharacter(windowsHelper::hConsole, windowsHelper::screen, nScreenWidth * nScreenHeight, { 0,0 }, &windowsHelper::dwBytesWritten);

		}

		initHelper::deleteFields();
		windowsHelper::deleteScreen();

		CloseHandle(windowsHelper::hConsole);
		
		bExit = miscHelper::handleGameOver(nScore);
	}

	return 0;
}