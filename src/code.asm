; -----------------------------------------------------------------------------------------------------------	
; Exported symbols
PUBLIC error_message_box

; -----------------------------------------------------------------------------------------------------------	
; Data segment
; -----------------------------------------------------------------------------------------------------------	
_DATA SEGMENT
	error_caption db "An error occured...", 0
_DATA ENDS

; -----------------------------------------------------------------------------------------------------------	
; Text segment
; -----------------------------------------------------------------------------------------------------------	
_TEXT	SEGMENT

; -----------------------------------------------------------------------------------------------------------	
EXTERN MessageBoxW: PROC

; -----------------------------------------------------------------------------------------------------------
; void error_message_box(wchar_t* message)
error_message_box PROC
	push rbp ; save frame pointer
	mov rbp, rsp ; fix stack pointer

	; WINUSERAPI int WINAPI MessageBoxA(
	;  RCX =>  _In_opt_ HWND hWnd,
	;  RDX =>  _In_opt_ LPCWSTR lpText,
	;  R8  =>  _In_opt_ LPCWSTR lpCaption,
	;  R9  =>  _In_ UINT uType);

	; rcx now contains message
	mov rdx, rcx
	mov rcx, 0
	mov r8, offset error_caption
	mov r9, 0 ; MB_OK

	and rsp, not 8 ; align stack to 16 bytes prior to API call
	;sub rsp, 8 ; mess up the alignment
	call MessageBoxW

	; epilog. restore stack pointer
	mov rsp, rbp
	pop rbp
	ret	
error_message_box ENDP

_TEXT	ENDS

END