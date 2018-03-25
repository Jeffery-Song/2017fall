define i32 @fib(i32 %n) {
    %1 = alloca i32, align 4         ; %1 for return value
    %2 = alloca i32, align 4         ; %2 store parameter %n
    store i32 %n, i32* %2, align 4   ; put n to *%2.since n is never changed, use %2 as n
    %3 = load i32, i32* %2, align 4  ; load %n to %3 from *%2
    %4 = icmp eq i32 %3, 0           ; %4 is n == 0
    br i1 %4, label %5, label %6     ; if %4, jump to %5, else jump to %6

; <label>:5                          ; represents n == 0, return 0
    store i32 0, i32* %1, align 4    ; put const 0 into return value's place
    br label %15                     ; jump to return block

; <label>:6                          ; represents n != 0, cmp n and 1
    %7 = icmp eq i32 %3, 1           ; %7 is n == 1
    br i1 %7, label %8, label %9     ; if %7, jump to %8, else jump to %9

; <label>:8                          ; represents n == 1, return 1
    store i32 1, i32* %1, align 4    ; put const 1 into return value's place
    br label %15 ; jmp to return     ; jump to return block

; <label>:9                          ; represents n != 1, return fib(n - 1) + fib(n + 1)
    %10 = sub nsw i32 %3, 1          ; %10 is n - 1
    %11 = call i32 @fib(i32 %10)     ; %11 is fib(n - 1)
    %12 = sub nsw i32 %3, 2          ; %12 is n - 2
    %13 = call i32 @fib(i32 %12)     ; %13 is fib(n - 2)
    %14 = add nsw i32 %11, %13       ; %14 is fib(n - 1) + fib(n - 2)
    store i32 %14, i32* %1, align 4  ; put %14 into return value's place
    br label  %15                    ; jump to return block
; <label>:15
    %16 = load i32, i32* %1, align 4 ; get retval
    ret i32 %16	                     ; return
}

define i32 @main() {
    %x = alloca i32, align 4         ; stack space for local variable x
    %i = alloca i32, align 4         ; stack space for local variable i
    store i32 0, i32* %x, align 4    ; init x to 0
    store i32 0, i32* %i, align 4    ; init i to 0
    br label %1                      ; jump to for's condition

; <label>:1
    %2 = load i32, i32* %i, align 4  ; load i to %2
    %3 = icmp slt i32 %2, 10         ; compare i to 10
    br i1 %3, label %4, label %9     ; jump to for's loop body if i < 10, else break

; <label>:4                          ; loop body
    %5 = call i32 @fib(i32 %2)       ; %5 is fib(i)
    %6 = load i32, i32* %x, align 4  ; %6 is origin x
    %7 = add nsw i32 %6, %5          ; %7 is x + fib(i)
    store i32 %7, i32* %x, align 4   ; store x + fib(i) to x
    %8 = add nsw i32 %2, 1           ; 
    store i32 %8, i32* %i, align 4   ; i++
    br label %1                  

; <label>:9
    %10 = load i32, i32* %x, align 4 ; get return value
    ret i32 %10                      ; return
}