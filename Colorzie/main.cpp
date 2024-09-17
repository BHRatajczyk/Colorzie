#include <iostream>
#include <windows.h>
#include <vector>
#include <conio.h>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdio>

#define _WIN32_WINNT 0x0601//font

using namespace std;

const char ammoSymbol = 3, healthSymbol = 4, spikerSymbol = 43, spergSymbol = 15, chargerSymbol = 21;
bool isWindowRendered = false, editorRunning, debugHud = true, gameRunning, tick = 0;
int hZ = 1, menuTickSpeed = 300, tickSpeed = 600, infoSpeed = 1500, resolution = 0;
int vision = 7, visionSpacer, planetSizeX = 10, planetSizeY = 10, playerPosition = 1;
int mapAmount = 0, storedPlayerPosition = 0, enemyToSpawn = 0, storedVision = 7, exitPoint = 0;
int playerMaxBullets = 3, health = 3;
int fontHeight = 16, fontWindth = 16, pushRenderStart = 6, lowerRenderStart = 7;//75//27

vector<int> BannedObjects{1, 2, 16, 17, 31, 30, 207, spikerSymbol, spergSymbol, chargerSymbol, 3, 4};
vector<int> BannedColor{119, 0, 144, 16};
vector<int> BannedForBullet{1, 2, 16, 17, 31, 30, 207, ammoSymbol, healthSymbol, spikerSymbol, spergSymbol,
                            chargerSymbol};
vector<int> BannedColorForBullet{119, 0, 2};

vector<int> BulletPlacement(0);

vector<int> EnemyBulletPlacement(0);
vector<char> EnemySymbols{spergSymbol, spikerSymbol, chargerSymbol};

vector<int> EnemyCharger(0);
vector<int> EnemyChargerDirection(0);
vector<int> EnemySperg(0);
vector<int> EnemySpiker(0);
vector<int> EnemySpikerShootDelay(0);
bool draw = true;

vector<char> planet((planetSizeX * planetSizeY), ' ');
vector<int> planetColor((planetSizeX * planetSizeY), 32);

vector<int> Moves{1, -1, planetSizeX, -planetSizeX};
vector<string> moveClicks{"D", "A", "S", "W"};

vector<char> bulletArrows{16, 17, 31, 30};
vector<char> bulletArrowsSymbols{16, 17, 31, 30};

vector<char> pickupSymbols = {ammoSymbol, healthSymbol};
char editorSymbol[3] = {' ', 3, 4};
int editorColor[3] = {119, 32, 144};
string colorButtons[3] = {"F", "G", "H"};
string symbolButtons[3] = {"V", "B", "N"};

vector<string> eV(0);
vector<string> menuOptions{"Start", "Editor", "Options", "Info", "Test Area", "Quit"};
vector<string> settingsOptions{"Tick Speed", "Horizontal Spacing", "Vision Range", "Delete Maps", "Debug Hud",
                               "Resolution", "Exit"};
vector<string> editorOptions{"New Map", "Load Map", "Exit"};
vector<string> newMapOptions{"Width", "Height", "Vision", "Accept"};
vector<string> saveModeOptions{"Save As New", "Save Over Existing", "Discard Map"};
vector<string> infoOptions{"Game", "Editor", "Exit"};

//vector <string> editorInfo{"Use W,S,A,D to move cursor","Press"};
vector<string> gameInfo{"Game by Bruno Ratajczyk", " ", "Use W,S,A,D to move", "Press X + W/S/A/D to shoot",
                        "Enemies: Sperk/Spiker/Charger ", "Sperk: Moves Randomly", "Spiker: Shoots when sees player",
                        "Charger: Charges at player", "Pickups: heart/box", "Heart: Adds one health",
                        "Box: Adds one ammo"};
vector<string> editorInfo{"Press L to wall corners", "Press O-P to change vision", "Press Enter to save",
                          "Press U to set Spawn Point"};
vector<string> resolutions{"1920x1080", "1280x1024"};
vector<string> state{"Off", "On"};

vector<string> Title{"C", "O", "L", "O", "R", "Z", "I", "E"};
vector<string> Buttons{"W", "S", "A", "D", "F", "G", "H", "V", "B", "N", "enter", "O", "P", "I", "U", "L", "space",
                       "E"};

void ClearConsole() { system("CLS"); }

void ResetColor() { SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7); }

void SetColor(int ColorId) { SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), ColorId); }

void Replace(int place, int object) { planet[place] = object; }

bool IsEven(int number) { if (number % 2 == 0)return true; else return false; }

void KeyReset() {
    Sleep(300);
    while (kbhit()) getch();
};

void ClearPlate() {
    ClearConsole();
    KeyReset();
}

// removes cursor/max window
void NoCursor(bool showFlag) {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = showFlag;
    SetConsoleCursorInfo(out, &cursorInfo);
}

void MaxWindow() {
    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_SHOWMAXIMIZED);
}

bool ButtonClicked(string button) {

    if (button.length() != 1) {
        if (button == "space" && (GetKeyState(VK_SPACE) & 0x8000))return true;
        else if (button == "enter" && (GetKeyState(VK_RETURN) & 0x8000))return true;
    }
    char buttonChar = button[0];
    if (GetKeyState(buttonChar) & 0x8000)return true;
    else return false;
}

#ifdef __cplusplus
extern "C" {
#endif
BOOL WINAPI SetCurrentConsoleFontEx(HANDLE hConsoleOutput, BOOL bMaximumWindow, PCONSOLE_FONT_INFOEX
lpConsoleCurrentFontEx);
#ifdef __cplusplus
}
#endif

void setFontSize(int newWidth, int newHeight) {
    CONSOLE_FONT_INFOEX fontStructure = {0};
    fontStructure.cbSize = sizeof(fontStructure);
    fontStructure.dwFontSize.X = newWidth;
    fontStructure.dwFontSize.Y = newHeight;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetCurrentConsoleFontEx(hConsole, true, &fontStructure);
}

void Move(int fromPlace, int toPlace) {
    char object = planet[fromPlace];
    planet[fromPlace] = ' ';
    planet[toPlace] = object;
}

void FillMapWithNumbers() {
    for (int i = 0; i < planetSizeX * planetSizeY; i++) {
        planet[i] = i;
    }
}

void VisionSpacerSetup() {
    visionSpacer = (vision - 1) / 2;
}

int RenderSkip(int p) {
    while (1 > 0) {
        if (p % vision == 0) { break; }
        p++;
    }
    return p;
}

void PushRenderWindow() {
    if (resolution == 0) {
        ResetColor();
        if (isWindowRendered)for (int b = 0; b < pushRenderStart + 25 - vision; b++) { cout << " "; }
        else for (int b = 0; b < pushRenderStart + 20; b++) { cout << " "; }
    } else if (resolution == 1) {
        ResetColor();
        if (isWindowRendered)for (int b = 0; b < pushRenderStart + 18 - vision; b++) { cout << " "; }
        else for (int b = 0; b < pushRenderStart + 16; b++) { cout << " "; }
    }


}

void LowerRenderWindow() {
    ResetColor();

    if (resolution == 0) {
        if (isWindowRendered) { for (int b = 0; b < lowerRenderStart; b++) { cout << endl; }}
        else for (int b = 0; b < lowerRenderStart + 5; b++) { cout << endl; }
    } else if (resolution == 1) {
        if (isWindowRendered) { for (int b = 0; b < lowerRenderStart; b++) { cout << endl; }}
        else for (int b = 0; b < lowerRenderStart + 5; b++) { cout << endl; }
    }


}

//Returns space in the range of vision
int SpaceAbove(int position) {
    int result = 0;
    for (int i = 1; i < visionSpacer + 1; i++) {
        position -= planetSizeX;
        if (position >= 0)result++;
        else break;
    }
    return result;


}

int SpaceLeft(int position) {
    int result = 0;
    position = position % planetSizeX;
    for (int i = 1; i < visionSpacer + 1; i++) {
        position -= 1;
        if (position >= 0)result++;
        else break;
    }
    return result;
}

int SpaceBelow(int position) {
    int result = 0;
    for (int i = 1; i < visionSpacer + 1; i++) {
        position += planetSizeX;
        if (position < planetSizeX * planetSizeY)result++;
        else break;
    }
    return result;
}

int SpaceRight(int position) {
    int result = 0;
    position = position % planetSizeX;
    for (int i = 1; i < visionSpacer + 1; i++) {
        position += 1;
        if (position < planetSizeX)result++;
        else break;
    }
    return result;
}

int SpaceAroundMe(int position) {
    int horizontal = SpaceLeft(position) + SpaceRight(position) + 1;
    int vertical = SpaceAbove(position) + SpaceBelow(position) + 1;
//cout<<horizontal*vertical<<endl;;
    return horizontal * vertical;

}

//Calculates space needed in the window
int SpacerTop() {
    return visionSpacer - SpaceAbove(playerPosition);
}

int SpacerBottom() {
    return visionSpacer - SpaceBelow(playerPosition);
}

int SpacerLeft() {
    return visionSpacer - SpaceLeft(playerPosition);
}

int SpacerRight() {
    return visionSpacer - SpaceRight(playerPosition);
}

//Various Map Checks
bool IsNotBannedColor(int place) {
    for (int i = 0; i < BannedColor.size(); i++) {
        if (planetColor[place] == BannedColor[i])return false;
    }
    return true;
}

bool IsNotBannedObject(int place) {
    for (int i = 0; i < BannedObjects.size(); i++) {
        if (planet[place] == BannedObjects[i])return false;
    }
    return true;
}

bool IsNotBannedBulletObject(int place) {
    for (int i = 0; i < BannedForBullet.size(); i++) {
        if (planet[place] == BannedForBullet[i])return false;
    }
    for (int i = 0; i < BannedColorForBullet.size(); i++) {
        if (planetColor[place] == BannedColorForBullet[i])return false;
    }
    return true;
}

bool IsMoveValid(int place, int move) {
    if (IsNotBannedColor(place + move) == false)return false;
    if (IsNotBannedObject(place + move) == false)return false;

    if (move != 1 && move != -1 && move != planetSizeX && move != -planetSizeX)return true;

    if (move == planetSizeX && SpaceBelow(place) != 0)return true;
    else if (move == -planetSizeX && SpaceAbove(place) != 0)return true;
    else if (move == -1 && SpaceLeft(place) != 0)return true;
    else if (move == 1 && SpaceRight(place) != 0)return true;
    else return false;
}

bool CheckIfOnEdge(int place) {
    if (place < planetSizeX || place % planetSizeX == 0 || place % planetSizeX == planetSizeX - 1 ||
        place > (planetSizeX * planetSizeY) - planetSizeX) {
        return true;
    } else return false;

}

bool IsAnyKeyPressed() {
    for (int b = 0; b < Buttons.size(); b++) {
        if (ButtonClicked(Buttons[b]))return true;
    }
    return false;
}

bool IsPoitingAtEdge(int place, int direction) {
/*
if(place<planetSizeX&&SpaceAbove()==0&&direction==planetSizeX)return true;
if(place%planetSizeX==0&&SpaceLeft()==0&&direction==-1)return true;
if(place%planetSizeX==planetSizeX-1&&SpaceRi)*/
    if (SpaceAbove(place) == 0 && direction == -planetSizeX)return true;
    if (SpaceLeft(place) == 0 && direction == -1)return true;
    if (SpaceRight(place) == 0 && direction == 1)return true;
    if (SpaceBelow(place) == 0 && direction == planetSizeX)return true;
    return false;


}

bool IsEnemy(int place) {
    for (int e = 0; e < EnemySymbols.size(); e++) { if (planet[place] == EnemySymbols[e])return true; }
    return false;
}

int WhereISee(int thing, int from) {
    int scan;
    for (int i = 0; i < 4; i++) {
        scan = from;
        while (1 > 0) {
            scan += Moves[i];
            if (planet[scan] == thing)return Moves[i];
            else if (planetColor[scan] == 119 || IsPoitingAtEdge(scan, Moves[i]))break;
        }
    }
    return 0;
}

bool IsABullet(int place) {
    int bullet = planet[place];
    for (int b = 0; b < 4; b++) {
        if (bullet == bulletArrowsSymbols[b]) {
            return true;
        }
    }
    return false;
}

bool IsAPickup(int place) {
    for (int p = 0; p < pickupSymbols.size(); p++) {
        if (planet[place] == pickupSymbols[p])return true;
    }
    return false;
}

//Window Render
void RenderBarRightBar() {
    ResetColor();
    cout << char(186);
}

void DrawBottomLine() {
    if (draw) {
        for (int b = 0; b < SpacerBottom(); b++) {
            if (b != 0)
                PushRenderWindow();
            cout << char(186);
            for (int c = 0; c < vision * (hZ + 1); c++)cout << " ";
            cout << char(186) << endl;
        }
        if (SpacerBottom() != 0)PushRenderWindow();
        cout << char(200);
        for (int b = 0; b < vision * (hZ + 1); b++)cout << char(205);
        cout << char(188) << endl;
    }
}

void DrawTopBar() {
    cout << char(201);
    for (int b = 0; b < vision * (hZ + 1); b++)cout << char(205);
    cout << char(187) << endl;
    for (int b = 0; b < SpacerTop(); b++) {
        PushRenderWindow();
        cout << char(186);
        for (int c = 0; c < vision * (hZ + 1); c++)cout << " ";
        cout << char(186) << endl;
    }
    PushRenderWindow();
    cout << char(186);
    for (int b = 0; b < SpacerLeft(); b++) { cout << " "; }

}

void ColoredChar(int color, char symbol) {
    SetColor(color);
    cout << symbol;
    ResetColor();
}

void ColoredString(int color, string sentence) {
    SetColor(color);
    cout << sentence;
    ResetColor();
}

int InputValue(string name, int min, int max) {
    ClearPlate();
    LowerRenderWindow();
    int value = 0;
    while (value == 0) {
        PushRenderWindow();
        cout << "    " << name << endl << endl;
        PushRenderWindow();
        cout << "  From " << min << " to " << max << endl;
        PushRenderWindow();
        cout << "    -> ";
        SetColor(240);
        cin >> value;
        ResetColor();
        if (value < min || value > max)value = 0;
        ClearConsole();
    }
    return value;
}

int SetValue(string name, int value, int increment, int min, int max) {
    ClearPlate();
    KeyReset();
    int pointer = 1;
    while (1 > 0) {
        if (ButtonClicked("space") && pointer == 1)break;
        else if (ButtonClicked("space") && pointer == 0) {
            ClearPlate();
            value = InputValue(name, min, max);
            break;
        }

        if (ButtonClicked("A") && value - increment >= min)value -= increment;
        else if (ButtonClicked("D") && value + increment <= max)value += increment;
        if (ButtonClicked("W"))pointer = 0;
        else if (ButtonClicked("S"))pointer = 1;

        LowerRenderWindow();
        PushRenderWindow();
        cout << "    " << name << endl << endl;
        PushRenderWindow();
        cout << "  From " << min << " to " << max << endl;
        PushRenderWindow();
        cout << "    < " << value << " >" << endl << endl;
        PushRenderWindow();
        if (pointer == 0) { SetColor(240); } else cout << "  ";
        cout << "     Input Value" << endl;
        PushRenderWindow();
        if (pointer == 1) { SetColor(240); } else cout << "  ";
        cout << "     Accept";
        ResetColor();


        Sleep(200);


        ClearConsole();

    }
    ClearConsole();
    KeyReset();
    return value;

}

int PickItemWithValueFromList(vector<string> itemList, vector<string> values, string nameOfWindow) {
    int pointer = 0;
    while (1 > 0) {

        if (ButtonClicked("W") && pointer > 0)pointer--;
        else if (ButtonClicked("S") && pointer < itemList.size() - 1)pointer++;

        cout << nameOfWindow << endl << endl;

        for (int m = 0; m < itemList.size(); m++) {
            if (m == pointer)cout << "->";
            else cout << "  ";
            cout << itemList[m] << ": " << values[m] << endl;
        }

        if (ButtonClicked("space")) {
            ClearPlate();
            return pointer + 1;
        }
        Sleep(500);
        ClearConsole();
    }

}

int PickItemFromListV2(vector<string> itemList, string nameOfWindow, vector<string> values, bool lastSpacer,
                       vector<string> coloredTitle, int startFrom) {
    if (startFrom > itemList.size() - 1)startFrom = 0;
    int pointer = startFrom;
    while (1 > 0) {//=====================================
        NoCursor(false);
        if (ButtonClicked("W") && pointer > 0)pointer--;
        else if (ButtonClicked("S") && pointer < itemList.size() - 1)pointer++;
        LowerRenderWindow();
        PushRenderWindow();
        //title
        if (coloredTitle.size() == 0)cout << nameOfWindow << endl << endl;

        else {
            for (int c = 0; c < coloredTitle.size(); c++) {
                SetColor(rand() % 256);
                cout << coloredTitle[c];
            }

            cout << endl << endl;
        }
        //title end
        //list start
        for (int m = 0; m < itemList.size(); m++) {
            if (lastSpacer && m == itemList.size() - 1)cout << endl;
            //if(m==pointer)cout<<"->";
            PushRenderWindow();
            if (m == pointer)SetColor(240);
            else cout << "  ";
            cout << itemList[m];
            if (values.size() > 0 && values[m] != " ")cout << ": " << values[m];
            ResetColor();
            cout << endl;
        }

        if (ButtonClicked("space")) {
            ClearPlate();
            return pointer + 1;
        }
        Sleep(menuTickSpeed);
        ClearConsole();

    }//==========================================

}

void InfoScreen(string text, int time, bool accept, vector<string> lines) {
    KeyReset();
    ClearConsole();
    LowerRenderWindow();
    if (lines.size() == 0) {
        PushRenderWindow();
        cout << text << endl;
    } else {
        for (int s = 0; s < lines.size(); s++) {
            PushRenderWindow();
            cout << lines[s] << endl;
        }
    }
    cout << endl;
    PushRenderWindow();
    int i = 0;
    if (accept) {
        cout << "   ";
        ColoredString(112, "Ok");
        while (i == 0) { if (ButtonClicked("space"))i = 1; }
    }
    else Sleep(time);
    ClearConsole();
//Sleep(300);
}

int FlipInt(int toFlip) {
    if (toFlip == 0)toFlip = 1;
    else toFlip = 0;
    return toFlip;
}

bool FlipBool(bool toFlip) {
    if (toFlip)toFlip = false;
    else toFlip = true;
    return toFlip;
}

int ScrollInt(int toScroll, int maximum, int minimum) {
    toScroll++;
    if (toScroll > maximum)toScroll = minimum;
    return toScroll;
}

void DisplayColorPallete() {
    for (int i = 0; i < 256; i++) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
        if (i % 16 == 0)cout << endl << "       ";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
        cout << i;
        if (i < 10)cout << "  ";
        else if (i < 100)cout << " ";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), i);
        cout << "A1 " << " ";
    }
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);

}

void GrassMap() {
    planet.resize(0);
    planetColor.resize(0);
    for (int p = 0; p < planetSizeX * planetSizeY; p++) {
        planet.push_back(' ');
        planetColor.push_back(32);
    }
}

void ClearMap() {
    for (int m = 0; m < planet.size(); m++) {
        planet[m] = ' ';
        planetColor[m] = 32;

    }
}

void ColorMapEdges() {

    for (int i = 0; i < planetSizeX * planetSizeY; i++) {
        if (i % planetSizeX == 0)planetColor[i] = 119;
        else if (i < planetSizeX)planetColor[i] = 119;
        else if (i > planetSizeX * planetSizeY - planetSizeX)planetColor[i] = 119;
        if ((i + 1) % planetSizeX == 0)planetColor[i] = 119;


    }
}

int PickColor() {
    ClearConsole();
    int color = 7;
    DisplayColorPallete();
    KeyReset();
    cout << endl << endl;
    PushRenderWindow();
    cout << "Pick Color: ";
    cin >> color;
    ResetColor();
    ClearPlate();
    return color;
}

void EnemySetup() {
    for (int a = 0; a < EnemySpiker.size(); a++) {
        EnemySpikerShootDelay.push_back(0);
    }
    for (int a = 0; a < EnemyCharger.size(); a++) {
        EnemyChargerDirection.push_back(0);
    }
}

void BannedColorsSetup() {
//BannedColor[0]=119;
}

void SetupThingPlacement() {
    /*for(int e=0;e<EnemySperg.size();e++){
    planet[EnemySperg[e]]=2;
 }*/

}

void ChangeVision() {
    if (ButtonClicked("O") && vision + 2 < planetSizeX && vision + 2 < planetSizeY) { vision += 2; }
    if (ButtonClicked("P") && vision - 2 >= 3) { vision -= 2; }
    if (ButtonClicked("P") || ButtonClicked("O")) { VisionSpacerSetup(); }

}

char InputSymbol() {
    KeyReset();
    char symbol = ' ';
    LowerRenderWindow();
    PushRenderWindow();
    cout << "Pick Symbol" << endl;
    PushRenderWindow();
    cout << "   ";
    SetColor(112);
    cin >> symbol;
    ResetColor();
    ClearPlate();
    KeyReset();
    return symbol;
}

void MovesSetup() {
    Moves[2] = planetSizeX;
    Moves[3] = -planetSizeX;
}

void CountMapAmount(bool increase, bool reset) {

    ifstream amount(".\\config\\config.txt");
    amount >> mapAmount;
    amount.close();

    if (increase) {
        ofstream amount(".\\config\\config.txt");
        amount << mapAmount + 1;
        amount.close();
    }

    if (reset) {
        ofstream reset(".\\config\\config.txt");
        reset << 0;
        reset.close();
    }
}

int VectorPointerPosition() {
    int place = playerPosition - SpaceLeft(playerPosition) - (SpaceAbove(playerPosition) * planetSizeX);
    return place;
// if(playerPosition-(planetSizeX*((vision-1)/2))-visionSpacer<0){vectorPointer=playerPosition-(SpaceAbove(playerPosition)*);}
    //else {vectorPointer=playerPosition-(planetSizeX*((vision-1)/2))-visionSpacer;}

}

int VectorPointerEnd() {
    int place = playerPosition + SpaceRight(playerPosition) + (SpaceBelow(playerPosition) * planetSizeX);
    return place;
}

bool PlayerAround(int place) {
    for (int a = 0; a < 4; a++) { if (place + Moves[a] == playerPosition) { return true; }}
    return false;
}

int BulletDirection(int place) {
    for (int b = 0; b < 4; b++) {
        if (planet[place] == char(bulletArrows[b])) { return Moves[b]; }
    }

    return 0;
}

void SpawnBullet(int direction, int fromWhere, bool isPlayers) {
    char bullet = ' ';

    for (int b = 0; b < 4; b++) {
        if (direction == Moves[b]) {
            bullet = char(bulletArrows[b]);
            break;
        }
    }

    planet[fromWhere + direction] = bullet;
    if (isPlayers)BulletPlacement.push_back(fromWhere + direction);
    else EnemyBulletPlacement.push_back(fromWhere + direction);

}

void EnemyBulletMovement() {
    for (int b = 0; b < EnemyBulletPlacement.size(); b++) {
        int direction = BulletDirection(EnemyBulletPlacement[b]);
        int target = EnemyBulletPlacement[b] + direction;
        int place = EnemyBulletPlacement[b];
        //Check if alive
        if (planet[place] != ' ') {
            //Check if can proceed
            if (IsNotBannedBulletObject(target) && IsPoitingAtEdge(place, direction) == false) {
                Move(place, target);
                EnemyBulletPlacement[b] += direction;
            }
                //Check if hit Player
            else if (planet[target] == 1) {
                health--;
                Replace(place, ' ');
                EnemyBulletPlacement.erase(EnemyBulletPlacement.begin() + b);
                b--;
            } else if (IsEnemy(target) || IsABullet(target)) {
                planet[target] = ' ';
                Replace(place, ' ');
                EnemyBulletPlacement.erase(EnemyBulletPlacement.begin() + b);
                b--;
            }
                //Destroy
            else {
                Replace(place, ' ');
                EnemyBulletPlacement.erase(EnemyBulletPlacement.begin() + b);
                b--;
            }
        } else {
            Replace(place, ' ');
            EnemyBulletPlacement.erase(EnemyBulletPlacement.begin() + b);
            b--;
        }
    }
}

void BulletMovement() {
    for (int b = 0; b < BulletPlacement.size(); b++) {
        //Check if alive
        if (planet[BulletPlacement[b]] != ' ') {
            int direction = BulletDirection(BulletPlacement[b]);
            //Check if can proceed
            if (IsNotBannedBulletObject(BulletPlacement[b] + direction) &&
                IsPoitingAtEdge(BulletPlacement[b], direction) == false &&
                IsABullet(BulletPlacement[b] + direction) == false) {
                Move(BulletPlacement[b], BulletPlacement[b] + direction);
                BulletPlacement[b] += direction;
            }
                //Check if hit enemy
            else if (IsEnemy(BulletPlacement[b] + direction) || IsABullet(BulletPlacement[b] + direction)) {
                planet[BulletPlacement[b] + direction] = ' ';
                Replace(BulletPlacement[b], ' ');
                BulletPlacement.erase(BulletPlacement.begin() + b);
                b--;
            } else//Delete Bullet
            {
                Replace(BulletPlacement[b], ' ');
                BulletPlacement.erase(BulletPlacement.begin() + b);
                b--;
            }
        } else//Delete
        {
            Replace(BulletPlacement[b], ' ');
            BulletPlacement.erase(BulletPlacement.begin() + b);
            b--;
        }
    }
}

void SteppingOn(int place) {
    if (planet[place] == 3)health++;
    else if (planet[place] == 4)playerMaxBullets++;
    if (planetColor[place] == 128) { gameRunning = false; }
}

void MovePlayer(int to) {
    if (IsMoveValid(playerPosition, to - playerPosition) || IsAPickup(to)) {
        Move(playerPosition, to);
        playerPosition = to;
    }
}

void PlayerBulletLogic() {
    int newBulletPosition = 0;
    if (ButtonClicked("X") && (playerMaxBullets - BulletPlacement.size() > 0)) {
        for (int b = 0; b < 4; b++) {
            if (ButtonClicked(moveClicks[b]) && IsNotBannedBulletObject(playerPosition + Moves[b])) {
                newBulletPosition = Moves[b];
                break;
            }
        }
        if (newBulletPosition != 0) {
            SpawnBullet(newBulletPosition, playerPosition, true);
        }
    }

}

void PlayerMovement() {
    if (ButtonClicked("X") == false) {
        int moveAmount = 0;
        if (ButtonClicked("W") && SpaceAbove(playerPosition) != 0)moveAmount -= planetSizeX;
        else if (ButtonClicked("A") && SpaceLeft(playerPosition) != 0)moveAmount--;
        else if (ButtonClicked("D") && SpaceRight(playerPosition) != 0)moveAmount++;
        else if (ButtonClicked("S") && SpaceBelow(playerPosition) != 0)moveAmount += planetSizeX;
        if (moveAmount != 0)SteppingOn(playerPosition + moveAmount);
        MovePlayer(playerPosition + moveAmount);
    }
}

void SetPlayerSpawn(int place) {
    if (planet[storedPlayerPosition] == 1)planet[storedPlayerPosition] = ' ';
    planet[place] = 1;
    storedPlayerPosition = place;

}

void PlayerLogic() {
    ChangeVision();
    PlayerBulletLogic();
    PlayerMovement();
}

void PlayerDeath() {
    Sleep(1000);
    Beep(784, 1000);
    InfoScreen("You died!", 2000, false, eV);
    ClearPlate();
    gameRunning = false;
}

void EnemyChargerLogic() {
    for (int e = 0; e < EnemyCharger.size(); e++) {
        if (planet[EnemyCharger[e]] == '*' || planet[EnemyCharger[e]] == ' ') {
            planet[EnemyCharger[e]] = ' ';
            EnemyCharger.erase(EnemyCharger.begin() + e);
            e--;
        } else {
            if (PlayerAround(EnemyCharger[e])) {
                health--;
                planet[EnemyCharger[e]] = ' ';
                EnemyCharger.erase(EnemyCharger.begin() + e);
                e--;
            } else {
                int move = 0;
                int playerDirection = WhereISee(1, EnemyCharger[e]);
                if (EnemyChargerDirection[e] == 0 && playerDirection == 0) {
                    int maxTries = 4;
                    while (maxTries > 0) {
                        int moveTry = Moves[rand() % 4];
                        if (IsMoveValid(EnemyCharger[e], moveTry)) {
                            move = moveTry;
                            break;
                        }
                        maxTries--;
                    }
                }
                if (playerDirection != 0)EnemyChargerDirection[e] = playerDirection;
                if (EnemyChargerDirection[e] != 0)move = EnemyChargerDirection[e];
                if (IsMoveValid(EnemyCharger[e], move)) {
                    Move(EnemyCharger[e], EnemyCharger[e] + move);
                    EnemyCharger[e] += move;
                } else EnemyChargerDirection[e] = 0;
            }
        }
    }
}

void EnemySpikerLogic() {
    for (int e = 0; e < EnemySpiker.size(); e++) {
        if (planet[EnemySpiker[e]] == '*' || planet[EnemySpiker[e]] == ' ') {
            planet[EnemySpiker[e]] = ' ';
            EnemySpiker.erase(EnemySpiker.begin() + e);
            e--;
        } else {
            int direction = WhereISee(1, EnemySpiker[e]);
            if (direction != 0 && PlayerAround(EnemySpiker[e]) == false && EnemySpikerShootDelay[e] == 0 &&
                IsEnemy(direction + EnemySpiker[e]) == false) {
                SpawnBullet(direction, EnemySpiker[e], false);
                EnemySpikerShootDelay[e] = 3;
            } else if (EnemySpikerShootDelay[e] > 0)EnemySpikerShootDelay[e]--;

        }
    }
}

void EnemySpergLogic() {
    for (int e = 0; e < EnemySperg.size(); e++) {
        if (planet[EnemySperg[e]] == '*' || planet[EnemySperg[e]] == ' ' || IsABullet(EnemySperg[e])) {
            planet[EnemySperg[e]] = ' ';
            EnemySperg.erase(EnemySperg.begin() + e);
            e--;
        } else {
            if (PlayerAround(EnemySperg[e])) {
                health--;
                planet[EnemySperg[e]] = ' ';
                EnemySperg.erase(EnemySperg.begin() + e);
                e--;
            } else {
                int maxTries = 4;
                while (maxTries > 0) {
                    int move = Moves[rand() % 4];
                    if (IsMoveValid(EnemySperg[e], move)) {
                        Move(EnemySperg[e], EnemySperg[e] + move);
                        EnemySperg[e] += move;
                        break;
                    }
                    maxTries--;
                }
            }
        }
    }
}

void EnemyLogic() {
    EnemyBulletMovement();
    EnemySpergLogic();
    EnemySpikerLogic();
    EnemyChargerLogic();
}

void AddEnemy(int place, int type) {
    switch (type) {
        case 0: {
            planet[place] = spergSymbol;
            EnemySperg.push_back(place);
            break;
        }
        case 1: {
            planet[place] = spikerSymbol;
            EnemySpiker.push_back(place);
            break;
        }
        case 2: {
            planet[place] = chargerSymbol;
            EnemyCharger.push_back(place);
            break;
        }
    }

}

void EditorNodeSymbolPlacement() {
    for (int s = 0; s < 3; s++) {
        if (ButtonClicked(symbolButtons[s])) {
            planet[playerPosition] = editorSymbol[s];
        }
    }
}

void EditorNodeColorPlacement() {
    for (int c = 0; c < 3; c++) {
        if (ButtonClicked(colorButtons[c])) {
            planetColor[playerPosition] = editorColor[c] + 16;
        }
    }
}

void RenderMap(bool numberRender) {
    int vectorPointer = 0;
    for (int y = 1; y < planetSizeY + 1; y++) {
        for (int x = 1; x < planetSizeX + 1; x++) {
            SetColor(planetColor[vectorPointer]);
            cout << planet[vectorPointer];
            if (hZ == 1)cout << " ";
            if (vectorPointer <= 11 && numberRender)cout << " ";
            if (vectorPointer <= 101 && numberRender)cout << " ";
            vectorPointer++;
        }
        cout << endl;
    }
    ResetColor();
}

void RenderMapWindow() {
    int vectorPointer = VectorPointerPosition();
    int vectorEnd = VectorPointerEnd();
    int spaceToRightBorder = SpaceRight(playerPosition);
    int plane = SpaceAroundMe(playerPosition);
    int renderLoop = SpaceLeft(playerPosition) + SpaceRight(playerPosition) + 1;

    if (SpaceAroundMe(playerPosition) > planetSizeX * planetSizeY) { plane = planetSizeX * planetSizeY; }

    isWindowRendered = true;

    LowerRenderWindow();
    PushRenderWindow();

//Top Bar
    DrawTopBar();
//Top Bar End

//Main Render
    for (int p = 0; p < plane; p++) {
        SetColor(planetColor[vectorPointer]);
        cout << planet[vectorPointer];
        if (hZ == 1)cout << " ";

        if (SpaceRight(vectorPointer) == 0) {
            vectorPointer += planetSizeX - SpaceRight(playerPosition) - visionSpacer;
            if (SpaceLeft(vectorPointer) != 0)vectorPointer--;
            renderLoop = SpaceLeft(playerPosition) + SpaceRight(playerPosition) + 1;
            ResetColor();

            for (int b = 0; b < SpacerRight() * (hZ + 1); b++) { cout << " "; }
            cout << char(186);
            cout << endl;
            PushRenderWindow();
            if (p != plane - 1) cout << char(186);
            for (int b = 0; b < SpacerLeft(); b++) { cout << " "; }
        } else renderLoop--;

        if (renderLoop == 0) {
            if (SpaceLeft(playerPosition) == 0) { vectorPointer += planetSizeX - visionSpacer - 1; }
            else {
                vectorPointer += planetSizeX - visionSpacer;
                vectorPointer -= SpaceLeft(vectorPointer) + 1;
            }

            ResetColor();
            for (int b = 0; b < SpacerLeft() * (hZ); b++) { cout << " "; }
            cout << char(186);
            cout << endl;
            PushRenderWindow();
            if (p != plane - 1) cout << char(186);
            if (p != plane - 1)for (int b = 0; b < SpacerLeft(); b++) { cout << " "; }
            renderLoop = SpaceLeft(playerPosition) + SpaceRight(playerPosition) + 1;
        }
        vectorPointer++;
    }

    //Bottom Lines
    DrawBottomLine();
    //Bottom Lines End
    isWindowRendered = false;
    ResetColor();
}

void RenderHud(int tick) {
    isWindowRendered = true;
    cout << endl;
    if (debugHud) {
        PushRenderWindow();
        cout << "Tick: " << tick << endl;
        PushRenderWindow();
        cout << "Position: " << playerPosition << endl;
        PushRenderWindow();
        cout << "On Edge: " << CheckIfOnEdge(playerPosition) << endl;
        PushRenderWindow();
        cout << "Enemy Amount: " << EnemySperg.size() << endl;
        PushRenderWindow();
        cout << "C: " << EnemyCharger.size() << " Se: " << EnemySperg.size() << " Sp: " << EnemySpiker.size() << endl;
        PushRenderWindow();
        cout << "PRW: " << pushRenderStart << endl;
    }
    PushRenderWindow();
    for (int h = 0; h < health; h++)ColoredChar(4, 3);
    cout << endl;
    PushRenderWindow();
    for (int a = 0; a < playerMaxBullets - BulletPlacement.size(); a++)ColoredChar(8, 17);
    cout << endl;
// PushRenderWindow();cout<<"Ammo: "<<playerMaxBullets-BulletPlacement.size()<<endl;

    isWindowRendered = false;
}


void SaveMap(int mapId) {
    ofstream Map(".\\maps\\map" + to_string(mapId) + ".txt");
    Map << planetSizeX << " " << planetSizeY << endl;
    Map << vision << " " << storedPlayerPosition << endl;


    for (int m = 0; m < planet.size(); m++) {
        if (m % planetSizeX == 0)Map << endl;
        switch (planet[m]) {
            case ' ':
                Map << '0';
                break;
            case spergSymbol  :
                Map << 's';
                break;
            case 1  :
                Map << 'p';
                break;
            case spikerSymbol :
                Map << 'x';
                break;
            case chargerSymbol :
                Map << 'c';
                break;
            default:
                Map << planet[m];
                break;
        }
    }

    Map << endl;

    for (int m = 0; m < planet.size(); m++) {
        if (m % planetSizeX == 0)Map << endl;
        if (m == playerPosition)Map << planetColor[m] - 16 << " ";
        else Map << planetColor[m] << " ";
    }

    Map.close();
}

void LoadMap(int mapId) {
    planet.clear();
    planetColor.clear();
    EnemySperg.clear();
    EnemyCharger.clear();
    EnemySpiker.clear();
    ifstream Map(".\\maps\\map" + to_string(mapId) + ".txt");
    Map >> planetSizeX;
    Map >> planetSizeY;
    Map >> vision;
    Map >> playerPosition;
    storedPlayerPosition = playerPosition;

    char symbol = ' ';
    for (int m = 0; m < planetSizeX * planetSizeY; m++) {
        Map >> symbol;
        switch (symbol) {
            case '0':
                planet.push_back(' ');
                break;
            case 's':
                EnemySperg.push_back(m);
                planet.push_back(spergSymbol);
                break;
            case 'p':
                planet.push_back(1);
                break;
            case 'x':
                EnemySpiker.push_back(m);
                planet.push_back(spikerSymbol);
                break;
            case 'c':
                EnemyCharger.push_back(m);
                planet.push_back(chargerSymbol);
                break;
            default:
                planet.push_back(symbol);
                break;
        }
    }

    int color = 32;
    for (int m = 0; m < planetSizeX * planetSizeY; m++) {
        Map >> color;
        planetColor.push_back(color);
    }

    SetupThingPlacement();
    VisionSpacerSetup();
    MovesSetup();
    Map.close();
}

void RenderMapPreview(int mapId) {
    LoadMap(mapId);
    storedVision = vision;
    if (vision > 9)vision = 9;
    VisionSpacerSetup();
    RenderMapWindow();
}

void SaveMode() {
    ClearConsole();
    PushRenderWindow();
    int pointer = PickItemFromListV2(saveModeOptions, "Save Map", eV, true, eV, 0);
    int mapId;
    switch (pointer) {
        case 1:
            SaveMap(mapAmount + 1);
            CountMapAmount(true, false);
            break;
        case 2:
            mapId = SetValue("Override Map", 1, 1, 1, mapAmount);
            SaveMap(mapId);
            break;
        case 3:
            ClearPlate();
            break;
    }

}

void LoadMode() {
    CountMapAmount(false, false);
    int mapId = 1;
    if (mapAmount > 0) {
        while (1 > 0) {
            if (ButtonClicked("space"))break;
            if (ButtonClicked("A") && mapId > 1)mapId--;
            else if (ButtonClicked("D") && mapId < mapAmount)mapId++;
            RenderMapPreview(mapId);
            PushRenderWindow();
            cout << " Map Id: " << mapId << endl;
            PushRenderWindow();
            if (ButtonClicked("A") && mapId > 1)ColoredString(112, "<");
            else cout << "<";
            cout << "  ";
            ColoredString(112, "Select");
            cout << "  ";
            if (ButtonClicked("D") && mapId < mapAmount)ColoredString(112, ">");
            else cout << ">";
            Sleep(300);
            ClearConsole();
        }
        vision = storedVision;
        VisionSpacerSetup();
    }
//LoadMap(mapId);
}

void RemoveMaps() {

    for (int m = 1; m <= mapAmount; m++) {
        const string filePath = ".\\maps\\map" + to_string(m) + ".txt";
        remove(filePath.c_str());
    }

    CountMapAmount(false, true);
}

void EditorNodeBuilding() {
    if (ButtonClicked("E") == false) {
        EditorNodeSymbolPlacement();
        EditorNodeColorPlacement();
    } else {
        for (int s = 0; s < 3; s++) { if (ButtonClicked(symbolButtons[s])) { editorSymbol[s] = InputSymbol(); }}
        for (int c = 0; c < 3; c++) { if (ButtonClicked(colorButtons[c])) { editorColor[c] = PickColor(); }}
    }
    ChangeVision();
}

void EditorNodeMovement() {
    int move = 0;
    if (ButtonClicked("D") && SpaceRight(playerPosition) != 0)move += Moves[0];
    else if (ButtonClicked("A") && SpaceLeft(playerPosition) != 0)move += Moves[1];
    else if (ButtonClicked("S") && SpaceBelow(playerPosition) != 0)move += Moves[2];
    else if (ButtonClicked("W") && SpaceAbove(playerPosition) != 0)move += Moves[3];
    if (move != 0) {
        planetColor[playerPosition] -= 16;
        playerPosition += move;
        planetColor[playerPosition] += 16;
    }

}

void EditorNodeLogic() {
    EditorNodeMovement();
    EditorNodeBuilding();
    if (ButtonClicked("I") && ButtonClicked("E")) { enemyToSpawn = ScrollInt(enemyToSpawn, 2, 0); }
    else if (ButtonClicked("I"))AddEnemy(playerPosition, enemyToSpawn);
    if (ButtonClicked("U"))SetPlayerSpawn(playerPosition);
    if (ButtonClicked("enter")) {
        editorRunning = false;
        SaveMode();
    }
    if (ButtonClicked("L")) { ColorMapEdges(); }
    if (ButtonClicked("K")) { InfoScreen("", 0, true, editorInfo); }
}

void EditorHud() {
    isWindowRendered = true;
    PushRenderWindow();
    cout << "Position: " << playerPosition << endl;
    PushRenderWindow();
    for (int s = 0; s < 3; s++) { cout << symbolButtons[s] << ": " << editorSymbol[s] << " "; }
    cout << endl;
    PushRenderWindow();
    for (int c = 0; c < 3; c++) {
        cout << colorButtons[c] << ": ";
        SetColor(editorColor[c]);
        cout << " ";
        ResetColor();
        cout << " ";
    }
    cout << endl;
    PushRenderWindow();
    cout << "Press I to add Enemy: ";
    ColoredChar(112, EnemySymbols[enemyToSpawn]);
    cout << endl;
    PushRenderWindow();
    cout << "Press K for more information" << endl;
    isWindowRendered = false;
}

void ChangeFontModule() {

    if (ButtonClicked("T") || ButtonClicked("Y") || ButtonClicked("H") || ButtonClicked("G")) {
        if (ButtonClicked("G"))fontWindth--;
        else if (ButtonClicked("T"))fontWindth++;

        if (ButtonClicked("Y"))fontHeight++;
        else if (ButtonClicked("H"))fontHeight--;
        setFontSize(fontWindth, fontHeight);
    }

}

void SaveSettings() {
    ofstream Settings(".\\config\\settings.txt");
    Settings << tickSpeed << endl;
    Settings << hZ << endl;
    Settings << vision << endl;
    Settings << debugHud << endl;
    Settings << resolution << endl;
    Settings << fontHeight << endl;
    Settings << fontWindth << endl;
    Settings.close();
}

void LoadSettings() {
    ifstream Settings(".\\config\\settings.txt");
    Settings >> tickSpeed;
    Settings >> hZ;
    Settings >> vision;
    Settings >> debugHud;
    Settings >> resolution;
    Settings >> fontHeight;
    Settings >> fontWindth;
    Settings.close();
}

void MapSetup(bool resetStats) {
    VisionSpacerSetup();
    SetupThingPlacement();
    EnemySetup();
    if (resetStats) {
        health = 3;
        playerMaxBullets = 3;
    }
}

void GameSetup() {
    LoadSettings();
    NoCursor(false);
    MaxWindow();
    CountMapAmount(false, false);
    setFontSize(fontWindth, fontHeight);
//SetupAvailableMoves();
    //FillMapWithNumbers();
}

void NewWorldSetup() {
    VisionSpacerSetup();
    MovesSetup();
    GrassMap();
}

void NewMapSettingsSetup() {
    planetSizeX = 20;
    planetSizeY = 20;
    vision = 11;

}

void NewMapCreation() {
    bool whileOn = true;

    NewMapSettingsSetup();
    while (whileOn) {
        ClearMap();

        vector<string> newMapValues{to_string(planetSizeX), to_string(planetSizeY), to_string(vision), " "};
        int pointer = PickItemFromListV2(newMapOptions, "New Map Settings", newMapValues, true, eV, 3);
        switch (pointer) {
            case 1:
                planetSizeX = SetValue("Map Width", planetSizeX, 10, 5, 1000);
                break;
            case 2:
                planetSizeY = SetValue("Map Height", planetSizeY, 10, 5, 1000);
                break;
            case 3: {
                int maxVision = planetSizeX;
                if (planetSizeX > planetSizeY)maxVision = planetSizeY;
                vision = SetValue("Vision Distance", vision, 2, 3, maxVision);
                break;
            }
            case 4:
                whileOn = false;
                break;
        }
    }


    NewWorldSetup();
}

void GameLoopEnd() {
    NoCursor(false);
    RenderMapWindow();
    RenderHud(tick);

    Sleep(tickSpeed);
    if (health <= 0) { PlayerDeath(); }
    if (ButtonClicked("enter")) {
        ClearPlate;
        gameRunning = false;
    }
    if (ButtonClicked("I")) {
        hZ = FlipInt(hZ);
        if (hZ == 1)pushRenderStart -= (vision + 1) / 2;
        if (hZ == 0)pushRenderStart += (vision + 1) / 2;
    }
    if (planetColor[playerPosition] == 128 && gameRunning == false) {
        ClearPlate();
        InfoScreen("You won", 2000, false, eV);
    };
    ChangeFontModule();
    ClearConsole();
    tick++;
}

void GameLoop() {
    NoCursor(false);
    MapSetup(true);
    gameRunning = true;
    while (gameRunning) {
        BulletMovement();
        PlayerLogic();
        EnemyLogic();

        GameLoopEnd();
    }
}

void Editor() {
    editorRunning = false;
    int pointer = PickItemFromListV2(editorOptions, "Editor", eV, true, eV, 0);
    switch (pointer) {
        case 1:
            NewMapCreation();
            editorRunning = true;
            break;
        case 2:
            LoadMode();
            editorRunning = true;
            break;
        case 3:
            break;
    }
    if (playerPosition > planetSizeX * planetSizeY)playerPosition = 0;
    planetColor[playerPosition] += 16;
    KeyReset();
    bool refresh = true;
    while (editorRunning) {

        EditorNodeLogic();

        if (refresh && editorRunning) {
            NoCursor(false);
            RenderMapWindow();
            EditorHud();
            Sleep(tickSpeed / 2);
        }

        if (IsAnyKeyPressed() == false)refresh = false;
        else if (IsAnyKeyPressed()) {
            refresh = true;
            ClearConsole();
        }
    }
}

void StartGame() {
    CountMapAmount(false, false);
    if (mapAmount != 0) {
        LoadMode();
        GameLoop();
    } else InfoScreen("No Maps Detected", infoSpeed, false, eV);
}

void Info() {
    bool infoRun = true;
    while (infoRun) {
        int pointer = PickItemFromListV2(infoOptions, "Info", eV, true, eV, 2);
        switch (pointer) {
            case 1:
                InfoScreen("Game", 0, true, gameInfo);
            case 2:
                InfoScreen("Editor", 0, true, editorInfo);
            case 3:
                infoRun = false;
                break;
        }
        KeyReset();
    }
    KeyReset();
}

void TestArea() {
    while (ButtonClicked("enter") == false) {
        LowerRenderWindow();
        PushRenderWindow();
        cout << "A/D TO CHANGE WIDTH" << endl;
        cout << "W/S TO CHANGE HEIGHT" << endl;
        cout << "Enter to exit" << endl;
        if (ButtonClicked("A"))fontWindth--;
        else if (ButtonClicked("D"))fontWindth++;

        if (ButtonClicked("W"))fontHeight++;
        else if (ButtonClicked("S"))fontHeight--;
        setFontSize(fontWindth, fontHeight);
        Sleep(300);
        ClearPlate();
    }
}

void Options() {
    bool whileOn = true;
    while (whileOn) {

        vector<string> optionsValues{to_string(tickSpeed), state[hZ], to_string(vision), " ", state[debugHud],
                                     resolutions[resolution], " "};
        int pointer = PickItemFromListV2(settingsOptions, "Settings", optionsValues, true, eV, 5);
        switch (pointer) {
            case 1:
                tickSpeed = SetValue("Game Speed", tickSpeed, 100, 0, 2000);
                break;
            case 2:
                hZ = SetValue("Horizontal Spacing", hZ, 1, 0, 1);
                break;
            case 3:
                vision = SetValue("Vision Range", vision, 2, 3, 51);
                break;
            case 4:
                RemoveMaps();
                break;
            case 5:
                debugHud = FlipBool(debugHud);
                break;
            case 6:
                resolution = FlipInt(resolution);
                break;
            case 7:
                SaveSettings();
                whileOn = false;
                break;
        }
    }
}

void Quit() {
    SaveSettings();
    Sleep(500);
    exit(1);
}

void Menu() {
    ClearPlate();
    while (1 > 0) {
        int menuPointer = PickItemFromListV2(menuOptions, " ", eV, true, Title, 0);
        switch (menuPointer) {
            case 1:
                StartGame();
                break;
            case 2:
                Editor();
                break;
            case 3:
                Options();
                break;
            case 4:
                Info();
                break;
            case 5:
                TestArea();
                break;
            case 6:
                Quit();
                break;
        }

    }
}

int main() {

    GameSetup();
    Menu();
    return 0;
}
