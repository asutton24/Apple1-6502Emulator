#include "assemble.h"

char opcodes[57][4] = {
    "adc", "and", "asl", "bcc", "bcs", "beq", "bit", "bmi",
    "bne", "bpl", "brk", "bvc", "bvs", "clc", "cld", "cli",
    "clv", "cmp", "cpx", "cpy", "dec", "dex", "dey", "eor",
    "inc", "inx", "iny", "jmp", "jsr", "lda", "ldx", "ldy",
    "lsr", "nop", "ora", "pha", "php", "pla", "plp", "rol",
    "ror", "rti", "rts", "sbc", "sec", "sed", "sei", "sta",
    "stx", "sty", "tax", "tay", "tsx", "txa", "txs", "tya",
    "dcb"
};

char** labels;
dbyte* labelVals;
int labelLen;
dbyte pc = 0;
byte instruct[4] = {0, 0, 0, 0};

int strequals(char* a, char* b){
    int off = 0;
    while (a[off] != 0 && b[off] != 0 && a[off] == b[off]){
        off++;
    }
    return a[off] == b[off];
}

int sequenceEqu(char* a, char* b, int len){
    for (int i = 0; i < len; i++){
        if (a[i] != b[i] || (a[i] == 0 && i == len - 1)){
            return 0;
        }
    }
    return 1;
}

int findNextNewline(char* a){
    int off = 0;
    while (a[off] != 0){
        if (a[off] == '\n'){
            return off;
        }
        off++;
    }
    return -1;
}

int findOpcode(char* a){
    char code[4] = {a[0], a[1], a[2], 0};
    for (int i = 0; i < 3; i++){
        if (code[i] >= 'A' && code[i] <= 'Z'){
            code[i] = 'a' + code[i] - 'A';
        }
    }
    if (!(a[3] == ' ' || a[3] == '\t' || a[3] == '\n' || a[3] == 0 || a[3] == '\r')) return -1;
    for (int i = 0; i < 57; i++){
        if (strequals(code, opcodes[i])) return i;
    }
    return -1;
}

int arblen(char d, char* str){
    int offset = 0;
    while (1){
        if (str[offset] == d) return offset;
        else if (str[offset] == 0) return -1;
        offset++;
    }
}

int stringlen(char* str){
    return arblen(0, str);
}

int spacelen(char* str){
    return arblen(' ', str);
}

dbyte parseNum(char type, char* num){
    dbyte ans = 0;
    switch (type){
        case 'b':
            for (int i = 0; i < 16; i++){
                if (num[i] == '0'){
                    ans = ans << 1;
                } else if (num[i] == '1'){
                    ans = (ans << 1) + 1;
                } else {
                    return ans;
                }
            }
            return ans;
        case 'd':
            for (int i = 0; i < 5; i++){
                if (num[i] >= '0' && num[i] <= '9'){
                    ans = ans * 10 + num[i] - '0';
                } else {
                    return ans;
                }
            }
            return ans;
        case 'h':
            for (int i = 0; i < 4; i++){
                if (num[i] >= '0' && num[i] <= '9'){
                    ans = (ans << 4) + num[i] - '0';
                } else if (num[i] >= 'A' && num[i] <= 'F'){
                    ans = (ans << 4) + num[i] - 'A' + 10;
                } else if (num[i] >= 'a' && num[i] <= 'f'){
                    ans = (ans << 4) + num[i] - 'a' + 10;
                } else {
                    return ans;
                }
            }
            return ans;
    }
    return 0;
}

int findLabels(char* buff){
    byte foundInLine = 0;
    int numOfLabels = 0;
    int i = 0;
    while (buff[i] != 0){
        if (buff[i] == ':'){
            if (foundInLine){
                return -1;
            } else {
                foundInLine = 1;
                numOfLabels++;
            }
        } else if (buff[i] == '\n'){
            foundInLine = 0;
        }
        i++;
    }
    return numOfLabels;
}

int identifyLabel(char* l){
    int len = 1;
    while (l[len - 1] != ' ' && l[len - 1] != '\t' && l[len - 1] != '\n' && l[len - 1] != '\r' && l[len - 1] != 0 && l[len - 1] != ')') len++;
    char* lbl = (char*)malloc(len);
    for (int i = 0; i < len; i++){
        lbl[i] = l[i];
    }
    lbl[len - 1] = 0;
    for (int i = 0; i < labelLen; i++){
        if (strequals(lbl, labels[i])){
            free(lbl);
            return i;
        }
    }
    free(lbl);
    return -1;
}

int parseInstruction(int op, int ad, int arg, byte pass){
    for (int i = 0; i < 4; i++){
        instruct[i] = 0;
    }
    instruct[2] = arg % 256;
    instruct[3] = arg / 256;
    byte splitType = 0;
    unsigned long long splitData = 0;
    byte* splits = (byte*)&splitData;
    switch (op){
        case 0:
            splitType = 1;
            splitData = 0x7161797D6D756569;
            break;
        case 1:
            splitType = 1;
            splitData = 0x3121393D2D352529;
            break;
        case 2:
            splitType = 2;
            splitData = 0x1E0E16060A;
            break;
        case 3:
            splitType = 3;
            splitData = 0x90;
            break;
        case 4:
            splitType = 3;
            splitData = 0xB0;
            break;
        case 5:
            splitType = 3;
            splitData = 0xF0;
            break;
        case 6:
            splitType = 4;
            splitData = 0x2C24;
            break;
        case 7:
            splitType = 3;
            splitData = 0x30;
            break;
        case 8:
            splitType = 3;
            splitData = 0xD0;
            break;
        case 9:
            splitType = 3;
            splitData = 0x10;
            break;
        case 10:
            splitType = 5;
            splitData = 0;
            break;
        case 11:
            splitType = 3;
            splitData = 0x50;
            break;
        case 12:
            splitType = 3;
            splitData = 0x70;
            break;
        case 13:
            splitType = 5;
            splitData = 0x18;
            break;
        case 14:
            splitType = 5;
            splitData = 0xD8;
            break;
        case 15:
            splitType = 5;
            splitData = 0x58;
            break;
        case 16:
            splitType = 5;
            splitData = 0xB8;
            break;
        case 17:
            splitType = 1;
            splitData = 0xD1C1D9DDCDD5C5C9;
            break;
        case 18:
            splitType = 6;
            splitData = 0xECE4E0;
            break;
        case 19:
            splitType = 6;
            splitData = 0xCCC4C0;
            break;
        case 20:
            splitType = 7;
            splitData = 0xDECED6C6;
            break;
        case 21:
            splitType = 5;
            splitData = 0xCA;
            break;
        case 22:
            splitType = 5;
            splitData = 0x88;
            break;
        case 23:
            splitType = 1;
            splitData = 0x5141595D4D554549;
            break;
        case 24:
            splitType = 7;
            splitData = 0xFEEEF6E6;
            break;
        case 25:
            splitType = 5;
            splitData = 0xE8;
            break;
        case 26:
            splitType = 5;
            splitData = 0xC8;
            break;
        case 27:
            splitType = 8;
            splitData = 0x6C4C;
            break;
        case 28:
            splitType = 9;
            splitData = 0x20;
            break;
        case 29:
            splitType = 1;
            splitData = 0xB1A1B9BDADB5A5A9;
            break;
        case 30:
            splitType = 10;
            splitData = 0xBEAEB6A6A2;
            break;
        case 31:
            splitType = 11;
            splitData = 0xBCACB4A4A0;
            break;
        case 32:
            splitType = 2;
            splitData = 0x5E4E56464A;
            break;
        case 33:
            splitType = 5;
            splitData = 0xEA;
            break;
        case 34:
            splitType = 1;
            splitData = 0x1101191D0D150509;
            break;
        case 35:
            splitType = 5;
            splitData = 0x48;
            break;
        case 36:
            splitType = 5;
            splitData = 0x08;
            break;
        case 37:
            splitType = 5;
            splitData = 0x68;
            break;
        case 38:
            splitType = 5;
            splitData = 0x28;
            break;
        case 39:
            splitType = 2;
            splitData = 0x3E2E36262A;
            break;
        case 40:
            splitType = 2;
            splitData = 0x7E6E76666A;
            break;
        case 41:
            splitType = 5;
            splitData = 0x40;
            break;
        case 42:
            splitType = 5;
            splitData = 0x60;
            break;
        case 43:
            splitType = 1;
            splitData = 0xF1E1F9FDEDF5E5E9;
            break;
        case 44:
            splitType = 5;
            splitData = 0x38;
            break;
        case 45:
            splitType = 5;
            splitData = 0xF8;
            break;
        case 46:
            splitType = 5;
            splitData = 0x78;
            break;
        case 47:
            if (ad == 1) return -1;
            splitType = 1;
            splitData = 0x9181999D8D958500;
            break;
        case 48:
            if (ad == 1 || ad == 7) return -1;
            splitType = 10;
            splitData = 0x8E968600;
            break;
        case 49:
            if (ad == 1 || ad == 5) return -1;
            splitType = 11;
            splitData = 0x8C948400;
            break;
        case 50:
            splitType = 5;
            splitData = 0xAA;
            break;
        case 51:
            splitType = 5;
            splitData = 0xA8;
            break;
        case 52:
            splitType = 5;
            splitData = 0xBA;
            break;
        case 53:
            splitType = 5;
            splitData = 0x8A;
            break;
        case 54:
            splitType = 5;
            splitData = 0x9A;
            break;
        case 55:
            splitType = 5;
            splitData = 0x98;
            break;
        default: return -1;
    }
    switch (splitType){
        case 1:
            instruct[0] = 2;
            switch (ad){
                case 1:
                    instruct[1] = splits[0];
                    break;
                case 2:
                    instruct[1] = splits[1];
                    break;
                case 3:
                    instruct[0]++;
                    instruct[1] = splits[3];
                    break;
                case 4:
                    instruct[1] = splits[2];
                    break;
                case 5:
                    instruct[0]++;
                    instruct[1] = splits[4];
                    break;
                case 7:
                    instruct[0]++;
                    instruct[1] = splits[5];
                    break;
                case 9:
                    instruct[1] = splits[6];
                    break;
                case 10:
                    instruct[1] = splits[7];
                    break;
                default: return -1;
            }
            break;
        case 2:
            instruct[0] = 2;
            switch (ad){
                case 0:
                    instruct[0]--;
                    instruct[1] = splits[0];
                    break;
                case 2:
                    instruct[1] = splits[1];
                    break;
                case 3:
                    instruct[0]++;
                    instruct[1] = splits[3];
                    break;
                case 4:
                    instruct[1] = splits[2];
                    break;
                case 5:
                    instruct[0]++;
                    instruct[1] = splits[4];
                    break;
                default: return -1;
            }
            break;
        case 3:
            instruct[0] = 2;
            instruct[1] = (byte)splitData;
            if (pass == 0){
                instruct[2] = 0;
                break;
            }
            if (arg - pc - 2 < -128 || arg - pc - 2 > 127) return -1;
            instruct[2] = (char)(arg - pc - 2);
            break;
        case 4:
            switch (ad){
                case 2:
                    instruct[0] = 2;
                    instruct[1] = splits[0];
                    break;
                case 3:
                    instruct[0] = 3;
                    instruct[1] = splits[1];
                    break;
                default: return -1;
            }
            break;
        case 5:
            if (ad) return -1;
            instruct[0] = 1;
            instruct[1] = (byte)splitData;
            break;
        case 6:
            instruct[0] = 2;
            switch (ad){
                case 1:
                    instruct[1] = splits[0];
                    break;
                case 2:
                    instruct[1] = splits[1];
                    break;
                case 3:
                    instruct[0]++;
                    instruct[1] = splits[2];
                    break;
                default: return -1;
            }
            break;
        case 7:
            instruct[0] = 2;
            switch (ad){
                case 2:
                    instruct[1] = splits[0];
                    break;
                case 3:
                    instruct[0]++;
                    instruct[1] = splits[2];
                    break;
                case 4:
                    instruct[1] = splits[1];
                    break;
                case 5:
                    instruct[0]++;
                    instruct[1] = splits[3];
                    break;
                default: return -1;
            }
            break;
        case 8:
            instruct[0] = 3;
            switch (ad){
                case 3:
                    instruct[1] = splits[0];
                    break;
                case 8:
                    instruct[1] = splits[1];
                    break;
                default: return -1;
            }
            break;
        case 9:
            if (ad != 3) return -1;
            instruct[0] = 3;
            instruct[1] = (byte)splitData;
            break;
        case 10:
            instruct[0] = 2;
            switch (ad){
                case 1:
                    instruct[1] = splits[0];
                    break;
                case 2:
                    instruct[1] = splits[1];
                    break;
                case 3:
                    instruct[0]++;
                    instruct[1] = splits[3];
                    break;
                case 6:
                    instruct[1] = splits[2];
                    break;
                case 7:
                    instruct[0]++;
                    instruct[1] = splits[4];
                    break;
                default: return -1;
            }
            break;
        case 11:
            instruct[0] = 2;
            switch (ad){
                case 1:
                    instruct[1] = splits[0];
                    break;
                case 2:
                    instruct[1] = splits[1];
                    break;
                case 3:
                    instruct[0]++;
                    instruct[1] = splits[3];
                    break;
                case 4:
                    instruct[1] = splits[2];
                    break;
                case 5:
                    instruct[0]++;
                    instruct[1] = splits[4];
                    break;
                default: return -1;
            }
            break;

    }   
    return 0;
}

int parseLine(char* line, byte pass, byte* m){
    int off = 0;
    instruct[0] = 0;
    while (line[off] == ' ' || line[off] == '\t' || line[off] == '\r'){
        off++;
    }
    if (line[off] == '\n') return 1;
    if (line[off] == 0) return 2;
    int op = findOpcode(line + off);
    if (op == -1){
        if (sequenceEqu(line + off, "*=", 2)){
            off += 2;
            while (line[off] == ' ' || line[off] == '\t'){
                off++;
            }
            if (line[off] >= '0' && line[off] <= '9'){
                pc = parseNum('d', line + off);
            } else if (line[off] == '$'){
                pc = parseNum('h', line + off + 1);
            } else if (line[off] == '%'){
                pc = parseNum('b', line + off + 1);
            } else {
                return -1;
            }
            return 0;
        } else if (line[off] == ';'){
            return 0;
        } else {
            while (line[off] != ':' && line[off] != 0 && line[off] != '\n'){
                off++;
            }
            if (line[off] == ':'){
                char* str;
                int len = 0;
                off--;
                while (line[off] != ' ' && line[off] != '\t' && line[off] != '\n' && line[off] != '\r' && off >= 0){
                    len++;
                    off--;
                }
                str = (char*)malloc(len + 1);
                str[len] = 0;
                len = 0;
                off++;
                while (line[off] != ':'){
                    str[len] = line[off];
                    off++;
                    len++;
                }
                for (int i = 0; i < labelLen; i++){
                    if (strequals(str, labels[i])){
                        if (pass == 0) labelVals[i] = pc;
                        free(str);
                        return 0;
                    }
                }
                free(str);
                return -1;
            } else {
                return -1;
            }
        }
    } else if (op == 56){
        off += 3;
        dbyte tmp;
        byte longHex = 0;
        while (1){
            while (line[off] == ' ' || line[off] == '\t') off++;
            if (line[off] == '\n' || line[off] == 0) return 0;
            if (line[off] >= '0' && line[off] <= '9'){
                tmp = parseNum('d', line + off);
            } else if (line[off] == '$'){
                tmp = parseNum('h', line + off + 1);
                if (tmp < 256){
                    longHex = 1;
                    for (int i = 1; i < 4; i++){
                        if ((line[off + i] < 'A' || line[off + i] > 'F') && (line[off + i] < '0' || line[off + i] > '9') && (line[off + i] < 'a' || line[off + i] > 'f')) longHex = 0;
                    }
                }
            } else if (line[off] == '%'){
                tmp = parseNum('b', line + off + 1);
            } else {
                return -1;
            }
            if (tmp > 255 || longHex){
                if (pass == 1){
                    m[pc] = tmp % 256;
                    m[pc + 1] = tmp / 256;
                }
                pc += 2;
            } else {
                if (pass == 1) m[pc] = (byte)tmp;
                pc++;
            }
            while (line[off] != ' ' && line[off] != '\t' && line[off] != '\n' && line[off] != 0) off++;
        }
    } else {
        byte admode;
        dbyte args;
        off += 3;
        while (line[off] == ' ' || line[off] == '\t' || line[off] == '\r') off++;
        if (line[off] == ';' || line[off] == '\n' || line[off] == 0) admode = 0;
        else if (line[off] == '#'){
            admode = 1;
            off++;
            if (line[off] >= '0' && line[off] <= '9'){
                args = parseNum('d', line + off);
            } else if (line[off] == '$'){
                args = parseNum('h', line + off + 1);
            } else if (line[off] == '%'){
                args = parseNum('b', line + off + 1);
            } else {
                return -1;
            }
            if (args > 255){
                return -1;
            }
        } else if (line[off] == '('){
            byte foundP = 0;
            byte foundXY = 0;
            off++;
            while (line[off] == ' ' || line[off] == '\t') off++;
            if (line[off] >= '0' && line[off] <= '9'){
                args = parseNum('d', line + off);
            } else if (line[off] == '$'){
                args = parseNum('h', line + off + 1);
            } else if (line[off] == '%'){
                args = parseNum('b', line + off + 1);
            } else {
                int index = identifyLabel(line + off);
                if (index == -1) return -1;
                if (pass == 0) args = 0;
                else args = labelVals[index];
            }
            while (line[off] != '\n' && line[off] != 0){
                if (line[off] == ')') foundP++;
                else if (sequenceEqu(line + off, ",x", 2)) foundXY = 1;
                else if (sequenceEqu(line + off, ",y", 2)) foundXY = 2;
                off++;
            }
            if (!foundP) return -1;
            switch (foundXY){
                case 0:
                    admode = 8;
                    break;
                case 1:
                    if (args >= 256) return -1;
                    admode = 9;
                    break;
                case 2: 
                    if (args >= 256) return -1;
                    admode = 10;
                    break;
            }
            if (admode >= 9 && args > 255) return -1;
        } else {
            byte longHex = 0;
            int index = -1;
            if (line[off] >= '0' && line[off] <= '9'){
                args = parseNum('d', line + off);
            } else if (line[off] == '$'){
                args = parseNum('h', line + off + 1);
                if (args < 256){
                    longHex = 1;
                    for (int i = 1; i < 4; i++){
                        if ((line[off + i] < 'A' || line[off + i] > 'F') && (line[off + i] < '0' || line[off + i] > '9') && (line[off + i] < 'a' || line[off + i] > 'f')) longHex = 0;
                    }
                }
            } else if (line[off] == '%'){
                args = parseNum('b', line + off + 1);
            } else {
                index = identifyLabel(line + off);
                if (index == -1) return -1;
                else if (pass == 0) args = 0;
                else args = labelVals[index]; 
            }
            while (1){
                if (line[off] == '\n' || line[off] == 0){
                    if (args > 255 || longHex || index != -1){
                        admode = 3;
                    } else {
                        admode = 2;
                    }
                    break;
                } else if (sequenceEqu(line + off, ",x", 2)){
                    if (args > 256 || longHex || index != -1){
                        admode = 5;
                    } else {
                        admode = 4;
                    }
                    break;
                } else if (sequenceEqu(line + off, ",y", 2)){
                    if (args > 256 || longHex || index != -1){
                        admode = 7;
                    } else {
                        admode = 6;
                    }
                    break;
                }
                off++;
            }
        }
        int stat = parseInstruction(op, admode, args, pass);
        if (stat == -1){
            printf("%d %d %d\n", op, admode, args);
            return -1;
        }
    }
    return 0;
}

int setUpLabels(char* buff){
    labelLen = findLabels(buff);
    if (labelLen == -1) return -1;
    labels = (char**)malloc(labelLen * sizeof(char*));
    labelVals = (dbyte*)malloc(labelLen * sizeof(dbyte));
    int current = 0;
    int off = 0;
    int currentLen;
    int index;
    while (current < labelLen){
        while (buff[off] != ':'){
            off++;
        }
        off--;
        currentLen = 1;
        while (buff[off] != '\n' && buff[off] != ' ' && buff[off] != '\t' && buff[off] != '\r' && off >= 0){
            currentLen++;
            off--;
        }
        off++;
        labels[current] = (char*)malloc(currentLen);
        labels[current][currentLen - 1] = 0;
        index = 0;
        while (buff[off] != ':'){
            labels[current][index] = buff[off];
            off++;
            index++;
        }
        off++;
        current++;
    }
    return 0;
}

int assemble(char* code, byte* res){
    byte pass = 0;
    pc = 0;
    int off = 0;
    int status = 0;
    int nextN;
    int lines = 1;
    setUpLabels(code);
    while (1){
        status = parseLine(code + off, pass, res);
        //printf("LINE %d, %d\n", lines, status);
        if (status == -1){
            for (int i = 0; i < labelLen; i++){
                free(labels[i]);
            }
            free(labels);
            free(labelVals);
            return -1 * lines;
        }
        for (int i = 0; i < instruct[0]; i++){
            if (pass == 1) res[pc] = instruct[i + 1];
            pc++;
        }
        nextN = findNextNewline(code + off);
        if (nextN == -1){
            pass++;
            if (pass == 2){
                for (int i = 0; i < labelLen; i++){
                    free(labels[i]);
                }
                free(labels);
                free(labelVals);
                return 0;
            }
            pc = 0;
            off = 0;
            lines = 0;
        } else {
            off += nextN + 1;
        }
        lines++;
    }

}