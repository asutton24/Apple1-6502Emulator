;F0-FF are the board, D8-DF is the edited line, EA-EB is the strpointer, E8/E9 are randBytes, E4-E7 are score, E3 is the movement flag, E1-E2 is the offset vector 
*= $300
jmp start
dcb $8D $CE $C9 $C7 $C5 $C2 $A0 $CF $D4 $A0 $D9 $C5 $CB $A0 $D9 $CE $C1 $A0 $D3 $D3 $C5 $D2 $D0 $8D $B8 $B4 $B0 $B2 $8D
dcb $A0 $A0 $A0 $A0 $B2 $A0 $A0 $A0 $B4 $A0 $A0 $A0 $B8 $A0 $A0 $A0 $B6 $B1 $A0 $A0 $B2 $B3 $A0 $A0 $B4 $B6 $A0 $A0 $B8 $B2 $B1 $A0 $B6 $B5 $B2 $A0 $B2 $B1 $B5 $A0 $B4 $B2 $B0 $B1 $B8 $B4 $B0 $B2 $B6 $B9 $B0 $B4 $B2 $B9 $B1 $B8
dcb $0000 $0002 $0004 $0008 $0016 $0032 $0064 $0128 $0256 $0512 $1024 $2048 $4096 $8192
dcb $A0 $C5 $D2 $CF $C3 $D3 $8D $D2 $C5 $D6 $CF $A0 $C5 $CD $C1 $C7 $8D
strout:
	lda ($EA),y
	jsr $FFEF
	dey
	bpl strout
	rts
scoreUpdate:
	clc
	sed
	asl
	tax
	lda $358,x
	adc $E4
	sta $E4
	inx
	lda $358,x
	adc $E5
	sta $E5
	lda $E6
	adc #$0
	sta $E6
	lda $E7
	adc #$0
	sta $E7
	ldx #$0
	cld
	clc
	rts
boardOut:
	lda #$8D
	jsr $FFEF
	ldx #$0
	lda #$3
	sta $EB
boardLoop:
	lda #$20
	sta $EA
	lda $F0,x
	asl
	asl
	clc
	adc $EA
	sta $EA
	ldy #$3
	jsr strout
	lda #$A0
	jsr $FFEF
	txa
	and #$3
	cmp #$3
	bne skipn
	lda #$8D
	jsr $FFEF
skipn:
	inx
	cpx #$10
	bne boardLoop
	lda #$8D
	jmp $FFEF
	rts
movesLeft:
	lda #$F0
	sta $E1
	lda #$0
	sta $E2
	sta $ED
	lda #$F4
	sta $EC
	ldy #$0
horizontal:
	lda ($E1),y
	iny
	cmp ($E1),y
	bne noHori
	lda #$1
	rts
noHori:
	tya
	cmp #$F
	beq vertSet
	and #$3
	cmp #$3
	bne horizontal
	iny
	jmp horizontal
vertSet:
	ldy #$0
vertical:
	lda ($E1),y
	cmp ($EC),y
	bne noVert
	lda #$1
	rts
noVert:
	iny
	cpy #$C
	bne vertical
	lda #$0
	rts 
twoTiles:
	lda $E8
	lsr
	lsr
	lsr
	lsr
	tax
	lda #$1
	sta $F0,x
newTile:
	lda $E8
	and #$F
	tax
	lda $F0,x
	beq valid
	inc $E8
	jmp newTile
valid:
	lda $E9
	bne notFour
	lda #$2
	sta $F0,x
	rts
notFour:
	lda #$1
	sta $F0,x
	rts
collapse:
	lda #$3
	sta $EF
cCount:
	lda ($DE,x)
	bne c1
	lda ($DC,x)
	beq c1
	sta ($DE,x)
	lda #$0
	sta ($DC,x)
	lda #$01
	sta $E3
c1:
	lda ($DC,x)
	bne c2
	lda ($DA,x)
	beq c2
	sta ($DC,x)
	lda #$0
	sta ($DA,x)
	lda #$01
	sta $E3
c2:
	lda ($DA,x)
	bne c3
	lda ($D8,x)
	beq c3
	sta ($DA,x)
	lda #$0
	sta ($D8,x)
	lda #$01
	sta $E3
c3:
	dec $EF
	bne cCount
	rts
rowLogic:
	ldx #$0
	clc
	jsr collapse
	lda ($DC,x)
	beq p1
	cmp ($DE,x)
	bne p1
	clc
	adc #$01
	sta ($DE,x)
	jsr scoreUpdate
	lda #$0
	sta ($DC,x)
	lda #$1
	sta $E3
p1:
	lda ($DA,x)
	beq p2
	cmp ($DC,x)
	bne p2
	clc
	adc #$01
	sta ($DC,x)
	jsr scoreUpdate
	lda #$0
	sta ($DA,x)
	lda #$1
	sta $E3
p2:
	lda ($D8,x)
	beq p3
	cmp ($DA,x)
	bne p3
	clc
	adc #$01
	sta ($DA,x)
	jsr scoreUpdate
	lda #$0
	sta ($D8,x)
	lda #$1
	sta $E3
p3:
	jsr collapse
	rts
getkey:
	inc $E8
	inc $E9
	lda $E9
	cmp #$A
	bne notOverflow
	lda #$0
	sta $E9
notOverflow:
	lda $D011
	bpl getkey
	lda $D010
	rts
start:
	cld
	clc
	ldx #$FF
	txs
	ldx #$10
	lda #$0
	sta $E2
	sta $D9
	sta $DB
	sta $DD
	sta $DF
	sta $E4
	sta $E5
	sta $E6
	sta $E7
clearBoard:
	sta $F0,x
	dex
	bpl clearBoard
	lda #$3
	ldy #28
	sta $EA
	sta $EB
	jsr strout
	jsr getkey
	jsr twoTiles
gameLoop:
	lda #$0
	sta $E3
	jsr boardOut
invalidChar:
	jsr getkey
	cmp #$D7
	bne notW
	lda #$FC
	sta $D8
	lda #$F8
	sta $DA
	lda #$F4
	sta $DC
	lda #$F0
	sta $DE
	jsr rowLogic
	lda #$FD
	sta $D8
	lda #$F9
	sta $DA
	lda #$F5
	sta $DC
	lda #$F1
	sta $DE
	jsr rowLogic
	lda #$FE
	sta $D8
	lda #$FA
	sta $DA
	lda #$F6
	sta $DC
	lda #$F2
	sta $DE
	jsr rowLogic
	lda #$FF
	sta $D8
	lda #$FB
	sta $DA
	lda #$F7
	sta $DC
	lda #$F3
	sta $DE
	jsr rowLogic
	jmp doneMove
notW:
	cmp #$C1
	bne notA
	lda #$F3
	sta $D8
	lda #$F2
	sta $DA
	lda #$F1
	sta $DC
	lda #$F0
	sta $DE
	jsr rowLogic
	lda #$F7
	sta $D8
	lda #$F6
	sta $DA
	lda #$F5
	sta $DC
	lda #$F4
	sta $DE
	jsr rowLogic
	lda #$FB
	sta $D8
	lda #$FA
	sta $DA
	lda #$F9
	sta $DC
	lda #$F8
	sta $DE
	jsr rowLogic
	lda #$FF
	sta $D8
	lda #$FE
	sta $DA
	lda #$FD
	sta $DC
	lda #$FC
	sta $DE
	jsr rowLogic
	jmp doneMove
notA:
	cmp #$D3
	bne notS
	lda #$F0
	sta $D8
	lda #$F4
	sta $DA
	lda #$F8
	sta $DC
	lda #$FC
	sta $DE
	jsr rowLogic
	lda #$F1
	sta $D8
	lda #$F5
	sta $DA
	lda #$F9
	sta $DC
	lda #$FD
	sta $DE
	jsr rowLogic
	lda #$F2
	sta $D8
	lda #$F6
	sta $DA
	lda #$FA
	sta $DC
	lda #$FE
	sta $DE
	jsr rowLogic
	lda #$F3
	sta $D8
	lda #$F7
	sta $DA
	lda #$FB
	sta $DC
	lda #$FF
	sta $DE
	jsr rowLogic
	jmp doneMove
notS:
	cmp #$C4
	beq validChar
	jmp invalidChar
validChar:
	lda #$F0
	sta $D8
	lda #$F1
	sta $DA
	lda #$F2
	sta $DC
	lda #$F3
	sta $DE
	jsr rowLogic
	lda #$F4
	sta $D8
	lda #$F5
	sta $DA
	lda #$F6
	sta $DC
	lda #$F7
	sta $DE
	jsr rowLogic
	lda #$F8
	sta $D8
	lda #$F9
	sta $DA
	lda #$FA
	sta $DC
	lda #$FB
	sta $DE
	jsr rowLogic
	lda #$FC
	sta $D8
	lda #$FD
	sta $DA
	lda #$FE
	sta $DC
	lda #$FF
	sta $DE
	jsr rowLogic
doneMove:
	ldx #$0
checkLoop:
	lda $F0,x
	beq foundZero
	adc $E8
	sta $E8
	clc
	inx
	cpx #$10
	beq noZeros
	jmp checkLoop
foundZero:	
	lda $E3
	beq noMove
	jsr newTile
noMove:
	jmp gameLoop
noZeros:
	jsr movesLeft
	cmp #$1
	bne gameOver
	jmp gameLoop
gameOver:
	ldy #$10
	lda #$74
	sta $EA
	lda #$3
	sta $EB
	jsr strout
	ldx #$3
scoreOut:
	lda $E4,x
	jsr $FFDC
	dex
	bpl scoreOut
	lda #$8D
	jsr $FFEF
	jmp $FF1A
	
	
	

	
	
	
	