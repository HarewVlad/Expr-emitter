# Expr-emitter
Takes an expression and converts it into x86 asm code (output.txt)  
# Example:  
1 + 2 + (10 - 5)  
  
0:  48 c7 c0 01 00 00 00    mov    rax,0x1  
7:  48 c7 c1 02 00 00 00    mov    rcx,0x2  
e:  48 03 c1                add    rax,rcx  
11: 48 c7 c1 0a 00 00 00    mov    rcx,0xa  
18: 48 c7 c2 05 00 00 00    mov    rdx,0x5  
1f: 48 2b ca                sub    rcx,rdx  
22: 48 03 c1                add    rax,rcx  
