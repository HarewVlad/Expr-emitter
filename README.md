# Expr-emitter
Takes an expression and converts it into x86 asm code (output.txt)  
# Example:  
1 + 2 + 3  
  
mov rax, 1  
mov rcx, 2  
add rax, rcx  
mov rcx, 3  
add rax, rcx  
