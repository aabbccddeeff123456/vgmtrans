# Advanced Real-time Dynamic Interplay:

## SNES Commands

```9x yy zz aa ..``` - note on (aa is note duration in MIDI length format)  
```ax ??``` - unused  
```bx yy zz``` - controllers
- special cases when bx is not being transfer to SPC:
- yy = 0x74 - loop start
- yy = 0x75 - loop end
- yy = 0x78 - goto subroutine, zz is the id of subroutines
- yy = 0x79 - return from subroutine

- subroutines table are listed on the song header as an indirect address
- subroutine commands is only used in two music as far as I know
- >"Spacestation Clavius" from Claymates
- >"Ending (Part 2)" from The Lost Vikings
  
```cx yy``` - program change  
```dx ??``` - unused  
```ex xx``` - pitch bend  
```fx``` - track end

## When translate to SPC

9x -> ```0x01 [note] [program] [velocity] [track number]```  
bx ->   
if yy <= 0x0c : ```0x08 [track number] [yy] [zz] 0x00```  
else : ```0x07 [track number] [yy] [zz] 0x00```  
cx -> na (wrote to a ram)  
ex -> ```0x08 [track number] 0xe0 [yy] 0x00```  
fx -> ```0x0e```  
if note duration reached 0x0000 (0xffff in early version(ootw,tlv1)) -> ```0x02 [note] [program] 0x00 [track number]```  

***

# Cube/Masumi Takimoto (Version 1):
The music is compatible with version 2 by just a little tweak  
*I used Majyuuou's SPC as a base*

**At 0x0b62**
```
0b62: e8 00      mov a,#$00
0b64: 8d 08      mov y,#$08
0b66: cf         mul ya
0b67: 5d         mov x,a
0b68: 8d 0f      mov y,#$0f
0b6a: f5 58 0b   mov a,$0b58+x
```
change to
```
0b62: 00         nop
0b63: 00         nop
0b64: 8d 08      mov y,#$08
0b66: cf         mul ya
0b67: 5d         mov x,a
0b68: 8d 0f      mov y,#$0f
0b6a: f5 40 17   mov a,$1740+x
```  

**At 0x13db**
```
13db: f6 01 1d  mov a,$1d01+y
13de: d4 69     mov $69+x,a
13e0: e8 03     mov a,#$03
```  
change to
```
13db: f6 00 1d  mov a,$1d00+y
13de: d4 69     mov $69+x,a
13e0: 6f        ret
```

**At 0x15c6**
```
15c6: dw $15fe
```
change to
```
15c6: dw $13ee
```

**At 0x1740**  
add these
```
7f 00 00 00 00 00 00 00
58 bf db f0 fe 07 0c 0c
0c 21 2b 2b 13 fe f3 f9
34 33 00 d9 e5 01 fc eb
10 08 14 14 14 fe fe fe
0b 21 28 28 18 fc fb f7
28 10 10 10 14 12 0c 0c
7f 00 00 00 02 04 04 04
0a 08 00 20 20 14 08 00
1a 10 12 12 12 fe fe fe
00 00 00 30 30 10 00 00
0c 21 2b 2b 13 fe f3 f9
1a 18 12 12 12 fc fc fc
44 0c 46 0c 55 0c b4 0c
44 0c 56 0c 55 0c c4 0c
44 0c 46 0c 75 0c c4 0c
```  

and you are ready to importing Ver1's music to Ver2 now.  

*Note that there may be sample data overflow, common at "Feda - The Emblem of Justice".  
In this case you will need to know which samples are used and only import them.*

# Other notes

## Carlo Perconti Sound Systeme
The songs are played like mod, as far as I know
## Fushigi no Dungeon 2 - Fuurai no Shiren
The songs are translated to MIDI format when played in SPC. (While the song itself isn't)
## Nintendo RD1 (Nintendo (later)?)
The command table is very similar to the one used on Mp2k, also known as Sappy, it may be a early version of Mp2k or something similar.
## Engine Software
The songs are played like mod, as far as I know
## ASCII/Shuichi Ukai
Ardy Lightfoot didn't use the same command set from other games used this sound engine.
## Wolfteam (Hiroya Hatsushiba)
This sound engine has 3 different variants  
### Variant 1:
- Arcus Spirits
- Neugier
- Sangokushi Seishi - Tenbu Spirits
### Variant 2:
- Ace o Nerae!
- Dark Kingdom
- Hiouden - Mamono-tachi to no Chikai
- Zan III Spirits
### Variant 3:
- All Parlor! games
- Leading Jockey
- Leading Jockey 2
- Sankyo Fever! Fever!
- Star Ocean
- Tales of Phantasia
- Tenshi no Uta: Shiroki Tsubasa no Inori
