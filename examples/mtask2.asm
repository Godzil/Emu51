$mod851

state   EQU 7fh
tmp	EQU 7eh

	org 0000h
	ljmp start

	org 000bh
t0int:  ljmp task_switch
	 
	org 0060h
start:  clr EA
	mov A,TMOD
	anl A,#11111000b
	orl A,#1
	mov TMOD,A 	; T0 mode = 1
	mov TL0,#0
	mov TH0,#0f0h
	mov state,#00h
	setb TR0 	; T0 enable
	setb ET0 	; T0 int enable
	setb EA 	; enable int

task1:  xrl P1,#0fh
	
	mov B,#070h
t1l2:	mov A,#0ffh
t1l1:	djnz Acc,t1l1
	djnz B,t1l2

	sjmp task1
	
task2:  xrl P1,#0f0h

	mov B,#030h
t2l2:	mov A,#0ffh
t2l1:	djnz Acc,t2l1
	djnz B,t2l2

	sjmp task2

;========================================

task_switch:
	push DPH
	push DPL
	push Acc
	mov A,state
	jz ts_go
	ljmp rev	

ts_go:	pop Acc		; Acc
	mov DPTR,#t1inf
	movx @DPTR,A

	pop Acc   	; DPL
	inc DPTR
	movx @DPTR,A

	pop Acc   	; DPH
	inc DPTR
	movx @DPTR,A

	inc DPTR	; B
	mov A,B
	movx @DPTR,A

	inc DPTR	; PSW
	mov A,PSW
	movx @DPTR,A

	pop Acc		; PC
	inc DPTR
	movx @DPTR,A
	pop Acc
	inc DPTR
	movx @DPTR,A

	inc DPTR	; SP
	mov A,SP
	movx @DPTR,A

	inc DPTR	; adr: 00h
	mov A,R0
	movx @DPTR,A

	mov B,#6fh	; adr: 01h-6fh
	mov R0,#01h
ts_l1:	inc DPTR
	mov A,@R0
	movx @DPTR,A
	inc R0
	djnz B,ts_l1	

	;------------

	mov DPTR,#(t2inf_end-1)

	mov B,#6fh	; adr: 01-6f
	mov R0,#6fh
ts_l2:	movx A,@DPTR
	mov @R0,A	
	dec R0
	lcall dec_dptr
	djnz B,ts_l2

	movx A,@DPTR
	mov R0,A	; adr: 00

	lcall dec_dptr  ; SP
	movx A,@DPTR
	mov SP,A

	lcall dec_dptr	; PC
	movx A,@DPTR	
	push Acc
	lcall dec_dptr
	movx A,@DPTR	
	push Acc

	lcall dec_dptr  ; PSW
	movx A,@DPTR
	mov PSW,A

	lcall dec_dptr  ; B
	movx A,@DPTR
	mov B,A
	
	lcall dec_dptr  ; DPH
	movx A,@DPTR
	push Acc
	lcall dec_dptr  ; DPL
	movx A,@DPTR
	push Acc

	lcall dec_dptr	; Acc
	movx A,@DPTR
	
	pop DPL
	pop DPH

	mov state,#0ffh
	mov TL0,#0
	mov TH0,#0f0h
	reti
	;-=-=-=-=-=-=-=-=-
rev:
	pop Acc		; Acc
	mov DPTR,#t2inf
	movx @DPTR,A

	pop Acc   	; DPL
	inc DPTR
	movx @DPTR,A

	pop Acc   	; DPH
	inc DPTR
	movx @DPTR,A

	inc DPTR	; B
	mov A,B
	movx @DPTR,A

	inc DPTR	; PSW
	mov A,PSW
	movx @DPTR,A

	pop Acc		; PC
	inc DPTR
	movx @DPTR,A
	pop Acc
	inc DPTR
	movx @DPTR,A

	inc DPTR	; SP
	mov A,SP
	movx @DPTR,A

	inc DPTR	; adr: 00h
	mov A,R0
	movx @DPTR,A

	mov B,#6fh	; adr: 01h-6fh
	mov R0,#01h
ts_l3:	inc DPTR
	mov A,@R0
	movx @DPTR,A
	inc R0
	djnz B,ts_l3

	;------------

	mov DPTR,#(t1inf_end-1)

	mov B,#6fh	; adr: 01-6f
	mov R0,#6fh
ts_l4:	movx A,@DPTR
	mov @R0,A	
	dec R0
	lcall dec_dptr
	djnz B,ts_l4

	movx A,@DPTR
	mov R0,A	; adr: 00

	lcall dec_dptr  ; SP
	movx A,@DPTR
	mov SP,A

	lcall dec_dptr	; PC
	movx A,@DPTR	
	push Acc
	lcall dec_dptr
	movx A,@DPTR	
	push Acc

	lcall dec_dptr  ; PSW
	movx A,@DPTR
	mov PSW,A

	lcall dec_dptr  ; B
	movx A,@DPTR
	mov B,A
	
	lcall dec_dptr  ; DPH
	movx A,@DPTR
	push Acc
	lcall dec_dptr  ; DPL
	movx A,@DPTR
	push Acc

	lcall dec_dptr	; Acc
	movx A,@DPTR
	
	pop DPL
	pop DPH

	mov state,#00h
	mov TL0,#0
	mov TH0,#0f0h
	reti

;========================================
dec_dptr: 
	xch A,DPL
	dec A	
	cjne A,#0ffh,decdp_lab1
	dec DPH
decdp_lab1:
	xch A,DPL
	ret
;========================================
	org 8000h
t1inf: 
db 00			; Acc
db 00			; DPL
db 00			; DPH
db 00			; B
db 00			; PSW
dw 00	 		; PC
db 07			; SP
db 0,0,0,0,0,0,0,0	; R0-R7 (from adress: 00h)
db 0,0,0,0,0,0,0,0	; Stack
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0	; (to adress: 6fh)
t1inf_end:


t2inf: 
db 00			; Acc
db 00			; DPL
db 00			; DPH
db 00			; B
db 00			; PSW
dw task2 		; PC
db 07			; SP
db 0,0,0,0,0,0,0,0	; R0-R7 (from adress: 00h)
db 0,0,0,0,0,0,0,0	; Stack
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0	; (to adress: 6fh)
t2inf_end:

	END

	

		  
