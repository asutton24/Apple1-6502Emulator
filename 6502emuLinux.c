#include <stdlib.h>
#include <ncurses.h>

typedef unsigned char byte;
typedef unsigned short int dbyte;

byte a;
byte x;
byte y;
byte flags = 32;
dbyte pgc;
byte mem[65536];
byte s = 0xFF;
byte column = 0;
byte cycles = 1;

void printb(byte b){
    if (b != 0 && b != 128 && b != 0x7F){
        if (b == 0x8D || b == 0xD){
            printw("\n");
            column = 0;
            mem[0xD012] = 0;
            return;
        }
        if (b > 128){
            printw("%c", b - 128);
        } else {
            printw("%c", b);
        }
        column++;
        if (column == 40){
         printw("\n");
        }
        napms(10);
    } 
    refresh();
    mem[0xD012] = 0;
}
void branch(byte b){
    cycles++;
    dbyte opc = pgc;
    int len;
    if (b <= 127){
        len = b;
    } else {
        len = (b & 127) - 128;
    }
    pgc += len;
    if (opc / 256 != pgc / 256){
        cycles++;
    }
}

void addFlags(byte p){
    if (flags & 8){
        byte d1 = (a/16)*10 + a%16;
        byte d2 = (p/16)*10 + p%16;
        byte dans = d1 + d2 + (flags & 1);
        if (dans > 99){
            flags = flags | 1;
            dans %= 100;
        } else {
            flags = flags & 254;
        }
        a = (dans/10) * 16 + (dans) % 10;
        if (((d1 & 128) && (d2 & 128) && !(dans & 128)) || (!(d1 & 128) && !(d2 & 128) && (dans & 128))){
            flags = flags | 64;
        } else {
            flags = flags & 191;
        }
        return;
    }
    byte ans = a + p + (flags & 1);
    if (((a & 128) && (p & 128) && !(ans & 128)) || (!(a & 128) && !(p & 128) && (ans & 128))){
        flags = flags | 64;
    } else {
        flags = flags & 191;
    }
    if (a + p + (flags & 1) > 255){
        flags = flags | 1;
    } else { 
        flags = flags & 254;
    }
    a = ans;
}

void subFlags(byte b){
    if (flags & 8){
        char d1 = (a/16)*10 + a%16;
        char d2 = (b/16)*10 + b%16;
        char dans = d1 - d2 - (1 - (flags & 1));
        if (dans >= 0){
            flags = flags | 1;
        } else{
            dans += 100;
            flags = flags & 254;
        }
        a = (dans / 10) * 16 + (dans % 10);
        if (((d1 & 128) && (d2 & 128) && !(dans & 128)) || (!(d1 & 128) && !(d2 & 128) && (dans & 128))){
            flags = flags | 64;
        } else {
            flags = flags & 191;
        }
        return;
    }
    addFlags(b ^ 255);
}

void cmpFlags(char c, byte b){
    byte *p;
    switch (c){
        case 'a':
            p = &a;
            break;
        case 'x':
            p = &x;
            break;
        case 'y':
            p = &y;
            break;
    }
    byte cm = *p - b;
    if (*p == b){
        flags = flags | 2;
    } else {
        flags = flags & 253;
    }
    if (*p >= b){
        flags = flags | 1;
    } else {
        flags = flags & 254;
    }
    if (cm & 128){
        flags = flags | 128;
    } else {
        flags = flags & 127;
    }
}

void ldFlags(char c){
    byte *p;
    switch (c){
        case 'a':
            p = &a;
            break;
        case 'x':
            p = &x;
            break;
        case 'y':
            p = &y;
            break;
    }
    if (*p == 0){
        flags = (flags | 2) & 127;
        return;
    } else {
        flags = flags & 253;
    }
    if (*p & 128){
        flags = flags | 128;
    } else {
        flags = flags & 127;
    }
}

int runcmd(){
    byte pointer;
    byte ans;
    dbyte add;
    dbyte nadd;
    switch (mem[pgc]){
        case 0xA9:
            pgc++;
            a = mem[pgc];
            cycles = 2;
            ldFlags('a');
            break;
        case 0xA5:
            pgc++;
            a = mem[mem[pgc]];
            cycles = 3;
            ldFlags('a');
            break;
        case 0xB5:
            pgc++;
            pointer = mem[pgc] + x;
            a = mem[pointer];
            cycles = 4;
            ldFlags('a');
            break;
        case 0xAD:
            a = mem[mem[pgc + 1] + mem[pgc + 2] * 256];
            pgc += 2;
            cycles = 4;
            ldFlags('a');
            break;
        case 0xBD:
            add = mem[pgc + 1] + mem[pgc + 2] * 256;
            pgc += 2;
            nadd = add + x;
            a = mem[nadd];
            if (nadd < add){
                cycles = 5;
            } else {
                cycles = 4 + ((nadd / 256) - (add / 256));
            }
            ldFlags('a');
            break;
        case 0xB9:
            add = mem[pgc + 1] + mem[pgc + 2] * 256;
            pgc += 2;
            nadd = add + y;
            a = mem[nadd];
            if (nadd < add){
                cycles = 5;
            } else {
                cycles = 4 + ((nadd / 256) - (add / 256));
            }
            ldFlags('a');
            break;
        case 0xA1:
            pgc++;
            pointer = mem[pgc] + x;
            add = mem[pointer] + 256 * mem[pointer + 1];
            a = mem[add];
            cycles = 6;
            ldFlags('a');
            break;
        case 0xB1:
            pgc++;
            pointer = mem[pgc];
            add = mem[pointer] + 256 * mem[pointer + 1];
            nadd = add + y;
            a = mem[nadd];
            if (nadd < add){
                cycles = 6;
            } else {
                cycles = 5 + ((nadd / 256) - (add / 256));
            }
            ldFlags('a');
            break;
        case 0xA2:
            pgc++;
            x = mem[pgc];
            cycles = 2;
            ldFlags('x');
            break;
        case 0xA6:
            pgc++;
            x = mem[mem[pgc]];
            cycles = 3;
            ldFlags('x');
            break;
        case 0xB6:
            pgc++;
            pointer = mem[pgc] + y;
            x = mem[pointer];
            cycles = 4;
            ldFlags('x');
            break;
        case 0xAE:
            x = mem[mem[pgc + 1] + mem[pgc + 2] * 256];
            pgc += 2;
            cycles = 4;
            ldFlags('x');
            break;
        case 0xBE:
            add = mem[pgc + 1] + mem[pgc + 2] * 256;
            pgc += 2;
            nadd = add + y;
            x = mem[nadd];
            if (nadd < add){
                cycles = 5;
            } else {
                cycles = 4 + ((nadd / 256) - (add / 256));
            }
            ldFlags('x');
            break;
        case 0xA0:
            pgc++;
            y = mem[pgc];
            cycles = 2;
            ldFlags('y');
            break;
        case 0xA4:
            pgc++;
            y = mem[mem[pgc]];
            cycles = 3;
            ldFlags('y');
            break;
        case 0xB4:
            pgc++;
            pointer = mem[pgc] + x;
            y = mem[pointer];
            cycles = 4;
            ldFlags('y');
            break;
        case 0xAC:
            y = mem[mem[pgc + 1] + mem[pgc + 2] * 256];
            pgc += 2;
            cycles = 4;
            ldFlags('y');
            break;
        case 0xBC:
            add = mem[pgc + 1] + mem[pgc + 2] * 256;
            pgc += 2;
            nadd = add + x;
            y = mem[nadd];
            if (nadd < add){
                cycles = 5;
            } else {
                cycles = 4 + ((nadd / 256) - (add / 256));
            }
            ldFlags('y');
            break;
        case 0x85:
            pgc++;
            mem[mem[pgc]] = a;
            cycles = 3;
            break;
        case 0x95:
            pgc++;
            pointer = mem[pgc] + x;
            mem[pointer] = a;
            cycles = 4;
            break;
        case 0x8D:
            mem[mem[pgc + 1] + 256 * mem[pgc + 2]] = a;
            cycles = 4;
            pgc += 2;
            break;
        case 0x9D:
            mem[x + mem[pgc + 1] + 256 * mem[pgc + 2]] = a;
            cycles = 5;
            pgc += 2;
            break;
        case 0x99:
            mem[y + mem[pgc + 1] + 256 * mem[pgc + 2]] = a;
            cycles = 5;
            pgc += 2;
            break;
        case 0x81:
            pgc++;
            pointer = mem[pgc] + x;
            mem[mem[pointer] + 256 * mem[pointer + 1]] = a;
            cycles = 6;
            break;
        case 0x91:
            pgc++;
            pointer = mem[pgc];
            add = mem[pointer] + 256 * mem[pointer + 1];
            nadd = add + y;
            mem[nadd] = a;
            cycles = 6;
            break;
        case 0x86:
            pgc++;
            mem[mem[pgc]] = x;
            cycles = 3;
            break;
        case 0x96:
            pgc++;
            pointer = mem[pgc] + y;
            mem[pointer] = x;
            cycles = 4;
            break;
        case 0x8E:
            mem[mem[pgc + 1] + 256 * mem[pgc + 2]] = x;
            cycles = 4;
            pgc += 2;
            break;
        case 0x84:
            pgc++;
            mem[mem[pgc]] = y;
            cycles = 3;
            break;
        case 0x94:
            pgc++;
            pointer = mem[pgc] + x;
            mem[pointer] = y;
            cycles = 4;
            break;
        case 0x8C:
            mem[mem[pgc + 1] + 256 * mem[pgc + 2]] = y;
            cycles = 4;
            pgc += 2;
            break;
        case 0xAA:
            x = a;
            ldFlags('x');
            cycles = 2;
            break;
        case 0xA8:
            y = a;
            ldFlags('y');
            cycles = 2;
            break;
        case 0x8A:
            a = x;
            ldFlags('a');
            cycles = 2;
            break;
        case 0x98:
            a = y;
            ldFlags('a');
            cycles = 2;
            break;
        case 0xBA:
            x = s;
            ldFlags('x');
            cycles = 2;
            break;
        case 0x9A:
            s = x;
            cycles = 2;
            break;
        case 0x48:
            mem[256 + s] = a;
            s--;
            cycles = 3;
            break;
        case 0x08:
            mem[256 + s] = flags;
            s--;
            cycles = 3;
            break;
        case 0x68:
            s++;
            a = mem[256 + s];
            cycles = 4;
            ldFlags('a');
            break;
        case 0x28:
            s++;
            flags = mem[256 + s];
            cycles = 4;
            break;
        case 0x29:
            pgc++;
            a = a & mem[pgc];
            cycles = 2;
            ldFlags('a');
            break;
        case 0x25:
            pgc++;
            a = a & mem[mem[pgc]];
            cycles = 3;
            ldFlags('a');
            break;
        case 0x35:
            pgc++;
            pointer = mem[pgc] + x;
            a = a & mem[pointer];
            cycles = 4;
            ldFlags('a');
            break;
        case 0x2D:
            a = a & mem[mem[pgc + 1] + 256 * mem[pgc + 2]];
            pgc += 2;
            cycles = 4;
            ldFlags('a');
            break;
        case 0x3D:
            add = mem[pgc + 1] + mem[pgc + 2] * 256;
            pgc += 2;
            nadd = add + x;
            a = a & mem[nadd];
            if (nadd < add){
                cycles = 5;
            } else {
                cycles = 4 + ((nadd / 256) - (add / 256));
            }
            ldFlags('a');
            break;
        case 0x39:
            add = mem[pgc + 1] + mem[pgc + 2] * 256;
            pgc += 2;
            nadd = add + y;
            a = a & mem[nadd];
            if (nadd < add){
                cycles = 5;
            } else {
                cycles = 4 + ((nadd / 256) - (add / 256));
            }
            ldFlags('a');
            break;
        case 0x21:
            pgc++;
            pointer = mem[pgc] + x;
            add = mem[pointer] + 256 * mem[pointer + 1];
            a = a & mem[add];
            cycles = 6;
            ldFlags('a');
            break;
        case 0x31:
            pgc++;
            pointer = mem[pgc];
            add = mem[pointer] + 256 * mem[pointer + 1];
            nadd = add + y;
            a = a & mem[nadd];
            if (nadd < add){
                cycles = 6;
            } else {
                cycles = 5 + ((nadd / 256) - (add / 256));
            }
            ldFlags('a');
            break;
        case 0x49:
            pgc++;
            a = a ^ mem[pgc];
            cycles = 2;
            ldFlags('a');
            break;
        case 0x45:
            pgc++;
            a = a ^ mem[mem[pgc]];
            cycles = 3;
            ldFlags('a');
            break;
        case 0x55:
            pgc++;
            pointer = mem[pgc] + x;
            a = a ^ mem[pointer];
            cycles = 4;
            ldFlags('a');
            break;
        case 0x4D:
            a = a ^ mem[mem[pgc + 1] + 256 * mem[pgc + 2]];
            pgc += 2;
            cycles = 4;
            ldFlags('a');
            break;
        case 0x5D:
            add = mem[pgc + 1] + mem[pgc + 2] * 256;
            pgc += 2;
            nadd = add + x;
            a = a ^ mem[nadd];
            if (nadd < add){
                cycles = 5;
            } else {
                cycles = 4 + ((nadd / 256) - (add / 256));
            }
            ldFlags('a');
            break;
        case 0x59:
            add = mem[pgc + 1] + mem[pgc + 2] * 256;
            pgc += 2;
            nadd = add + y;
            a = a ^ mem[nadd];
            if (nadd < add){
                cycles = 5;
            } else {
                cycles = 4 + ((nadd / 256) - (add / 256));
            }
            ldFlags('a');
            break;
        case 0x41:
            pgc++;
            pointer = mem[pgc] + x;
            add = mem[pointer] + 256 * mem[pointer + 1];
            a = a ^ mem[add];
            cycles = 6;
            ldFlags('a');
            break;
        case 0x51:
            pgc++;
            pointer = mem[pgc];
            add = mem[pointer] + 256 * mem[pointer + 1];
            nadd = add + y;
            a = a ^ mem[nadd];
            if (nadd < add){
                cycles = 6;
            } else {
                cycles = 5 + ((nadd / 256) - (add / 256));
            }
            ldFlags('a');
            break;
        case 0x09:
            pgc++;
            a = a | mem[pgc];
            cycles = 2;
            ldFlags('a');
            break;
        case 0x05:
            pgc++;
            a = a | mem[mem[pgc]];
            cycles = 3;
            ldFlags('a');
            break;
        case 0x15:
            pgc++;
            pointer = mem[pgc] + x;
            a = a | mem[pointer];
            cycles = 4;
            ldFlags('a');
            break;
        case 0x0D:
            a = a | mem[mem[pgc + 1] + 256 * mem[pgc + 2]];
            pgc += 2;
            cycles = 4;
            ldFlags('a');
            break;
        case 0x1D:
            add = mem[pgc + 1] + mem[pgc + 2] * 256;
            pgc += 2;
            nadd = add + x;
            a = a | mem[nadd];
            if (nadd < add){
                cycles = 5;
            } else {
                cycles = 4 + ((nadd / 256) - (add / 256));
            }
            ldFlags('a');
            break;
        case 0x19:
            add = mem[pgc + 1] + mem[pgc + 2] * 256;
            pgc += 2;
            nadd = add + y;
            a = a | mem[nadd];
            if (nadd < add){
                cycles = 5;
            } else {
                cycles = 4 + ((nadd / 256) - (add / 256));
            }
            ldFlags('a');
            break;
        case 0x01:
            pgc++;
            pointer = mem[pgc] + x;
            add = mem[pointer] + 256 * mem[pointer + 1];
            a = a | mem[add];
            cycles = 6;
            ldFlags('a');
            break;
        case 0x11:
            pgc++;
            pointer = mem[pgc];
            add = mem[pointer] + 256 * mem[pointer + 1];
            nadd = add + y;
            a = a | mem[nadd];
            if (nadd < add){
                cycles = 6;
            } else {
                cycles = 5 + ((nadd / 256) - (add / 256));
            }
            ldFlags('a');
            break;
        case 0x24:
            pgc++;
            flags = (flags & 63) | (mem[mem[pgc]] & 192);
            if (a & mem[mem[pgc]] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            cycles = 3;
            break;
        case 0x2C:
            add = mem[pgc + 1] + 256 * mem[pgc + 2];
            flags = (flags & 63) | (mem[add] & 192);
            if (a & mem[add] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            pgc += 2;
            cycles = 4;
            break;
        case 0x69:
            pgc++;
            addFlags(mem[pgc]);
            cycles = 2;
            ldFlags('a');
            break;
        case 0x65:
            pgc++;
            addFlags(mem[mem[pgc]]);
            cycles = 3;
            ldFlags('a');
            break;
        case 0x75:
            pgc++;
            pointer = mem[pgc] + x;
            addFlags(mem[pointer]);
            cycles = 4;
            ldFlags('a');
            break;
        case 0x6D:
            addFlags(mem[mem[pgc + 1] + 256 * mem[pgc + 2]]);
            pgc += 2;
            cycles = 4;
            ldFlags('a');
            break;
        case 0x7D:
            add = mem[pgc + 1] + 256 * mem[pgc + 2];
            nadd = add + x;
            addFlags(mem[nadd]);
            if (nadd < add){
                cycles = 5;
            } else {
                cycles = 4 + ((nadd / 256) - (add / 256));
            }
            pgc += 2;
            ldFlags('a');
            break;
        case 0x79:
            add = mem[pgc + 1] + 256 * mem[pgc + 2];
            nadd = add + y;
            addFlags(mem[nadd]);
            if (nadd < add){
                cycles = 5;
            } else {
                cycles = 4 + ((nadd / 256) - (add / 256));
            }
            pgc += 2;
            ldFlags('a');
            break;
        case 0x61:
            pgc++;
            pointer = mem[pgc] + x;
            add = mem[pointer] + 256 * mem[pointer + 1];
            addFlags(mem[add]);
            cycles = 6;
            ldFlags('a');
            break;
        case 0x71:
            pgc++;
            pointer = mem[pgc];
            add = mem[pointer] + 256 * mem[pointer + 1];
            nadd = add + y;
            addFlags(mem[nadd]);
            if (nadd < add){
                cycles = 6;
            } else {
                cycles = 5 + ((nadd / 256) - (add / 256));
            }
            ldFlags('a');
            break;
        case 0xE9:
            pgc++;
            subFlags(mem[pgc]);
            cycles = 2;
            ldFlags('a');
            break;
        case 0xE5:
            pgc++;
            subFlags(mem[mem[pgc]]);
            cycles = 3;
            ldFlags('a');
            break;
        case 0xF5:
            pgc++;
            pointer = mem[pgc] + x;
            subFlags(mem[pointer]);
            cycles = 4;
            ldFlags('a');
            break;
        case 0xED:
            subFlags(mem[mem[pgc + 1] + 256 * mem[pgc + 2]]);
            pgc += 2;
            cycles = 4;
            ldFlags('a');
            break;
        case 0xFD:
            add = mem[pgc + 1] + 256 * mem[pgc + 2];
            nadd = add + x;
            subFlags(mem[nadd]);
            if (nadd < add){
                cycles = 5;
            } else {
                cycles = 4 + ((nadd / 256) - (add / 256));
            }
            pgc += 2;
            ldFlags('a');
            break;
        case 0xF9:
            add = mem[pgc + 1] + 256 * mem[pgc + 2];
            nadd = add + y;
            subFlags(mem[nadd]);
            if (nadd < add){
                cycles = 5;
            } else {
                cycles = 4 + ((nadd / 256) - (add / 256));
            }
            pgc += 2;
            ldFlags('a');
            break;
        case 0xE1:
            pgc++;
            pointer = mem[pgc] + x;
            add = mem[pointer] + 256 * mem[pointer + 1];
            subFlags(mem[add]);
            cycles = 6;
            ldFlags('a');
            break;
        case 0xF1:
            pgc++;
            pointer = mem[pgc];
            add = mem[pointer] + 256 * mem[pointer + 1];
            nadd = add + y;
            subFlags(mem[nadd]);
            if (nadd < add){
                cycles = 6;
            } else {
                cycles = 5 + ((nadd / 256) - (add / 256));
            }
            ldFlags('a');
            break;
        case 0xC9:
            pgc++;
            cmpFlags('a', mem[pgc]);
            cycles = 2;
            break;
        case 0xC5:
            pgc++;
            cmpFlags('a', mem[mem[pgc]]);
            cycles = 3;
            break;
        case 0xD5:
            pgc++;
            pointer = mem[pgc] + x;
            cmpFlags('a', mem[pointer]);
            cycles = 4;
            break;
        case 0xCD:
            cmpFlags('a', mem[mem[pgc + 1] + 256 * mem[pgc + 2]]);
            pgc += 2;
            cycles = 4;
            break;
        case 0xDD:
            add = mem[pgc + 1] + 256 * mem[pgc + 2];
            nadd = add + x;
            cmpFlags('a', mem[nadd]);
            if (nadd < add){
                cycles = 5;
            } else {
                cycles = 4 + ((nadd / 256) - (add / 256));
            }
            pgc += 2;
            break;
        case 0xD9:
            add = mem[pgc + 1] + 256 * mem[pgc + 2];
            nadd = add + y;
            cmpFlags('a', mem[nadd]);
            if (nadd < add){
                cycles = 5;
            } else {
                cycles = 4 + ((nadd / 256) - (add / 256));
            }
            pgc += 2;
            break;
        case 0xC1:
            pgc++;
            pointer = mem[pgc] + x;
            add = mem[pointer] + 256 * mem[pointer + 1];
            cmpFlags('a', mem[add]);
            cycles = 6;
            break;
        case 0xD1:
            pgc++;
            pointer = mem[pgc];
            add = mem[pointer] + 256 * mem[pointer + 1];
            nadd = add + y;
            cmpFlags('a', mem[nadd]);
            if (nadd < add){
                cycles = 6;
            } else {
                cycles = 5 + ((nadd / 256) - (add / 256));
            }
            break;
        case 0xE0:
            pgc++;
            cmpFlags('x', mem[pgc]);
            cycles = 2;
            break;
        case 0xE4:
            pgc++;
            cmpFlags('x', mem[mem[pgc]]);
            cycles = 3;
            break;
        case 0xEC:
            cmpFlags('x', mem[mem[pgc + 1] + 256 * mem[pgc + 2]]);
            pgc += 2;
            cycles = 4;
            break;
        case 0xC0:
            pgc++;
            cmpFlags('y', mem[pgc]);
            cycles = 2;
            break;
        case 0xC4:
            pgc++;
            cmpFlags('y', mem[mem[pgc]]);
            cycles = 3;
            break;
        case 0xCC:
            cmpFlags('y', mem[mem[pgc + 1] + 256 * mem[pgc + 2]]);
            pgc += 2;
            cycles = 4;
            break;
        case 0xE6:
            pgc++;
            mem[mem[pgc]]++;
            if (mem[mem[pgc]] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[mem[pgc]] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 5;
            break;
        case 0xF6:
            pgc++;
            pointer = mem[pgc] + x;
            mem[pointer]++;
            if (mem[pointer] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[pointer] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 6;
            break;
        case 0xEE:
            add = mem[pgc + 1] + 256 * mem[pgc + 2];
            mem[add]++;
            if (mem[add] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[add] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 6;
            break;
        case 0xFE:
            add = x + mem[pgc + 1] + 256 * mem[pgc + 2];
            mem[add]++;
            if (mem[add] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[add] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 7;
            break;
        case 0xE8:
            x++;
            ldFlags('x');
            cycles = 2;
            break;
        case 0xC8:
            y++;
            ldFlags('y');
            cycles = 2;
            break;
        case 0xC6:
            pgc++;
            mem[mem[pgc]]--;
            if (mem[mem[pgc]] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[mem[pgc]] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 5;
            break;
        case 0xD6:
            pgc++;
            pointer = mem[pgc] + x;
            mem[pointer]--;
            if (mem[pointer] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[pointer] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 6;
            break;
        case 0xCE:
            add = mem[pgc + 1] + 256 * mem[pgc + 2];
            mem[add]--;
            if (mem[add] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[add] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 6;
            break;
        case 0xDE:
            add = x + mem[pgc + 1] + 256 * mem[pgc + 2];
            mem[add]--;
            if (mem[add] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[add] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 7;
            break;
        case 0xCA:
            x--;
            ldFlags('x');
            cycles = 2;
            break;
        case 0x88:
            y--;
            ldFlags('y');
            cycles = 2;
            break;
        case 0x0A:
            if (a & 128){
                flags = flags | 1;
            } else {
                flags = flags & 254;
            }
            a = a<<1;
            ldFlags('a');
            cycles = 2;
            break;
        case 0x06:
            pgc++;
            if (mem[mem[pgc]] & 128){
                flags = flags | 1;
            } else {
                flags = flags & 254;
            }
            mem[mem[pgc]] = mem[mem[pgc]]<<1;
            if (mem[mem[pgc]] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[mem[pgc]] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 5;
            break;
        case 0x16:
            pgc++;
            pointer = mem[pgc] + x;
            if (mem[pointer] & 128){
                flags = flags | 1;
            } else {
                flags = flags & 254;
            }
            mem[pointer] = mem[pointer]<<1;
            if (mem[pointer] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[pointer] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 6;
            break;
        case 0x0E:
            add = mem[pgc + 1] + 256 * mem[pgc + 2];
            pgc += 2;
            if (mem[add] & 128){
                flags = flags | 1;
            } else {
                flags = flags & 254;
            }
            mem[add] = mem[add]<<1;
            if (mem[add] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[add] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 6;
            break;
        case 0x1E:
            add = x + mem[pgc + 1] + 256 * mem[pgc + 2];
            pgc += 2;
            if (mem[add] & 128){
                flags = flags | 1;
            } else {
                flags = flags & 254;
            }
            mem[add] = mem[add]<<1;
            if (mem[add] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[add] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 7;
            break;
        case 0x4A:
            if (a & 1){
                flags = flags | 1;
            } else {
                flags = flags & 254;
            }
            a = a>>1;
            ldFlags('a');
            cycles = 2;
            break;
        case 0x46:
            pgc++;
            if (mem[mem[pgc]] & 1){
                flags = flags | 1;
            } else {
                flags = flags & 254;
            }
            mem[mem[pgc]] = mem[mem[pgc]]>>1;
            if (mem[mem[pgc]] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[mem[pgc]] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 5;
            break;
        case 0x56:
            pgc++;
            pointer = mem[pgc] + x;
            if (mem[pointer] & 1){
                flags = flags | 1;
            } else {
                flags = flags & 254;
            }
            mem[pointer] = mem[pointer]>>1;
            if (mem[pointer] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[pointer] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 6;
            break;
        case 0x4E:
            add = mem[pgc + 1] + 256 * mem[pgc + 2];
            pgc += 2;
            if (mem[add] & 1){
                flags = flags | 1;
            } else {
                flags = flags & 254;
            }
            mem[add] = mem[add]>>1;
            if (mem[add] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[add] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 6;
            break;
        case 0x5E:
            add = x + mem[pgc + 1] + 256 * mem[pgc + 2];
            pgc += 2;
            if (mem[add] & 1){
                flags = flags | 1;
            } else {
                flags = flags & 254;
            }
            mem[add] = mem[add]>>1;
            if (mem[add] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[add] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 7;
            break;
        case 0x2A:
            ans = flags & 1;
            if (a & 128){
                flags = flags | 1;
            } else {
                flags = flags & 254;
            }
            a = (a<<1) + ans;
            ldFlags('a');
            cycles = 2;
            break;
        case 0x26:
            pgc++;
            ans = flags & 1;
            if (mem[mem[pgc]] & 128){
                flags = flags | 1;
            } else {
                flags = flags & 254;
            }
            mem[mem[pgc]] = (mem[mem[pgc]]<<1) + ans;
            if (mem[mem[pgc]] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[mem[pgc]] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 5;
            break;
        case 0x36:
            ans = flags & 1;
            pgc++;
            pointer = mem[pgc] + x;
            if (mem[pointer] & 128){
                flags = flags | 1;
            } else {
                flags = flags & 254;
            }
            mem[pointer] = (mem[pointer]<<1) + ans;
            if (mem[pointer] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[pointer] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 6;
            break;
        case 0x2E:
            ans = flags & 1;
            add = mem[pgc + 1] + 256 * mem[pgc + 2];
            pgc += 2;
            if (mem[add] & 128){
                flags = flags | 1;
            } else {
                flags = flags & 254;
            }
            mem[add] = (mem[add]<<1) + ans;
            if (mem[add] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[add] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 6;
            break;
        case 0x3E:
            ans = flags & 1;
            add = x + mem[pgc + 1] + 256 * mem[pgc + 2];
            pgc += 2;
            if (mem[add] & 128){
                flags = flags | 1;
            } else {
                flags = flags & 254;
            }
            mem[add] = (mem[add]<<1) + ans;
            if (mem[add] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[add] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 7;
            break;
        case 0x6A:
            ans = 128 * (flags & 1);
            if (a & 1){
                flags = flags | 1;
            } else {
                flags = flags & 254;
            }
            a = (a>>1) + ans;
            ldFlags('a');
            cycles = 2;
            break;
        case 0x66:
            ans = 128 * (flags & 1);
            pgc++;
            if (mem[mem[pgc]] & 1){
                flags = flags | 1;
            } else {
                flags = flags & 254;
            }
            mem[mem[pgc]] = (mem[mem[pgc]]>>1) + ans;
            if (mem[mem[pgc]] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[mem[pgc]] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 5;
            break;
        case 0x76:
            ans = 128 * (flags & 1);
            pgc++;
            pointer = mem[pgc] + x;
            if (mem[pointer] & 1){
                flags = flags | 1;
            } else {
                flags = flags & 254;
            }
            mem[pointer] = (mem[pointer]>>1) + ans;
            if (mem[pointer] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[pointer] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 6;
            break;
        case 0x6E:
            ans = 128 * (flags & 1);
            add = mem[pgc + 1] + 256 * mem[pgc + 2];
            pgc += 2;
            if (mem[add] & 1){
                flags = flags | 1;
            } else {
                flags = flags & 254;
            }
            mem[add] = (mem[add]>>1) + ans;
            if (mem[add] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[add] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 6;
            break;
        case 0x7E:
            ans = 128 * (flags & 1);
            add = x + mem[pgc + 1] + 256 * mem[pgc + 2];
            pgc += 2;
            if (mem[add] & 1){
                flags = flags | 1;
            } else {
                flags = flags & 254;
            }
            mem[add] = (mem[add]>>1) + ans;
            if (mem[add] == 0){
                flags = flags | 2;
            } else {
                flags = flags & 253;
            }
            if (mem[add] & 128){
                flags = flags | 128;
            } else {
                flags = flags & 127;
            }
            cycles = 7;
            break;
        case 0x4C:
            pgc = mem[pgc + 1] + 256 * mem[pgc + 2] - 1;
            cycles = 3;
            break;
        case 0x6C:
            add = mem[pgc + 1] + 256 * mem[pgc + 2];
            pgc = mem[add] + 256 * mem[add + 1] - 1;
            cycles = 5;
            break;
        case 0x20:
            mem[256 + s - 1] = (pgc + 2) % 256;
            mem[256 + s] = (pgc + 2) / 256;
            s -= 2;
            pgc = mem[pgc + 1] + 256 * mem[pgc + 2] - 1;
            cycles = 6;
            break;
        case 0x60:
            pgc = mem[256 + s + 1] + 256 * mem[256 + s + 2];
            s += 2;
            cycles = 6;
            break;
        case 0x90:
            pgc++;
            cycles = 2;
            if (!(flags & 1)){
                branch(mem[pgc]);
            }
            break;
        case 0xB0:
            pgc++;
            cycles = 2;
            if (flags & 1){
                branch(mem[pgc]);
            }
            break;
        case 0xF0:
            pgc++;
            cycles = 2;
            if (flags & 2){
                branch(mem[pgc]);
            }
            break;
        case 0x30:
            pgc++;
            cycles = 2;
            if (flags & 128){
                branch(mem[pgc]);
            }
            break;
        case 0xD0:
            pgc++;
            cycles = 2;
            if (!(flags & 2)){
                branch(mem[pgc]);
            }
            break;
        case 0x10:
            pgc++;
            cycles = 2;
            if (!(flags & 128)){
                branch(mem[pgc]);
            }
            break;
        case 0x50:
            pgc++;
            cycles = 2;
            if (!(flags & 64)){
                branch(mem[pgc]);
            }
            break;
        case 0x70:
            pgc++;
            cycles = 2;
            if (flags & 64){
                branch(mem[pgc]);
            }
            break;
        case 0x18:
            flags = flags & 254;
            cycles = 2;
            break;
        case 0xD8:
            flags = flags & 247;
            cycles = 2;
            break;
        case 0x58:
            flags = flags & 251;
            cycles = 2;
            break;
        case 0xB8:
            flags = flags & 191;
            cycles = 2;
            break;
        case 0x38:
            flags = flags | 1;
            cycles = 2;
            break;
        case 0xF8:
            flags = flags | 8;
            cycles = 2;
            break;
        case 0x78:
            flags = flags | 4;
            cycles = 2;
            break;
        case 0x00:
            cycles = 7;
            flags = flags | 16;
            break;
        case 0xEA:
            cycles = 2;
            break;
        case 0x40:
            flags = mem[256 + s + 1];
            pgc = mem[256 + s + 2] + 256 * mem[256 + s + 3];
            s += 3;
            cycles = 6;
            break;
    }
    pgc++;
    return 0;
}

int toHex(char* c){
    int ans = 0;
    int pow = 4096;
    for (int i = 0; i < 4; i++){
        if (c[i] >= '0' && c[i] <= '9'){
            ans += (c[i] - '0') * pow;
        } else if (c[i] >= 'A' && c[i] <= 'F'){
            ans += (c[i] - 'A' + 10) * pow;
        } else if (c[i] >= 'a' && c[i] <= 'f'){
            ans += (c[i] - 'f' + 10) * pow;
        } else {
            return -1;
        }
        pow /= 16;
    }
    return ans;
}

int main(int argc, char** argv){
    byte* wozmon = (byte*)malloc(256);
    byte* basic = (byte*)malloc(4096);
    byte* buffer = (byte*)malloc(8192);
    FILE *file;
    int low;
    int high;
    char in;
    if (argc > 1){
        if ((argc - 1) % 3 != 0){
            printf("Arguement error\n");
            return 0;
        }
        for (int i = 1; i < argc; i += 3){
            file = fopen(argv[i], "rb");
            if (file == NULL){
                printf("%s not found\n", argv[i + 1]);
                return 0;
            }
            low = toHex(argv[i + 1]);
            high = toHex(argv[i + 2]);
            if (high <= low || low == -1 || high == -1){
                printf("Hex parsing error\n");
                return 0;
            }
            fread(buffer, 1, 8192, file);
            for (int i = 0; i <= high - low; i++){
                mem[low + i] = buffer[i];
            }
        }
    }
    file = fopen("monitor.rom", "rb");
    if (file == NULL){
        printf("Monitor not found\n");
        return 0;
    }
    fread(wozmon, 1, 256, file);
    for (int i = 0; i < 256 ; i++){
        mem[0xFF00 + i] = wozmon[i];
    }
    file = fopen("basic.rom", "rb");
    if (file == NULL){
        printf("Basic not found\n");
        return 0;
    }
    fread(basic, 1, 4096, file);
    for (int i = 0; i < 4096; i++){
        mem[0xE000 + i] = basic[i];
    }
    pgc = 0xFF00;
    byte pulse = 0;
    char lastChar = 0;
    byte hitLast = 0;
    free(wozmon);
    free(basic);
    free(buffer);
    initscr();
    cbreak();
    noecho();
    scrollok(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    while (1){
        runcmd();
        pulse += cycles;
        in = getch();
        if (in == 10){
            in = 13;
        }
        if (in != -1){
            if (!hitLast){
                mem[0xD010] = in + 128;
                mem[0xd011] = 128; 
                if (mem[0xD010] - 128 == 27){
                    pgc = 0xFF00;
                }
                pulse = 0;
                hitLast = 1;
            }
        } else {
            hitLast = 0;
        }
        if (pulse > 20){
            mem[0xd011] = 0;
        }
        if (flags & 16){
            pgc = 0xFF00;
            flags = flags & (255 - 16);
        }
        printb(mem[0xD012]);
    }
    printf("a: %.02x\nx: %.02x\ny:%.02x\nf:%.02x\n", a, x, y, flags);
    endwin();
    return 0;
}
