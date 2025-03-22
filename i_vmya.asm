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

PLANEWIDTH equ 80

extern source
extern dest

last_pixel_jump_table:
	dw last_pixel0,
	dw last_pixel1,
	dw last_pixel2,
	dw last_pixel3,
	dw last_pixel4,
	dw last_pixel5,
	dw last_pixel6,
	dw last_pixel7,
	dw last_pixel8,
	dw last_pixel9,
	dw last_pixel10,
	dw last_pixel11,
	dw last_pixel12,
	dw last_pixel13,
	dw last_pixel14,
	dw last_pixel15

;
; input:
;   ax = fracstep
;   dx = frac
;   cx = count		1 <= count <= 160	=>	ch = 0
;

global ScaleGlue
ScaleGlue:
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
	jz last_pixels					;  then jump to last_pixels

loop_pixels:
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
	jnz loop_pixels					; if --ah != 0 then jump to loop_pixels


last_pixels:
	xchg ax, bx						; ax = source
	mov bx, cx						; bx = count
	and bl, 15						; 0 <= count <= 15
	shl bl, 1
	mov cx, cs:last_pixel_jump_table[bx]
	xchg ax, bx						; bx = source
	jmp cx


last_pixel15:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel14:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel13:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel12:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel11:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel10:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel9:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel8:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel7:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel6:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel5:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel4:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel3:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel2:
	mov al, dh
	xlat
	stosb
	add di, si
	add dx, bp

last_pixel1:
	mov al, dh
	xlat
	stosb

last_pixel0:
	pop bp
	pop es
	pop di
	pop si
	mov ax, ss
	mov ds, ax
	ret
