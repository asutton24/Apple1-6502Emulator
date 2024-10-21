#include "raylib.h"
#include "sprite.h"
#include "6502emu.h" 
#include "assemble.h"

const Color green = (Color){41, 245, 43, 255};
double flashClock;
byte flashMode;
byte* vram;
byte cx;
byte cy;
byte color = 0;
char* byteList = "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&\'()*+,-./0123456789:;<=>?";
byte printTen = 0;
byte firstFrame = 1;
char** tapes;
dbyte* ranges;
byte tapeNum = 0;
byte tapeMax = 0;
byte debug = 0;
int fileLen = 0;

byte toIndex(byte ch){
    ch = ch & 127;
    if (!ch){
        return 32;
    }
    if (ch > 95){
        ch = ch - 32;
    }
    for (int i = 0; i < 64; i++){
        if (byteList[i] == ch){
            return i;
        }
    }
    return 255;
}

void cycleScreen(){
    for (int i = 1; i < 24; i++){
        for (int j = 0; j < 40; j++){
            vram[40 * (i - 1) + j] = vram[40 * i + j];
        }
    }
    for (int i = 920; i < 960; i++){
        vram[i] = 0;
    }
}

void vramUpdate(){
    if (!firstFrame && (mem[0xD012] & 127) != 0){
        if ((mem[0xD012] & 127) == 13){
            cy++;
            cx = 0;
        } else {
            if (toIndex(mem[0xD012]) != 255){
                vram[cy * 40 + cx] = mem[0xD012];
                cx++;
                if (cx == 40){
                    cx = 0;
                    cy++;
                }
            }
        }
        if (cy == 24){
            cy--;
            cycleScreen();
        }
    } else {
        firstFrame = 0;
    }
    mem[0xD012] = 0;
}

void updateScreen(){
    for (int i = 0; i < 24; i++){
        for (int j = 0; j < 40; j++){
            drawSprite(toIndex(vram[40 * i + j]), color * 2, 1, 1, 7 * j, 8 * i);
        }
    }
    if (flashMode){
        drawSprite(0, color * 2, 1, 1, 7 * cx, 8 * cy);
    }
}

void clearScreen(){
    cx = 0;
    cy = 0;
    for (int i = 0; i < 960; i++){
        vram[i] = 0;
    }
}

int getAscii(){
    bool shiftMode = false;
    char* nums = ")!@#$%^&*(";
    if (IsKeyDown(KEY_LEFT_SHIFT)){
        shiftMode = true;
    }
    for (int i = 65; i < 91; i++){
        if (IsKeyDown(i)){
            return i;
        }
    }
    for (int i = 48; i < 58; i++){
        if (IsKeyDown(i)){
            if (shiftMode){
                return nums[i - 48];
            }
            return i;
        }
    }
    if (IsKeyDown(KEY_SEMICOLON)){
        if (shiftMode){
            return ':';
        }
        return ';';
    }
    if (IsKeyDown(KEY_ENTER)){
        return 13;
    }
    if (IsKeyDown(KEY_APOSTROPHE)){
        if (shiftMode){
            return '\"';
        }
        return '\'';
    }
    if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_RIGHT)){
        return ' ';
    }
    if (IsKeyDown(KEY_LEFT_BRACKET)){
        return '[';
    }
    if (IsKeyDown(KEY_RIGHT_BRACKET)){
        return ']';
    }
    if (IsKeyDown(KEY_EQUAL)){
        if (shiftMode){
            return '+';
        }
        return '=';
    }
    if (IsKeyDown(KEY_PERIOD)){
        if (shiftMode){
            return '>';
        }
        return '.';
    }
    if (IsKeyDown(KEY_COMMA)){
        if (shiftMode){
            return '<';
        }
        return ',';
    }
    if (IsKeyDown(KEY_MINUS)){
        if (shiftMode){
            return '_';
        }
        return '-';
    }
    if (IsKeyDown(KEY_SLASH)){
        if (shiftMode){
            return '?';
        }
        return '/';
    }
    if (IsKeyDown(KEY_BACKSLASH)){
        return '\\';
    }
    if (IsKeyDown(KEY_LEFT)){
        return 8;
    }
    return -1;
}

dbyte HexParse(char* num){
    dbyte sum = 0;
    for (int i = 0; i < 4; i++){
        sum = sum << 4;
        if (num[i] >= '0' && num[i] <= '9'){
            sum += num[i] - '0';
        } else if (num[i] >= 'A' && num[i] <= 'F'){
            sum += num[i] - 'A' + 10;
        } else {
            return 0;
        }
    }
    return sum;
}

void freeTapes(){
    for (int i = 0; i < tapeMax; i++){
        free(tapes[i]);
    }
    free(tapes);
    free(ranges);
}

int tapeSetup(){
    FILE* file;
    file = fopen("tapes.txt", "r");
    if (file == NULL){
        return -1;
    }
    char* buffer = (char*)malloc(512 * sizeof(char));
    for (int i = 0; i < 512; i++){
        buffer[i] = 0;
    }
    fread(buffer, sizeof(char), 512, file);
    tapeMax = 0;
    for (int i = 0; i < 512; i++){
        if (buffer[i] == 0){
            fileLen = i;
            i += 512;
        } else if (buffer[i] == '\n'){
            tapeMax++;
        }
    }
    tapes = (char**)malloc(tapeMax * sizeof(char*));
    ranges = (dbyte*)malloc(tapeMax * 2 * sizeof(dbyte));
    byte current = 0;
    dbyte p = 0;
    dbyte lastAdd;
    while (current < tapeMax){
        lastAdd = p;
        while (buffer[p] != ' '){
            p++;
        }
        tapes[current] = (char*)malloc(p - lastAdd + 1);
        for (int i = lastAdd; i < p; i++){
            tapes[current][i - lastAdd] = buffer[i];
        }
        tapes[current][p - lastAdd] = 0;
        ranges[2 * current] = HexParse(buffer + p + 1);
        ranges[2 * current + 1] = HexParse(buffer + p + 6);
        while (buffer[p] != '\n'){
            p++;
        }
        p++;
        if (buffer[p] == 10){
            p++;
        }
        current++;
    }
    free(buffer);
    return 0;
}

void toggleDebug(){
    if (debug){
        debug = 0;
        baseh = 192;
    } else {
        debug = 1;
        baseh = 250;
    }
    resizeScreen(res);
}

void save(){
    byte revert = 0;
    dbyte saveLow;
    dbyte saveHigh;
    if (!debug){
        toggleDebug();
        revert = 1;
    }
    char* buffer = (char*)malloc(10 * sizeof(char));
    for (int i = 0; i < 9; i++){
        buffer[i] = '_';
    }
    buffer[4] = ' ';
    buffer[9] = 0;
    int input;
    byte pointer = 0;
    byte hold = 0;
    while (pointer != 9){
        BeginDrawing();
        ClearBackground(BLACK);
        updateScreen();
        DrawText(buffer, 80 * res, 210 * res, 20 * res, RED);
        EndDrawing();
        for (int i = '0'; i <= 'F'; i++){
            input = -1;
            if (IsKeyDown(i)){
                input = i;
                break;
            }
            if (i == '9'){
                i = 'A' - 1;
            }
        }
        if (IsKeyDown(KEY_ESCAPE) || IsKeyDown(KEY_BACKSPACE)){
            if (revert){
                toggleDebug();
            }
            free(buffer);
            return;
        }
        if (input != -1){
            if (!hold){
                buffer[pointer] = input;
                pointer++;
                if (pointer == 4){
                    pointer++;
                }
            }
            hold = 1;
        } else {
            hold = 0;
        }
    }
    saveLow = HexParse(buffer);
    saveHigh = HexParse(buffer + 5);
    free(buffer);
    if (saveHigh <= saveLow){
        if (revert){
            toggleDebug();
        }
        return;
    }
    FILE* file = fopen("tape.rom", "wb");
    fwrite(mem + saveLow, 1, saveHigh - saveLow + 1, file);
    if (revert){
        toggleDebug();
    }
}

int assembleFromSource(char * src){
    FILE* file = fopen(src, "rb");
    if (file == NULL) return -1;
    fseek(file, 0L, SEEK_END);
    int sz = ftell(file);
    char* str = (char*)malloc(sz + 1);
    rewind(file);
    fread(str, 1, sz, file);
    str[sz] = 0;
    int ret = assemble(str, mem);
    free(str);
    return ret;
}

int main(int argc, char** argv){
    InitWindow(280, 192, "APPLE 1");
    memSetup();
    loadData("monitor.rom", 0xFF00, 0xFFFF, 1);
    loadData("basic.rom", 0xE000, 0xF000, 0);
    if (argc > 1) assembleFromSource(argv[1]);
    tapeSetup();
    reset();
    loadAssets();
    resizeScreen(2);
    vram = (byte*)malloc(960 * sizeof(byte));
    clearScreen();
    const double refresh = 1.0 / 60.0;
    double cycle = 1.0 / 1022727.0;
    double wait;
    double curTime = GetTime();
    double time2;
    int key = -1;
    int check = 0;
    int checklen = 4;
    byte monoCheck = 0;
    int keyHold = -1;
    flashClock = GetTime();
    firstFrame = 1;
    byte keyRefresh;
    byte cycleHold = 0;
    byte debugHold = 0;
    byte sizeHold = 0;
    while (!WindowShouldClose()){
        keyRefresh = 0;
        while (GetTime() - curTime < refresh){
            time2 = GetTime();
            runcmd();
            if (key == -1) key = getAscii();
            if (mem[0xD012] != 0 && mem[0xD012] < 128) mem[0xD012] = mem[0xD012] + 128;
            //DrawText(TextFormat("PC: %.04X A: %.02X X: %.02X Y: %.02X F: %.02X SLOW\n %.02X %.02X", pgc, a, x, y, flags, mem[0x4C], mem[0x4D]), 10 * res, 210 * res, 10 * res, RED);
            wait = cycles * cycle;
            while (GetTime() - time2 < wait){
                continue;
            }
            keyRefresh++;
        }
        BeginDrawing();
        curTime = GetTime();
        ClearBackground(BLACK);
        if (curTime - flashClock >= .5){
            flashClock = curTime;
            if (flashMode == 0){
                flashMode = 1;
            } else {
                flashMode = 0;
            }
        }
        vramUpdate();
        updateScreen();
        if (debug) DrawText(TextFormat("PC: %.04X A: %.02X X: %.02X Y: %.02X F: %.02X S: %.02X", pgc, a, x, y, flags, s), 10 * res, 210 * res, 10 * res, RED);
        if (cycleHold) DrawText(TextFormat("TAPE #%d", tapeNum), 220 * res, 4 * res, 10 * res, RED);
        EndDrawing();
        if (IsKeyDown(KEY_F1)){
            if (monoCheck){
                monoCheck = 0;
                if (color == 0){
                    color = 1;
                } else {
                    color = 0;
                }
            }
        } else {
            monoCheck = 1;
        }
        if (IsKeyDown(KEY_F2)){
            reset();
            firstFrame = 1;
        }
        if (IsKeyDown(KEY_F8)){
            if (!debugHold) toggleDebug();
            debugHold = 1;
        } else {
            debugHold = 0;
        }
        if (IsKeyDown(KEY_F6) && tapeMax > 0){
            loadData(tapes[tapeNum], ranges[2 * tapeNum], ranges[2 * tapeNum + 1], 0);
        }
        if (IsKeyDown(KEY_F7)){
            save();
        }
        if (IsKeyDown(KEY_F3)){
            clearScreen();
        }
        if ((IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN)) && tapeMax > 0){
            if (cycleHold == 0){
                if (IsKeyDown(KEY_UP)){
                    tapeNum = (tapeNum + 1) % tapeMax;
                } else {
                    if (tapeNum == 0){
                        tapeNum = tapeMax - 1;
                    } else {
                        tapeNum--;
                    }
                }
            }
            cycleHold = 1;
        } else {
            cycleHold = 0;
        }
        if (IsKeyDown(KEY_F4) || IsKeyDown(KEY_F5)){
            if (!sizeHold){
                if (IsKeyDown(KEY_F4) && res > 1){
                    resizeScreen(res - 1);
                } else if (res < 4) {
                    resizeScreen(res + 1);
                }
            }
            sizeHold = 1;
        } else {
            sizeHold = 0;
        }
        if (key != -1 && key != keyHold && check >= checklen){
            mem[0xD010] = key + 128;
            mem[0xD011] = 128;
            check = 0;
            keyHold = key;
            kbdReset = 0;
        }
        if (key == -1){
            keyHold = -1;
        } 
        check++;
        key = -1;
    }
    freeAssets();
    freeMem();
    freeTapes();
    free(vram);
    return 0;
}