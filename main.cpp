#include <iostream>
#include <string>
#include <windows.h>
#include <vector>
#include "mingw.thread.h"

using namespace std;


int nScreenWidth = 80;
int nScreenHeight = 30;
wstring tetromino[7];
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char *pField = nullptr;

int Rotate(int px, int py, int r) {
    switch(r % 4) {
        case 0: return py * 4 + px; // 0 degrees
        case 1: return 12 + py - (px * 4); // 90 degrees
        case 2: return 15 - (py * 4) - px; // 180 degrees
        case 3: return 3 - py + (px * 4); // 270 degrees
    }
    return 0;
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY) {
    for (int px = 0; px < 4; px++) {
        for (int py = 0; py < 4; py++) {
            int pi = Rotate(px, py, nRotation);         
            int fi = (nPosY + py) * nFieldWidth + (nPosX + px);     //Provides coordinate in terms of field
            if (nPosX + px >= 0 && nPosX + px < nFieldWidth) {      //Checking if it's in field in x-dirn (taking memory precautions)
                if (nPosY + py >= 0 && nPosY + py < nFieldHeight) { //Checking if it's in field in y-dirn
                    if (tetromino[nTetromino][pi] == L'X' && pField[fi] != 0) { //If the coordinate is a block-part and the coordinate it occupies isn't empty: FALSE
                        return false;
                    }
                }
            }
        }
    }
    return true;
}  

int main() {

    //assets - the L is a wchar_t literal: distinguishing these strings from string of chars
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

    tetromino[5].append(L"....");
    tetromino[5].append(L".XX.");   
    tetromino[5].append(L"..X.");
    tetromino[5].append(L"..X.");
    
    tetromino[6].append(L"....");
    tetromino[6].append(L".XX.");   
    tetromino[6].append(L".X..");
    tetromino[6].append(L".X..");
    
    pField = new unsigned char [nFieldWidth*nFieldHeight]; //requesting memory allocation for dynamic regime
    for (int x = 0; x < nFieldWidth; x++) {         //Set everything to 0 unless it's on side
        for (int y = 0; y < nFieldHeight; y++) {    //or bottom
            pField[y*nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
            //a ? b : c means: if a is true then b, otherwise c
        }
    }

    wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight];
    for (int i = 0; i < nScreenWidth*nScreenHeight; i++) {
        screen[i] = L' ';
    }
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

    //Stuff For Game Logic
    bool bGameOver = false;
    int nCurrentPiece = 0; //What current piece is falling
    int nCurrentRotation = 0; //Is current piece rotated
    int nCurrentX = nFieldWidth / 2; //Where is current piece located within field
    int nCurrentY = 0; 

    bool bKey[4];
    bool bRotateHold = false;

    int nSpeed = 20;
    int nSpeedCounter = 0;
    bool bForceDown = false;
    int nPieceCount = 0;
    int nScore = 0;
    vector<int> vLines;

    while (!bGameOver) {
        //Game Timing
        this_thread::sleep_for(50ms);
        nSpeedCounter++;
        bForceDown = (nSpeedCounter == nSpeed);

        //Input
        for (int k = 0; k < 4; k++) {
            bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
        }                                   //x27 is RIGHT, x25 is LEFT, x28 is DOWN, Z is ROTATE
        //Game Logic
        nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
        nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
        nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;
        if (bKey[3]) {
            nCurrentRotation += (!bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
            bRotateHold = true;
        }
        else {
            bRotateHold = false;
        }

        if(bForceDown) {
            if(DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) {
                nCurrentY++;
            }
            else {
                //Lock current piece into field
                for (int px = 0; px < 4; px++) {
                    for (int py = 0; py < 4; py++) {
                        if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X') {
                            pField[(nCurrentY + py)*nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;
                        }
                    }
                }
                nPieceCount++;
                if (nPieceCount % 10 == 0) {
                    if (nSpeed >= 10) {
                        nSpeed--;
                    }
                }
                //Check does this lock complete a line
                for (int py = 0; py < 4; py++) {
                    if (nCurrentY + py < nFieldHeight - 1) {
                        bool bLine = true;
                        for (int px = 1; px < nFieldWidth - 1; px++) {
                            bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;
                        }
                        if (bLine) {
                            for (int px = 1; px < nFieldWidth - 1; px++) {
                                pField[(nCurrentY + py) * nFieldWidth + px] = 8;
                            }
                            vLines.push_back(nCurrentY + py);
                        }
                    }
                }
                nScore += 25;
                if (!vLines.empty()) {
                    nScore += (1 << vLines.size()) * 100; //Gives more points if multiple lines
                }
                //Choose next piece
                nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;

                //If next piece does not fit, Game over
                bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
            }
            nSpeedCounter = 0;
        }
        
        //Render Output

        //Draw Field
        for (int x = 0; x < nFieldWidth; x++) {
            for (int y = 0; y < nFieldHeight; y++) {
                screen[(y + 2)*nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y*nFieldWidth + x]];
            }   //Will draw either ' ' or #, depending on 0 or 9 value of pField index
        }
        //Draw Current Piece
        for (int px = 0; px < 4; px++) {
            for (int py = 0; py < 4; py++) {
                if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X') {
                    screen[(nCurrentY + py + 2)*nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;
                }
            }
        }
        //Draw Score
        swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

        if (!vLines.empty()) { //Display Frame to draw lines
            WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms); // Delay a bit
            for (auto &v : vLines) {
                for (int px = 1; px < nFieldWidth - 1; px++) {
                    for (int py = v; py > 0; py--) {
                        pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
					    pField[px] = 0;
                    }
                    
                }
            }
            vLines.clear();
        }
        // Display Frame
        WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten); //This breaks often
    }

    CloseHandle(hConsole);
	cout << "Game Over!! Score:" << nScore << endl;
	system("pause");
	return 0;
}