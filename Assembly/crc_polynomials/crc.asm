global _start

section .data
buffer_size equ 65542   ; 2^16 + 6
newline db 0x0A

section .bss
buffer resb buffer_size         ; Buffer to hold file data
char resb 1

section .rodata
SYS_READ equ 0
SYS_WRITE equ 1
SYS_OPEN equ 2
SYS_CLOSE equ 3
SYS_LSEEK equ 8
SYS_EXIT  equ 60
STDOUT_FD equ 1

section .text

; Używane zmienne:
; rdi - przechowuje fd.
; rbx - przesunięcie początku fragmentu względem początku pliku.
; rcx - służy do przesuwania rejestrów.
; r8 - po początkowym wczytaniu crc, później przechowuje rozmiar segmentu.
; r9 - wielomian crc.
; r10 - wskazuje na pierwszy nieprzeczytany bajt w buforze.
; r12 - rozmiar wielomianu crc.
; r14 - tutaj są wykonywane operacje XOR.
; r15 - rejestr przepuszczający bity do r14.


%macro .read_segment_to_buffer 0
    mov rax, SYS_READ
    mov rsi, buffer
    mov rdx, buffer_size
    syscall
    .check_positive_syscall_exit_code
%%.read_segment_size:
    xor r8, r8
    mov r8b, byte [buffer + 1]
    shl r8, 8
    mov r8b, byte [buffer]
    cmp r8, 0     ; Jeśli nie ma danych, to od razu zajmujemy się przesunięciem.
    je .handle_end_of_segment
    xor r10, r10
%endmacro


; Makra do sprawdzania poprawności wywołań systemowych.

; Dla wywołań, po których oczekujemy nieujemnego exit code.
%macro .check_not_negative_syscall_exit_code 0
    cmp rax, 0
    jl .error_while_file_opened
%endmacro

; Dla wywołań, po których oczekujemy dodatniego exit code.
%macro .check_positive_syscall_exit_code 0
    cmp rax, 0
    jle .error_while_file_opened
%endmacro


_start:

.read_cmd_arguments:
    mov rcx, [rsp]      ; Liczba argumentów.
    cmp rcx, 3
    jne .error_end
    mov r8, [rsp + 24]  ; Adres wielomianu w r8.

.read_crc_poly:
    xor r9, r9
    xor rcx, rcx
    xor r12, r12
.reading_loop:
    xor rax, rax
    mov al, byte [r8 + rcx]
    cmp rax, 0                      ; Wczytanie 0, oznacza koniec stringu.
    je .shift_poly_to_reg_start     ; Wtedy przechodzimy do końca czytania.
    sub rax, '0'
.check_if_correct_char:             ; Sprawdza czy każdy znak to 0 lub 1.
    cmp rax, 0
    jl .error_end
    cmp rax, 1
    jg .error_end
.add_digit_to_poly:
    shl r9, 1
    add r9, rax
    inc rcx
    inc r12
    jmp .reading_loop

; Na koniec wyrównujemy, żeby crc znajdował się na początku rejestru.
; Obliczamy ilość bitów, o które trzeba przesunąć, żeby crc był na początku.
.shift_poly_to_reg_start:
    mov r11, 64
    sub r11, rcx
    ; rcx przechowywał pozycję, na której był najbardziej znaczący bit crc. O 64
    ; minus tyle trzeba przesunąć w lewo, żeby ten bit  znajdował się też na
    ; najbardziej znaczącym miejscu całego 64 bitowego rejestru.
    mov rcx, r11
    shl r9, cl
    cmp r12, 0      ; Jeśli rozmiar crc 0, to traktujemy to jako błąd.
    je .error_end

.open_file:
    mov rdi, [rsp + 16]
    mov eax, SYS_OPEN
    mov ecx, 0
    syscall
    cmp rax, 0
    jl .error_end   ; W tym momencie plik jeszcze nie jest otworzony.
    mov rdi, rax
    .read_segment_to_buffer
    xor r14, r14
    xor r15, r15
    xor rcx, rcx
    xor rbx, rbx

.xoring:
    cmp r10, r8     ; Sprawdzenie, czy właśnie zostało przeczytane wszystko z bufora;
    je .handle_end_of_segment
    cmp r14, 0      ; Jeśli najbardziej znaczący bit jest 1, to przesuwamy
                    ; cyklicznie o jeden i wykonujemy xor wielomianem.
    jl .shift_and_xor
.idle_shift:       ; Przesunięcie bez xora.
    call .shift_through_buffer
    jmp .xoring
.shift_and_xor:
    call .shift_through_buffer
    xor r14, r9
    jmp .xoring

.handle_end_of_segment:
    xor r11, r11    ; Czytamy wartość przesunięcia.
    mov r11b, byte [buffer + r8 + 3 + 2]
    shl r11, 8
    mov r11b, byte [buffer + r8 + 2 + 2]
    shl r11, 8
    mov r11b, byte [buffer + r8 + 1 + 2]
    shl r11, 8
    mov r11b, byte [buffer + r8 + 2]

.is_end_of_file:
    neg r11d
    add r8, 6       ; Do r8 dodajemy długość metedanych segmentu.
    cmp r11, r8     ; Sprawdzenie czy przesunięcie wskazuje na początek segmentu.
    je .end_of_file
.move_to_next_segment:
    neg r11d
.set_new_segment_address:
    add ebx, r8d
    add ebx, r11d
    mov esi, ebx
    mov rdx, 0
    mov rax, SYS_LSEEK
    syscall
    .check_not_negative_syscall_exit_code
    mov ebx, eax
    .read_segment_to_buffer
    xor rcx, rcx
    jmp .xoring

; Na koniec aby otrzymać resztę przesuwamy 128 bitów przez r14:r15.
; Wtedy końcowy wynik znajdzie się w rejestrze r14.
.end_of_file:
    mov rcx, 0      ; Liczba bitów zerowych przesunięta w r14:r15.
.end_xoring:
    cmp rcx, 128
    je .return_result
    inc rcx
    cmp r14, 0
    jl .end_shift_and_xor
.end_idle_shift:
    shl r15, 1
    rcl r14, 1
    jmp .end_xoring
.end_shift_and_xor:
    shl r15, 1
    rcl r14, 1
    xor r14, r9
    jmp .end_xoring

.return_result:
    xor rax, rax
.print_result:
    mov rcx, r12
.process_digit:
    shl r14, 1
    jc .set_1
.set_0:
    mov [char], byte '0'
    jmp .print_digit
.set_1:
    mov [char], byte '1'
.print_digit:
    mov rax, SYS_WRITE
    mov rdi, STDOUT_FD
    mov rdx, 1      ; Rozmiar chara to 1.
    mov rsi, char
    syscall
    .check_positive_syscall_exit_code
    dec r12
    cmp r12, 0
    jne .process_digit

.print_new_line:
    mov rsi, newline
    syscall
    .check_positive_syscall_exit_code

.successfull_end:
    mov rax, SYS_CLOSE
    syscall
    .check_not_negative_syscall_exit_code
    mov rax, SYS_EXIT
    xor rdi, rdi        ; Exit code ustawiamy na 0.
    syscall

 ; Kiedy nastąpił błąd po otwarciu pliku, trzeba go zamknąć jest zamykany.
.error_while_file_opened:
    mov rax, SYS_CLOSE
    syscall
.error_end:             ; Następnie ustawiany jest EXIT_CODE = 1.
    mov rdi, 1          ; Exit code ustawiamy na 1.
    mov rax, SYS_EXIT
    syscall

.shift_through_buffer:
    shl r15, 1
    rcl r14, 1
    inc rcx
    cmp rcx, 8
    je .update_from_buffer
    ret
.update_from_buffer:
    mov r15b, byte [buffer + r10 + 2]
    inc r10
    mov rcx, 0
    ret
