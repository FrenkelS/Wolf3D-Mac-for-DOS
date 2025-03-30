;
; Copyright (C) 2025 Frenkel Smeijers
;
; This program is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 2
; of the License, or (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program. If not, see <https://www.gnu.org/licenses/>.
;

%ifidn CPU, i8088
cpu 8086
%elifidn CPU, i286
cpu 286
%else
%error unsupported cpu CPU
%endif

bits 16

PLANEWIDTH equ 320

extern source
extern dest

last_pixel_high_jump_table:
	dw last_pixel_high_0,
	dw last_pixel_high_1,
	dw last_pixel_high_2,
	dw last_pixel_high_3,
	dw last_pixel_high_4,
	dw last_pixel_high_5,
	dw last_pixel_high_6,
	dw last_pixel_high_7,
	dw last_pixel_high_8,
	dw last_pixel_high_9,
	dw last_pixel_high_10,
	dw last_pixel_high_11,
	dw last_pixel_high_12,
	dw last_pixel_high_13,
	dw last_pixel_high_14,
	dw last_pixel_high_15

last_pixel_low_jump_table:
	dw last_pixel_low_0,
	dw last_pixel_low_1,
	dw last_pixel_low_2,
	dw last_pixel_low_3,
	dw last_pixel_low_4,
	dw last_pixel_low_5,
	dw last_pixel_low_6,
	dw last_pixel_low_7,
	dw last_pixel_low_8,
	dw last_pixel_low_9,
	dw last_pixel_low_10,
	dw last_pixel_low_11,
	dw last_pixel_low_12,
	dw last_pixel_low_13,
	dw last_pixel_low_14,
	dw last_pixel_low_15

last_pixel_potato_jump_table:
	dw last_pixel_potato_0,
	dw last_pixel_potato_1,
	dw last_pixel_potato_2,
	dw last_pixel_potato_3,
	dw last_pixel_potato_4,
	dw last_pixel_potato_5,
	dw last_pixel_potato_6,
	dw last_pixel_potato_7,
	dw last_pixel_potato_8,
	dw last_pixel_potato_9,
	dw last_pixel_potato_10,
	dw last_pixel_potato_11,
	dw last_pixel_potato_12,
	dw last_pixel_potato_13,
	dw last_pixel_potato_14,
	dw last_pixel_potato_15

;
; input:
;   ax = fracstep
;   dx = frac
;   cx = count		1 <= count <= 160	=>	ch = 0
;

global ScaleGlueHigh
ScaleGlueHigh:
	push si
	push di
	push es
	push bp

	xchg bp, ax						; bp = fracstep

	mov ah, cl						; 1 <= ah <= 160
%ifidn CPU, i8088
	shr ah, 1
	shr ah, 1
	shr ah, 1
	shr ah, 1						; 0 <= ah <= 10
%else
	shr ah, 4						; 0 <= ah <= 10
%endif

	les di, [dest]					; es:di = dest
	lds bx, [source]				; ds:bx = source

	mov si, PLANEWIDTH - 1

	or ah, ah						; if ah = 0
	jz last_pixels_high				;  then jump to last_pixels_high

loop_pixels_high:
	mov al, dh						; al = hi byte of frac
	xlat							; al = source[al]
	stosb							; write pixel
	add di, si						; point to next line
	add dx, bp						; frac += fracstep

	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

	dec ah
	jnz loop_pixels_high			; if --ah != 0 then jump to loop_pixels_high


last_pixels_high:
	xchg ax, bx						; ax = source
	mov bx, cx						; bx = count
	and bl, 15						; 0 <= count <= 15
	shl bl, 1
	mov cx, cs:last_pixel_high_jump_table[bx]
	xchg ax, bx						; bx = source
	jmp cx


last_pixel_high_15:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel_high_14:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel_high_13:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel_high_12:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel_high_11:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel_high_10:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel_high_9:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel_high_8:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel_high_7:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel_high_6:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel_high_5:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel_high_4:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel_high_3:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel_high_2:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel_high_1:
	mov al, dh
	xlat
	stosb

last_pixel_high_0:
	pop bp
	pop es
	pop di
	pop si
	mov ax, ss
	mov ds, ax
	ret


;
; input:
;   ax = fracstep
;   dx = frac
;   cx = count		1 <= count <= 160	=>	ch = 0
;

global ScaleGlueLow
ScaleGlueLow:
	push si
	push di
	push es
	push bp

	xchg bp, ax						; bp = fracstep

	mov ch, cl						; 1 <= ch <= 160
%ifidn CPU, i8088
	shr ch, 1
	shr ch, 1
	shr ch, 1
	shr ch, 1						; 0 <= ah <= 10
%else
	shr ch, 4						; 0 <= ah <= 10
%endif

	les di, [dest]					; es:di = dest
	lds bx, [source]				; ds:bx = source

	mov si, PLANEWIDTH - 2

	or ch, ch						; if ch = 0
	jz last_pixels_low				;  then jump to last_pixels_low

loop_pixels_low:
	mov al, dh						; al = hi byte of frac
	xlat							; al = source[al]
	mov ah, al						; ax = source[al]
	stosw							; write pixel
	add di, si						; point to next line
	add dx, bp						; frac += fracstep

	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

	dec ch
	jnz loop_pixels_low				; if --ch != 0 then jump to loop_pixels_low


last_pixels_low:
	xchg ax, bx						; ax = source
	mov bx, cx						; bx = count
	and bl, 15						; 0 <= count <= 15
	shl bl, 1
	mov cx, cs:last_pixel_low_jump_table[bx]
	xchg ax, bx						; bx = source
	jmp cx


last_pixel_low_15:
	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

last_pixel_low_14:
	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

last_pixel_low_13:
	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

last_pixel_low_12:
	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

last_pixel_low_11:
	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

last_pixel_low_10:
	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

last_pixel_low_9:
	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

last_pixel_low_8:
	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

last_pixel_low_7:
	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

last_pixel_low_6:
	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

last_pixel_low_5:
	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

last_pixel_low_4:
	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

last_pixel_low_3:
	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

last_pixel_low_2:
	mov al, dh
	xlat
	mov ah, al
	stosw
	add di, si
	add dx, bp

last_pixel_low_1:
	mov al, dh
	xlat
	mov ah, al
	stosw

last_pixel_low_0:
	pop bp
	pop es
	pop di
	pop si
	mov ax, ss
	mov ds, ax
	ret


;
; input:
;   ax = fracstep
;   dx = frac
;   cx = count		1 <= count <= 160	=>	ch = 0
;

global ScaleGluePotato
ScaleGluePotato:
	push si
	push di
	push es
	push bp

	xchg bp, ax						; bp = fracstep

	mov ch, cl						; 1 <= ch <= 160
%ifidn CPU, i8088
	shr ch, 1
	shr ch, 1
	shr ch, 1
	shr ch, 1						; 0 <= ah <= 10
%else
	shr ch, 4						; 0 <= ah <= 10
%endif

	les di, [dest]					; es:di = dest
	lds bx, [source]				; ds:bx = source

	mov si, PLANEWIDTH - 4

	or ch, ch						; if ch = 0
	jz last_pixels_potato			;  then jump to last_pixels_potato

loop_pixels_potato:
	mov al, dh						; al = hi byte of frac
	xlat							; al = source[al]
	mov ah, al						; ax = source[al]
	stosw							; write pixel
	stosw							; write pixel
	add di, si						; point to next line
	add dx, bp						; frac += fracstep

	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

	dec ch
	jnz loop_pixels_potato			; if --ch != 0 then jump to loop_pixels_potato


last_pixels_potato:
	xchg ax, bx						; ax = source
	mov bx, cx						; bx = count
	and bl, 15						; 0 <= count <= 15
	shl bl, 1
	mov cx, cs:last_pixel_potato_jump_table[bx]
	xchg ax, bx						; bx = source
	jmp cx


last_pixel_potato_15:
	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

last_pixel_potato_14:
	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

last_pixel_potato_13:
	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

last_pixel_potato_12:
	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

last_pixel_potato_11:
	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

last_pixel_potato_10:
	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

last_pixel_potato_9:
	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

last_pixel_potato_8:
	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

last_pixel_potato_7:
	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

last_pixel_potato_6:
	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

last_pixel_potato_5:
	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

last_pixel_potato_4:
	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

last_pixel_potato_3:
	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

last_pixel_potato_2:
	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw
	add di, si
	add dx, bp

last_pixel_potato_1:
	mov al, dh
	xlat
	mov ah, al
	stosw
	stosw

last_pixel_potato_0:
	pop bp
	pop es
	pop di
	pop si
	mov ax, ss
	mov ds, ax
	ret
