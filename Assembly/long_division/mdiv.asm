global mdiv

section .text

; Argumenty funkcji mdiv:
;   rdi - wskaźnik na x
;   rsi - wartość n
;   rdx - wartość y

; Zarys działania programu:
; W razie potrzeby, zmienia x,y na liczby przeciwne, tak aby zawsze mieć liczby nieujemne.
; Wykonuje zwykłe dzielenie na nieujemnych liczbach.
; Na koniec w zależności od początkowych znaków x,y ustala prawidłowe co do znaku odpowiedzi.
; Przypadki szczególne:
;   - dzielenie przez 0: Jeśli y jest zerem, to w trakcie wykonywania operacji div, zostanie zgłoszone przerwanie numer 0.
;   - overflow: To ma miejsce wtedy i tylko wtedy, gdy x jest najmniejszą możliwą liczbą z zakresu i y równe jest -1.
;               Stosowne sprawdzenie jest wykonywane w odpowiednim miejscu.
;
; Używane zmienne:
; r8 - pamięta oryginalne y
; r10 - pamięta, czy x ujemna, 0 - nieujemna, 1 - ujemna
; r11 - pamięta, czy y ujemna, 0 - nieujemna, 1 - ujemna


; Neguje liczbę x. Jako argument przyjmuje etykietę, do której wykonuje skok po zakończeniu działania.
%macro .negate_x 0
    mov r9, 0                   ; W rsi będziemy trzymali indeks aktualnie negowanego bloku.
    mov rcx, 1                  ; Przy negowaniu w U2 trzeba dodac 1 do liczby powstałej po zanegowaniu bitów. W tym rejestrze będziemy trzymać przeniesienie.
%%.negate_x_loop:
    not qword [rdi + r9*8]
    add [rdi + r9*8], rcx
    mov rcx, 0
    adc rcx, 0
    inc r9
    cmp r9, rsi                 ; Sprawdzenie, czy teraz wskaźnik jest na pierwszy blok niezajmowany przez x.
    jne %%.negate_x_loop        ; Jeśli nie - to znaczy, że zostały jeszcze bloki do odwrócenia
%endmacro

mdiv:
    mov r8, rdx     ; Musimy przepisać y do pomocniczego rejestru, ponieważ rdx jest używany przez div.

    mov rax, 1      ; Sprawdzenie znaku x.
    xor r10, r10
    cmp qword [rdi + rsi*8 - 8], 0
    cmovl r10, rax
    
    xor r11, r11    ; Sprawdzenie znaku y.
    cmp rdx, 0
    cmovl r11, rax

.make_x_not_negative:                   ; W przypadku, gdy x ujemne, wpisujemy w jej miejsce liczbę przeciwną.
    cmp r10, 1
    jne .make_y_not_negative            ; Jeśli x nieujemne, to od razu, możemy zająć się znakiem y.
    .negate_x                           ; W przeciwnym razie negujemy x. Ale też trzeba sprawdzić, czy nie ma overflow.

.check_overflow:
    cmp rdx, -1                         ; Overflow może zdarzyć się jedynie, gdy y równe jest -1.
    jne .make_y_not_negative
    mov rax, 0x8000000000000000
    cmp qword [rdi + rsi*8 - 8], rax    ; Jeśli x był ujemny, a liczba przeciwna wciąż ma 1 na samym początku, to oznacza,
                                        ; że x musiał być najmniejszą możliwą liczbą z zakresu. Po podzieleniu przez -1, mamy overflow.
    jne .make_y_not_negative

.overflow_error:
    mov r8, 0
    div r8

.make_y_not_negative:   ; W przypadku, gdy y ujemne, wpisujemy w jej miejsce liczbę przeciwną.
    cmp r11, 1
    jne .unsigned_division 
    neg r8

.unsigned_division:
    mov rcx, rsi                 ; rcx będziemy wskazywać na aktualnie dzielony blok.
    dec rcx                      ; Teraz + rcx wskazuje na początek ostatniego bloku.
    mov rax, [rdi + rcx*8]       ; W pierwszym kroku, dzielimy tylko blok 64 najbardziej znaczących bitów. Trzeba więc je załadować do RAX.
    xor rdx, rdx                 ; A górne bity w RDX trzeba ustawić na 0.
    div r8
    mov [rdi + rcx*8], rax       ; Przepisujemy wynik w miejsce, z którego będziemy zwracać.
.divide_block:
    dec rcx                      ; Schodzimy blok niżej
    js .after_unsigned_division  ; Jeśli znak ujemny, to znaczy, że wszystkie bloki już podzielone. Możemy więc zakończyć etap dzielenia.
    mov rax, [rdi + rcx*8]
    div r8
    mov [rdi + rcx*8], rax
    jmp .divide_block

.after_unsigned_division:
    mov rax, rdx                ; zapisujemy otrzymana reszte z dzielenia
    add r11, r10                ; Jeśli otrzymamy 1, to znaczy, że dokładnie jedna z x,y była ujemna. Wtedy należy zmienić znak wyniku.
    cmp r11, 1
    je .revert_result_sign
.after_result_sign_correction:
    cmp r10, 1                  ; Jeśli oryginalnie y było ujemne, to trzeba zmienić znak reszty.
    je .set_correct_remainder_sign
.end:
    ret

.revert_result_sign:
    .negate_x 
    jmp .after_result_sign_correction

.set_correct_remainder_sign:
    neg rax
    jmp .end
